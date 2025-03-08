#ifndef SGPSYMBIONT_H
#define SGPSYMBIONT_H

#include "../default_mode/Symbiont.h"
#include "hardware/SGPHardware.h"
#include "SGPHost.h"

#include "emp/base/Ptr.hpp"
#include "emp/Evolve/World_structure.hpp"

namespace sgpmode {

template<typename HW_SPEC_T>
class SGPSymbiont : public Symbiont {
public:
  using this_t = SGPSymbiont<HW_SPEC_T>;
  using world_t = typename HW_SPEC_T::world_t;
  using hw_spec_t = HW_SPEC_T;
  using hw_t = SGPHardware<hw_spec_t>;
  using program_t = typename hw_t::program_t;
  using host_t = SGPHost<HW_SPEC_T>;

protected:
  // SignalGP hardware
  hw_t hardware;

  /**
    *
    * Purpose: Represents the SGPWorld that the symbionts are living in.
    *
  */
  const emp::Ptr<world_t> my_world;

  /**
   *
   * Purpose: Tracks the number of reproductive events in this symbiont's lineage.
   *
   */
  size_t reproductions = 0;

  /**
   *
   * Purpose: Holds all configuration settings and points to same configuration
   * object as my_config from superclass, but with the correct subtype.
   *
   */
  // emp::Ptr<SymConfigSGP> sgp_config = NULL;
public:
  /**
   * Constructs a new SGPSymbiont as an ancestor organism, with either a random
   * genome or a blank genome that knows how to do a simple task depending on
   * the config setting RANDOM_ANCESTOR.
   */
  SGPSymbiont(
    emp::Ptr<emp::Random> _random,
    emp::Ptr<world_t> _world,
    emp::Ptr<SymConfigSGP> _config,
    double _intval = 0.0,  /* Interaction value */
    double _points = 0.0
  ) :
    Symbiont(_random, _world, _config, _intval, _points),
    hardware(_world, this),
    my_world(_world)
  {
    // sgp_config = _config;
  }

  /**
   * Constructs an SGPSymbiont with a copy of the provided genome.
   */
  SGPSymbiont(
    emp::Ptr<emp::Random> _random,
    emp::Ptr<world_t> _world,
    emp::Ptr<SymConfigSGP> _config,
    const program_t& genome,
    double _intval = 0.0, /* Interaction value */
    double _points = 0.0
  ) :
    Symbiont(_random, _world, _config, _intval, _points),
    hardware(_world, this, genome),
    my_world(_world)
  {
    // sgp_config = _config;
  }

  SGPSymbiont(const SGPSymbiont& symbiont) :
    Symbiont(symbiont),
    hardware(symbiont.my_world, this, symbiont.hardware.GetProgram()),
    my_world(symbiont.my_world)
  { }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Perform necessary cleanup when a symbiont dies, freeing
   * heap-allocated state and canceling any in-progress reproduction.
   */
  ~SGPSymbiont() {
    // if (!my_host) {
    //   cpu.state.internal_environment.Delete();
    //   cpu.state.used_resources.Delete();
    //   cpu.state.shared_available_dependencies.Delete();
    // }
    // Invalidate any in-progress reproduction
    auto& cpu_state = hardware.GetCPUState();
    if (cpu_state.ReproInProgress()) {
      my_world->GetReproQueue().Invalidate(cpu_state.GetReproQueuePos());
    }
    // TODO - put this functionality back once repro queue is re-implemented
    // if (cpu_state.ReproInProgress()) {
    //   my_world->to_reproduce[cpu_state.GetReproQueuePos()].second =
    //     emp::WorldPosition::invalid_id;
    // }
    // if (cpu.state.in_progress_repro != -1) {
    //   my_world->to_reproduce[cpu.state.in_progress_repro].second =
    //       emp::WorldPosition::invalid_id;
    // }
  }

  bool operator<(const Organism& other) const {
    if (const SGPSymbiont* sgp = dynamic_cast<const SGPSymbiont*>(&other)) {
      return GetProgram() < sgp->GetProgram();
    } else {
      return false;
    }
  }

  bool operator<(const SGPSymbiont& other) const {
    return GetProgram() < other.GetProgram();
  }

  // NOTE / TODO - What about host interaction values?
  bool operator==(const Organism& other) const {
    if (const SGPSymbiont* sgp = dynamic_cast<const SGPSymbiont*>(&other)) {
      return GetProgram() == sgp->GetProgram();
    } else {
      return false;
    }
  }

