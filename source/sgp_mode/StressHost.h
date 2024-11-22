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
    : SGPHost(_random, _world, _config, _intval, _syms, _repro_syms, _points) {}

  StressHost(const SGPHost& host)
    : SGPHost(host) {}

  /**
   * Input: None.
   *
   * Output: A new host with same properties as this host.
   *
   * Purpose: To avoid creating an organism via constructor in other methods.
   */
  emp::Ptr<Organism> MakeNew() override {
    emp::Ptr<SGPHost> host_baby = emp::NewPtr<StressHost>(
      random, GetWorld(), sgp_config, GetCPU().GetProgram(), GetIntVal());
    // This organism is reproducing, so it must have gotten off the queue
    GetCPU().state.in_progress_repro = -1;
    if (sgp_config->TRACK_PARENT_TASKS()) {
      host_baby->GetCPU().state.parent_tasks_performed->Import(*GetCPU().state.tasks_performed);
    }
    return host_baby;
  }

  /**
   * Input: The location of the host.
   *
   * Output: None
   *
   * Purpose: TBD
   */
  void Process(emp::WorldPosition pos) override {
    if (my_world->GetUpdate() % sgp_config->EXTINCTION_FREQUENCY() == 0) {
      double death_chance = sgp_config->BASE_DEATH_CHANCE();
      if (HasSym()) {
        if (sgp_config->STRESS_TYPE() == 0) death_chance = sgp_config->MUTUALIST_DEATH_CHANCE();
        else if (sgp_config->STRESS_TYPE() == 1) death_chance = sgp_config->PARASITE_DEATH_CHANCE();
      }
      if (random->P(death_chance)) {
        SetDead();
      }
    }
    
    SGPHost::Process(pos);
  }
};
#endif // STRESSHOST_H