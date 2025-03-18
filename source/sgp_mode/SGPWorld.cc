#ifndef SGP_WORLD_C
#define SGP_WORLD_C

#include "SGPWorld.h"
#include "SGPHost.h"
#include "SGPSymbiont.h"
#include "utils.h"

namespace sgpmode {

// TODO - Make clear that this will process host and free-living symbiont
//        ProcessOrgsAt?
void SGPWorld::ProcessOrgsAt(size_t pop_id) {
  // std::cout << "ProcessOrgsAt " << pop_id << "; " << IsOccupied(pop_id) << std::endl;
  // Process host at this location (if any)
  if (IsOccupied(pop_id)) {
    auto& org = GetOrg(pop_id);;
    emp_assert(org.IsHost());
    ProcessHostAt(
      {pop_id},
      static_cast<sgp_host_t&>(org)
    );
  }
  // TODO - double check that my interpretation is correct here
  // TODO - can we condiitonally tack this onto processing only
  //        when free-living syms are turned on
  // Process free-living symbiont at this location (if any)
  if (IsSymPopOccupied(pop_id)) {
    emp_assert(!GetSymAt(pop_id)->IsHost());
    ProcessFreeLivingSymAt(
      emp::WorldPosition(0, pop_id),
      static_cast<sgp_sym_t&>(*(GetSymAt(pop_id)))
    );

  }
}

// TODO - discuss timing
// NOTE - DoDeath repeated several times here. Maybe move that check out to ProcessOrgsAt?
void SGPWorld::ProcessHostAt(const emp::WorldPosition& pos, sgp_host_t& host) {
  // std::cout << "ProcessHostAt" << std::endl;
  // If host is dead, don't process.
  if (host.GetDead()) {
    DoDeath(pos);
    return;
  }
  before_host_process_sig.Trigger(host);
  // Host may have died as a result of this signal.
  // NOTE - Do we want to return early here + do death?
  //        Anywhere else we want to check for death?
  if (host.GetDead()) {
    DoDeath(pos);
    return;
  }
  // Update host location
  host.GetHardware().GetCPUState().SetLocation(pos); // TODO - is this necessary here?
  // TODO - do we need to update org location every update? (this was being done in RunCPUStep every cpu step)
  // Execute organism hardware for CYCLES_PER_UPDATE steps
  // NOTE - Discuss possibility of host dying because of instruction executions.
  //        As-is, still run hardware forward full amount regardless
  for (size_t i = 0; i < sgp_config.CYCLES_PER_UPDATE(); ++i) {
    // Execute 1 CPU cycle
    host.GetHardware().RunCPUStep(pos, 1);

    // Did host attempt to reproduce?
    // NOTE - could move into a signal response
    // NOTE - want to handle this after every clock cycle?
    if (host.GetHardware().GetCPUState().ReproAttempt()) {
      // upside to handling this here: we have direct access to organism
      HostAttemptRepro(pos, host);
    }

    after_host_cpu_step_sig.Trigger(host);
  }
  after_host_cpu_exec_sig.Trigger(host);
  // Handle any endosymbionts (configurable at setup-time)
  // NOTE - is there any reason that this might need to be a functor?
  ProcessEndosymbionts(host);
  // Endosymbionts might kill host.
  if (host.GetDead()) {
    DoDeath(pos);
    return;
  }
  // Run host's process function (post cpu steps, post endosymbiont processing)
  // TODO - when do we actually want to run this?
  host.Process(pos);
  after_host_process_sig.Trigger(host);
  // If process resulted in death, DoDeath.
  if (host.GetDead()) {
    DoDeath(pos);
  }
}

void SGPWorld::ProcessEndosymbionts(sgp_host_t& host) {
  // If host doesn't have a symbiont, return.
  if (!host.HasSym()) {
    return;
  }
  // TODO - signal?
  emp::vector<emp::Ptr<Organism>>& syms = host.GetSymbionts();
  size_t sym_cnt = syms.size();
  for (size_t sym_i = 0; sym_i < sym_cnt; /*sym_i handled internally*/) {
    emp_assert(!(syms[sym_i]->IsHost()));
    // If host is dead (e.g., because of previous symbiont), stop processing.
    if (host.GetDead()) {
      return;
    }
    emp::Ptr<sgp_sym_t> cur_symbiont = static_cast<sgp_sym_t*>(syms[sym_i].Raw());
    const bool dead = cur_symbiont->GetDead();
    if (!dead) {
      // Symbiont not dead, process it
      // TODO - change to functor?
      // cur_symbiont->Process({sym_i + 1, host.GetLocation().GetIndex()});
      ProcessEndosymbiont(
        {sym_i + 1, host.GetLocation().GetIndex()},
        *cur_symbiont,
        host
      );
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

// TODO - discuss timing
// NOTE - Go over reproduction
void SGPWorld::ProcessEndosymbiont(
  const emp::WorldPosition& sym_pos,
  sgp_sym_t& sym,
  sgp_host_t& host
) {
  // NOTE - is there anything that should be moved here common to all endosymbionts?
  // If symbiont is dead, no need to process it.
  if (sym.GetDead()) {
    return;
  }
  before_endosym_process_sig.Trigger(sym_pos, sym, host);
  for (size_t i = 0; i < sgp_config.CYCLES_PER_UPDATE(); ++i) {
    sym.GetHardware().RunCPUStep(sym_pos, 1);
    after_endosym_cpu_step_sig.Trigger(sym_pos, sym, host);

    // Did endosymbiont attempt to reproduce?
    // NOTE - want to handle this after every clock cycle?
    if (sym.GetHardware().GetCPUState().ReproAttempt()) {
      // upside to handling this here: we have direct access to organism
      EndosymAttemptRepro(sym_pos, sym, host);
    }

  }
  after_endosym_cpu_exec_sig.Trigger(sym_pos, sym, host);
  // Call symbiont's process function
  sym.Process(sym_pos);
  after_endosym_process_sig.Trigger(sym_pos, sym, host);
}

// TODO - discuss timing

// TODO - Handle Reproduction?
// TODO - Go over support for free-living symbionts. Not sure it was
//        fully supported to begin with, so should discuss what needs to be added.
void SGPWorld::ProcessFreeLivingSymAt(const emp::WorldPosition& pos, sgp_sym_t& sym) {
  // TODO - ask about the code below (should it ever be run on endosymbionts?)
  // if (my_host == nullptr && my_world->GetUpdate() % sgp_config->LIMITED_TASK_RESET_INTERVAL() == 0)
  //   cpu.state.used_resources->reset();
  emp_assert(!sym.IsHost()); // NOTE - IsSym function?
  // have to check for death first, because it might have moved
  if (sym.GetDead()) {
    DoSymDeath(pos.GetPopID());
  } else {
    // Not dead, process.
    before_freeliving_sym_process_sig.Trigger(sym);
    for (size_t i = 0; i < sgp_config.CYCLES_PER_UPDATE(); ++i) {
      sym.GetHardware().RunCPUStep(pos, 1);

      // Did this sym attempt to reproduce?
      if (sym.GetHardware().GetCPUState().ReproAttempt()) {
        FreeLivingSymAttemptRepro(pos, sym);
      }

      after_freeliving_sym_cpu_step_sig.Trigger(sym);
    }
    after_freeliving_sym_cpu_exec_sig.Trigger(sym);
    // Call symbiont's process function
    sym.Process(pos);
    after_freeliving_sym_process_sig.Trigger(sym);
  }
  // TODO - double check that this belongs just here and not also in endosymbiont code
  if (IsSymPopOccupied(pos.GetPopID()) && sym.GetDead()) {
    DoSymDeath(pos.GetPopID());
  }
}

void SGPWorld::HostAttemptRepro(const emp::WorldPosition& pos, sgp_host_t& host) {
  // sgp_cpu_peripheral_t& state = host.GetHardware().GetCPUState();
  // NOTE - >= here or >? (used to be >)
  // NOTE - Could make this a configurable functor if we envision wanting different
  //        reproduction requirements
  // std::cout << "HostAttemptRepro" << std::endl;
  const double repro_cost = sgp_config.HOST_REPRO_RES();
  if (host.GetPoints() >= repro_cost) {
    // Host pays cost
    host.DecPoints(repro_cost);
    // Add host to repro queue
    // TODO - protect with mutex?
    // std::cout << "  Org queued in repro queue" << std::endl;
    const size_t queue_id = repro_queue.Enqueue(
      host.GetHardware().GetCPUState().GetOrgPtr(),
      pos
    );
    // Mark host hardware as repro in progress, no longer in repro "attempt" state.
    host.GetHardware().GetCPUState().MarkReproInProgress(queue_id);
  } else {
    // Attempt failed, so reset repro state.
    host.GetHardware().GetCPUState().ResetReproState();
  }
}

void SGPWorld::EndosymAttemptRepro(
  const emp::WorldPosition& pos,
  sgp_sym_t& sym,
  sgp_host_t& host
) {
  // NOTE - could make this a configurable functor if we want different success/failure
  //        conditions on attempt
  // NOTE - Do we want to be using the horizontal transmission cost here?
  //        Is this always horizontal transmisstion?
  // NOTE - Do we need a flag indicating horizontal transmission vs. free-living?
  std::cout << "EndosymAttemptRepro" << std::endl;
  const double repro_cost = sgp_config.SYM_HORIZ_TRANS_RES();
  if (sym.GetPoints() >= repro_cost) {
    std::cout << "  Endosym queued in repro queue" << std::endl;
    // Sym pays cost
    sym.DecPoints(repro_cost);
    // Add sym to repro queue
    // TODO - protect with mutex for threading
    const size_t queue_id = repro_queue.Enqueue(
      sym.GetHardware().GetCPUState().GetOrgPtr(),
      pos
    );
    // Mark symbiont's hardware as repro in progress, no longer in "attempt" state
    sym.GetHardware().GetCPUState().MarkReproInProgress(queue_id);
  } else {
    // Attempt failed, so reset repro state.
    sym.GetHardware().GetCPUState().ResetReproState();
  }
}

void SGPWorld::FreeLivingSymAttemptRepro(
  const emp::WorldPosition& pos,
  sgp_sym_t& sym
) {
  // NOTE - this is largely redundant with other attempt functions. Need to think
  //        think about whether attempt logic should be different
  // NOTE - could make this a configurable functor if we want different success/failure
  //        conditions on attempt
  const double repro_cost = sgp_config.FREE_SYM_REPRO_RES();
  if (sym.GetPoints() >= repro_cost) {
    // Sym pays cost
    sym.DecPoints(repro_cost);
    // Add sym to repro queue
    // TODO - protect with mutex for threading
    const size_t queue_id = repro_queue.Enqueue(
      sym.GetHardware().GetCPUState().GetOrgPtr(),
      pos
    );
    // Mark symbiont's hardware as repro in progress, no longer in "attempt" state
    sym.GetHardware().GetCPUState().MarkReproInProgress(queue_id);
  } else {
    // Attempt failed, so reset repro state.
    sym.GetHardware().GetCPUState().ResetReproState();
  }
}

void SGPWorld::DoReproduction() {
  // Process reproduction queue
  // NOTE - If do repro remains simplified to just calling the repro_queue's
  //        process function, can get rid of this function.
  repro_queue.Process();
}

// Called for symbionts in the reproduction queue
// TODO - SymDoBirth is for horizontal(?) and free-living repro
//      - How to distinguish between the two?
emp::WorldPosition SGPWorld::SymDoBirth(
  emp::Ptr<Organism> sym_baby,
  emp::WorldPosition parent_pos
) {
  emp_assert(!sym_baby->IsHost());
  emp::Ptr<sgp_sym_t> sym_baby_ptr = static_cast<sgp_sym_t*>(sym_baby.Raw());
  // Trigger any before birth actions
  before_sym_do_birth_sig.Trigger(sym_baby_ptr, parent_pos);
  emp::WorldPosition sym_baby_pos(fun_sym_do_birth(sym_baby_ptr, parent_pos));
  // Trigger any post-birth actions
  after_sym_do_birth_sig.Trigger(sym_baby_pos);
  return sym_baby_pos;
}

emp::WorldPosition SGPWorld::HostDoBirth(
  emp::Ptr<Organism> host_offspring_ptr,
  emp::Ptr<Organism> host_parent_ptr,
  const emp::WorldPosition& parent_pos
) {
  emp_assert(host_offspring_ptr->IsHost());
  emp_assert(host_parent_ptr->IsHost());

  // Static cast host offspring and host parent pointers
  emp::Ptr<sgp_host_t> offspring_ptr = static_cast<sgp_host_t*>(host_offspring_ptr.Raw());
  emp::Ptr<sgp_host_t> parent_ptr = static_cast<sgp_host_t*>(host_parent_ptr.Raw());

  before_host_do_birth_sig.Trigger(
    *offspring_ptr,
    *parent_ptr,
    parent_pos
  );

  // NOTE - Can make contents of this function into a functor if needs to be
  //        configurable for different types of hosts.
  // Host::Reproduce() doesn't take care of vertical transmission, that
  //  happens here. Loop over parent's symbiont, check if each can transmit
  //  vertically to host offspring.
  for (emp::Ptr<Organism> sym_org_ptr : parent_ptr->GetSymbionts()) {
    emp_assert(!sym_org_ptr->IsHost());
    // Cast generic org pointer to more specific sym pointer type
    emp::Ptr<sgp_sym_t> sym_ptr = static_cast<sgp_sym_t*>(sym_org_ptr.Raw());
    // Check if symbiont can attempt vertical transmission
    const bool can_attempt = fun_can_attempt_vert_trans(
      *sym_ptr, /* Symbiont attempting to vert. trans */
      *offspring_ptr, /* Host offspring (trans into) */
      *parent_ptr, /* Host parent (trans from) */
      parent_pos
    );
    if (!can_attempt) {
      continue;
    }
    // This symbiont attempts vertical transmission (returns success if necessary)
    EndosymAttemptVertTransmission(
      sym_ptr,
      offspring_ptr,
      parent_ptr,
      parent_pos
    );
  }

  // Call emp::World's DoBirth for host offspring that we're currently "birthing".
  const emp::WorldPosition offspring_pos(DoBirth(host_offspring_ptr, parent_pos));

  after_host_do_birth_sig.Trigger(offspring_pos);
  return offspring_pos;
}

emp::WorldPosition SGPWorld::FreeLivingSymDoBirth(
  emp::Ptr<sgp_sym_t> sym_baby_ptr,
  const emp::WorldPosition& parent_pos
) {
  // TODO - add any signals?
  return MoveIntoNewFreeWorldPos(sym_baby_ptr, parent_pos);
}

emp::WorldPosition SGPWorld::SymAttemptHorizontalTrans(
  emp::Ptr<sgp_sym_t> sym_baby_ptr,
  const emp::WorldPosition& parent_pos
) {
  // TODO - add any signals?
  const size_t parent_pop_idx = parent_pos.GetPopID();
  emp::Ptr<Organism> parent = this->GetOrgPtr(parent_pop_idx)->GetSymbionts()[parent_pos.GetIndex() - 1];
  emp_assert(!parent->IsHost());
  emp::Ptr<sgp_sym_t> sym_parent = static_cast<sgp_sym_t*>(parent.Raw());
  // hew_host_pos is an optional<emp::WorldPosition>
  const auto new_host_pos = FindHostForHorizontalTrans(parent_pop_idx, sym_parent);
  if (new_host_pos) {
    // -1 means no living neighbors
    const size_t host_id = new_host_pos.value().GetIndex();
    int new_index = pop[host_id]->AddSymbiont(sym_baby_ptr);
    if (new_index > 0) {
      //sym successfully infected
      return emp::WorldPosition(new_index, host_id);
    } else {
      //sym got killed trying to infect
      return emp::WorldPosition();
    }
  } else {
    sym_baby_ptr.Delete();
    return emp::WorldPosition();
  }
}

bool SGPWorld::EndosymAttemptVertTransmission(
  emp::Ptr<sgp_sym_t> endosym_ptr,                  /* Endosymbiont attempting transmission */
  emp::Ptr<sgp_host_t> host_offspring_ptr,          /* Host offspring (transmit to) */
  emp::Ptr<sgp_host_t> host_parent_ptr,             /* Host parent (transmit from) */
  const emp::WorldPosition& parent_pos /* Parent location */
) {
  // NOTE - Make DoVerticalTransmission function?
  // No need to mark reproduction in progress here, as this isn't managed by repro queue.
  // endosym_ptr->GetHardware().GetCPUState().MarkReproInProgress();
  // Trigger before transmission signal.
  before_sym_vert_transmission_sig.Trigger(
    endosym_ptr,          /* symbiont producing offspring */
    host_offspring_ptr, /* transmission to */
    host_parent_ptr,  /* transmission from */
    parent_pos
  );
  // Do vertical transmission
  // NOTE - easy way to grab the new symbiont?
  // TODO - How to know if vertical transmission is successful?
  auto endosym_offspring = endosym_ptr->VerticalTransmission(host_offspring_ptr);
  const bool success = (bool)endosym_offspring;
  emp::Ptr<sgp_sym_t> endosym_offspring_ptr = (success) ?
    static_cast<sgp_sym_t*>(endosym_offspring.value().Raw()) :
    nullptr;
  // TODO -
  // Trigger after transmission signal.
  after_sym_vert_transmission_sig.Trigger(
    endosym_offspring_ptr, /* endosym offspring (if successful) */
    endosym_ptr,                         /* endosym parent*/
    host_offspring_ptr,                  /* transmission to */
    host_parent_ptr,                     /* transmission from */
    parent_pos,
    success
  );
  // NOTE - ResetReproState here or in Reproduce function?
  // endosym_ptr->GetHardware().GetCPUState().ResetReproState();
  return success;
}

void SGPWorld::ProcessGraveyard() {
  // clean up the graveyard
  for (size_t i = 0; i < graveyard.size(); ++i) {
    // NOTE - Does this need to call DoDeath?
    //        the original implementation (in old Update function) does not
    graveyard[i].Delete();
  }
  graveyard.clear();
}

// TODO - add test to make sure this works for hosts as well
void SGPWorld::SendToGraveyard(emp::Ptr<Organism> org) {
  // NOTE - Previous version of this function assumed symbiont
  //        Just in case we end up needing it for host's, might as well make it
  //        work for them as well?
  auto& cpu_state = GetCPUState(org);
  if (cpu_state.ReproInProgress()) {
    repro_queue.Invalidate(
      cpu_state.GetReproQueuePos()
    );
  }

  SymWorld::SendToGraveyard(org);
}

std::optional<emp::WorldPosition> SGPWorld::FindHostForHorizontalTrans(
  size_t host_world_id,                 /* Parent's host location id in world (pops[0][id])*/
  emp::Ptr<sgp_sym_t> sym_parent_ptr    /* Pointer to symbiont parent (producing the sym offspring) */
) {
  // Outsource to configurable functor
  return fun_find_host_for_horizontal_trans(host_world_id, sym_parent_ptr);
}

void SGPWorld::ProcessHostOutputBuffer(sgp_host_t& host) {
  auto& cpu_state = host.GetHardware().GetCPUState();
  const size_t env_task_id = cpu_state.GetTaskEnvID();
  const auto& task_io = task_env.GetIOBank().GetIO(env_task_id);
  // Process output buffer
  auto& output_buffer = cpu_state.GetOutputBuffer();
  for (uint32_t val : output_buffer) {
    // Is this the correct output for any tasks?
    if (task_io.IsValidOutput(val)) {
      // Yes, this output is correct.
      // Get all task ids associated with this output value
      const emp::vector<size_t>& task_ids = task_io.GetTaskIDs(val);
      // Give credit for completed tasks
      for (size_t task_id : task_ids) {
        // Is this a host task?
        if (!task_env.IsHostTask(task_id)) continue;
        // Check task requirements
        auto& task_req_info = task_env.GetHostTaskReq(task_id);
        if (!CanPerformTask(cpu_state, task_req_info)) {
          continue;
        }
        // Mark task as being performed
        cpu_state.MarkTaskPerformed(task_id);
        // Calc value, add to organism points
        host.SetPoints(
          task_req_info.fun_calc_task_val(
            task_env,
            task_req_info,
            host.GetPoints()
          )
        );
        ++host_task_successes[task_id];
      }
    }
  }
  // Clear output buffer
  output_buffer.clear();
}

void SGPWorld::ProcessSymOutputBuffer(sgp_sym_t& sym) {
  auto& cpu_state = sym.GetHardware().GetCPUState();
  const size_t env_task_id = cpu_state.GetTaskEnvID();
  const auto& task_io = task_env.GetIOBank().GetIO(env_task_id);
  // Process output buffer
  auto& output_buffer = cpu_state.GetOutputBuffer();
  for (uint32_t val : output_buffer) {
    // Is this the correct output for any tasks?
    if (task_io.IsValidOutput(val)) {
      // Yes, this output is correct.
      // Get all task ids associated with this output value
      const emp::vector<size_t>& task_ids = task_io.GetTaskIDs(val);
      // Give credit for completed tasks
      for (size_t task_id : task_ids) {
        // Is this a valid sym task?
        if (!task_env.IsSymTask(task_id)) continue;
        // Check task requirements
        auto& task_req_info = task_env.GetSymTaskReq(task_id);
        if (!CanPerformTask(cpu_state, task_req_info)) {
          continue;
        }
        // Mark task as being performed
        cpu_state.MarkTaskPerformed(task_id);
        // Calc value, add to organism points
        sym.SetPoints(
          task_req_info.fun_calc_task_val(
            task_env,
            task_req_info,
            sym.GetPoints()
          )
        );
        ++sym_task_successes[task_id];
      }
    }
  }
  // Clear output buffer
  output_buffer.clear();
}

void SGPWorld::HostDoMutation(sgp_host_t& host) {
  mutator.MutateProgram(host.GetProgram());
}

void SGPWorld::SymDoMutation(sgp_sym_t& sym) {
  mutator.MutateProgram(sym.GetProgram());
}

void SGPWorld::SymDonateToHost(Organism& from_sym, Organism& to_host) {
  emp_assert(!from_sym.IsHost());
  emp_assert(to_host.IsHost());
  // NOTE - could make this a configurable functor if we think
  //        that different config settings will need different donate logic.
  // NOTE - could static cast sym to sgp_sym, host to sgp_host if necessary
  sgp_sym_t& sym = static_cast<sgp_sym_t&>(from_sym);
  sgp_host_t& host = static_cast<sgp_host_t&>(to_host);

  // Donate X% of the total points of the symbiont-host system
  // This way, a sym can donate e.g. 40 or 60 percent of their points in a
  // couple of instructions
  const double sym_points = sym.GetPoints();
  const double to_donate = emp::Min(
    sym_points,
    (sym_points + host.GetPoints()) * sgp_config.SYM_DONATE_PROP()
  );
  // TODO - Protect for threaded implementation
  // TODO - setup data tracking
  // state.world->GetSymDonatedDataNode().WithMonitor(
  //   [=](auto &m) { m.AddDatum(to_donate); });

  // Adjust host/sym points accordingly
  const double donate_value = to_donate * (1.0 - sgp_config.DONATE_PENALTY());
  host.AddPoints(donate_value);
  sym.DecPoints(to_donate);
}

void SGPWorld::SymStealFromHost(Organism& to_sym, Organism& from_host) {
  emp_assert(!to_sym.IsHost());
  emp_assert(from_host.IsHost());
  // NOTE - could make this a configurable functor if we think
  //        that different config settings will need different steal logic.
  // NOTE - could static cast sym to sgp_sym, host to sgp_host if necessary
  sgp_sym_t& sym = static_cast<sgp_sym_t&>(to_sym);
  sgp_host_t& host = static_cast<sgp_host_t&>(from_host);

  const double to_steal = emp::Min(
    host.GetPoints(),
    (sym.GetPoints() + host.GetPoints()) * sgp_config.SYM_STEAL_PROP()
  );
  // TODO - make safe for threading mode + setup data tracking
  // state.world->GetSymStolenDataNode().WithMonitor(
  //   [=](auto &m) { m.AddDatum(to_steal); });

  const double steal_value = to_steal * (1.0 - sgp_config.STEAL_PENALTY());
  host.DecPoints(to_steal);
  sym.AddPoints(steal_value);
}

void SGPWorld::FreeLivingSymDoInfect(Organism& sym) {
  emp_assert(!sym.IsHost());
  emp_assert(sgp_config.SYM_LIMIT() >= 0);
  // NOTE - Could add some runtime customizability here if we want. E.g., functors, etc.
  sgp_sym_t& sgp_sym = static_cast<sgp_sym_t&>(sym);
  // Get sym's location in emp::World pop
  const size_t pop_index = sgp_sym.GetHardware().GetCPUState().GetLocation().GetPopID();
  // Check that there's an available host
  // If this location isn't occupied, infect fails (at no cost?).
  if (!IsOccupied(pop_index)) {
    return;
  }
  // Check that there's enough space for infection
  const size_t num_syms = pop[pop_index]->GetSymbionts().size();
  // NOTE - Should sym_limit be allowed to be negative in config?
  if (num_syms < (size_t)sgp_config.SYM_LIMIT()) {
    // Extract the symbiont from the fls vector and decrement the free-living org
    // count. Then add the sym to the host's sym list.
    // TODO - consider whether we want signals here + if there are some
    //        bookkeeping things we need to do. E.g., add signals, etc.
    // TODO - Do we need to assign a new environment here? I don't think so?
    //        Symbiont should have been assigned an environment on birth.
    // NOTE - Previously, the infect instruction did not check whether AddSymbiont
    //        was successful. Discuss whether we want to check that here.
    pop[pop_index]->AddSymbiont(ExtractSym(pop_index));
    sgp_sym.GetHardware().GetCPUState().SetLocation(
      emp::WorldPosition(pop_index, num_syms)
    );
  } else {
    // Injection failed, set it dead and do deletion next update
    sgp_sym.SetDead();
  }
}

}

#endif