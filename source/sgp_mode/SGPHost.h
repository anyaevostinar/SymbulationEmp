#ifndef SGPHOST_H
#define SGPHOST_H

#include "../default_mode/Host.h"
#include "hardware/SGPHardware.h"
// #include "SGPWorld.h"

#include "emp/base/Ptr.hpp"

#include "sgpl/utility/ThreadLocalRandom.hpp"

#include <functional>


namespace sgpmode {

template <typename HW_SPEC_T>
class SGPHost : public Host {
public:
  using this_t = SGPHost<HW_SPEC_T>;
  using world_t = typename HW_SPEC_T::world_t;
  using hw_spec_t = HW_SPEC_T;
  using hw_t = SGPHardware<hw_spec_t>;
  using program_t = typename hw_t::program_t;

protected:
  // CPU cpu;
  hw_t hardware;

  /**
   *
   * Purpose: Tracks the number of reproductive events in this host's lineage.
   *
   */
  size_t reproductions = 0;

  /**
    *
    * Purpose: Represents the SGPWorld that the hosts are living in.
    *
  */
  const emp::Ptr<world_t> my_world;

  /**
   *
   * Purpose: Holds all configuration settings and points to same configuration
   * object as my_config from superclass, but with the correct subtype.
   *
   */
  // emp::Ptr<SymConfigSGP> sgp_config;

  // // Function to configure functionality.
  // void ConfigureDefaults() {
  //   // Clear signals.
  //   process_sig_before_cpu_step.Clear();
  //   process_sig_before_syms.Clear();
  //   process_sig_after_syms.Clear();
  //   process_sig_end.Clear();
  //   // Set process syms to default behavior.
  //   process_syms =
  // }

public:
  /**
   * Constructs a new SGPHost as an ancestor organism, with either a random
   * genome or a blank genome that knows how to do a simple task depending on
   * the config setting RANDOM_ANCESTOR.
   */
  SGPHost(
    emp::Ptr<emp::Random> _random,
    emp::Ptr<world_t> _world,
    emp::Ptr<SymConfigSGP> _config,
    double _intval = 0.0,                         /* Interaction value */
    const emp::vector<emp::Ptr<Organism>>& _syms = {},
    const emp::vector<emp::Ptr<Organism>>& _repro_syms = {}, // NOTE - what are repro_syms?
    double _points = 0.0
  ) :
    Host(_random, _world, _config, _intval, _syms, _repro_syms, _points),
    hardware(_world, this),
    my_world(_world)
    // sgp_config(_config)
  { }

  /**
   * Constructs an SGPHost with a copy of the provided genome.
   */
  SGPHost(
    emp::Ptr<emp::Random> _random,
    emp::Ptr<world_t> _world,
    emp::Ptr<SymConfigSGP> _config,
    const program_t& genome,
    double _intval = 0.0,                         /* Interaction value */
    const emp::vector<emp::Ptr<Organism>>& _syms = {},
    const emp::vector<emp::Ptr<Organism>>& _repro_syms = {},
    double _points = 0.0
  ) :
    Host(_random, _world, _config, _intval, _syms, _repro_syms, _points),
    hardware(_world, this, genome),
    my_world(_world)
    // sgp_config(_config)
  { }

  SGPHost(const SGPHost& host) :
    Host(host),
    hardware(host.my_world, this, host.hardware.GetProgram()),
    my_world(host.my_world)
  { }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Perform necessary cleanup when a host dies, freeing heap-allocated
   * state and canceling any in-progress reproduction.
   */
  ~SGPHost() {
    // cpu.state.used_resources.Delete();
    // cpu.state.shared_available_dependencies.Delete();
    // cpu.state.internal_environment.Delete();
    // Invalidate any in-progress reproduction
    // TODO - move this out of this class?
    // - Or, move functionality into world (add world function for invalidating queued repro)
    auto& cpu_state = hardware.GetCPUState();
    if (cpu_state.ReproInProgress()) {
      my_world->to_reproduce[cpu_state.GetReproQueuePos()].second =
        emp::WorldPosition::invalid_id;
    }
    // if (hardware.state.in_progress_repro != -1) {
    //   my_world->to_reproduce[cpu.state.in_progress_repro].second =
    //     emp::WorldPosition::invalid_id;
    // }
  }

  bool operator<(const Organism& other) const {
    if (const SGPHost* sgp = dynamic_cast<const SGPHost*>(&other)) {
      return GetProgram() < sgp->GetProgram();
    } else {
      return false;
    }
  }

  bool operator<(const SGPHost& other) const {
    return GetProgram() < other.GetProgram();
  }

  // NOTE / TODO - What about host interaction values?
  bool operator==(const Organism& other) const {
    if (const SGPHost* sgp = dynamic_cast<const SGPHost*>(&other)) {
      return GetProgram() == sgp->GetProgram();
    } else {
      return false;
    }
  }

