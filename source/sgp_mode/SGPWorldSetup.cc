#ifndef SGP_WORLD_SETUP_C
#define SGP_WORLD_SETUP_C

#include "SGPConfigSetup.h"
#include "HealthHost.h"
#include "StressHost.h"
#include "SGPSymbiont.h"
#include "SGPWorld.h"

void SGPWorld::SetupHosts(unsigned long *POP_SIZE) {
  for (size_t i = 0; i < *POP_SIZE; i++) {
    emp::Ptr<SGPHost> new_org;
    switch (sgp_config->ORGANISM_TYPE()) {
      case DEFAULT:
        new_org = emp::NewPtr<SGPHost>(
          &GetRandom(), this, sgp_config, CreateNotProgram(100), sgp_config->HOST_INT());
        break;
      case HEALTH:
        new_org = emp::NewPtr<HealthHost>(
          &GetRandom(), this, sgp_config, CreateNotProgram(100), sgp_config->HOST_INT());
        break;
      case STRESS:
        new_org = emp::NewPtr<StressHost>(
          &GetRandom(), this, sgp_config, CreateNotProgram(100), sgp_config->HOST_INT());
        break;
      default:
        //TODO: make this an assert and add to Host Setup Test
        std::cout << "Please request a supported sgp organism type" << std::endl;
        return;
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

    emp::Ptr<emp::BitSet<CPU_BITSET_LENGTH>> grand_parent_tasks;
    emp::Ptr<emp::BitSet<CPU_BITSET_LENGTH>> parent_host_tasks;

    grand_parent_tasks = sym_parent.DynamicCast<SGPSymbiont>()->GetCPU().state.parent_tasks_performed;
    parent_host_tasks = host_parent.DynamicCast<SGPHost>()->GetCPU().state.parent_tasks_performed;

    parent_tasks = sym_parent.DynamicCast<SGPSymbiont>()->GetCPU().state.tasks_performed;
    host_tasks = host_parent.DynamicCast<SGPHost>()->GetCPU().state.tasks_performed;


    for (int i = CPU_BITSET_LENGTH - 1; i > -1; i--) {

      if ((parent_tasks->Get(i) || grand_parent_tasks->Get(i)) && (host_tasks->Get(i) || parent_host_tasks->Get(i))) {

        //If either parent sym or grandparent sym can do task 
        //host and parent host can do task then sym baby can infect
        return true;
      }
  }
  }
  else {
    parent_tasks = sym_parent.DynamicCast<SGPSymbiont>()->GetCPU().state.tasks_performed;
    host_tasks = host_parent.DynamicCast<SGPHost>()->GetCPU().state.tasks_performed;

    for (int i = host_tasks - 1; i > -1; i--) {

     
      if (parent_tasks->Get(i) && host_tasks->Get(i)) {
      
        //both parent sym and host can do this task, symbiont baby can infect
        return true;
      }
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



#endif
