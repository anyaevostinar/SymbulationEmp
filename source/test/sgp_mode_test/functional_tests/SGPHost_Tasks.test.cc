#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"
//#include "../../../sgp_mode/SGPDataNodes.h"

/**
 * This file is dedicated to interactions between Hosts and Tasks
 */

using world_t = sgpmode::SGPWorld;
using cpu_state_t = sgpmode::CPUState<world_t>;
using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
using program_t = typename world_t::sgp_prog_t;
using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;

/// SEG FAULT
TEST_CASE("Host Task Credit", "[sgp]") {
  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.GRID_X(2);
  config.GRID_Y(2);
  config.SYM_LIMIT(2);
  config.POP_SIZE(1);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");

  world_t world(random, &config);
  world.Setup();
  world.Resize(2, 2);
  auto& builder = world.GetProgramBuilder();


  emp::Ptr<sgp_host_t> NOT_host = emp::NewPtr<sgp_host_t>(&random, &world, &config, builder.CreateNotProgram(100));
  world.AddOrgAt(NOT_host, 0);
  for (int i = 0; i < 5; i++) {
    world.Update(); 
  }

  WHEN("Host should have done NOT task") {
    THEN("Host should get credit for completing NOT task") {
      REQUIRE(NOT_host->GetHardware().GetCPUState().GetTaskPerformed(1) == true);
    }
    THEN("Host should not get credit for completing any other tasks") {
        REQUIRE(NOT_host->GetHardware().GetCPUState().GetTaskPerformed(0) == false);
      for (size_t task_id = 2; task_id < world.GetTaskEnv().GetTaskSet().GetSize(); task_id++) {
        REQUIRE(NOT_host->GetHardware().GetCPUState().GetTaskPerformed(task_id) == false);
      }
    }
  }
}