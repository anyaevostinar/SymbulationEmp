#ifndef LYSIS_WORLD_SETUP_C
#define LYSIS_WORLD_SETUP_C

#include "LysisWorld.h"
#include "Phage.h"
#include "Bacterium.h"

/**
 * Input: The number of bacteria.
 *
 * Output: None.
 *
 * Purpose: To populate the world with bacteria with appropriate phenotypes.
 */
void LysisWorld::SetupHosts(long unsigned int* POP_SIZE) {
  for (size_t i = 0; i < *POP_SIZE; i++) {
    emp::Ptr<Bacterium> new_org;
    new_org.New(&GetRandom(), this, my_config, my_config->HOST_INT());
    InjectHost(new_org);
  }
}


/**
 * Input: The number of phage.
 *
 * Output: None.
 *
 * Purpose: To populate the world with phage with appropriate phenotypes.
 */
void LysisWorld::SetupSymbionts(long unsigned int* total_syms) {
  bool STAGGER_STARTING_BURST_TIMERS = true;
  for (size_t j = 0; j < *total_syms; j++) {
    emp::Ptr<Phage> new_sym = emp::NewPtr<Phage>(&GetRandom(), this, my_config, my_config->SYM_INT(), 0);
    if (STAGGER_STARTING_BURST_TIMERS) {
      new_sym->SetBurstTimer(GetRandom().GetInt(-5, 5));
    }
    InjectSymbiont(new_sym);
  }
}

#endif
