#include "emp/math/Random.hpp"

#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorld.cc"
#include "../../../sgp_mode/SGPWorldSetup.cc"
#include "../../../sgp_mode/SGPWorldData.cc"

#include "../../../catch/catch.hpp"

using world_t = sgpmode::SGPWorld;
using cpu_state_t = sgpmode::CPUState<world_t>;
using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
using program_t = typename world_t::sgp_prog_t;
using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;

TEST_CASE("TaskValueEvent", "[sgp][events]") {
  emp::Random random(2);
  sgpmode::SymConfigSGP config;
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.EVENTS_CFG_PATH("source/test/sgp_mode_test/test-events.json");
  // Fill in any generic configs here

  world_t world(random, &config);
  world.Setup();
  // world.Resize(2, 2);

  std::cout << "Hello!" << std::endl;

  // TODO

}