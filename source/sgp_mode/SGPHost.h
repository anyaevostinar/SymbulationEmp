#ifndef SGPHOST_H
#define SGPHOST_H

#include "../default_mode/Host.h"
#include "hardware/SGPHardware.h"
// #include "SGPWorld.h"
#include "SGPConfigSetup.h"

#include "emp/base/Ptr.hpp"
#include "emp/bits/bits.hpp"

#include "sgpl/utility/ThreadLocalRandom.hpp"

#include <functional>


namespace sgpmode {

template <typename HW_SPEC_T>
class SGPHost : public Host {
public:
  using this_t = SGPHost<HW_SPEC_T>;
  using world_t = typename HW_SPEC_T::world_t;
  using sgp_sym_t = typename world_t::sgp_sym_t;
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
  emp::Ptr<SymConfigSGP> sgp_config;

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
    my_world(_world),
    sgp_config(_config)
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
    my_world(_world),
    sgp_config(_config)
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
      my_world->GetReproQueue().Invalidate(cpu_state.GetReproQueuePos());
      // my_world->to_reproduce[cpu_state.GetReproQueuePos()].second =
      //   emp::WorldPosition::invalid_id;
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
  void AddPoints(double amt) {
    points += amt;
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
  program_t& GetProgram() { return hardware.GetProgram(); }

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
      // Update host location
    GetHardware().GetCPUState().SetLocation(pos); // TODO - is this necessary here?
    // NOTE - Will symbionts be able to modify host's cycles during *their* executation?
    //        How do we want to handle that? (modify host's execution on next update?)
    // NOTE - Will need to update/revist this if we have instruction-mediated interactions

    // Hosts gain baseline number of CPU cycles
    GetHardware().GetCPUState().GainCPUCycles(
      sgp_config->CYCLES_PER_UPDATE()
    );

    // NOTE - Discuss timing of endosym pre-process signal and host preprocess signal
    //        Currently endosyms go first and then hosts. This is to model endosyms
    //        having opportunity to steal / donate cpu cycles and then host responding
    //        to endosym behavior (but could argue it should be the other way around).
    // Give endosymbionts their baseline CPU cycles
    // Trigger signal to all endosymbionts that host is about to process
    //   Gives endosymbionts chance to interact with host before it processes.
    //   E.g., symbiont could steal / donate cpu cycles, resources, etc.
    emp::vector<emp::Ptr<Organism>>& syms = GetSymbionts();
    for (size_t endosym_i = 0; endosym_i < syms.size(); ++endosym_i) {
      emp_assert(!(syms[endosym_i]->IsHost()));
      emp::Ptr<sgp_sym_t> cur_symbiont = static_cast<sgp_sym_t*>(syms[endosym_i].Raw());
      const bool dead = cur_symbiont->GetDead();
      // Skip if dead
      if (dead) {
        continue;
      }
      // Endosymbiont gains baseline number of CPU cycles
      cur_symbiont->GetHardware().GetCPUState().GainCPUCycles(
        sgp_config->CYCLES_PER_UPDATE()
      );
      my_world.before_endosym_host_process_sig.Trigger(
        {endosym_i + 1, GetLocation().GetIndex()},
        *cur_symbiont,
        this
      );
    }

    my_world.before_host_process_sig.Trigger(this);

    // Host may have died as a result of this signal.
    // NOTE - Do we want to return early here + do death?
    //        Anywhere else we want to check for death?
    if (GetDead()) {
      return;
    }

    // NOTE - Do we want to drain cpu cycles here (i.e., get cashed in for execution?)
    const size_t cycles_to_exec = GetHardware().GetCPUState().ExtractCPUCycles();
    // host.GetHardware().GetCPUState().LoseCPUCycles(cycles_to_execute);
    // Execute organism hardware according to cycles_to_exec
    // NOTE - Discuss possibility of host dying because of instruction executions.
    //        As-is, still run hardware forward full amount regardless
    for (size_t i = 0; i < cycles_to_exec; ++i) {
      // TODO - do we need to update org location every update? (this was being done in RunCPUStep every cpu step)
      // Execute 1 CPU cycle
      GetHardware().RunCPUStep(1);

      // Did host attempt to reproduce?
      // NOTE - could move into a signal response
      // NOTE - want to handle this after every clock cycle?
      if (GetHardware().GetCPUState().ReproAttempt()) {
        // upside to handling this here: we have direct access to organism
        my_world.HostAttemptRepro(pos, this);
      }

      my_world.after_host_cpu_step_sig.Trigger(this);
      // NOTE - Check death here?
    }
    my_world.after_host_cpu_exec_sig.Trigger(this);

    // Handle any endosymbionts (configurable at setup-time)
    // NOTE - is there any reason that this might need to be a functor?
    ProcessEndosymbionts();
    if (GetDead()) {
      return;
    }

    GrowOlder();
    return;
  }

