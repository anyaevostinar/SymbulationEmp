#ifndef SGP_WORLD_C
#define SGP_WORLD_C

#include "SGPWorld.h"
#include "SGPHost.h"
#include "SGPSymbiont.h"
#include "utils.h"

namespace sgpmode {
  void SGPWorld::DoReproduction() { /*TODO*/ }

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
    emp::WorldPosition parent_pos
  ) {
    /* TODO */
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