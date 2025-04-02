#include "emp/math/Random.hpp"

#include "../../sgp_mode/hardware/SGPHardware.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPWorld.cc"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPWorldSetup.cc"
#include "../../sgp_mode/SGPWorldData.cc"
#include "../../sgp_mode/ProgramBuilder.h"

#include "../../catch/catch.hpp"

// TODO - add tests for parental task tracking

TEST_CASE("SGPHost Reproduce", "[sgp]") {
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
  config.FILE_PATH("SGPHost_test_output");
  config.POP_SIZE(1);
  config.START_MOI(0);
  config.TASK_IO_UNIQUE_OUTPUT(true);

  // Initialize world with one host
  emp::Random random(config.SEED());
  world_t world(random, &config);
  world.Setup();
  auto& prog_builder = world.GetProgramBuilder();
  REQUIRE(world.IsOccupied(0));

  // Grab host to use for test
  auto& org = world.GetOrg(0);
  auto& sgp_host = static_cast<sgp_host_t&>(org);
  hardware_t& hw = sgp_host.GetHardware();
  REQUIRE(hw.GetCPUState().GetNumTasks() == 9);

  // Configure host's program to include a NOT task
  hw.SetProgram(
    prog_builder.CreateNotProgram(50)
  );

  SECTION("Host offspring increases lineage reproduction count and resets parent repro state") {
    emp::Ptr<sgp_host_t> offspring_ptr = static_cast<sgp_host_t*>(sgp_host.Reproduce().Raw());
    REQUIRE(offspring_ptr->GetReproCount() == (sgp_host.GetReproCount() + 1));
    REQUIRE(!sgp_host.GetHardware().GetCPUState().ReproAttempt());
    REQUIRE(!sgp_host.GetHardware().GetCPUState().ReproInProgress());
    offspring_ptr.Delete();
  }

  // (3) Check that offspring has been configured appropriately
  SECTION("Host offspring inherits parent task profile correctly") {
    // Mark some of parent's tasks completed
    hw.GetCPUState().MarkTaskPerformed(0);
    hw.GetCPUState().MarkTaskPerformed(0);
    hw.GetCPUState().MarkTaskPerformed(2);
    hw.GetCPUState().MarkTaskPerformed(4);
    hw.GetCPUState().MarkTaskPerformed(6);
    hw.GetCPUState().MarkTaskPerformed(8);
    // Assert that MarkTaskPerformed worked as expected
    REQUIRE(hw.GetCPUState().GetTaskPerformanceCount(0) == 2);
    REQUIRE(hw.GetCPUState().GetTaskPerformanceCount(1) == 0);
    REQUIRE(hw.GetCPUState().GetTaskPerformanceCount(2) == 1);
    REQUIRE(hw.GetCPUState().GetTaskPerformed(0));
    REQUIRE(!hw.GetCPUState().GetTaskPerformed(1));
    REQUIRE(hw.GetCPUState().GetTaskPerformed(2));
    // Reproduce
    emp::Ptr<sgp_host_t> offspring_ptr = static_cast<sgp_host_t*>(sgp_host.Reproduce().Raw());
    auto& offspring_hw = offspring_ptr->GetHardware();
    REQUIRE(!offspring_hw.GetCPUState().GetParentTaskPerformed(1));
    REQUIRE(!offspring_hw.GetCPUState().GetParentTaskPerformed(3));
    REQUIRE(!offspring_hw.GetCPUState().GetParentTaskPerformed(5));
    REQUIRE(!offspring_hw.GetCPUState().GetParentTaskPerformed(7));
    REQUIRE(offspring_hw.GetCPUState().GetParentTaskPerformed(0));
    REQUIRE(offspring_hw.GetCPUState().GetParentTaskPerformed(2));
    REQUIRE(offspring_hw.GetCPUState().GetParentTaskPerformed(4));
    REQUIRE(offspring_hw.GetCPUState().GetParentTaskPerformed(6));
    REQUIRE(offspring_hw.GetCPUState().GetParentTaskPerformed(8));
    offspring_ptr.Delete();
  }

  // BOOKMARK

  // TODO


}