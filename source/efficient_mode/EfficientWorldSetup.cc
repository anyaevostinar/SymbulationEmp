#ifndef EFFWORLD_SETUP_C
#define EFFWORLD_SETUP_C

#include "EfficientWorld.h"
#include "EfficientSymbiont.h"
#include "EfficientHost.h"

/**
 * Input: None.
 *
 * Output: A pointer to a new efficient host.
 *
 * Purpose: To create and get the pointer to a new efficient host.
 */
emp::Ptr<Organism> EfficientWorld::GetNewHost() {
  emp::Ptr<EfficientHost> new_org;
  new_org.New(&GetRandom(), this, my_config, my_config->HOST_INT());
  return new_org;
}

/**
 * Input: None.
 *
 * Output: A pointer to a new efficient symbiont.
 *
 * Purpose: To create and get the pointer to a new efficient symbiont.
 */
emp::Ptr<Organism> EfficientWorld::GetNewSym() {
  emp::Ptr<EfficientSymbiont> new_sym = emp::NewPtr<EfficientSymbiont>(&GetRandom(), this, my_config, my_config->SYM_INT(), 0, 1);
  return new_sym;
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
