#ifndef LYSIS_WORLD_SETUP_C
#define LYSIS_WORLD_SETUP_C

#include "LysisWorld.h"
#include "Phage.h"
#include "Bacterium.h"

/**
 * Input: The number of bacteria, whether the bacteria have random phenotypes, and the
 * interaction values for bacteria if competition mode is on.
 *
 * Output: None.
 *
 * Purpose: To populate the world with bacteria with appropriate phenotypes.
 */
void LysisWorld::SetupHosts(long unsigned int* POP_SIZE, bool random_phen_host, double comp_host_1, double comp_host_2) {
  for (size_t i = 0; i < *POP_SIZE; i++) {
    emp::Ptr<Bacterium> new_org;

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
 * Input: The number of phage and whether the phage have random phenotypes.
 *
 * Output: None.
 *
 * Purpose: To populate the world with phage with appropriate phenotypes.
 */
void LysisWorld::SetupSymbionts(int* total_syms, bool random_phen_sym) {
  bool STAGGER_STARTING_BURST_TIMERS = true;
  for (int j = 0; j < *total_syms; j++) {
    double sym_int = 0;
    if (random_phen_sym) { sym_int = GetRandom().GetDouble(-1, 1); }
    else { sym_int = my_config->SYM_INT(); }

    emp::Ptr<Phage> new_sym = emp::NewPtr<Phage>(&GetRandom(), this, my_config,
      sym_int, 0);
    if (STAGGER_STARTING_BURST_TIMERS) {
      new_sym->SetBurstTimer(GetRandom().GetInt(-5, 5));
    }
    InjectSymbiont(new_sym);
  }
}

#endif
