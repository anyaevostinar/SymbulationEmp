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
void PGGWorld::SetupHosts(long unsigned int* POP_SIZE){
  for (size_t i = 0; i < *POP_SIZE; i++) {
    emp::Ptr<PGGHost> new_org;
    new_org.New(&GetRandom(), this, my_config, my_config->HOST_INT());
    InjectHost(new_org);
  }
}

  //sets up the world size
  world->Resize(my_config->GRID_X(), my_config->GRID_Y());

  //This loop must be outside of the host generation loop since otherwise
  //syms try to inject into mostly empty spots at first
  int total_syms = POP_SIZE * start_moi;
  for (int j = 0; j < total_syms; j++){
      double sym_int = 0;
      if (random_phen_sym) {sym_int = random.GetDouble(-1,1);}
      else {sym_int = my_config->SYM_INT();}
      
      double sym_donation = 0;

      //If the user wants the symbionts' donation rate to be randomly initialized
      if (my_config->PGG_DONATE_RANDOM())
      {
	   //If the user wants the initialization of the symbionts' donation
	   //rate to be selected using a normal distribution  
	   if (my_config->PGG_DONATE_NORMAL())
	   {   
                double mean = my_config->PGG_DONATE_NORMAL_MEAN();
		double std = my_config->PGG_DONATE_NORMAL_STD();

		sym_donation = random.GetRandNormal(mean, std);

		//If selected value is out of the initialization range
		if (sym_donation > my_config->PGG_DONATE_MAX())
		{
		     sym_donation = my_config->PGG_DONATE_MAX();
		}	
		else if (sym_donation < my_config->PGG_DONATE_MIN())
		{
		     sym_donation = my_config->PGG_DONATE_MIN();
		}	
	   }
	   else
	   {	   
                sym_donation = random.GetDouble(my_config->PGG_DONATE_MIN(), my_config->PGG_DONATE_MAX());
           }
      }	      
      else
      {
           sym_donation = my_config->PGG_DONATE();
      }	      
      emp::Ptr<PGGSymbiont> new_sym = emp::NewPtr<PGGSymbiont>(&random, world, my_config,
          sym_int,sym_donation,0);
      world->InjectSymbiont(new_sym);
  }
}

#endif
