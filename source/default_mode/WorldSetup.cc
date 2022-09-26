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
void SymWorld::SetupHosts(long unsigned int* POP_SIZE, bool random_phen_host, double comp_host_1, double comp_host_2){ 
  std::cout << "symworld setup hosts" << std::endl;
  for (size_t i = 0; i < *POP_SIZE; i++) {
  emp::Ptr<Host> new_org;
  if (random_phen_host) {new_org.New(&GetRandom(), this, my_config, GetRandom().GetDouble(-1, 1));
   } else if (my_config->COMPETITION_MODE() && i%2==0) {
       new_org.New(&GetRandom(), this, my_config, comp_host_1);
   } else if (my_config->COMPETITION_MODE() && i%2==1) {
       new_org.New(&GetRandom(), this, my_config, comp_host_2);
   } else { new_org.New(&GetRandom(), this, my_config, my_config->HOST_INT());
   }

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
void SymWorld::SetupSymbionts(int *total_syms, bool random_phen_sym) {
  //This loop must be outside of the host generation loop since otherwise
  //syms try to inject into mostly empty spots at first
  for (int j = 0; j < *total_syms; j++) {
    double sym_int = 0;
    if (random_phen_sym) { sym_int = GetRandom().GetDouble(-1, 1); }
    else { sym_int = my_config->SYM_INT(); }

    emp::Ptr<Symbiont> new_sym = emp::NewPtr<Symbiont>(&GetRandom(), this, my_config,
      sym_int, 0);
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
  bool random_phen_host = false;
  bool random_phen_sym = false;
  if(my_config->HOST_INT() == -2 && !my_config->COMPETITION_MODE()) random_phen_host = true;
  if(my_config->SYM_INT() == -2) random_phen_sym = true;

  if (my_config->GRID() == 0) {SetPopStruct_Mixed(false);}
  else SetPopStruct_Grid(my_config->GRID_X(), my_config->GRID_Y(), false);

  double comp_host_1 = 0;
  double comp_host_2 = 0.95;

  SetupHosts(&POP_SIZE, random_phen_host, comp_host_1, comp_host_2);

  Resize(my_config->GRID_X(), my_config->GRID_Y());

  int total_syms = POP_SIZE * start_moi;
  SetupSymbionts(&total_syms, random_phen_sym);
}
#endif
