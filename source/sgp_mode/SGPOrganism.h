#ifndef SGPORGANISM_H
#define SGPORGANISM_H

#include "../Organism.h"
#include "CPU.h"
#include "SGPWorld.h"
#include "emp/base/Ptr.hpp"
#include "sgpl/utility/ThreadLocalRandom.hpp"

class SGPOrganism : public virtual Organism {
protected:
  CPU cpu;
  const emp::Ptr<SGPWorld> my_world;

public:
  /**
   * Constructs a new SGPHost as an ancestor organism, with either a random
   * genome or a blank genome that knows how to do a simple task depending on
   * the config setting RANDOM_ANCESTOR.
   */
  SGPOrganism(emp::Ptr<emp::Random> _random, emp::Ptr<SGPWorld> _world)
      : cpu(this, _world), my_world(_world) {}

  /**
   * Constructs an SGPHost with a copy of the provided genome.
   */
  SGPOrganism(emp::Ptr<emp::Random> _random, emp::Ptr<SGPWorld> _world,
          const sgpl::Program<Spec> &genome)
      : cpu(this, _world, genome), my_world(_world) {}

  SGPOrganism(const SGPOrganism &host)
      : cpu(this, host.my_world, host.cpu.GetProgram()),
        my_world(host.my_world) {}

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Perform necessary cleanup when a host dies, freeing heap-allocated
   * state and canceling any in-progress reproduction.
   */
  ~SGPOrganism() {
    cpu.state.used_resources.Delete();
    cpu.state.shared_available_dependencies.Delete();
    // Invalidate any in-progress reproduction
    if (cpu.state.in_progress_repro != -1) {
      my_world->to_reproduce[cpu.state.in_progress_repro].second =
          emp::WorldPosition::invalid_id;
    }
  }

  bool operator<(const Organism &other) const override {
    if (const SGPOrganism *sgp = dynamic_cast<const SGPOrganism *>(&other)) {
      return cpu.GetProgram() < sgp->cpu.GetProgram();
    } else {
      return false;
    }
  }

  bool operator==(const Organism &other) const override {
    if (const SGPOrganism *sgp = dynamic_cast<const SGPOrganism *>(&other)) {
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
   * Input: The location of the host.
   *
   * Output: None
   *
   * Purpose: To process the host, meaning: running its program code, which can
   * include reproduction and acquisition of resources; removing dead syms; and
   * processing alive syms.
   */
  void Process(emp::WorldPosition pos) override {
    cpu.RunCPUStep(pos, my_world->GetConfig()->CYCLES_PER_UPDATE());
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To mutate the code in the genome of this host.
   */
  void Mutate() override {
    cpu.Mutate();
  }
};

#endif