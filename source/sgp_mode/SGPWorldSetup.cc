#ifndef SGP_WORLD_SETUP_C
#define SGP_WORLD_SETUP_C

#include "SGPWorld.h"
#include "org_type_info.h"
#include "utils.h"
#include "sgpl/utility/ThreadLocalRandom.hpp"

#include "emp/datastructs/map_utils.hpp"
#include "emp/tools/string_utils.hpp"

namespace sgpmode {
  // TODO - implement "empty initialization" option
  //        - Particularly useful for testing
  void SGPWorld::Setup() {
    // Reset the seed of the main sgp thread based on the config
    // TODO - should this be here? (used to be inside scheduler)
    sgpl::tlrand.Get().ResetSeed(sgp_config.SEED());

    // Configure SGP organism type
    SetupOrgMode();

    // NOTE - Some of this code is repeated from base class.
    //  - Could do some reorganization to copy-paste. E.g., make functions for this,
    //     add hooks into the base setup to give more downstream flexibility.
    double start_moi = sgp_config.START_MOI();
    long unsigned int POP_SIZE;
    // TODO - add pop mode?
    if (sgp_config.POP_SIZE() == -1) {
      POP_SIZE = sgp_config.GRID_X() * sgp_config.GRID_Y();
    } else {
      POP_SIZE = sgp_config.POP_SIZE();
    }
    max_world_size = (size_t)POP_SIZE;

    // set world structure (either mixed or a grid with some dimensions)
    // and set synchronous generations to false
    if (!sgp_config.GRID()) {
      SetPopStruct_Mixed(false);
    } else {
      SetPopStruct_Grid(sgp_config.GRID_X(), sgp_config.GRID_Y(), false);
    }

    // Setup scheduler
    SetupScheduler();

    // Setup host reproduction
    repro_queue.Clear(); // Clear reproduction queue
    SetupHostReproduction();

    // -- bookmark --
    // - setuphostreproduction, doreproduction, host::reproduce, etc

  }

  void SGPWorld::SetupOrgMode() {
    // Convert cfg org type to lowercase
    std::string cfg_org_type(emp::to_lower(sgp_config.ORGANISM_TYPE()));
    // Get organism type (asserts validity)
    sgp_org_type = org_info::GetOrganismType(cfg_org_type);

    // Configure stress sym type
    // TODO - Implement SetupStressMode / etc
    std::string cfg_stress_sym_type(emp::to_lower(sgp_config.STRESS_TYPE()));
    // Get stress symbiont type (asserts validity)
    stress_sym_type = org_info::GetStressSymType(cfg_stress_sym_type);

    // TODO - configure other organism modes as appropriate
  }

  void SGPWorld::SetupScheduler() {
    // Configure scheduler w/max world size (updated in SGPWorld::Setup, and cfg thread count)
    scheduler.SetupScheduler(max_world_size, sgp_config.THREAD_COUNT());
    // Scheduler calls world's ProcessOrgAt function
  }

  void SGPWorld::SetupHostReproduction() {
    // Reset host birth signals
    before_host_do_birth_sig.Clear();
    after_host_do_birth_sig.Clear();
    // -- bookmark --
  }

  void SGPWorld::SetupSymReproduction() {/*TODO*/}
  void SGPWorld::SetupHostSymInteractions() {/*TODO*/}

  // TODO - clear host process signals
  void SGPWorld::SetupHosts(long unsigned int* POP_SIZE) {/*TODO*/}
  // TODO - clear symbiont process signals
  void SGPWorld::SetupSymbionts(long unsigned int* total_syms) {/*TODO*/}


}

#endif