  bool operator==(const SGPSymbiont& other) const {
    return hardware.GetProgram() == other.hardware.GetProgram();
  }

  /**
   * Input: Set the reproduction counter
   *
   * Output: None
   *
   * Purpose: To set the count of reproductions in this lineage.
   */
  void SetReproCount(size_t _in) { reproductions = _in; }

  /**
   * Input: None.
   *
   * Output: The reproduction count
   *
   * Purpose: To get the count of reproductions in this lineage.
   */
  size_t GetReproCount() const { return reproductions; }

  /**
   * Input: None
   *
   * Output: The sgp hardware associated with this symbiont.
   *
   * Purpose: Allows accessing the symbiont's hardware.
   */
  hw_t& GetHardware() { return hardware; }
  const hw_t& GetHardware() const { return hardware; }

  const program_t& GetProgram() const { return hardware.GetProgram(); }
  program_t& GetProgram() { return hardware.GetProgram(); }


  /**
   * Input: The pointer to an organism that will be set as the symbiont's host
   *
   * Output: None
   *
   * Purpose: To set a symbiont's host
   */
  void SetHost(emp::Ptr<Organism> host) {
    emp_assert(host.DynamicCast<host_t>(), "SGPSymbiont must have an SGPHost host");
    Symbiont::SetHost(host);
    // TODO - add has host flag? (rather condition on boolean than pointer)
    // TODO - refactor shared resources (world should control?)
    // if (my_host) {
    //   cpu.state.shared_available_dependencies.Delete();
    //   cpu.state.used_resources.Delete();
    //   cpu.state.internal_environment.Delete();

    //   cpu.state.used_resources =
    //     host.DynamicCast<SGPHost>()->GetCPU().state.used_resources;
    //   cpu.state.shared_available_dependencies =
    //     host.DynamicCast<SGPHost>()->GetCPU()
    //         .state.shared_available_dependencies;
    //   cpu.state.internal_environment =
    //     host.DynamicCast<SGPHost>()->GetCPU().state.internal_environment;
    // }
  }

  void AddPoints(double _in) {
    points += _in;
  }

  void DecPoints(double amt) {
    points -= amt;
  }

  /**
   * Input: The location of the symbiont, which includes the symbiont's position
   * in the host (default -1 if it doesn't have a host)
   *
   * Output: None
   *
   * Purpose: To process a symbiont, meaning running its program code, which
   * can include reproduction and acquisition of resources; and to allow for
   * movement
   */
  void Process(emp::WorldPosition pos) {
    // Age the organism
    GrowOlder();
  }

  /**
   * Input: The pointer to the organism that is the new host baby
   *
   * Output: None
   *
   * Purpose: To allow for vertical transmission to occur. This performs extra
   * bookkeeping on top of `Symbiont::VerticalTransmission()` to avoid messing
   * with the reproduction queue which is used for horizontal transmission.
   */
  std::optional<emp::Ptr<Organism>> VerticalTransmission(emp::Ptr<Organism> host_baby) {
    // Save and restore the in-progress reproduction, since Reproduce() will be
    // called but it will still be on the queue for horizontal transmission
    // size_t old = cpu.state.in_progress_repro;
    // TODO - clean this up? Does cpu state need to manage this?
    //  - E.g., have cpu state flag repro attempt, but let world manage repro progress?
    // const bool repro_in_progress = hardware.GetCPUState().ReproInProgress();
    // const size_t repro_queue_pos = hardware.GetCPUState().GetReproQueuePos();
    // std::cout << "vt" << std::endl;
    auto sym_baby = Symbiont::VerticalTransmission(host_baby);
    // hardware.GetCPUState().SetReproInProgress(repro_in_progress);
    // hardware.GetCPUState().SetReproQueuePos(repro_queue_pos);
    // cpu.state.in_progress_repro = old;
    return sym_baby;
  }


