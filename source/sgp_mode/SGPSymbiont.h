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
    if (GetDead()) {
      return;
    }

    GetHardware().GetCPUState().SetLocation(pos);
    if(my_host) my_world->TriggerBeforeEndoSymProcessSig(pos, *this, my_host); //Note: this is different than before_endosym_host_process_sig
    // Cash in cycles for this update
    // NOTE - Do we want to drain cpu cycles here (i.e., get cashed in for execution?)
    const size_t cycles_to_exec = GetHardware().GetCPUState().ExtractCPUCycles();
    for (size_t i = 0; i < cycles_to_exec; ++i) {
      GetHardware().RunCPUStep(1);
      if(my_host) my_world->TriggerAfterEndosymCPUStepSig(pos, *this, my_host);

      // Did endosymbiont attempt to reproduce?
      // NOTE - want to handle this after every clock cycle?
      if (GetHardware().GetCPUState().ReproAttempt()) {
        AttemptReproduction(pos);
      }

    }

    if(my_host) my_world->TriggerAfterEndosymCPUExecSig(pos, *this, my_host);
    
    // Age the organism
    GrowOlder();
    if(my_host) my_world->TriggerAfterEndosymProcessSig(pos, *this, my_host);
  }

  /**
  * Input: emp::Ptr<Organism> to host offspring, emp::Ptr<Organism> to symbiont offspring
  * 
  * Output: boolean, whether or not sym/sym offspring meets requirements to successfully vertically transmit
  * 
  * Purpose: Overwritten to add functor call for task profiles
  * Originally, to test for compatibility between sym parent/offspring and host parent/offspring, such as tags
  * */
  //TODO: AEV: add test for tags and sgp together
  bool SuccessfulVT(emp::Ptr<Organism> host_baby, emp::Ptr<Organism> sym_baby) {
    bool super_result = Symbiont::SuccessfulVT(host_baby, sym_baby);
    bool world_reqs = my_world->CheckVertTransCompatibility(*this, host_baby, my_host); 
    return super_result && world_reqs;
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


    // NOTE - Make DoVerticalTransmission function?
  // No need to mark reproduction in progress here, as this isn't managed by repro queue.
  // endosym_ptr->GetHardware().GetCPUState().MarkReproInProgress();
  // Trigger before transmission signal.
  my_world->TriggerBeforeSymVertTransmissionSig(
    this,          /* symbiont producing offspring */
    host_baby, /* transmission to */
    my_host  /* transmission from */
  );

    auto sym_offspring = Symbiont::VerticalTransmission(host_baby);

    const bool success = (bool)sym_offspring;

    

    // Trigger after transmission signal.
    my_world->TriggerAfterSymVertTransmissionSig(
    sym_offspring, /* endosym offspring (if successful) */
    this,                         /* endosym parent*/
    host_baby,                  /* transmission to */
    my_host,                     /* transmission from */
    success
  );

    return sym_offspring;
  }


  /*
  * Input: sym_pos, world position
  * 
  * Output: None
  * 
  * Purpose: Start the process for horizontal transmission by marking in progress repo and removing points
  */
  //TODO: change name and streamline
  void AttemptReproduction(emp::WorldPosition sym_pos) {
    // NOTE - could make this a configurable functor if we want different success/failure
    //        conditions on attempt
    // NOTE - Do we want to be using the horizontal transmission cost here?
    //        Is this always horizontal transmisstion?
    // NOTE - Do we need a flag indicating horizontal transmission vs. free-living?
   emp_assert(my_host.DynamicCast<host_t>(), "SGPSymbiont must have an SGPHost host");
    const double repro_cost = my_world->GetConfig().SYM_HORIZ_TRANS_RES();
    if (GetPoints() >= repro_cost) {
      // Sym pays cost
      DecPoints(repro_cost);
      // Add sym to repro queue
      // TODO - protect with mutex for threading
        const size_t queue_id = my_world->GetReproQueue().Enqueue(
          GetHardware().GetCPUState().GetOrgPtr(),
          sym_pos
        );
      // Mark symbiont's hardware as repro in progress, no longer in "attempt" state
      GetHardware().GetCPUState().MarkReproInProgress(queue_id);
    } else {
      // Attempt failed, so reset repro state.
      GetHardware().GetCPUState().ResetReproState();
    }
  }


  /**
   * Input: None
   *
   * Output: The pointer to the newly created organism
   *
   * Purpose: To produce a new SGPSymbiont
   */
  emp::Ptr<Organism> Reproduce() {
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

#endif