#ifndef STRESSHOST_H
#define STRESSHOST_H

#include "SGPHost.h"

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
   * Input: The location of the host.
   *
   * Output: None
   *
   * Purpose: To run one update of the host
   */
  void Process(emp::WorldPosition pos) override {
    if (IsExtinctionUpdate()) {
      double death_chance = sgp_config->BASE_DEATH_CHANCE();
      if (HasSym()) {
        if (sgp_config->SYMBIONT_TYPE() == MUTUALIST) death_chance = sgp_config->MUTUALIST_DEATH_CHANCE();
        else if (sgp_config->SYMBIONT_TYPE() == PARASITE) death_chance = sgp_config->PARASITE_DEATH_CHANCE();
      }
      if (random->P(death_chance)) {
        //Symbionts get to escape  during an extinction event
        for (size_t j = 0; j < syms.size(); j++) {
          emp::Ptr<Organism> cur_sym = syms[j];
          RemoveSymbiont(j+1); //RemoveSymbiont uses 1-indexed value
          my_world->SymDoBirth(cur_sym, emp::WorldPosition(j+1, pos.GetIndex()));
        }
        SetDead();
      }
    }
    
    SGPHost::Process(pos);
  }

};
#endif // STRESSHOST_H