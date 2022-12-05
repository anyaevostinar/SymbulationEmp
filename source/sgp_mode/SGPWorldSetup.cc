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
    InjectHost(new_org);
  }
}

void SGPWorld::SetupSymbionts(unsigned long *total_syms) {
  for (size_t j = 0; j < *total_syms; j++) {
    emp::Ptr<Symbiont> new_sym = emp::NewPtr<SGPSymbiont>(
        &GetRandom(), this, my_config, my_config->SYM_INT(), 0);
    InjectSymbiont(new_sym);
  }
}

#endif
