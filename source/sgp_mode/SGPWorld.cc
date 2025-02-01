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

int SGPWorld::GetNeighborHost(size_t id, emp::Ptr<Organism> symbiont) {
  // TODO - Why magic number here?
  // Attempt to find host that matches some tasks
  for (int i = 0; i < 10; i++) {
    emp::WorldPosition neighbor = GetRandomNeighborPos(id);
    if (neighbor.IsValid() && IsOccupied(neighbor)){
      //check if neighbor host does any task that parent sym did & return if so
      if (TaskMatchCheck(symbiont, GetOrgPtr(neighbor.GetIndex()))) {
        return neighbor.GetIndex();
      }
    }
  }
  //Otherwise parasite can't infect host
  return -1;
}


bool SGPWorld::TaskMatchCheck(emp::Ptr<Organism> sym_parent, emp::Ptr<Organism> host_parent) {
  emp::Ptr<emp::BitSet<CPU_BITSET_LENGTH>> parent_tasks;
  emp::Ptr<emp::BitSet<CPU_BITSET_LENGTH>> host_tasks;
  // TODO - shift to one-time configurable option
  if (sgp_config->TRACK_PARENT_TASKS()) {
    parent_tasks = sym_parent.DynamicCast<SGPSymbiont>()->GetCPU().state.parent_tasks_performed;
    host_tasks = host_parent.DynamicCast<SGPHost>()->GetCPU().state.parent_tasks_performed;
  }
  else {
    parent_tasks = sym_parent.DynamicCast<SGPSymbiont>()->GetCPU().state.tasks_performed;
    host_tasks = host_parent.DynamicCast<SGPHost>()->GetCPU().state.tasks_performed;
  }
  for (int i = host_tasks->size() - 1; i > -1; i--) {
    if (parent_tasks->Get(i) && host_tasks->Get(i)) {
      //both parent sym and host can do this task, symbiont baby can infect
      return true;
    }
  }
  return false;
}

void SGPWorld::DoReproduction() {
  for (auto& info : to_reproduce) {
    const emp::WorldPosition& position = info.second;
    emp::Ptr<Organism> org = info.first;

    if (!position.IsValid() || org->GetDead()) {
      continue;
    }
    emp::Ptr<Organism> child = org->Reproduce();
    if (child->IsHost()) {
      // Host::Reproduce() doesn't take care of vertical transmission, that
      // happens here
      for (auto& sym : org->GetSymbionts()) {
        // don't vertically transmit if they must task match but don't
        // TODO - Make condition for vertical transmission configurable
        if (sgp_config->VT_TASK_MATCH() && !TaskMatchCheck(sym, org)) continue;
        sym->VerticalTransmission(child);
      }
      DoBirth(child, position);
    } else {
      emp::WorldPosition new_pos = SymDoBirth(child, position);
      // Because we're not calling HorizontalTransmission, we need to adjust
      // these data nodes here
      emp::DataMonitor<int>& data_node_attempts_horiztrans =
        GetHorizontalTransmissionAttemptCount();

      data_node_attempts_horiztrans.AddDatum(1);

      emp::DataMonitor<int>& data_node_successes_horiztrans =
          GetHorizontalTransmissionSuccessCount();

      if (new_pos.IsValid()) {
        data_node_successes_horiztrans.AddDatum(1);
      }
    }
  }
  to_reproduce.clear();
}

void SGPWorld::ProcessGraveyard() {
  // clean up the graveyard
  for (size_t i = 0; i < graveyard.size(); ++i) {
    graveyard[i].Delete();
  }
  graveyard.clear();
}

}

#endif