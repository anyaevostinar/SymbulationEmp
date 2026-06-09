#ifndef WORLD_SETUP_C
#define WORLD_SETUP_C

#include "SymWorld.h"
#include "Host.h"
#include "Symbiont.h"

/**
 * Input: The number of hosts.
 *
 * Output: None.
 *
 * Purpose: To populate the world with hosts with appropriate phenotypes. 
 */
void SymWorld::SetupHosts(long unsigned int* POP_SIZE){ 
  for (size_t i = 0; i < *POP_SIZE; i++) {
    emp::Ptr<Host> new_org;
    new_org.New(&GetRandom(), this, my_config, my_config->HOST_INT());
    if (my_config->TAG_MATCHING()) {
      emp::BitSet<TAG_LENGTH> new_tag = emp::BitSet<TAG_LENGTH>(GetRandom(), my_config->STARTING_TAGS_ONE_PROB());
      new_org->SetTag(new_tag);
    }
    InjectHost(new_org);
  }

}


/**
 * Input: The number of symbionts.
 *
 * Output: None.
 *
 * Purpose: To populate the world with symbionts with appropriate phenotypes.
 */
void SymWorld::SetupSymbionts(long unsigned int *total_syms) {
  for (size_t j = 0; j < *total_syms; j++) {
    emp::Ptr<Symbiont> new_sym = emp::NewPtr<Symbiont>(&GetRandom(), this, my_config, my_config->SYM_INT(), 0);
    if (my_config->TAG_MATCHING()) {
      emp::BitSet<TAG_LENGTH> new_tag = emp::BitSet<TAG_LENGTH>(GetRandom(), my_config->STARTING_TAGS_ONE_PROB());
      new_sym->SetTag(new_tag); // if this sym is hosted, this tag will be overwritten upon injection
    }
    InjectSymbiont(new_sym);
  }
}

/**
 * Input: None.
 *
 * Output: None.
 *
 * Purpose: Prepare the world for an experiment by applying the configuration settings 
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

  // set world structure (either mixed or a grid with some dimensions) and set synchronous generations to false
  if (my_config->GRID() == 0) {SetPopStruct_Mixed(false);}
  else SetPopStruct_Grid(my_config->GRID_X(), my_config->GRID_Y(), false);

  SetupHosts(&POP_SIZE);

  Resize(my_config->GRID_X(), my_config->GRID_Y());
  long unsigned int total_syms = POP_SIZE * start_moi;
  SetupSymbionts(&total_syms);
}
#endif