  bool operator==(const SGPHost& other) const {
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

  void SetLocation(const emp::WorldPosition& pos) {
    hardware.GetCPUState().SetLocation(pos);
  }
  const emp::WorldPosition& GetLocation() const {
    return hardware.GetCPUState().GetLocation();
  }

  void DecPoints(double amt) {
    points -= amt;
  }

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
   * Output: The CPU associated with this host.
   *
   * Purpose: Allows accessing the host's CPU.
   */
  hw_t& GetHardware() { return hardware; }
  const hw_t& GetHardware() const { return hardware; }

  const program_t& GetProgram() const { return hardware.GetProgram(); }

  /**
   * Input: None
   *
   * Output: A pointer to the world this host belongs to.
   *
   * Purpose: Allows accessing the host's world.
   */
  emp::Ptr<world_t> GetWorld() { return my_world; }

  /**
   * Input: The location of the host.
   *
   * Output: None
   *
   * Purpose: To process the host, meaning: running its program code, which can
   * include reproduction and acquisition of resources; removing dead syms; and
   * processing alive syms.
   */
  // TODO - why pass a copy of the position?
  //        - Need to override parent implementation
  void Process(emp::WorldPosition pos) {
    GrowOlder();
  }

  /**
    * Input: None.
    *
    * Output: A new host baby of the current host, mutated.
    *
    * Purpose: To create a new baby host and reset this host's points to 0.
    */
  emp::Ptr<Organism> Reproduce() {
    // emp::Ptr<SGPHost> host_baby = Host::Reproduce().DynamicCast<SGPHost>();
    emp::Ptr<this_t> host_baby = static_cast<this_t*>(Host::Reproduce().Raw());
    host_baby->SetReproCount(reproductions + 1);
    // This organism is reproducing, so it must have gotten off the queue
    // cpu.state.in_progress_repro = -1;
    // Moved reset repro state into reproduction queue
    // hardware.GetCPUState().ResetReproState();
    // TODO - move this tracking functionality into the world
    // if (sgp_config->TRACK_PARENT_TASKS()) {
    //   host_baby->GetCPU().state.parent_tasks_performed->Import(*GetCPU().state.tasks_performed);

    //   for (size_t i = 0; i < spec::NUM_TASKS; i++) {
    //     host_baby->GetCPU().state.task_change_lose[i] = cpu.state.task_change_lose[i];
    //     host_baby->GetCPU().state.task_change_gain[i] = cpu.state.task_change_gain[i];
    //     // lineage task gain / loss
    //     if (cpu.state.tasks_performed->Get(i) && !cpu.state.parent_tasks_performed->Get(i)) {
    //       // child gains the ability to be infected by syms whose parents have done this task
    //       host_baby->GetCPU().state.task_change_gain[i] = cpu.state.task_change_gain[i] + 1;
    //     }
    //     else if (!cpu.state.tasks_performed->Get(i) && cpu.state.parent_tasks_performed->Get(i)) {
    //       // child loses the ability to be infected by syms with whom this parent had only this task in common
    //       host_baby->GetCPU().state.task_change_lose[i] = cpu.state.task_change_lose[i] + 1;
    //     }

    //     // divergence from/convergence towards parent's partner
    //     host_baby->GetCPU().state.task_from_partner[i] = cpu.state.task_from_partner[i];
    //     host_baby->GetCPU().state.task_toward_partner[i] = cpu.state.task_toward_partner[i];
    //     if (HasSym()) {
    //       emp::Ptr<emp::BitSet<spec::NUM_TASKS>> symbiont_tasks = syms[0].DynamicCast<SGPSymbiont>()->GetCPU().state.parent_tasks_performed;
    //       if (cpu.state.parent_tasks_performed->Get(i) != symbiont_tasks->Get(i) &&
    //         cpu.state.tasks_performed->Get(i) == symbiont_tasks->Get(i)) {
    //         // parent != partner and child == partner
    //         host_baby->GetCPU().state.task_toward_partner[i] = cpu.state.task_toward_partner[i] + 1;
    //       }
    //       else if (cpu.state.parent_tasks_performed->Get(i) == symbiont_tasks->Get(i) &&
    //         cpu.state.tasks_performed->Get(i) != symbiont_tasks->Get(i)) {
    //         // parent == partner and child != partner
    //         host_baby->GetCPU().state.task_from_partner[i] = cpu.state.task_from_partner[i] + 1;
    //       }
    //     }
    //   }
    // }
    return host_baby;
  }

  /**
   * Input: None.
   *
   * Output: A new host with same properties as this host.
   *
   * Purpose: To avoid creating an organism via constructor in other methods.
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
   * Purpose: To mutate the code in the genome of this host.
   */
  // TODO - move this out
  void Mutate(double mut_rate) {
    Host::Mutate(); // Mutates interaction value
    hardware.Mutate(mut_rate); // Mutates program
  }
};

// SGPHost& AsSGPHost(emp::Ptr<Organism> org_ptr) {
//   return *(static_cast<SGPHost*>(org_ptr.Raw()));
// }

// const SGPHost& AsSGPHost(emp::Ptr<Organism> org_ptr) const {
//   return *(static_cast<SGPHost*>(org_ptr.Raw()));
// }

}

#endif