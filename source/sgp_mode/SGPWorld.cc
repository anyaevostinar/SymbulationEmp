#ifndef SGP_WORLD_C
#define SGP_WORLD_C

#include "SGPWorld.h"
#include "SGPHost.h"
#include "SGPSymbiont.h"

namespace sgpmode {

emp::WorldPosition SGPWorld::SymDoBirth(
  emp::Ptr<Organism> sym_baby,
  emp::WorldPosition parent_pos
) {
  // Trigger any before birth actions.
  before_sym_do_birth.Trigger(sym_baby, parent_pos);

  emp::WorldPosition sym_baby_pos = fun_sym_do_birth(sym_baby, parent_pos);

  // Trigger any after birth actions
  // NOTE - Currently triggers regardless of success. Should this only trigger on successful births?
  //   Or, have separate signals for successful / unsuccessful births?
  after_sym_do_birth.Trigger(sym_baby_pos);

  return sym_baby_pos;
}

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

}

#endif