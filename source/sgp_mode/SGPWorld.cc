#ifndef SGP_WORLD_C
#define SGP_WORLD_C

#include "SGPWorld.h"
#include "SGPHost.h"
#include "SGPSymbiont.h"
#include "utils.h"

namespace sgpmode {

void SGPWorld::ProcessOrgAt(size_t pop_id) {
  // Process host at this location (if any)
  if (IsOccupied(pop_id)) {
    emp_assert(GetOrg(pop_id).IsHost());
    ProcessHostAt(
      {pop_id},
      static_cast<sgp_host_t&>(GetOrg(pop_id))
    );
  }
  // TODO - double check that my interpretation is correct here
  // Process free-living symbiont at this location (if any)
  if (IsSymPopOccupied(pop_id)) {
    emp_assert(!GetSymAt(pop_id)->IsHost());
    ProcessFreeLivingSymAt(
      emp::WorldPosition(0, pop_id),
      static_cast<sgp_sym_t&>(*(GetSymAt(pop_id)))
    );
  }
}

void SGPWorld::ProcessHostAt(const emp::WorldPosition& pos, sgp_host_t& host) {
  // If host is dead, don't process.
  if (host.GetDead()) {
    return;
  }
  before_host_process_sig.Trigger(host);
  // Update host location
  host.GetHardware().GetCPUState().SetLocation(pos); // TODO - is this necessary here?
  // TODO - do we need to update org location every update? (this was being done in RunCPUStep every cpu step)
  // Execute organism hardware for CYCLES_PER_UPDATE steps
  for (size_t i = 0; i < sgp_config.CYCLES_PER_UPDATE(); ++i) {
    // Execute 1 CPU cycle
    host.GetHardware().RunCPUStep(pos, 1);
    after_host_cpu_step_sig.Trigger(host);
  }
  // Handle any endosymbionts (configurable at setup-time)
  // NOTE - is there any reason that this might need to be a functor?
  ProcessEndosymbionts(host);
  // Run host's process function (post cpu steps, post endosymbiont processing)
  // TODO - when do we actually want to run this?
  host.Process(pos);
  after_host_process_sig.Trigger(host);
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
  }
  // Call symbiont's process function
  sym.Process(sym_pos);
  after_endosym_process_sig.Trigger(sym_pos, sym, host);
}

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
      after_freeliving_sym_cpu_step_sig.Trigger(sym);
    }
    // Call symbiont's process function
    sym.Process(pos);
    after_freeliving_sym_process_sig.Trigger(sym);
  }
  // TODO - double check that this belongs just here and not also in endosymbiont code
  if (IsSymPopOccupied(pos.GetPopID()) && sym.GetDead()) {
    DoSymDeath(pos.GetPopID());
  }
}

void SGPWorld::DoReproduction() {
  // Process reproduction queue
  for (ReproEvent& repro_info : repro_queue.GetQueue()) {
    emp::Ptr<Organism> org = repro_info.org;
    // If queued organism is is dead or repro event has been invalidated, don't reproduce.
    if (!repro_info.valid || org->GetDead()) {
      continue;
    }
    // Reproduce organism, producing an offspring
    emp::Ptr<Organism> child = org->Reproduce();
    // Run appropriate do birth function based on organism type.
    (child->IsHost()) ?
      HostDoBirth(child, org, repro_info.pos) :
      SymDoBirth(child, repro_info.pos);
  }
  // Clear the queue now that it has been processed
  repro_queue.Clear();
}

emp::WorldPosition SGPWorld::SymDoBirth(
  emp::Ptr<Organism> sym_baby,
  emp::WorldPosition parent_pos
) {
  /* TODO */
  return emp::WorldPosition{};
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
    // -- BOOKMARK --
  }





  return emp::WorldPosition{};
}

void SGPWorld::ProcessGraveyard() {
  // clean up the graveyard
  for (size_t i = 0; i < graveyard.size(); ++i) {
    graveyard[i].Delete();
  }
  graveyard.clear();
}

// TODO - add test to make sure this works for hosts as well
void SGPWorld::SendToGraveyard(emp::Ptr<Organism> org) { /*TODO*/ }

}

#endif