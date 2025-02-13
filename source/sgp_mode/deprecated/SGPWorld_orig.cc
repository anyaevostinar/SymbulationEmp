#ifndef SGP_WORLD_C
#define SGP_WORLD_C

#include "SGPWorld.h"
#include "SGPHost.h"
#include "SGPSymbiont.h"
#include "utils.h"

namespace sgpmode {

void SGPWorld::DoReproduction() {
  for (auto& info : to_reproduce) {
    const emp::WorldPosition& position = info.second;
    emp::Ptr<Organism> org = info.first;
    // If queued organism is in an invalid position or is dead, don't reproduce.
    if (!position.IsValid() || org->GetDead()) {
      continue;
    }
    // Reproduce organism.
    emp::Ptr<Organism> child = org->Reproduce();
    // Run appropriate do birth function based on organism type.
    // TODO - host parent might no longer be in world? Need to use pointer from repro queue
    (child->IsHost()) ? HostDoBirth(child, org, position) : SymDoBirth(child, position);

  }
  to_reproduce.clear();
}

emp::WorldPosition SGPWorld::SymDoBirth(
  emp::Ptr<Organism> sym_baby,
  emp::WorldPosition parent_pos
) {
  // Trigger any before birth actions.
  before_sym_do_birth.Trigger(sym_baby, parent_pos);

  emp::WorldPosition sym_baby_pos = fun_sym_do_birth(sym_baby, parent_pos);

  // Trigger any after birth actions
  // NOTE - Triggers on both successful and unsuccessful births.
  after_sym_do_birth.Trigger(sym_baby_pos);

  return sym_baby_pos;
}

emp::WorldPosition SGPWorld::HostDoBirth(
  emp::Ptr<Organism> host_offspring_ptr,
  emp::Ptr<Organism> host_parent_ptr,
  emp::WorldPosition parent_pos
) {
  // TODO - can static cast pointers to SGP-specific base classes.
  emp_assert(host_offspring_ptr->IsHost());
  emp_assert(host_parent_ptr->IsHost());
  // emp_assert(IsOccupied(parent_pos)); NOTE - Should this assert be true (fails in tests)?

  before_host_do_birth.Trigger(host_offspring_ptr, host_parent_ptr, parent_pos);

  // NOTE - Can make contents of this function into a functor if needs to be
  //        configurable for different types of hosts.
  // Host::Reproduce() doesn't take care of vertical transmission, that
  //  happens here. Loop over parent's symbiont, check if each can transmit
  //  vertically to host offspring.
  for (auto sym_ptr : host_parent_ptr->GetSymbionts()) {
    emp_assert(!sym_ptr->IsHost());
    // Check if symbiont will succeed in attempted vertical transmission.
    if (!can_attempt_vert_trans(sym_ptr, host_offspring_ptr, host_parent_ptr, parent_pos)) {
      continue;
    }
    // NOTE - Make DoVerticalTransmission function?
    // Trigger before transmission signal.
    before_sym_vert_transmission.Trigger(
      sym_ptr,          /* symbiont producing offspring */
      host_parent_ptr,  /* transmission from */
      host_offspring_ptr, /* transmission to */
      parent_pos
    );
    // Do vertical transmission
    // NOTE - easy way to grab the new symbiont?
    // TODO - How to know if vertical transmission is successful?
    sym_ptr->VerticalTransmission(host_offspring_ptr);
    // Trigger after transmission signal.
    after_sym_vert_transmission.Trigger(
      host_parent_ptr,  /* transmission from */
      host_offspring_ptr, /* transmission to */
      parent_pos
    );
  }

  const emp::WorldPosition offspring_pos = DoBirth(host_offspring_ptr, parent_pos);

  after_host_do_birth.Trigger(offspring_pos);

  return offspring_pos;
}

void SGPWorld::ProcessGraveyard() {
  // clean up the graveyard
  for (size_t i = 0; i < graveyard.size(); ++i) {
    graveyard[i].Delete();
  }
  graveyard.clear();
}

// TODO - add test to make sure this works for hosts as well
void SGPWorld::SendToGraveyard(emp::Ptr<Organism> org) {
  // NOTE: Currently, if not host, assume symbiont.
  // Determine if repro is in progress.
  const int in_progress_repo = (org->IsHost()) ?
    (static_cast<SGPHost*>(org.Raw()))->GetCPU().state.in_progress_repro :
    (static_cast<SGPSymbiont*>(org.Raw()))->GetCPU().state.in_progress_repro;
  if (in_progress_repo != -1) {
    to_reproduce[in_progress_repo].second = emp::WorldPosition::invalid_id;
  }
  SymWorld::SendToGraveyard(org);
}

int SGPWorld::GetNeighborHost(size_t id, emp::Ptr<Organism> symbiont) {
  // TODO - Why magic number here?
  // Attempt to find host that matches some tasks
  for (int i = 0; i < 10; i++) {
    emp::WorldPosition neighbor = GetRandomNeighborPos(id);
    if (neighbor.IsValid() && IsOccupied(neighbor)) {
      auto neighbor_host_ptr = GetOrgPtr(neighbor.GetIndex());
      emp_assert(neighbor_host_ptr->IsHost());
      //check if neighbor host does any task that parent sym did & return if so
      const bool compatible = host_sym_compatibility_check(
        // AsSGPHost(neighbor_host_ptr),
        // AsSGPSymbiont(symbiont)
        neighbor_host_ptr,
        symbiont
      );
      if (compatible) {
        return neighbor.GetIndex();
      }
    }
  }
  //Otherwise parasite can't infect host
  return -1;
}

}

#endif