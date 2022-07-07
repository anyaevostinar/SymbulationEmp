#ifndef SGPSYMBIONT_H
#define SGPSYMBIONT_H

#include "../default_mode/Symbiont.h"
#include "CPU.h"
#include "SGPHost.h"
#include "SGPWorld.h"
#include "emp/Evolve/World_structure.hpp"

class SGPSymbiont : public Symbiont {
private:
  CPU cpu;
  emp::Ptr<SGPWorld> my_world;

public:
  SGPSymbiont(emp::Ptr<emp::Random> _random, emp::Ptr<SGPWorld> _world,
              emp::Ptr<SymConfigBase> _config, double _intval = 0.0,
              double _points = 0.0)
      : Symbiont(_random, _world, _config, _intval, _points),
        cpu(this, _world, _random) {
    my_world = _world;
  }

  SGPSymbiont(emp::Ptr<emp::Random> _random, emp::Ptr<SGPWorld> _world,
              emp::Ptr<SymConfigBase> _config, CPU &oldCpu,
              double _intval = 0.0, double _points = 0.0)
      : Symbiont(_random, _world, _config, _intval, _points),
        cpu(this, _world, _random, oldCpu) {
    my_world = _world;
  }

  ~SGPSymbiont() {
    if (!my_host) {
      cpu.state.used_resources.Delete();
    }
    // Invalidate any in-progress reproduction
    if (cpu.state.in_progress_repro != -1) {
      my_world->to_reproduce[cpu.state.in_progress_repro].second =
          emp::WorldPosition::invalid_id;
    }
  }

  void SetHost(emp::Ptr<Organism> host) {
    Symbiont::SetHost(host);
    cpu.state.used_resources.Delete();
    cpu.state.used_resources =
        host.DynamicCast<SGPHost>()->GetCPU().state.used_resources;
  }

  CPU &GetCPU() { return cpu; }

  void Process(emp::WorldPosition pos) {
    cpu.RunCPUStep(pos);

    // The parts of Symbiont::Process that don't use resources or reproduction

    // Age the organism
    GrowOlder();
    // Check if the organism should move and do it
    if (my_host.IsNull() && my_config->FREE_LIVING_SYMS() && !dead) {
      // if the symbiont should move, and hasn't been killed
      my_world->MoveFreeSym(pos);
    }
  }

  emp::Ptr<Organism> MakeNew() {
    emp::Ptr<SGPSymbiont> host_baby =
        emp::NewPtr<SGPSymbiont>(random, my_world, my_config, cpu, GetIntVal());
    // This organism is reproducing, so it must have gotten off the queue
    cpu.state.in_progress_repro = -1;
    return host_baby;
  }

  void Mutate() {
    Symbiont::Mutate();

    cpu.Mutate();
  }
};

#endif