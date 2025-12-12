#ifndef STRESSHOST_H
#define STRESSHOST_H

#include "SGPSymbiont.h"

class StressHost : public SGPHost {

public:


  /**
   * Constructs a new StressHost as an ancestor organism, with either a random
   * genome or a blank genome that knows how to do a simple task depending on
   * the config setting RANDOM_ANCESTOR.
   */
  StressHost(emp::Ptr<emp::Random> _random, emp::Ptr<SGPWorld> _world,
    emp::Ptr<SymConfigSGP> _config, double _intval = 0.0,
    emp::vector<emp::Ptr<Organism>> _syms = {},
    emp::vector<emp::Ptr<Organism>> _repro_syms = {},
    double _points = 0.0)
    : SGPHost(_random, _world, _config, _intval, _syms, _repro_syms, _points) {}

  /**
   * Constructs an StressHost with a copy of the provided genome.
   */
  StressHost(emp::Ptr<emp::Random> _random, emp::Ptr<SGPWorld> _world,
    emp::Ptr<SymConfigSGP> _config, const sgpl::Program<Spec>& genome,
    double _intval = 0.0, emp::vector<emp::Ptr<Organism>> _syms = {},
    emp::vector<emp::Ptr<Organism>> _repro_syms = {},
    double _points = 0.0)
    : SGPHost(_random, _world, _config, genome, _intval, _syms, _repro_syms, _points) {}

  StressHost(const StressHost& host)
    : SGPHost(host) {}

  /**
     * Input: None.
     *
     * Output: The string "StressHost"
     *
     * Purpose: Allows tests to check what class an Organism has
     */
  std::string const GetName() override{
    return "StressHost";
  }
  /**
   * Input: None.
   *
   * Output: A new host with same properties as this host.
   *
   * Purpose: To avoid creating an organism via constructor in other methods.
   */
  emp::Ptr<Organism> MakeNew() override {
    emp::Ptr<StressHost> host_baby = emp::NewPtr<StressHost>(
      random, GetWorld(), sgp_config, GetCPU().GetProgram(), GetIntVal());
    return host_baby;
  }

  /**
   * Input: None
   * 
   * Output: Boolean for whether the current update should have an extinction event happen
   * 
   * Purpose: To calculate if its an extinction update
   */
  bool IsExtinctionUpdate() {
    if (my_world->GetUpdate() < sgp_config->SAFE_TIME()) {
      return false;
    } else if (my_world->GetUpdate() % sgp_config->EXTINCTION_FREQUENCY() == 0) {
      return true;
    } else {
      return false;
    }
  }

  /**
   * Input: None
   *
   * Output: Double probability of death during an extinction update
   *
   * Purpose: To calculate a host's death chance based on their symbiont status
   */
  double GetDeathChance() {
    double death_chance = sgp_config->BASE_DEATH_CHANCE();
    if (HasSym()) {
      bool tasks_satisfactory = false;
      size_t j = 0;
      for (; j < syms.size() && !tasks_satisfactory; j++) {
        tasks_satisfactory = my_world->TaskMatchCheck(my_world->fun_get_task_profile(syms[j]), my_world->fun_get_task_profile(this));
      }
      if (sgp_config->ALLOW_TRANSITION_EVOLUTION() && tasks_satisfactory) {
        // assumption: MUTUALIST_DEATH_CHANCE <= BASE_DEATH_CHANCE <= PARASITE_DEATH_CHANCE
        // todo: check assumption
        
        double sym_int_val = syms[j-1]->GetIntVal();
        double interval = sgp_config->BASE_DEATH_CHANCE() - sgp_config->MUTUALIST_DEATH_CHANCE();
        if (sym_int_val < 0) interval = sgp_config->PARASITE_DEATH_CHANCE() - sgp_config->BASE_DEATH_CHANCE();
        death_chance = sgp_config->BASE_DEATH_CHANCE() + (-1 * sym_int_val * interval);
      }
      else {
        if (sgp_config->SYMBIONT_TYPE() == MUTUALIST && tasks_satisfactory) death_chance = sgp_config->MUTUALIST_DEATH_CHANCE();
        else if (sgp_config->SYMBIONT_TYPE() == PARASITE && tasks_satisfactory) death_chance = sgp_config->PARASITE_DEATH_CHANCE();
      }
    }
    return death_chance;
  }
  /**
   * Input: The location of the host.
   *
   * Output: None
   *
   * Purpose: To run one update of the host
   */
  void Process(emp::WorldPosition pos) override {
    if (IsExtinctionUpdate()) {
      double death_chance = GetDeathChance();
      if (random->P(death_chance)) {
        // escapee offspring go first (so that parent sym is not removed before they can use it!)
        if (sgp_config->SYMBIONT_TYPE() == PARASITE && sgp_config->PARASITE_NUM_OFFSPRING_ON_STRESS_INTERACTION() > 0) {
          for (size_t j = 0; j < syms.size(); j++) {
            const emp::BitSet<CPU_BITSET_LENGTH>& sym_infection_tasks = my_world->fun_get_task_profile(syms[j]);
            if (my_world->TaskMatchCheck(sym_infection_tasks, my_world->fun_get_task_profile(this))) {
              my_world->GetStressEscapeeOffspringAttemptCount().AddDatum(sgp_config->PARASITE_NUM_OFFSPRING_ON_STRESS_INTERACTION());
              for (size_t k = 0; k < sgp_config->PARASITE_NUM_OFFSPRING_ON_STRESS_INTERACTION(); k++) {
                my_world->symbiont_stress_escapee_offspring.emplace_back(StressEscapeeOffspring(syms[j]->Reproduce(), pos.GetIndex(), sym_infection_tasks));
              }
            }
          }
        }
        
        SetDead();
      }
    }
    
    SGPHost::Process(pos);
  }

};
#endif // STRESSHOST_H