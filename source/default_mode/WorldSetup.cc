#ifndef WORLD_SETUP_C
#define WORLD_SETUP_C

#include "SymWorld.h"
#include "Host.h"
#include "Symbiont.h"

/**
 * Input: None.
 *
 * Output: A pointer to a new host.
 *
 * Purpose: To create and get the pointer to a new host.
 */
emp::Ptr<Organism> SymWorld::GetNewHost(){ 
  return emp::NewPtr<Host>(&GetRandom(), this, my_config, my_config->HOST_INT());
}


/**
 * Input: None.
 *
 * Output: A pointer to a new symbiont.
 *
 * Purpose: To create and get the pointer to a new symbiont.
 */
emp::Ptr<Organism> SymWorld::GetNewSym() {
  return emp::NewPtr<Symbiont>(&GetRandom(), this, my_config, my_config->SYM_INT(), 0);
}


/**
 * Input: The number of hosts.
 *
 * Output: None.
 *
 * Purpose: To populate the world with hosts with appropriate phenotypes.
 */
void SymWorld::SetupHosts(long unsigned int* POP_SIZE) {
  if (my_config->RANDOM_INJECTION()) {
    for (size_t i = 0; i < *POP_SIZE; i++) {
      InjectHost(GetNewHost());
    }
  }
  else {
    int host_offset = 1;
    if (my_config->GRID()) {
      host_offset = (my_config->GRID_X() * my_config->GRID_Y()) / *POP_SIZE;
    }
    for (size_t i = 0; i < host_offset * *POP_SIZE; i += host_offset) {
      emp::Ptr<Organism> new_org = GetNewHost();
      AddOrgAt(new_org, i);
    }
  }

}


/**
 * Input: The number of symbionts and the number of hosts.
 *
 * Output: None.
 *
 * Purpose: To populate the world with symbionts with appropriate phenotypes.
 */
void SymWorld::SetupSymbionts(long unsigned int* total_syms, long unsigned int* POP_SIZE) {
  if (my_config->RANDOM_INJECTION()) {
    for (size_t j = 0; j < *total_syms; j++) {
      InjectSymbiont(GetNewSym());
    }
  }
  else {
    int host_offset = 1;
    int sym_offset = 1;
    if (my_config->GRID()) {
      host_offset = (my_config->GRID_X() * my_config->GRID_Y()) / *POP_SIZE;
      if (my_config->FREE_LIVING_SYMS()) {
        sym_offset = (my_config->GRID_X() * my_config->GRID_Y()) / *total_syms;
      }
      else {
        sym_offset = host_offset * (*POP_SIZE / (*total_syms));
      }
    }
    for (size_t i = 0; i < sym_offset * *total_syms; i += sym_offset) {
      emp::Ptr<Organism> new_sym = GetNewSym();
      if (my_config->FREE_LIVING_SYMS()) {
        AddOrgAt(new_sym, emp::WorldPosition(0, i));
      }
      else if (IsOccupied(i)) {
        pop[i]->AddSymbiont(new_sym);
      }
    }
  }
}

/**
 * Input: None.
 *
 * Output: None.
 *
 * Purpose: Prepare the world for an experiment by applying the configuration settings
 * and populating the world with hosts and symbionts.
 */
void SymWorld::Setup() {
  double start_moi = my_config->START_MOI();
  long unsigned int POP_SIZE;
  if (my_config->POP_SIZE() == -1) {
    POP_SIZE = my_config->GRID_X() * my_config->GRID_Y();
  }
  else {
    POP_SIZE = my_config->POP_SIZE();
  }
  long unsigned int total_syms = POP_SIZE * start_moi;

  // set world structure (either mixed or a grid with some dimensions) and set synchronous generations to false
  if (my_config->GRID() == 0) { SetPopStruct_Mixed(false); }
  else SetPopStruct_Grid(my_config->GRID_X(), my_config->GRID_Y(), false);

  Resize(my_config->GRID_X(), my_config->GRID_Y());

  SetupHosts(&POP_SIZE);
  SetupSymbionts(&total_syms, &POP_SIZE);
}
#endif
