#include "emp/math/Random.hpp"

#include "../../sgp_mode/hardware/SGPHardware.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPWorld.cc"
#include "../../sgp_mode/SGPWorldSetup.cc"
#include "../../sgp_mode/SGPWorldData.cc"
#include "../../sgp_mode/ProgramBuilder.h"

#include "../../catch/catch.hpp"

void CheckTaskProfile(
  sgpmode::SGPWorld& world,
  sgpmode::SGPWorld::sgp_hw_t& hardware,
  const emp::vector<std::string>& includes,
  const emp::vector<std::string>& excludes
) {
  for (auto& task_name : includes) {
    const size_t task_id = world.GetTaskEnv().GetTaskSet().GetID(task_name);
    REQUIRE(hardware.GetCPUState().GetTaskPerformed(task_id));
  }
  for (auto& task_name : excludes) {
    const size_t task_id = world.GetTaskEnv().GetTaskSet().GetID(task_name);
    REQUIRE(!hardware.GetCPUState().GetTaskPerformed(task_id));
  }
}

TEST_CASE("ProgramBuilder generates a programs as advertised", "[sgp]") {
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
  config.FILE_PATH("ProgramBuilder_test_output");
  config.POP_SIZE(1);
  config.START_MOI(0);

  emp::Random random(config.SEED());
  world_t world(random, &config);
  world.Setup();
  const size_t not_task_id = world.GetTaskEnv().GetTaskSet().GetID("NOT");
  const size_t nand_task_id = world.GetTaskEnv().GetTaskSet().GetID("NAND");

  // World should have one organism inside.
  REQUIRE(world.IsOccupied(0));
  auto& org = world.GetOrg(0);
  auto& sgp_host = static_cast<sgp_host_t&>(org);
  hardware_t& hw = sgp_host.GetHardware();
  REQUIRE(hw.GetCPUState().GetNumTasks() == 9);

  WHEN("creating a NOT program") {

    REQUIRE(true);
    hw.Reset();
    // Set program of organism to something else
    hw.SetProgram(
      world.GetProgramBuilder().CreateNotProgram(50)
    );
    world.AssignNewEnvIO(hw.GetCPUState());
    // Run organism's hardware for 50 steps
    hw.RunCPUStep(50);
    auto& output_buffer = hw.GetCPUState().GetOutputBuffer();

    // Before processing output buffer, program will not have been marked as
    // performing anything
      CheckTaskProfile(
        world,
        hw,
        {},
        {"NOT", "NAND", "OR_NOT","AND","OR","AND_NOT","XOR","EQU"}
      );
    REQUIRE(output_buffer.size() == 2);
    world.ProcessHostOutputBuffer(sgp_host);
    // After processing output, this program should perform NOT
    CheckTaskProfile(
      world,
      hw,
      {"NOT"},
      {"NAND", "OR_NOT","AND","OR","AND_NOT","XOR","EQU"}
    );
    REQUIRE(output_buffer.size() == 0);
  }

  WHEN("creating a NOT NAND program") {
    hw.Reset();
    // Set program of organism to something else
    hw.SetProgram(
      world.GetProgramBuilder().CreateNotNandProgram(50)
    );
    world.AssignNewEnvIO(hw.GetCPUState());

    // Run organism's hardware for 50 steps
    hw.RunCPUStep(50);
    auto& output_buffer = hw.GetCPUState().GetOutputBuffer();
    CheckTaskProfile(
      world,
      hw,
      {},
      {"NOT", "NAND", "OR_NOT","AND","OR","AND_NOT","XOR","EQU"}
    );
    REQUIRE(output_buffer.size() > 0);
    world.ProcessHostOutputBuffer(sgp_host);
    REQUIRE(output_buffer.size() == 0);
    CheckTaskProfile(
      world,
      hw,
      {"NOT", "NAND"},
      {"OR_NOT","AND","OR","AND_NOT","XOR","EQU"}
    );
  }

  // BOOKMARK
  // WHEN("creating a repro program") {

  // }

  // TODO - test each add inst function


}