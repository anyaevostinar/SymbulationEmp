#ifndef PGG_WORLD_SETUP_C
#define PGG_WORLD_SETUP_C

#include "PGGWorld.h"
#include "PGGHost.h"
#include "PGGSymbiont.h"

/**
 * Input: None.
 *
 * Output: A pointer to a new PGG host.
 *
 * Purpose: To create and get the pointer to a new PGG host.
 */
emp::Ptr<Organism> PGGWorld::GetNewHost() {
  emp::Ptr<PGGHost> new_org;
  new_org.New(&GetRandom(), this, my_config, my_config->HOST_INT());
  return new_org;
}


/**
 * Input: None.
 *
 * Output: A pointer to a new PGG symbiont.
 *
 * Purpose: To create and get the pointer to a new PGG symbiont.
 */
emp::Ptr<Organism> PGGWorld::GetNewSym() {
  double sym_donation = 0;

  //If the user wants the symbionts' donation rate to be randomly initialized
  if (my_config->PGG_DONATE_RANDOM()) {
	//If the user wants the initialization of the symbionts' donation
	//rate to be selected using a normal distribution  
	if (my_config->PGG_DONATE_NORMAL()) {
	  double mean = my_config->PGG_DONATE_NORMAL_MEAN();
	  double std = my_config->PGG_DONATE_NORMAL_STD();

	  sym_donation = random_ptr->GetRandNormal(mean, std);

	  //If selected value is out of the initialization range
	  if (sym_donation > my_config->PGG_DONATE_MAX()) {
		sym_donation = my_config->PGG_DONATE_MAX();
	  }
	  else if (sym_donation < my_config->PGG_DONATE_MIN()) {
		sym_donation = my_config->PGG_DONATE_MIN();
	  }
	}
	else {
	  sym_donation = random_ptr->GetDouble(my_config->PGG_DONATE_MIN(), my_config->PGG_DONATE_MAX());
	}
  }
  else {
	sym_donation = my_config->PGG_DONATE();
  }
  emp::Ptr<PGGSymbiont> new_sym = emp::NewPtr<PGGSymbiont>(&GetRandom(), this, my_config, my_config->SYM_INT(), sym_donation, 0);
  return new_sym;
}

#endif
