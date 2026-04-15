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
<<<<<<< HEAD:source/old_sgp_mode/SGPSymbiont.h
    // Invalidate any in-progress reproduction
    if (cpu.state.in_progress_repro != -1 && (int)my_world->to_reproduce.size() > cpu.state.in_progress_repro) {
      //TODO: figure out why the second part of the line above is necessary and also write a test
      my_world->to_reproduce[cpu.state.in_progress_repro] = nullptr;
=======
    // if (!my_host) {
    //   cpu.state.internal_environment.Delete();
    //   cpu.state.used_resources.Delete();
    //   cpu.state.shared_available_dependencies.Delete();
    // }
    // Invalidate any in-progress reproduction
    auto& cpu_state = hardware.GetCPUState();
    if (cpu_state.ReproInProgress()) {
      my_world->GetReproQueue().Invalidate(cpu_state.GetReproQueuePos());
>>>>>>> alex-fork/sgp-mode-refactor:source/sgp_mode/SGPSymbiont.h
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

<<<<<<< HEAD:source/old_sgp_mode/SGPSymbiont.h
=======
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

>>>>>>> alex-fork/sgp-mode-refactor:source/sgp_mode/SGPSymbiont.h
  void AddPoints(double _in) {
    points += _in;
  }

  void DecPoints(double amt) {
    points -= amt;
  }

  /**
   * Input: Score of completed task, id of completed task
   *
   * Output: Calculated score to be received post interaction
   *
   * Purpose: Adjusts score that symbiont will receive based on whether it is giving some of its points to host
   * or stealing some of the hosts points. 
   */
  float DoTaskInteraction(float score, size_t task_id) {
    if(sgp_config->INTERACTION_MECHANISM() == 3){ //Nutrient mode
      emp::Ptr<SGPHost> host = my_host.DynamicCast<SGPHost>();
      bool host_performed = host->GetCPU().state.tasks_performed->Get(task_id);

      int is_parasite = sgp_config->SYMBIONT_TYPE();

      if (is_parasite == 1  && host_performed) { //Parasite
        double to_steal = sgp_config->NUTRIENT_DONATE_STEAL_PROP() * score;
        double from_host = emp::Min(my_host->GetPoints(), to_steal); //parasite only can steal what's available
        my_host->AddPoints(-from_host); 
        score += from_host;
      }
      else if (is_parasite == 0 && host_performed) { //Mutualist 
        double to_donate = sgp_config->NUTRIENT_DONATE_STEAL_PROP() * score;
        my_host->AddPoints(to_donate);
        score -= to_donate; // sym gets remainder that isn't donated
      }
    }
    return score;
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
<<<<<<< HEAD:source/old_sgp_mode/SGPSymbiont.h
    // Instead of calling Host::Process, do the important stuff here
    // Our instruction handles reproduction
    if (GetDead()) {
      return;
    }
    
    cpu.RunCPUStep(pos, sgp_config->CYCLES_PER_UPDATE());
    // The parts of Symbiont::Process that don't use resources or reproduction

=======
>>>>>>> alex-fork/sgp-mode-refactor:source/sgp_mode/SGPSymbiont.h
    // Age the organism
    GrowOlder();
  }

  /**
   * Input: None
   *
   * Output: Whether the symbiont is able to vertically transmit
   *
   * Purpose: To answer if this symbiont has enough points to vertically transmit
   * and if required, matches tasks with the host
   */
  bool MeetsVTRequirements() override{
    if(sgp_config->VT_TASK_MATCH()){
      bool task_match_success = my_world->TaskMatchCheck(my_world->fun_get_task_profile(this), my_world->fun_get_task_profile(my_host));
      if(!task_match_success){
        return false;
      }
    }
    
    return Symbiont::MeetsVTRequirements();
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
    auto sym_offspring = Symbiont::VerticalTransmission(host_baby);
    // hardware.GetCPUState().SetReproInProgress(repro_in_progress);
    // hardware.GetCPUState().SetReproQueuePos(repro_queue_pos);
    // cpu.state.in_progress_repro = old;
    return sym_offspring;
  }


  /**
   * Input: None
   *
   * Output: The pointer to the newly created organism
   *
   * Purpose: To produce a new SGPSymbiont
   */
  emp::Ptr<Organism> Reproduce() {
<<<<<<< HEAD:source/old_sgp_mode/SGPSymbiont.h
    emp::Ptr<SGPSymbiont> sym_baby = Symbiont::Reproduce().DynamicCast<SGPSymbiont>();
    sym_baby->SetReproCount(reproductions + 1);
    // This organism is reproducing, so it must have gotten off the queue
    cpu.state.in_progress_repro = -1;
    sym_baby->GetCPU().state.parent_tasks_performed->Import(*GetCPU().state.tasks_performed);
    if (sgp_config->TRACK_PARENT_TASKS() == CURRENTORPARENT) {
      sym_baby->GetCPU().state.parent_or_current_tasks_performed->Import(*GetCPU().state.tasks_performed);
    }
    if (sgp_config->TRACK_PARENT_TASKS()) {
      //inherit towards-from tracking
      for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
=======
    // NOTE - should be able to static cast here
    emp::Ptr<SGPSymbiont> sym_offspring = static_cast<SGPSymbiont*>(Symbiont::Reproduce().Raw());
    sym_offspring->SetReproCount(reproductions + 1); // QUESTION - why does child have +1 repro count? (is repro count lineage length?)
    auto& offspring_hw = sym_offspring->GetHardware();
    auto& offspring_cpu_state = offspring_hw.GetCPUState();
    auto& cpu_state = hardware.GetCPUState();
    // Offspring needs to be given parent's (this) task profile
    offspring_cpu_state.SetParentTasksPerformed(
      hardware.GetCPUState().GetTasksPerformed()
    );
    offspring_cpu_state.SetParentFirstTaskPerformed(
      hardware.GetCPUState().GetFirstTaskPerformed()
    );
    // Update "lineage" information
    // NOTE - This functionality is shared across symbiont/hosts.
    //        Could move into its own struct that manages/centralizes this logic.
    const size_t num_tasks = offspring_cpu_state.GetNumTasks();
    emp_assert(num_tasks == cpu_state.GetNumTasks());
    for (size_t task_id = 0; task_id < num_tasks; ++task_id) {
      const bool performed_task = cpu_state.GetTaskPerformed(task_id);
      const bool parent_performed_task = cpu_state.GetParentTaskPerformed(task_id);
>>>>>>> alex-fork/sgp-mode-refactor:source/sgp_mode/SGPSymbiont.h

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
      // NOTE - is this info on the offspring's convergence/divergence or info on *this* sym's convergence/divergence?
      bool converges = false;
      bool diverges = false;
      if (cpu_state.HasHost()) {
        host_t& host = *static_cast<host_t*>(my_host.Raw());
        // NOTE - Looking at host's parent tasks here (do we want to do this or look at host tasks?)
        const emp::BitVector& host_tasks = host.GetHardware().GetCPUState().GetParentTasksPerformed();
        const bool host_performed_task = host_tasks[task_id];
        // converge: sym_parent != host_partner and sym == host_partner
        converges = (parent_performed_task != host_performed_task) && (performed_task == host_performed_task);
        // diverge: host_parent == sym_partner and host != sym_partner
        diverges = (parent_performed_task == host_performed_task) && (performed_task != host_performed_task);
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

    return sym_offspring;
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