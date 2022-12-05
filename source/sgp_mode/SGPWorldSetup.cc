#ifndef SGP_WORLD_SETUP_C
#define SGP_WORLD_SETUP_C

#include "../ConfigSetup.h"
#include "SGPHost.h"
#include "SGPSymbiont.h"
#include "SGPWorld.h"

void SGPWorld::SetupHosts(unsigned long *POP_SIZE) {
  for (size_t i = 0; i < *POP_SIZE; i++) {
    emp::Ptr<SGPHost> new_org = emp::NewPtr<SGPHost>(
        &GetRandom(), this, my_config, my_config->HOST_INT());
    if(my_config->START_MOI()==1){
      emp::Ptr<SGPSymbiont> new_sym = emp::NewPtr<SGPSymbiont>(
          &GetRandom(), this, my_config, my_config->SYM_INT(), 0);
      new_org->AddSymbiont(new_sym);
    }
    InjectHost(new_org);
  }
}

void SGPWorld::SetupSymbionts(unsigned long *total_syms) {

}

#endif
