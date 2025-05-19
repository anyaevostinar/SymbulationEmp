#include "emp/math/Random.hpp"

#include "../../sgp_mode/hardware/SGPHardware.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPWorld.cc"
#include "../../sgp_mode/SGPWorldSetup.cc"
#include "../../sgp_mode/SGPWorldData.cc"

#include "../../catch/catch.hpp"

TEST_CASE("Ancestor hardware can attempt reproduction", "[sgp]") {
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
  using program_t = typename world_t::sgp_prog_t;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;

  sgpmode::SymConfigSGP config;
  config.CYCLES_PER_UPDATE(0);
  config.RANDOM_ANCESTOR(false);
  config.HOST_REPRO_RES(1);
  config.SEED(61);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.FILE_PATH("SGPHardware_test_output");

  emp::Random random(config.SEED());

  WHEN("logic tasks are used") {
    config.POP_SIZE(1);  // Initialize 1 host.
    config.START_MOI(0); // No symbionts
    world_t world(random, &config);
    world.Setup();
    // World should have one organism inside.
    REQUIRE(world.IsOccupied(0));
    auto& org = world.GetOrg(0);
    auto& sgp_host = static_cast<sgp_host_t&>(org);
    hardware_t& hw = sgp_host.GetHardware();
    REQUIRE(sgp_host.GetReproCount() == 0);
    REQUIRE(!sgp_host.GetHardware().GetCPUState().ReproAttempt());
    REQUIRE(!sgp_host.GetHardware().GetCPUState().ReproInProgress());
    REQUIRE(hw.GetCPUState().GetLocation().IsValid());
    // Run organism's hardware for 100 steps
    hw.RunCPUStep(100);
    // After 100 updates, the hardware should have flagged a repro attempt.
    REQUIRE(hw.GetCPUState().ReproAttempt());
    REQUIRE(!hw.GetCPUState().ReproInProgress());
    auto& output_buffer = hw.GetCPUState().GetOutputBuffer();
    // Initial program calls IO twice, so there will be 2 things in the output buffer
    REQUIRE(output_buffer.size() == 2);
    world.ProcessHostOutputBuffer(sgp_host);
    // Hardware should also have completed a NOT task
    const size_t not_task_id = world.GetTaskEnv().GetTaskSet().GetID("NOT");
    REQUIRE(hw.GetCPUState().GetTaskPerformed(not_task_id));
    REQUIRE(output_buffer.size() == 0);

    // Reset should clear CPU state, etc
    hw.Reset();
    REQUIRE(!hw.GetCPUState().GetTaskPerformed(not_task_id));
  }


}
