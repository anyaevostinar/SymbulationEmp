#ifndef SGPHOST_H
#define SGPHOST_H

#include "../default_mode/Host.h"
#include "SGPCpu.h"
#include "SGPWorld.h"

class SGPHost : public Host {
private:
  SGPCpu<SGPHost> cpu;
  emp::Ptr<SGPWorld> my_world;

public:
  SGPHost(emp::Ptr<emp::Random> _random, emp::Ptr<SGPWorld> _world,
          emp::Ptr<SymConfigBase> _config, double _intval = 0.0,
          emp::vector<emp::Ptr<Organism>> _syms = {},
          emp::vector<emp::Ptr<Organism>> _repro_syms = {},
          std::set<int> _set = std::set<int>(), double _points = 0.0)
      : Host(_random, _world, _config, _intval, _syms, _repro_syms, _set,
             _points),
        cpu(this, _random) {
    my_world = _world;
  }

  SGPHost(emp::Ptr<emp::Random> _random, emp::Ptr<SGPWorld> _world,
          emp::Ptr<SymConfigBase> _config, SGPCpu<SGPHost> oldCpu,
          double _intval = 0.0, emp::vector<emp::Ptr<Organism>> _syms = {},
          emp::vector<emp::Ptr<Organism>> _repro_syms = {},
          std::set<int> _set = std::set<int>(), double _points = 0.0)
      : Host(_random, _world, _config, _intval, _syms, _repro_syms, _set,
             _points),
        cpu(this, _random, oldCpu) {
    my_world = _world;
  }

  SGPCpu<SGPHost> &getCpu() { return cpu; }

  emp::WorldPosition lastPos;

  void Process(emp::WorldPosition pos) {
    // Run cpu step
    lastPos = pos;

    cpu.runCpuStep();

    // Instead of calling Host::Process, do the important stuff here
    // Symbiotes are ignored for the time being
    // Our instruction handles reproduction
    if (GetDead()) {
      return;
    }
    GrowOlder();
  }

  void maybeReproduce(float chance) {
    if (random->P(chance)) {
      auto p = reproduce();
      my_world->DoBirth(p, lastPos.GetIndex());
    }
  }

  emp::Ptr<Organism> makeNew() {
    emp::Ptr<SGPHost> host_baby =
        emp::NewPtr<SGPHost>(random, my_world, my_config, cpu, GetIntVal());
    return host_baby;
  }

  void mutate() {
    Host::mutate();
    cpu.mutate();
  }
};

#endif