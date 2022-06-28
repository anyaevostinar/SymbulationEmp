#ifndef SGPSYMBIONT_H
#define SGPSYMBIONT_H

#include "../default_mode/Symbiont.h"
#include "SGPCpu.h"
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
        cpu(this, _random) {
    my_world = _world;
  }

  SGPSymbiont(emp::Ptr<emp::Random> _random, emp::Ptr<SGPWorld> _world,
              emp::Ptr<SymConfigBase> _config, SGPCpu oldCpu,
              double _intval = 0.0, double _points = 0.0)
      : Symbiont(_random, _world, _config, _intval, _points),
        cpu(this, _random, oldCpu) {
    my_world = _world;
  }

  SGPCpu &getCpu() { return cpu; }

  emp::WorldPosition lastPos;

  void Process(emp::WorldPosition pos) {
    // Run cpu step
    lastPos = pos;

    cpu.runCpuStep(pos);

    // Make sure we don't reproduce accidentally using normal points
    SetPoints(0);
    Symbiont::Process(pos);
  }

  void maybeReproduce(float chance) {
    AddPoints(chance * 1000);
    // if (random->P(chance)) {
    //   // Just add points, since symbiont reproduction is more complicated (vertical & horizontal transmission)
    //   // This is enough points for either method with the default config
    //   AddPoints(100);
    // }
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