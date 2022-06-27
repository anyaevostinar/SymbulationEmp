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
    // Our instruction handles reproduction
    if (GetDead()) {
      return;
    }
    if (HasSym()) { //let each sym do whatever they need to do
        emp::vector<emp::Ptr<Organism>>& syms = GetSymbionts();
        for(size_t j = 0; j < syms.size(); j++){
          emp::Ptr<Organism> curSym = syms[j];
          if (GetDead()){
            return; //If previous symbiont killed host, we're done
          }
          //sym position should have host index as id and
          //position in syms list + 1 as index (0 as fls index)
          emp::WorldPosition sym_pos = emp::WorldPosition(j+1, pos.GetIndex());
          if(!curSym->GetDead()){
            curSym->Process(sym_pos);
          }
          if(curSym->GetDead()){
            syms.erase(syms.begin() + j); //if the symbiont dies during their process, remove from syms list
            curSym.Delete();
          }
        } //for each sym in syms
      } //if org has syms
    GrowOlder();
  }

  void maybeReproduce(float chance) {
    if (random->P(chance)) {
      auto p = Reproduce();

      //Now check if symbionts get to vertically transmit
      for(auto parent : GetSymbionts()){
        parent->VerticalTransmission(p);
      }
      // std::cout << "Reproducing, keeping " << p->GetSymbionts().size() << " of " << GetSymbionts().size() <<  " symbionts" << std::endl;
      my_world->DoBirth(p, lastPos.GetIndex());
    }
  }

  emp::Ptr<Organism> MakeNew() {
    emp::Ptr<SGPHost> host_baby =
        emp::NewPtr<SGPHost>(random, my_world, my_config, cpu, GetIntVal());
    return host_baby;
  }

  void Mutate() {
    Host::Mutate();

    cpu.Mutate();

  }
};

#endif