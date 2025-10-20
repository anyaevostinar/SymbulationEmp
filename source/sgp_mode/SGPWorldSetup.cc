#ifndef SGP_WORLD_SETUP_C
#define SGP_WORLD_SETUP_C

#include "SGPConfigSetup.h"
#include "HealthHost.h"
#include "StressHost.h"
#include "SGPSymbiont.h"
#include "SGPWorld.h"


/**
 * Input: A pointer to a long for the number of hosts to add to the world
 *
 * Output: None
 *
 * Purpose: Adds a number of hosts of a set type to the world. 
 */
void SGPWorld::SetupHosts(unsigned long *POP_SIZE) {
  for (size_t i = 0; i < *POP_SIZE; i++) {
    emp::Ptr<SGPHost> new_org;
    switch (sgp_config->INTERACTION_MECHANISM()) {
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
      case NUTRIENT:
        new_org = emp::NewPtr<SGPHost>(
          &GetRandom(), this, sgp_config, CreateNotProgram(100), sgp_config->HOST_INT());
        break;
      default:
       
        throw "Please request a supported sgp organism type";

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
    to_reproduce[org.DynamicCast<SGPSymbiont>()->GetCPU().state.in_progress_repro] = nullptr;
  }
  SymWorld::SendToGraveyard(org);
}


/**
 * Input: An id for the the parent symbiont and a pointer to that symbiont. 
 *
 * Output: The id of a host that can be infected by the symbiont
 *
 * Purpose: Searches through up to 10 hosts in the world to find a host that the symbiont can infect. 
 */
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
  * Purpose: To check for task matching before transmission
  */
bool SGPWorld::TaskMatchCheck(emp::Ptr<Organism> sym_parent, emp::Ptr<Organism> host_parent) {

  if(sgp_config->INTERACTION_MECHANISM() == NUTRIENT) {
    return true;
  }

  emp::BitSet<CPU_BITSET_LENGTH> host_infection_tasks = host_parent.DynamicCast<SGPHost>()->GetInfectionTaskSet();
  emp::BitSet<CPU_BITSET_LENGTH> sym_infection_tasks = sym_parent.DynamicCast<SGPSymbiont>()->GetInfectionTaskSet();

  for (int i = CPU_BITSET_LENGTH - 1; i > -1; i--) {
    if (sym_infection_tasks.Get(i) && host_infection_tasks.Get(i)) {
      return true;
    }
  }
  
  return false;
}


 /**
  * Input: Pointers to a symbiont offspring and the position of the symbiont's parent. 
  * Note that the position of symbiont parent is a WorldPosition with index as 1-index position
  * in host's syms list and pop_id as host's location in the world
  *
  * Output: Returns a WorldPosition pointer, a valid one for successful 
  * infection and an invalid for a failed infection
  *
  * Purpose: To place a symbiont in a new location or host in the world. 
  */
emp::WorldPosition SGPWorld::SymDoBirth(emp::Ptr<Organism> sym_baby, emp::WorldPosition parent_pos) {
   size_t i = parent_pos.GetPopID();
    emp::Ptr<Organism> parent = GetOrgPtr(i)->GetSymbionts()[parent_pos.GetIndex()-1];
    int new_host_pos = GetNeighborHost(i, parent);
    if (new_host_pos > -1) { //-1 means no living neighbors
      if(sgp_config->OUSTING() && sgp_config->PREFERENTIAL_OUSTING() && (int)pop[new_host_pos]->GetSymbionts().size() == sgp_config->SYM_LIMIT()){
        if(!PreferentialOustingAllowed(parent, pop[new_host_pos])){
          sym_baby.Delete();
          return emp::WorldPosition();
        }
      }
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
  }


 /**
  * Input: Pointers to a symbiont and the position of the symbiont. 
  * Note that the position of the symbiont is a WorldPosition with index as 1-index position
  * in host's syms list and pop_id as host's location in the world
  *
  * Output: Returns a WorldPosition pointer, a valid one for successful 
  * infection and an invalid for a failed infection
  *
  * Purpose: To place a symbiont in a new location or host in the world. 
  */
  emp::WorldPosition SGPWorld::SymFindHost(emp::Ptr<Organism> symbiont, emp::WorldPosition cur_pos) {
   size_t i = cur_pos.GetPopID();
    int new_host_pos = GetNeighborHost(i, symbiont);
    if (new_host_pos > -1) { //-1 means no living neighbors
      if(sgp_config->OUSTING() && sgp_config->PREFERENTIAL_OUSTING() && (int)pop[new_host_pos]->GetSymbionts().size() == sgp_config->SYM_LIMIT()){
        if(!PreferentialOustingAllowed(symbiont, pop[new_host_pos])){
          symbiont.Delete();
          return emp::WorldPosition();
        }
      }
      int new_index = pop[new_host_pos]->AddSymbiont(symbiont);
      if(new_index > 0){ //sym successfully infected
        return emp::WorldPosition(new_index, new_host_pos);
      } else { //sym got killed trying to infect
        return emp::WorldPosition();
      }
    } else {
      symbiont.Delete();
      return emp::WorldPosition();
    }
  }

  /**
   * Input: None
   *
   * Output: None.
   *
   * Purpose: Attempts to inject all of the stress escapee offspring in the
   * stress escapee offspring vector into hosts; if no host can be found,
   * the offspring is killed.
   */
  void SGPWorld::ProcessStressEscapeeOffspring() {
    for (auto escapee_data : symbiont_stress_escapee_offspring) {
      // TODO:stress escape data nodes

      emp::Ptr<emp::BitSet<CPU_BITSET_LENGTH>> sym_parent_tasks = escapee_data.escapee_offspring.DynamicCast<SGPSymbiont>()->GetCPU().state.parent_tasks_performed;
      bool matching = false;
      emp::WorldPosition neighbor;
      for (int i = 0; i < 10 && !matching; i++) {
        neighbor = GetRandomNeighborPos(escapee_data.parent_pos);
        if (neighbor.IsValid() && IsOccupied(neighbor)) {
          //check if neighbor host does any task that parent sym did & return if so
          emp::Ptr<SGPHost> host = pop[neighbor.GetIndex()].DynamicCast<SGPHost>();
          for (int i = CPU_BITSET_LENGTH - 1; i > -1 && !matching; i--) {
            if (sgp_config->TRACK_PARENT_TASKS()) {
              matching = (sym_parent_tasks->Get(i) || escapee_data.grandparent_tasks.Get(i)) &&
                (host->GetCPU().state.tasks_performed->Get(i) || host->GetCPU().state.parent_tasks_performed->Get(i));
            }
            else {
              matching = sym_parent_tasks->Get(i) && host->GetCPU().state.tasks_performed->Get(i);
            }
          }
        }
      }

      if (matching && neighbor.IsValid() && IsOccupied(neighbor)) {
        pop[neighbor.GetIndex()]->AddSymbiont(escapee_data.escapee_offspring);
      }
      else {
        escapee_data.escapee_offspring.Delete();
      }
    }

    symbiont_stress_escapee_offspring.clear();
  }

  /**
  * Input: Pointers to a symbiont and a host.
  *
  * Output: Returns a bool if the incoming symbiont should be allowed to oust
  *
  * Purpose: Calculate preferential ousting success
  */
  bool SGPWorld::PreferentialOustingAllowed(emp::Ptr<Organism> sym_parent, emp::Ptr<Organism> host){
    emp::BitSet<CPU_BITSET_LENGTH> host_tasks = host.DynamicCast<SGPHost>()->GetInfectionTaskSet(); 
    emp::BitSet<CPU_BITSET_LENGTH> incoming_sym_tasks = sym_parent.DynamicCast<SGPSymbiont>()->GetInfectionTaskSet();

    for(emp::Ptr<Organism> sym : host->GetSymbionts()){
      emp::BitSet<CPU_BITSET_LENGTH> target_sym_tasks = sym.DynamicCast<SGPSymbiont>()->GetInfectionTaskSet();

      if(sgp_config->PREFERENTIAL_OUSTING() == 1){
        // if has worse task match with any hosted sym, fail
        if(host_tasks.AND(incoming_sym_tasks).CountOnes() < host_tasks.AND(target_sym_tasks).CountOnes()){
          return false;
        }
      }
      else if(sgp_config->PREFERENTIAL_OUSTING() == 2){
        // if has equal or worse task match with any hosted sym, fail
        if(host_tasks.AND(incoming_sym_tasks).CountOnes() <= host_tasks.AND(target_sym_tasks).CountOnes()){
          return false;
        }
      }
    }

    return true; 
 }
  
#endif
