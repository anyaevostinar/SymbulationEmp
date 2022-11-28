#ifndef SGPHOST_H
#define SGPHOST_H

#include "../default_mode/Host.h"
#include "CPU.h"
#include "SGPOrganism.h"
#include "SGPWorld.h"
#include "emp/base/Ptr.hpp"
#include "sgpl/utility/ThreadLocalRandom.hpp"

class SGPHost : public BaseHost, public SGPOrganism {
public:
  /**
   * Constructs a new SGPHost as an ancestor organism, with either a random
   * genome or a blank genome that knows how to do a simple task depending on
   * the config setting RANDOM_ANCESTOR.
   */
  SGPHost(emp::Ptr<emp::Random> _random, emp::Ptr<SGPWorld> _world,
          emp::Ptr<SymConfigBase> _config, double _points = 0.0)
      : Organism(_config, _world, _random, _points),
        SGPOrganism(_random, _world) {}

  /**
   * Constructs an SGPHost with a copy of the provided genome.
   */
  SGPHost(emp::Ptr<emp::Random> _random, emp::Ptr<SGPWorld> _world,
          emp::Ptr<SymConfigBase> _config, const sgpl::Program<Spec> &genome,
          double _points = 0.0)
      : Organism(_config, _world, _random, _points),
        SGPOrganism(_random, _world, genome) {}

  SGPHost(const SGPHost &host)
      : BaseHost(host), Organism(host), SGPOrganism(host) {}

  /**
   * Input: The location of the host.
   *
   * Output: None
   *
   * Purpose: To process the host, meaning: running its program code, which can
   * include reproduction and acquisition of resources; removing dead syms; and
   * processing alive syms.
   */
  void Process(emp::WorldPosition pos) override {
    if (SGPOrganism::my_world->GetUpdate() %
            my_config->LIMITED_TASK_RESET_INTERVAL() ==
        0)
      cpu.state.used_resources->reset();
    // Instead of calling Host::Process, do the important stuff here
    // Our instruction handles reproduction
    if (GetDead()) {
      return;
    }

    // Randomly decide whether to run before or after the symbiont
    bool run_before = random->P(0.5);
    if (run_before) {
      SGPOrganism::Process(pos);
    }

    if (HasSym()) { // let each sym do whatever they need to do
      for (size_t j = 0; j < syms.size(); j++) {
        emp::Ptr<Organism> curSym = syms[j];
        if (GetDead()) {
          return; // If previous symbiont killed host, we're done
        }
        // sym position should have host index as id and
        // position in syms list + 1 as index (0 as fls index)
        emp::WorldPosition sym_pos = emp::WorldPosition(j + 1, pos.GetIndex());
        if (!curSym->GetDead()) {
          curSym->Process(sym_pos);
        }
        if (curSym->GetDead()) {
          syms.erase(syms.begin() + j); // if the symbiont dies during their
                                        // process, remove from syms list
          curSym.Delete();
        }
      } // for each sym in syms
    }   // if org has syms

    if (!run_before) {
      SGPOrganism::Process(pos);
    }

    GrowOlder();
  }

  /**
   * Input: None.
   *
   * Output: A new host with same properties as this host.
   *
   * Purpose: To avoid creating an organism via constructor in other methods.
   */
  emp::Ptr<Organism> MakeNew() {
    emp::Ptr<SGPHost> host_baby = emp::NewPtr<SGPHost>(
        random, SGPOrganism::my_world, my_config, cpu.GetProgram());
    // This organism is reproducing, so it must have gotten off the queue
    cpu.state.in_progress_repro = -1;
    return host_baby;
  }

  /**
   * Input: None.
   *
   * Output: A new host baby of the current host, mutated.
   *
   * Purpose: To create a new baby host and reset this host's points to 0.
   */
  emp::Ptr<Organism> Reproduce() override {
    emp::Ptr<Organism> host_baby = MakeNew();
    host_baby->Mutate();
    SetPoints(0);
    return host_baby;
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To mutate the code in the genome of this host.
   */
  void Mutate() override { cpu.Mutate(); }
};

#endif