  /**
   * Input: None
   *
   * Output: The pointer to the newly created organism
   *
   * Purpose: To produce a new SGPSymbiont
   */
  emp::Ptr<Organism> Reproduce() {
    // std::cout << "  sym repro" << std::endl;
    // emp::Ptr<SGPSymbiont> sym_baby = Symbiont::Reproduce().DynamicCast<SGPSymbiont>();
    // NOTE - should be able to static cast here
    emp::Ptr<SGPSymbiont> sym_baby = static_cast<SGPSymbiont*>(Symbiont::Reproduce().Raw());
    sym_baby->SetReproCount(reproductions + 1); // QUESTION - why does child have +1 repro count? (is repro count lineage length?)
    // Offspring needs to be given parent's (this) task profile
    sym_baby->GetHardware().GetCPUState().SetParentTasksPerformed(
      hardware.GetCPUState().GetTasksPerformed()
    );
    // This organism reproduced, reset repro state.
    hardware.GetCPUState().ResetReproState();

    // This organism is reproducing, so it must have gotten off the queue
    // cpu.state.in_progress_repro = -1;
    // NOTE - we don't always want to reset the repro state
    // hardware.GetCPUState().ResetReproState();
    // TODO - move out of symbiont into world
    // if (my_world->GetConfig()->TRACK_PARENT_TASKS()) {
    //   sym_baby->GetCPU().state.parent_tasks_performed->Import(*GetCPU().state.tasks_performed);
    //   //inherit towards-from tracking
    //   for (size_t i = 0; i < spec::NUM_TASKS; i++) {

    //     // lineage task gain / loss
    //     sym_baby->GetCPU().state.task_change_lose[i] = cpu.state.task_change_lose[i];
    //     sym_baby->GetCPU().state.task_change_gain[i] = cpu.state.task_change_gain[i];
    //     if (cpu.state.tasks_performed->Get(i) && !cpu.state.parent_tasks_performed->Get(i)) {
    //       // child gains the ability to infect hosts whose parents have done this task
    //       sym_baby->GetCPU().state.task_change_gain[i] = cpu.state.task_change_gain[i] + 1;
    //     }
    //     else if (!cpu.state.tasks_performed->Get(i) && cpu.state.parent_tasks_performed->Get(i)) {
    //       // child loses the ability to infect hosts with whom this parent had only this task in common
    //       sym_baby->GetCPU().state.task_change_lose[i] = cpu.state.task_change_lose[i] + 1;
    //     }

    //     if (my_host) {
    //       // divergence from/convergence towards parent's partner
    //       emp::Ptr<emp::BitSet<spec::NUM_TASKS>> host_tasks = my_host.DynamicCast<SGPHost>()->GetCPU().state.parent_tasks_performed;
    //       sym_baby->GetCPU().state.task_from_partner[i] = cpu.state.task_from_partner[i];
    //       sym_baby->GetCPU().state.task_toward_partner[i] = cpu.state.task_toward_partner[i];
    //       if (cpu.state.parent_tasks_performed->Get(i) != host_tasks->Get(i) &&
    //         cpu.state.tasks_performed->Get(i) == host_tasks->Get(i)) {
    //         // parent != partner and child == partner
    //         sym_baby->GetCPU().state.task_toward_partner[i] = cpu.state.task_toward_partner[i] + 1;
    //       }
    //       else if (cpu.state.parent_tasks_performed->Get(i) == host_tasks->Get(i) &&
    //         cpu.state.tasks_performed->Get(i) != host_tasks->Get(i)) {
    //         // parent == partner and child != partner
    //         sym_baby->GetCPU().state.task_from_partner[i] = cpu.state.task_from_partner[i] + 1;
    //       }
    //     }
    //   }
    // }
    return sym_baby;
  }


  /**
   * Input: None
   *
   * Output: The pointer to the newly created organism
   *
   * Purpose: To produce a new symbiont, identical to the original
   */
  emp::Ptr<Organism> MakeNew() {
    return emp::NewPtr<this_t>(
      random,
      my_world,
      my_world->GetConfigPtr(),
      GetProgram(),
      GetIntVal()
    );
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To mutate the code in the genome of this symbiont.
   */
  // Called by Symbiont::Reproduce (which is called for both VT/HT)
  void Mutate() {
    // Mutate the interaction value
    // NOTE - could also move this into the SGPMutator, which would allow us
    //        to deviate from what happens in the base class mutate functions
    Symbiont::Mutate();
    // Apply SGP-specific mutations (managed by world)
    my_world->SymDoMutation(*this);
    // Reset host's hardware
    hardware.Reset(); // NOTE - this function was previously just Initializing state,
                      // which didn't reset the cpu. I think we want to reset the CPU here also?
  }

};

}

// SGPSymbiont& AsSGPSymbiont(emp::Ptr<Organism> org_ptr) {
//   return *(static_cast<SGPSymbiont*>(org_ptr.Raw()));
// }

// const SGPSymbiont& AsSGPSymbiont(emp::Ptr<Organism> org_ptr) const {
//   return *(static_cast<SGPSymbiont*>(org_ptr.Raw()));
// }

#endif