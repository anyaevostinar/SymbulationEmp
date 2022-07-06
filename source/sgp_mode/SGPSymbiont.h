#ifndef SGPSYMBIONT_H
#define SGPSYMBIONT_H

#include "../default_mode/Symbiont.h"
#include "SGPCpu.h"
#include "SGPHost.h"
#include "SGPWorld.h"

class SGPSymbiont : public Symbiont {
private:
  SGPCpu cpu;
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
              emp::Ptr<SymConfigBase> _config, SGPCpu &oldCpu,
              double _intval = 0.0, double _points = 0.0)
      : Symbiont(_random, _world, _config, _intval, _points),
        cpu(this, _world, _random, oldCpu) {
    my_world = _world;
  }

  void SetHost(emp::Ptr<Organism> host) {
    Symbiont::SetHost(host);
    cpu.peripheral.usedResources = host.DynamicCast<SGPHost>()->getCpu().peripheral.usedResources;
    cpu.peripheral.waste = host.DynamicCast<SGPHost>()->getCpu().peripheral.waste;
  }

  SGPCpu &getCpu() { return cpu; }

  void Process(emp::WorldPosition pos) {
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
    return host_baby;
  }

  void Mutate() {
    Symbiont::Mutate();

    cpu.Mutate();
  }
};

#endif