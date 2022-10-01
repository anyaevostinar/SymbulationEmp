#ifndef WORLD_SETUP_C
#define WORLD_SETUP_C

#include "SymWorld.h"
#include "Host.h"
#include "Symbiont.h"

/**
 * Input: The number of hosts, whether the hosts have random phenotypes, and the 
 * interaction values for hosts if competition mode is on.
 *
 * Output: None.
 *
 * Purpose: To populate the world with hosts with appropriate phenotypes. 
 */
void SymWorld::SetupHosts(long unsigned int* POP_SIZE){ 
  for (size_t i = 0; i < *POP_SIZE; i++) {
  emp::Ptr<Host> new_org;
  //since competition is checked first, don't  need the && !competition part of  rand host phenotype
  new_org.New(&GetRandom(), this, my_config, my_config->HOST_INT());
  InjectHost(new_org);
  }

}


/**
 * Input: The number of symbionts and whether the symbionts have random phenotypes.
 *
 * Output: None.
 *
 * Purpose: To populate the world with symbionts with appropriate phenotypes.
 */
void SymWorld::SetupSymbionts(int *total_syms) {
  //This loop must be outside of the host generation loop since otherwise
  //syms try to inject into mostly empty spots at first
  for (int j = 0; j < *total_syms; j++) {
    emp::Ptr<Symbiont> new_sym = emp::NewPtr<Symbiont>(&GetRandom(), this, my_config, my_config->SYM_INT(), 0);
    InjectSymbiont(new_sym);
  }
}

/**
 * Input: None.
 *
 * Output: None.
 *
 * Purpose: Prepare the world for a simulation by applying the configuration settings 
 * and populating the world with hosts and symbionts.
 */
void SymWorld::Setup() {
  double start_moi = my_config->START_MOI();
  long unsigned int POP_SIZE;
  if (my_config->POP_SIZE() == -1) {
    POP_SIZE = my_config->GRID_X() * my_config->GRID_Y();
  } else {
    POP_SIZE = my_config->POP_SIZE();
  }

  if (my_config->GRID() == 0) {SetPopStruct_Mixed(false);}
  else SetPopStruct_Grid(my_config->GRID_X(), my_config->GRID_Y(), false);

  SetupHosts(&POP_SIZE);

  Resize(my_config->GRID_X(), my_config->GRID_Y());

  int total_syms = POP_SIZE * start_moi;
  SetupSymbionts(&total_syms);
}
#endif
