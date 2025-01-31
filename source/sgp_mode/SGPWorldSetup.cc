#ifndef SGP_WORLD_SETUP_C
#define SGP_WORLD_SETUP_C

#include "emp/datastructs/map_utils.hpp"
#include "emp/tools/string_utils.hpp"

#include "SGPConfigSetup.h"
#include "HealthHost.h"
#include "StressHost.h"
#include "SGPSymbiont.h"
#include "SGPWorld.h"

namespace sgpmode {

/**
 * Input: None.
 *
 * Output: None.
 *
 * Purpose: Prepare the SGPWorld for an experiment by applying the configuration settings
 * and populating the world with hosts and symbionts.
 */
void SGPWorld::Setup() {
  std::cout << "Running sgp world setup..." << std::endl;
  // Configure sgp org type
  SetupOrgMode();

  // NOTE - Some of this code is repeated from base class.
  //  - Could to some reorganization to copy-paste. E.g., make functions for this,
  //     add hooks into the base setup to give more downstream flexibility.
  double start_moi = sgp_config->START_MOI();
  long unsigned int POP_SIZE;
  if (sgp_config->POP_SIZE() == -1) {
    POP_SIZE = sgp_config->GRID_X() * sgp_config->GRID_Y();
  } else {
    POP_SIZE = sgp_config->POP_SIZE();
  }

  // set world structure (either mixed or a grid with some dimensions)
  // and set synchronous generations to false
  if (sgp_config->GRID() == 0) {
    SetPopStruct_Mixed(false);
  } else {
    SetPopStruct_Grid(sgp_config->GRID_X(), sgp_config->GRID_Y(), false);
  }

  // Setup scheduler
  SetupScheduler();

  // Setup host population
  SetupHosts(&POP_SIZE);
  // QUESTION - Why does resize happen after setuphosts?
  Resize(sgp_config->GRID_X(), sgp_config->GRID_Y());
  long unsigned int total_syms = POP_SIZE * start_moi;
  SetupSymbionts(&total_syms);
}

void SGPWorld::SetupOrgMode() {
  std::string cfg_org_type(emp::to_lower(sgp_config->ORGANISM_TYPE()));
  emp_assert(
    emp::Has(sgp_org_type_map, cfg_org_type),
    "Invalid SGP organism type.",
    sgp_config->ORGANISM_TYPE()
  );
  sgp_org_type = sgp_org_type_map[cfg_org_type];

  // Configure stress sym type
  // TODO - Implement SetupStressMode / etc
  std::string cfg_stress_sym_type(emp::to_lower(sgp_config->STRESS_TYPE()));
  emp_assert(
    emp::Has(sgp_stress_sym_type_map, cfg_stress_sym_type),
    "Invalid stress symbiont type.",
    sgp_config->STRESS_TYPE()
  );
  stress_sym_type = sgp_stress_sym_type_map[cfg_stress_sym_type];

  // TODO - clean this up
  // stress hard-coded transmission modes
  if (sgp_org_type == SGPOrganismType::STRESS) {
    if (stress_sym_type == StressSymbiontType::MUTUALIST) {
      // mutualists
      sgp_config->VERTICAL_TRANSMISSION(1.0);
      sgp_config->HORIZ_TRANS(0);
    } else if (stress_sym_type == StressSymbiontType::PARASITE) {
      // parasites
      sgp_config->VERTICAL_TRANSMISSION(0);
      sgp_config->HORIZ_TRANS(1);
    }
  }
}

void SGPWorld::SetupScheduler() {
  // Configure scheduler's process host function
  scheduler.SetProcessHostFun(
    [this](emp::WorldPosition pos, Organism& org) {
      emp_assert(org.IsHost());
      org.Process(pos);
      if (org.GetDead()) {
        this->DoDeath(pos);
      }
    }
  );

  // Configure scheduler's process sym function
  scheduler.SetProcessSymFun(
    [this](emp::WorldPosition pos, Organism& org) {
      emp_assert(!org.IsHost()); // NOTE - IsSym function?
      // have to check for death first, because it might have moved
      // process takes worldposition, dosymdeath takes popid
      if (org.GetDead()) {
        this->DoSymDeath(pos.GetPopID());
      } else {
        org.Process(pos);
      }
      if(this->IsSymPopOccupied(pos.GetPopID()) && org.GetDead()) {
        this->DoSymDeath(pos.GetPopID());
      }
    }
  );
}

void SGPWorld::SetupHosts(unsigned long *POP_SIZE) {

  for (size_t i = 0; i < *POP_SIZE; i++) {
    emp::Ptr<SGPHost> new_org;
    switch (sgp_org_type) {
      case SGPOrganismType::DEFAULT:
        new_org = emp::NewPtr<SGPHost>(
          &GetRandom(), this, sgp_config, CreateNotProgram(100), sgp_config->HOST_INT());
        break;
      case SGPOrganismType::HEALTH:
        new_org = emp::NewPtr<HealthHost>(
          &GetRandom(), this, sgp_config, CreateNotProgram(100), sgp_config->HOST_INT());
        break;
      case SGPOrganismType::STRESS:
        new_org = emp::NewPtr<StressHost>(
          &GetRandom(), this, sgp_config, CreateNotProgram(100), sgp_config->HOST_INT());
        break;
      default:
        std::cout << "Please request a supported sgp organism type" << std::endl;
        break;
    }

    if(sgp_config->START_MOI()==1){

      emp::Ptr<SGPSymbiont> new_sym = emp::NewPtr<SGPSymbiont>(
          &GetRandom(), this, sgp_config, sgp_config->SYM_INT());
      new_org->AddSymbiont(new_sym);
    }
    InjectHost(new_org);
  }
}

void SGPWorld::SetupSymbionts(unsigned long *total_syms) {

}

/**
 * Input: An organism pointer to add to the graveyard
 *
 * Output: None
 *
 * Purpose: To add organisms to the graveyard
 */
void SGPWorld::SendToGraveyard(emp::Ptr<Organism> org) {
  if (org.DynamicCast<SGPSymbiont>()->GetCPU().state.in_progress_repro != -1) {
    to_reproduce[org.DynamicCast<SGPSymbiont>()->GetCPU().state.in_progress_repro].second =
      emp::WorldPosition::invalid_id;
  }
  SymWorld::SendToGraveyard(org);
}

int SGPWorld::GetNeighborHost (size_t id, emp::Ptr<Organism> symbiont){
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

 /**
  * Input: Pointers to a host and to a symbiont
  *
  * Output: Whether host and symbiont parent are able to accomplish
  * at least one task in common
  *
  * Purpose: To check for task matching before vertical transmission
  */
bool SGPWorld::TaskMatchCheck(emp::Ptr<Organism> sym_parent, emp::Ptr<Organism> host_parent) {
  emp::Ptr<emp::BitSet<CPU_BITSET_LENGTH>> parent_tasks;
  emp::Ptr<emp::BitSet<CPU_BITSET_LENGTH>> host_tasks;
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



emp::WorldPosition SGPWorld::SymDoBirth(emp::Ptr<Organism> sym_baby, emp::WorldPosition parent_pos) {
   size_t i = parent_pos.GetPopID();
    emp::Ptr<Organism> parent = GetOrgPtr(i)->GetSymbionts()[parent_pos.GetIndex()-1];
    if(sgp_config->FREE_LIVING_SYMS() == 0){
      int new_host_pos = GetNeighborHost(i, parent);
      if (new_host_pos > -1) { //-1 means no living neighbors
        int new_index = pop[new_host_pos]->AddSymbiont(sym_baby);
        if(new_index > 0){ //sym successfully infected
          return emp::WorldPosition(new_index, new_host_pos);
        } else { //sym got killed trying to infect
          return emp::WorldPosition();
        }
      } else {
        sym_baby.Delete();
        return emp::WorldPosition();
      }
    } else {
      return MoveIntoNewFreeWorldPos(sym_baby, parent_pos);
    }
  }

}

#endif
