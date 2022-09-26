#ifndef EFFWORLD_SETUP_C
#define EFFWORLD_SETUP_C

#include "EfficientWorld.h"
#include "EfficientSymbiont.h"
#include "EfficientHost.h"

/**
 * Input: The number of efficient hosts, whether the efficient hosts have random phenotypes, and the
 * interaction values for efficient hosts if competition mode is on.
 *
 * Output: None.
 *
 * Purpose: To populate the world with efficient hosts with appropriate phenotypes.
 */
void EfficientWorld::SetupHosts(long unsigned int* POP_SIZE, bool random_phen_host, double comp_host_1, double comp_host_2) {
  for (size_t i = 0; i < *POP_SIZE; i++) {
    emp::Ptr<EfficientHost> new_org;

    if (random_phen_host) {
      new_org.New(&GetRandom(), this, my_config, GetRandom().GetDouble(-1, 1));
    }
    else if (my_config->COMPETITION_MODE() && i % 2 == 0) {
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
 * Input: The number of efficient symbionts and whether the efficient symbionts have random phenotypes.
 *
 * Output: None.
 *
 * Purpose: To populate the world with efficient symbionts with appropriate phenotypes.
 */
void EfficientWorld::SetupSymbionts(int* total_syms, bool random_phen_sym) {
  for (int j = 0; j < *total_syms; j++) {
    double sym_int = 0;
    if (random_phen_sym) { sym_int = GetRandom().GetDouble(-1, 1); }
    else { sym_int = my_config->SYM_INT(); }

    emp::Ptr<EfficientSymbiont> new_sym = emp::NewPtr<EfficientSymbiont>(&GetRandom(), this, my_config, sym_int, 0, 1);
    InjectSymbiont(new_sym);
  }
}

/**
 * Input: None.
 *
 * Output: None.
 *
 * Purpose: Prepare the world for a simulation by applying the configuration settings
 * and populating the world with efficient hosts and efficient symbionts.
 */
void EfficientWorld::Setup() {
  if (my_config->EFFICIENCY_MUT_RATE() == -1) my_config->EFFICIENCY_MUT_RATE(my_config->HORIZ_MUTATION_RATE());
  SymWorld::Setup();
}
#endif
