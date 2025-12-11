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
          &GetRandom(), this, sgp_config, CreateNandProgram(PROGRAM_LENGTH), sgp_config->HOST_INT());
        break;
      case HEALTH:
        new_org = emp::NewPtr<HealthHost>(
          &GetRandom(), this, sgp_config, CreateNandProgram(PROGRAM_LENGTH), sgp_config->HOST_INT());
        break;
      case STRESS:
        new_org = emp::NewPtr<StressHost>(
          &GetRandom(), this, sgp_config, CreateNandProgram(PROGRAM_LENGTH), sgp_config->HOST_INT());
        break;
      case NUTRIENT:
        new_org = emp::NewPtr<SGPHost>(
          &GetRandom(), this, sgp_config, CreateNandProgram(PROGRAM_LENGTH), sgp_config->HOST_INT());
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
  * Input: None
  *
  * Output: None
  *
  * Purpose: Setup the task profile retriever function.
  */
void SGPWorld::SetupTaskProfileFun() {
  if (sgp_config->TRACK_PARENT_TASKS() == 2) {
    fun_get_task_profile = [](const emp::Ptr<Organism> org) ->  const emp::BitSet<CPU_BITSET_LENGTH>&{
      if (org->IsHost()) {
        return (*org.DynamicCast<SGPHost>()->GetCPU().state.parent_or_current_tasks_performed).OR_SELF(*org.DynamicCast<SGPHost>()->GetCPU().state.tasks_performed);
      }
      else {
        return (*org.DynamicCast<SGPSymbiont>()->GetCPU().state.parent_or_current_tasks_performed).OR_SELF(*org.DynamicCast<SGPSymbiont>()->GetCPU().state.tasks_performed);
      }
    };
  }
  else if (sgp_config->TRACK_PARENT_TASKS() == 1) {
    fun_get_task_profile = [](const emp::Ptr<Organism> org) ->  const emp::BitSet<CPU_BITSET_LENGTH>&{
      if (org->IsHost()) {
        return *org.DynamicCast<SGPHost>()->GetCPU().state.parent_tasks_performed;
      }
      else {
        return *org.DynamicCast<SGPSymbiont>()->GetCPU().state.parent_tasks_performed;
      }
      };
  }
  else if (sgp_config->TRACK_PARENT_TASKS() == 0) {
    fun_get_task_profile = [](const emp::Ptr<Organism> org) ->  const emp::BitSet<CPU_BITSET_LENGTH>&{
      if (org->IsHost()) {
        return *org.DynamicCast<SGPHost>()->GetCPU().state.tasks_performed;
      }
      else {
        return *org.DynamicCast<SGPSymbiont>()->GetCPU().state.tasks_performed;
      }
    };
  }
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
 * Input: None
 *
 * Output: None
 *
 * Purpose: To handle the organism reproduction queue
 */
void SGPWorld::ProcessReproductionQueue() {
  for (auto org : to_reproduce) {
    if (org == nullptr || org->GetDead())
      continue;
    emp::Ptr<Organism> child = org->Reproduce();
    if (child->IsHost()) {
      // Host::Reproduce() doesn't take care of vertical transmission, that
      // happens here
      for (auto& sym : org->GetSymbionts()) {
        // don't vertically transmit if they must task match but don't
        if (sgp_config->VT_TASK_MATCH() && !TaskMatchCheck(fun_get_task_profile(sym), fun_get_task_profile(org))) continue;
        sym->VerticalTransmission(child);
      }
      DoBirth(child, org->GetLocation());
    }
    else {
      emp::WorldPosition new_pos = SymDoBirth(child, org->GetLocation());
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

/**
 * Input: An id for the the parent symbiont and a pointer to that symbiont. 
 *
 * Output: The id of a host that can be infected by the symbiont
 *
 * Purpose: Searches through up to 10 hosts in the world to find a host that the symbiont can infect. 
 */
int SGPWorld::GetNeighborHost (size_t source_id, const emp::BitSet<CPU_BITSET_LENGTH>& symbiont_tasks){
  // Attempt to find host that matches some tasks
  for (int i = 0; i < 10; i++) {
    emp::WorldPosition neighbor = GetRandomNeighborPos(source_id);
    if (neighbor.IsValid() && IsOccupied(neighbor)){
      //if we check task matching, check if neighbor host does any task that parent sym did & return if so
      if (sgp_config->HT_TASK_MATCH() == 0 || TaskMatchCheck(symbiont_tasks, fun_get_task_profile(GetOrgPtr(neighbor.GetIndex())))) {
        return neighbor.GetIndex();
      }
    }
  }
  //Otherwise parasite can't infect host
  return -1;
}

/**
  * Input: References to symbiont and host task sets
  *
  * Output: Whether host and symbiont task sets are able to accomplish
  * at least one task in common
  *
  * Purpose: To check for task matching before transmission
  */
bool SGPWorld::TaskMatchCheck(const emp::BitSet<CPU_BITSET_LENGTH>& symbiont_tasks, const emp::BitSet<CPU_BITSET_LENGTH>& host_tasks) {
  for (int i = CPU_BITSET_LENGTH - 1; i > -1; i--) {
    if (symbiont_tasks.Get(i) && host_tasks.Get(i)) {
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
    return PlaceSymbiontInHost(sym_baby, fun_get_task_profile(parent), i);
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
    return PlaceSymbiontInHost(symbiont, fun_get_task_profile(symbiont), i);
  }

  /**
   * Input: None
   *
   * Output: None.
   *
   * Purpose: Attempts to inject all of the stress escapee offspring in the
   * stress escapee offspring vector into hosts
   */
  void SGPWorld::ProcessStressEscapeeOffspring() {
    // Shuffle escapees to avoid timing bias 
    emp::vector<size_t> e(symbiont_stress_escapee_offspring.size());
    std::iota(e.begin(), e.end(), 0);
    emp::Shuffle(*random_ptr, e);

    emp::DataMonitor<size_t>& escapee_sucess_data_node = GetStressEscapeeOffspringSuccessCount();

    for (size_t escapee_i : e) {
      StressEscapeeOffspring& escapee_data = symbiont_stress_escapee_offspring[escapee_i];
      // TODO:stress escape data nodes
      emp::WorldPosition new_pos = PlaceSymbiontInHost(escapee_data.escapee_offspring, escapee_data.infection_tasks, escapee_data.parent_pos);
      if (new_pos.IsValid()) {
        escapee_sucess_data_node.AddDatum(1);
      }
    }

    symbiont_stress_escapee_offspring.clear();
  }

  /**
  * Input: A reference to a symbiont task set and a pointer to a host.
  *
  * Output: Returns a bool if the incoming symbiont should be allowed to oust
  *
  * Purpose: Calculate preferential ousting success
  */
  bool SGPWorld::PreferentialOustingAllowed(const emp::BitSet<CPU_BITSET_LENGTH>& incoming_sym_tasks, emp::Ptr<Organism> host){
    const emp::BitSet<CPU_BITSET_LENGTH>& host_tasks = fun_get_task_profile(host);

    for(emp::Ptr<Organism> sym : host->GetSymbionts()){
      const emp::BitSet<CPU_BITSET_LENGTH>& target_sym_tasks = fun_get_task_profile(sym);

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

  /**
  * Input: Pointers to the symbiont to be placed, the taskset to use for infection, and the source position
  *
  * Output: Returns the WorldPosition of the symbiont if it's placed successfully or -1 otherwise
  *
  * Purpose: Place a symbiont based on the passed task set
  */
  emp::WorldPosition SGPWorld::PlaceSymbiontInHost(emp::Ptr<Organism> symbiont, const emp::BitSet<CPU_BITSET_LENGTH>& symbiont_infection_tasks, size_t source_pos) {
    int new_host_pos = GetNeighborHost(source_pos, symbiont_infection_tasks);
    if (new_host_pos > -1) { //-1 means no living neighbors
      if (sgp_config->OUSTING() && sgp_config->PREFERENTIAL_OUSTING() && (int)pop[new_host_pos]->GetSymbionts().size() == sgp_config->SYM_LIMIT()) {
        if (!PreferentialOustingAllowed(symbiont_infection_tasks, pop[new_host_pos])) {
          symbiont.Delete();
          return emp::WorldPosition();
        }
      }
      int new_index = pop[new_host_pos]->AddSymbiont(symbiont);
      if (new_index > 0) { //sym successfully infected
        return emp::WorldPosition(new_index, new_host_pos);
      }
      else { //sym got killed trying to infect
        return emp::WorldPosition();
      }
    }
    else {
      symbiont.Delete();
      return emp::WorldPosition();
    }
  }

#endif
