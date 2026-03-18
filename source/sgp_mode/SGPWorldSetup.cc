#ifndef SGP_WORLD_SETUP_C
#define SGP_WORLD_SETUP_C

#include "SGPWorld.h"
// #include "org_type_info.h"
// #include "utils.h"
// #include "hardware/SGPHardware.h"
#include "sgpl/utility/ThreadLocalRandom.hpp"

#include "emp/datastructs/map_utils.hpp"
#include "emp/tools/string_utils.hpp"
#include "emp/math/math.hpp"

// TODO - should AssignNewIOEnv be attached to signal that triggers more broadely (e.g., on placement, etc)

// TODO - assert that sym / host has program
namespace sgpmode {

void SGPWorld::Setup() {
  // Clear all world signals
  ClearWorldSignals();

  // Reset the seed of the main sgp thread based on the config
  // TODO - should this be here? (used to be inside scheduler)
  sgpl::tlrand.Get().ResetSeed(sgp_config.SEED());

  long unsigned int POP_SIZE;
  // TODO - add pop mode?
  max_world_size = sgp_config.GRID_X() * sgp_config.GRID_Y();
  if (sgp_config.POP_SIZE() < 0) {
    POP_SIZE = max_world_size;
  } else {
    POP_SIZE = sgp_config.POP_SIZE();
  }

  setup = true;
}
}
#endif