#include "emp/math/Random.hpp"

#include "../../../sgp_mode/hardware/SGPHardware.h"
#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorld.cc"
#include "../../../sgp_mode/SGPWorldSetup.cc"
//include "../../../sgp_mode/SGPWorldData.cc"

#include "../../../catch/catch.hpp"

using world_t = sgpmode::SGPWorld;
using cpu_state_t = sgpmode::CPUState<world_t>;
using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
using program_t = typename world_t::sgp_prog_t;
using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;

TEST_CASE("Ancestor hardware can attempt reproduction and do NOT", "[sgp]") {


  sgpmode::SymConfigSGP config;
  config.CYCLES_PER_UPDATE(0);
  config.HOST_REPRO_RES(1);
  config.SEED(61);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.FILE_PATH("SGPHardware_test_output");

  emp::Random random(config.SEED());

  WHEN("logic tasks are used") {
    config.POP_SIZE(1);  // Initialize 1 host.
    config.START_MOI(0); // No symbionts
    config.CYCLES_PER_UPDATE(1);
    
    world_t world(random, &config);
    world.Setup();

    // World should have one organism inside.
    REQUIRE(world.GetNumOrgs() == 1);
    auto& org = world.GetOrg(0);
    auto& sgp_host = static_cast<sgp_host_t&>(org);
    
    hardware_t& hw = sgp_host.GetHardware();
    
    REQUIRE(sgp_host.GetReproCount() == 0);
    REQUIRE(!sgp_host.GetHardware().GetCPUState().ReproAttempt());
    REQUIRE(!sgp_host.GetHardware().GetCPUState().ReproInProgress());
    REQUIRE(hw.GetCPUState().GetLocation().IsValid());

    // Run organism's hardware for 200 steps
    hw.RunCPUStep(200);
    // After 200 updates, the hardware should have flagged a repro attempt.
    REQUIRE(hw.GetCPUState().ReproAttempt());
    REQUIRE(!hw.GetCPUState().ReproInProgress());
    auto& output_buffer = hw.GetCPUState().GetOutputBuffer();

    REQUIRE(sgp_host.GetPoints() == 0);
    sgp_host.ProcessOutputBuffer();
    // Hardware should also have completed NOT task seven times, which is also checking that they 
    // are able to fully cycle through all possible inputs (they don't do the last one because they need to go back to their first IO to get it)
    const size_t not_task_id = world.GetTaskEnv().GetTaskSet().GetID("NOT");
    REQUIRE(hw.GetCPUState().GetTaskPerformed(not_task_id));
    REQUIRE(output_buffer.size() == 0);
    REQUIRE(sgp_host.GetPoints() == 5 * 7); // should have done NOT 7 times, 5 points each

    // Reset should clear CPU state, etc
    hw.Reset();
    REQUIRE(!hw.GetCPUState().GetTaskPerformed(not_task_id));
  }


}