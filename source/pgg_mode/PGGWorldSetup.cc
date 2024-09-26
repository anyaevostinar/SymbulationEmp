#ifndef PGG_WORLD_SETUP_C
#define PGG_WORLD_SETUP_C

#include "PGGWorld.h"
#include "PGGHost.h"
#include "PGGSymbiont.h"
/**
 * Input: The number of PGG hosts.
 *
 * Output: None.
 *
 * Purpose: To populate the world with PGG hosts with appropriate phenotypes.
 */
void PGGWorld::SetupHosts(long unsigned int* POP_SIZE){
  for (size_t i = 0; i < *POP_SIZE; i++) {
    emp::Ptr<PGGHost> new_org;
    new_org.New(&GetRandom(), this, pgg_config, pgg_config->HOST_INT());
    InjectHost(new_org);
  }
}


/**
 * Input: The number of PGG symbionts.
 *
 * Output: None.
 *
 * Purpose: To populate the world with PGG symbionts with appropriate phenotypes.
 */
void PGGWorld::SetupSymbionts(long unsigned int* total_syms) {
  for (size_t j = 0; j < *total_syms; j++) {
	double sym_donation = 0;

	//If the user wants the symbionts' donation rate to be randomly initialized
	if (pgg_config->PGG_DONATE_RANDOM()) {
	  //If the user wants the initialization of the symbionts' donation
	  //rate to be selected using a normal distribution  
	  if (pgg_config->PGG_DONATE_NORMAL()) {
		double mean = pgg_config->PGG_DONATE_NORMAL_MEAN();
		double std = pgg_config->PGG_DONATE_NORMAL_STD();

		sym_donation = random_ptr->GetRandNormal(mean, std);

		//If selected value is out of the initialization range
		if (sym_donation > pgg_config->PGG_DONATE_MAX()) {
		  sym_donation = pgg_config->PGG_DONATE_MAX();
		}
		else if (sym_donation < pgg_config->PGG_DONATE_MIN()) {
		  sym_donation = pgg_config->PGG_DONATE_MIN();
		}
	  }
	  else {
		sym_donation = random_ptr->GetDouble(pgg_config->PGG_DONATE_MIN(), pgg_config->PGG_DONATE_MAX());
	  }
	}
	else {
	  sym_donation = pgg_config->PGG_DONATE();
	}
	emp::Ptr<PGGSymbiont> new_sym = emp::NewPtr<PGGSymbiont>(&GetRandom(), this, pgg_config, pgg_config->SYM_INT(), sym_donation, 0);
	InjectSymbiont(new_sym);
  }
}

#endif
