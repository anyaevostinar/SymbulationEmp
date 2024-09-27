#ifndef SGPHOST_H
#define SGPHOST_H

#include "../default_mode/Host.h"
#include "CPU.h"
#include "SGPWorld.h"
#include "emp/base/Ptr.hpp"
#include "sgpl/utility/ThreadLocalRandom.hpp"

class SGPHost : public Host {
private:
  CPU cpu;
  const emp::Ptr<SGPWorld> my_world;

protected:
  /**
   *
   * Purpose: Holds all configuration settings and points to same configuration
   * object as my_config from superclass, but with the correct subtype.
   *
   */
  emp::Ptr<SymConfigSGP> sgp_config = NULL;
public:
  /**
   * Constructs a new SGPHost as an ancestor organism, with either a random
   * genome or a blank genome that knows how to do a simple task depending on
   * the config setting RANDOM_ANCESTOR.
   */
  SGPHost(emp::Ptr<emp::Random> _random, emp::Ptr<SGPWorld> _world,
          emp::Ptr<SymConfigSGP> _config, double _intval = 0.0,
          emp::vector<emp::Ptr<Organism>> _syms = {},
          emp::vector<emp::Ptr<Organism>> _repro_syms = {},
          double _points = 0.0)
      : Host(_random, _world, _config, _intval, _syms, _repro_syms, _points),
    cpu(this, _world), my_world(_world) {
    sgp_config = _config;
  }

  /**
   * Constructs an SGPHost with a copy of the provided genome.
   */
  SGPHost(emp::Ptr<emp::Random> _random, emp::Ptr<SGPWorld> _world,
          emp::Ptr<SymConfigSGP> _config, const sgpl::Program<Spec> &genome,
          double _intval = 0.0, emp::vector<emp::Ptr<Organism>> _syms = {},
          emp::vector<emp::Ptr<Organism>> _repro_syms = {},
          double _points = 0.0)
      : Host(_random, _world, _config, _intval, _syms, _repro_syms, _points),
        cpu(this, _world, genome), my_world(_world) {
    sgp_config = _config;
  }

  SGPHost(const SGPHost &host)
      : Host(host), cpu(this, host.my_world, host.cpu.GetProgram()),
        my_world(host.my_world) {}

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Perform necessary cleanup when a host dies, freeing heap-allocated
   * state and canceling any in-progress reproduction.
   */
  ~SGPHost() {
    cpu.state.used_resources.Delete();
    cpu.state.tasks_performed.Delete();
    cpu.state.shared_available_dependencies.Delete();
    cpu.state.internal_environment.Delete();
    // Invalidate any in-progress reproduction
    if (cpu.state.in_progress_repro != -1) {
      my_world->to_reproduce[cpu.state.in_progress_repro].second =
          emp::WorldPosition::invalid_id;
    }
  }

  bool operator<(const Organism &other) const {
    if (const SGPHost *sgp = dynamic_cast<const SGPHost *>(&other)) {
      return cpu.GetProgram() < sgp->cpu.GetProgram();
    } else {
      return false;
    }
  }

  bool operator==(const Organism &other) const {
    if (const SGPHost *sgp = dynamic_cast<const SGPHost *>(&other)) {
      return cpu.GetProgram() == sgp->cpu.GetProgram();
    } else {
      return false;
    }
  }

  /**
   * Input: None
   *
   * Output: The CPU associated with this host.
   *
   * Purpose: Allows accessing the host's CPU.
   */
  CPU &GetCPU() { return cpu; }

  /**
   * Input: None
   * 
   * Output: A pointer to the world this host belongs to.
   * 
   * Purpose: Allows accessing the host's world.
   */
  emp::Ptr<SGPWorld> GetWorld() { return my_world; }

  /**
   * Input: The location of the host.
   *
   * Output: None
   *
   * Purpose: To process the host, meaning: running its program code, which can
   * include reproduction and acquisition of resources; removing dead syms; and
   * processing alive syms.
   */
  void Process(emp::WorldPosition pos) {
    //if (my_world->GetUpdate() % my_config->LIMITED_TASK_RESET_INTERVAL() == 0)
      //cpu.state.used_resources->reset();
    // Instead of calling Host::Process, do the important stuff here
    // Our instruction handles reproduction
    if (GetDead()) {
      return;
    }

    cpu.RunCPUStep(pos, sgp_config->CYCLES_PER_UPDATE());
    

    if (HasSym()) { // let each sym do whatever they need to do
      emp::vector<emp::Ptr<Organism>> &syms = GetSymbionts();
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
        random, my_world, sgp_config, cpu.GetProgram(), GetIntVal());
    // This organism is reproducing, so it must have gotten off the queue
    cpu.state.in_progress_repro = -1;
    return host_baby;
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To mutate the code in the genome of this host.
   */
  void Mutate() {
    Host::Mutate();

    cpu.Mutate();
  }
};

#endif