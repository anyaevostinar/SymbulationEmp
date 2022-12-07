#ifndef LYSIS_WORLD_SETUP_C
#define LYSIS_WORLD_SETUP_C

#include "LysisWorld.h"
#include "Phage.h"
#include "Bacterium.h"

/**
 * Input: None.
 *
 * Output: A pointer to a new bacterium.
 *
 * Purpose: To create and get the pointer to a new bacterium.
 */
emp::Ptr<Organism> LysisWorld::GetNewHost() {
  emp::Ptr<Bacterium> new_org;
  new_org.New(&GetRandom(), this, my_config, my_config->HOST_INT());
  return new_org;
}


/**
 * Input: None.
 *
 * Output: A pointer to a new phage.
 *
 * Purpose: To create and get the pointer to a new phage.
 */
emp::Ptr<Organism> LysisWorld::GetNewSym() {
  bool STAGGER_STARTING_BURST_TIMERS = true;
  emp::Ptr<Phage> new_sym = emp::NewPtr<Phage>(&GetRandom(), this, my_config, my_config->SYM_INT(), 0);
  if (STAGGER_STARTING_BURST_TIMERS) {
    new_sym->SetBurstTimer(GetRandom().GetInt(-5, 5));
  }  
  return new_sym;
}

#endif