  void ProcessEndosymbionts() {
  // If host doesn't have a symbiont, return.
  if (!HasSym()) {
    return;
  }
  emp::vector<emp::Ptr<Organism>>& syms = GetSymbionts();
  size_t sym_cnt = syms.size();
  for (size_t sym_i = 0; sym_i < sym_cnt; /*sym_i handled internally*/) {
    emp_assert(!(syms[sym_i]->IsHost()));
    // If host is dead (e.g., because of previous symbiont), stop processing.
    if (GetDead()) {
      return;
    }
    emp::Ptr<sgp_sym_t> cur_symbiont = static_cast<sgp_sym_t*>(syms[sym_i].Raw());
    const bool dead = cur_symbiont->GetDead();
    if (!dead) {
      // Symbiont not dead, process it
      // TODO - change to functor?
      // cur_symbiont->Process({sym_i + 1, host.GetLocation().GetIndex()});
      
      cur_symbiont->Process({sym_i + 1, GetLocation().GetIndex()});
      ++sym_i;
    } else {
      emp_assert(sym_cnt > 0);
      // TODO - Check that it is okay to re-order symbionts to avoid erase calls
      // Symbiont is dead, need to delete it.
      cur_symbiont.Delete();
      // Swap this symbiont with last in list, decrementing sym_cnt
      std::swap(syms[sym_i], syms[--sym_cnt]);
      // We will need to process what we just swapped into place, so
      // re-process sym_i (don't increment it)
    }
  }
  // Resize syms to remove deleted dead symbionts swapped to end
  emp_assert(sym_cnt <= syms.size());
  syms.resize(sym_cnt);
  // TODO - signal?
}

  /**
    * Input: None.
    *
    * Output: A new host baby of the current host, mutated.
    *
    * Purpose: To create a new baby host and reset this host's points to 0.
    */
  emp::Ptr<Organism> Reproduce() {
    emp::Ptr<this_t> host_offspring = static_cast<this_t*>(Host::Reproduce().Raw());
    auto& offspring_hw = host_offspring->GetHardware();
    auto& offspring_cpu_state = offspring_hw.GetCPUState();
    auto& cpu_state = hardware.GetCPUState();

    host_offspring->SetReproCount(reproductions + 1);
    // Offspring needs to be given parent's (this) task profile
    offspring_cpu_state.SetParentTasksPerformed(
      cpu_state.GetTasksPerformed()
    );

    // NOTE - Discuss how we use this information + how it's updated
    // NOTE - This was previously behind a config setting; do we want to re-add
    //        ability to turn off this tracking?
    // NOTE - Could have the systematics manager track this?
    // TODO - move the lineage tracking logic into its own struct / functions
    // Update "lineage" information:
    // - lineage task loss / gain
    // - lineage divergence / convergence toward parent's partner
    const size_t num_tasks = offspring_cpu_state.GetNumTasks();
    emp_assert(num_tasks == cpu_state.GetNumTasks());
    for (size_t task_id = 0; task_id < num_tasks; ++task_id) {
      const bool performed_task = cpu_state.GetTaskPerformed(task_id);
      const bool parent_performed_task = cpu_state.GetParentTaskPerformed(task_id);

      // Offspring gains susceptibility to be infected by sym with this task
      const bool task_gain = performed_task && !parent_performed_task;
      const bool task_loss = !performed_task && parent_performed_task;
      offspring_cpu_state.SetLineageTaskGainCount(
        task_id,
        cpu_state.GetLineageTaskGainCount(task_id) + (size_t)task_gain
      );
      offspring_cpu_state.SetLineageTaskLossCount(
        task_id,
        cpu_state.GetLineageTaskLossCount(task_id) + (size_t)task_loss
      );

      // Divergence / convergence toward parent's partner
      const size_t cur_task_diverge_partner = cpu_state.GetLineageTaskDivergeFromPartner(task_id);
      const size_t cur_task_converge_partner = cpu_state.GetLineageTaskConvergeToPartner(task_id);
      // NOTE - is this info on the offspring's convergence/divergence or info on *this* host's convergence/divergence?
      bool converges = false;
      bool diverges = false;
      if (HasSym()) {
        sgp_sym_t& sym = *static_cast<sgp_sym_t*>(syms[0].Raw());
        // NOTE - Looking at sym's parent here (do we want to do this or look at sym?)
        const emp::BitVector& sym_tasks = sym.GetHardware().GetCPUState().GetParentTasksPerformed();
        const bool sym_performed_task = sym_tasks[task_id];
        // converge: host_parent != sym_partner and host == sym_partner
        converges = (parent_performed_task != sym_performed_task) && (performed_task == sym_performed_task);
        // diverge: host_parent == sym_partner and host != sym_partner
        diverges = (parent_performed_task == sym_performed_task) && (performed_task != sym_performed_task);
      }
      offspring_cpu_state.SetLineageTaskConvergeToPartner(
        task_id,
        cur_task_converge_partner + (size_t)converges
      );
      offspring_cpu_state.SetLineageTaskDivergeFromPartner(
        task_id,
        cur_task_diverge_partner + (size_t)diverges
      );
    }
    // This organism reproduced, reset repro state.
    hardware.GetCPUState().ResetReproState();

    return host_offspring;
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
  // Called by Host::Reproduce
  void Mutate() {
    // Mutate the interaction value
    // NOTE - could also move this into the SGPMutator, which would allow us
    //        to deviate from what happens in the base class mutate functions
    Host::Mutate();
    // Apply SGP-specific mutations (managed by world)
    my_world->HostDoMutation(*this);
    // Reset host's hardware
    hardware.Reset(); // NOTE - this function was previously just Initializing state,
                      // which didn't reset the cpu. I think we want to reset the CPU here also?
  }
};

}

#endif