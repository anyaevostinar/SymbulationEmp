#ifndef PGG_WORLD_SETUP_C
#define PGG_WORLD_SETUP_C

#include "PGGWorld.h"
#include "PGGHost.h"
#include "PGGSymbiont.h"
/**
 * Input: The number of PGG hosts, whether the PGG hosts have random phenotypes, and the
 * interaction values for PGG hosts if competition mode is on.
 *
 * Output: None.
 *
 * Purpose: To populate the world with PGG hosts with appropriate phenotypes.
 */
void PGGWorld::SetupHosts(long unsigned int* POP_SIZE, double comp_host_1, double comp_host_2){
  for (size_t i = 0; i < *POP_SIZE; i++) {
    emp::Ptr<PGGHost> new_org;

    if (my_config->COMPETITION_MODE() && i % 2 == 0) {
      new_org.New(&GetRandom(), this, my_config, comp_host_1);
    }
    else if (my_config->COMPETITION_MODE() && i % 2 == 1) {
      new_org.New(&GetRandom(), this, my_config, comp_host_2);
    }
    else {
      new_org.New(&GetRandom(), this, my_config, my_config->HOST_INT());
    }
    InjectHost(new_org);
  }
}

/**
 * Input: The number of PGG symbionts and whether the PGG symbionts have random phenotypes.
 *
 * Output: None.
 *
 * Purpose: To populate the world with PGG symbionts with appropriate phenotypes.
 */
void PGGWorld::SetupSymbionts(int* total_syms){
  for (int j = 0; j < *total_syms; j++) {
    double sym_donation = my_config->PGG_DONATE();
    emp::Ptr<PGGSymbiont> new_sym = emp::NewPtr<PGGSymbiont>(&GetRandom(), this, my_config, my_config->SYM_INT(), sym_donation, 0);
    InjectSymbiont(new_sym);
  }
}

#endif
