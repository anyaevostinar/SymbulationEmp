#ifndef SGPHOST_H
#define SGPHOST_H

#include "../default_mode/Host.h"
#include "hardware/SGPHardware.h"
// #include "SGPWorld.h"

#include "emp/base/Ptr.hpp"
#include "emp/bits/Bits.hpp"

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
<<<<<<< HEAD:source/old_sgp_mode/SGPHost.h
    // Invalidate any in-progress reproduction
    if (cpu.state.in_progress_repro != -1) {
      my_world->to_reproduce[cpu.state.in_progress_repro] = nullptr;
=======
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
>>>>>>> alex-fork/sgp-mode-refactor:source/sgp_mode/SGPHost.h
    }
    // if (hardware.state.in_progress_repro != -1) {
    //   my_world->to_reproduce[cpu.state.in_progress_repro].second =
    //     emp::WorldPosition::invalid_id;
    // }
  }

<<<<<<< HEAD:source/old_sgp_mode/SGPHost.h
  /**
     * Input: None.
     *
     * Output: The string "SGPHost"
     *
     * Purpose: Allows tests to check what class an Organism has
     */
  std::string const GetName() override{
      return "SGPHost";
  }

  bool operator<(const Organism &other) const override {
    if (const SGPHost *sgp = dynamic_cast<const SGPHost *>(&other)) {
      return cpu.GetProgram() < sgp->cpu.GetProgram();
=======
  bool operator<(const Organism& other) const {
    if (const SGPHost* sgp = dynamic_cast<const SGPHost*>(&other)) {
      return GetProgram() < sgp->GetProgram();
>>>>>>> alex-fork/sgp-mode-refactor:source/sgp_mode/SGPHost.h
    } else {
      return false;
    }
  }

<<<<<<< HEAD:source/old_sgp_mode/SGPHost.h
  bool operator==(const Organism &other) const override {
    if (const SGPHost *sgp = dynamic_cast<const SGPHost *>(&other)) {
      return cpu.GetProgram() == sgp->cpu.GetProgram();
=======
  bool operator<(const SGPHost& other) const {
    return GetProgram() < other.GetProgram();
  }

  // NOTE / TODO - What about host interaction values?
  bool operator==(const Organism& other) const {
    if (const SGPHost* sgp = dynamic_cast<const SGPHost*>(&other)) {
      return GetProgram() == sgp->GetProgram();
>>>>>>> alex-fork/sgp-mode-refactor:source/sgp_mode/SGPHost.h
    } else {
      return false;
    }
  }

  bool operator==(const SGPHost& other) const {
    return hardware.GetProgram() == other.hardware.GetProgram();
  }

  /**
<<<<<<< HEAD:source/old_sgp_mode/SGPHost.h
=======
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
>>>>>>> alex-fork/sgp-mode-refactor:source/sgp_mode/SGPHost.h
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
   * Input: A new world position
   * 
   * Output: None
   * 
   * Purpose: To set the organism's world position
   */
  void SetLocation(emp::WorldPosition _in) override {
    cpu.state.location = _in;
    Host::SetLocation(_in);
  } 

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
<<<<<<< HEAD:source/old_sgp_mode/SGPHost.h
  void Process(emp::WorldPosition pos) override {
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

=======
  // TODO - why pass a copy of the position?
  //        - Need to override parent implementation
  void Process(emp::WorldPosition pos) {
>>>>>>> alex-fork/sgp-mode-refactor:source/sgp_mode/SGPHost.h
    GrowOlder();
    
  }

<<<<<<< HEAD:source/old_sgp_mode/SGPHost.h
  // Prototype for this host's reproduce method
  emp::Ptr<Organism> Reproduce() override;
=======
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
    cpu_state.SetCPUCyclesSinceRepro(0);
    offspring_cpu_state.SetCPUCyclesSinceRepro(0);
    // Offspring needs to be given parent's (this) task profile
    offspring_cpu_state.SetParentTasksPerformed(
      cpu_state.GetTasksPerformed()
    );
    offspring_cpu_state.SetParentFirstTaskPerformed(
      cpu_state.GetFirstTaskPerformed()
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
>>>>>>> alex-fork/sgp-mode-refactor:source/sgp_mode/SGPHost.h

  /**
   * Input: None.
   *
   * Output: A new host with same properties as this host.
   *
   * Purpose: To avoid creating an organism via constructor in other methods.
   */
<<<<<<< HEAD:source/old_sgp_mode/SGPHost.h
  emp::Ptr<Organism> MakeNew() override {
    emp::Ptr<SGPHost> host_baby = emp::NewPtr<SGPHost>(
        random, my_world, sgp_config, cpu.GetProgram(), GetIntVal());
    
    return host_baby;
=======
  emp::Ptr<Organism> MakeNew() {
    return emp::NewPtr<this_t>(
      random,
      my_world,
      my_world->GetConfigPtr(),
      GetProgram(),
      GetIntVal()
    );
>>>>>>> alex-fork/sgp-mode-refactor:source/sgp_mode/SGPHost.h
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To mutate the code in the genome of this host.
   */
<<<<<<< HEAD:source/old_sgp_mode/SGPHost.h
  void Mutate() override {
=======
  // Called by Host::Reproduce
  void Mutate() {
    // Mutate the interaction value
    // NOTE - could also move this into the SGPMutator, which would allow us
    //        to deviate from what happens in the base class mutate functions
>>>>>>> alex-fork/sgp-mode-refactor:source/sgp_mode/SGPHost.h
    Host::Mutate();
    // Apply SGP-specific mutations (managed by world)
    my_world->HostDoMutation(*this);
    // TODO - Switch from HostDoMutation() to:
    //   -> my_world->GetHostMutator().DoMutation(*this);
    // TODO - move Hardware Reset to makenew, keep initializeState (need to reset jumptable)
    // Reset host's hardware
    hardware.Reset(); // NOTE - this function was previously just Initializing state,
                      // which didn't reset the cpu. I think we want to reset the CPU here also?
  }
};

}

#endif