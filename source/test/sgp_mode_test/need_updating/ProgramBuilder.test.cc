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
  config.TASK_IO_UNIQUE_OUTPUT(true);

  emp::Random random(config.SEED());
  world_t world(random, &config);
  world.Setup();

  // World should have one organism inside.
  REQUIRE(world.IsOccupied(0));
  auto& org = world.GetOrg(0);
  auto& sgp_host = static_cast<sgp_host_t&>(org);
  hardware_t& hw = sgp_host.GetHardware();
  REQUIRE(hw.GetCPUState().GetNumTasks() == 9);

  WHEN("creating a NOT program") {
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
        {"NOT","NAND","OR_NOT","AND","OR","AND_NOT","NOR","XOR","EQU"}
      );
    REQUIRE(output_buffer.size() == 2);
    world.ProcessHostOutputBuffer(sgp_host);
    // After processing output, this program should perform NOT
    CheckTaskProfile(
      world,
      hw,
      {"NOT"},
      {"NAND","OR_NOT","AND","OR","AND_NOT","NOR","XOR","EQU"}
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
      {"NOT","NAND","OR_NOT","AND","OR","AND_NOT","NOR","XOR","EQU"}
    );
    REQUIRE(output_buffer.size() > 0);
    world.ProcessHostOutputBuffer(sgp_host);
    REQUIRE(output_buffer.size() == 0);
    CheckTaskProfile(
      world,
      hw,
      {"NOT", "NAND"},
      {"OR_NOT","AND","OR","AND_NOT","XOR","NOR","EQU"}
    );
  }

  WHEN("creating a repro program") {
    hw.Reset();
    // Set program of organism to something else
    hw.SetProgram(
      world.GetProgramBuilder().CreateReproProgram(50)
    );
    world.AssignNewEnvIO(hw.GetCPUState());

    // Run organism's hardware for 50 steps
    hw.RunCPUStep(50);
    auto& output_buffer = hw.GetCPUState().GetOutputBuffer();
    CheckTaskProfile(
      world,
      hw,
      {},
      {"NOT","NAND","OR_NOT","AND","OR","AND_NOT","NOR","XOR","EQU"}
    );
    // There should be nothing in the output buffer, and no tasks performed.
    REQUIRE(output_buffer.size() == 0);
    world.ProcessHostOutputBuffer(sgp_host);
    REQUIRE(output_buffer.size() == 0);
    CheckTaskProfile(
      world,
      hw,
      {},
      {"NOT","NAND","OR_NOT","AND","OR","AND_NOT","NOR","XOR","EQU"}
    );
  }

  WHEN("creating a random program") {
    // We should be able to create random programs that run
    const size_t num_rand_progs = 20;
    // Generate N random programs, run each to make sure things work
    for (size_t i = 0; i < num_rand_progs; ++i) {
      hw.Reset();
      // Set program of organism to something else
      hw.SetProgram(
        world.GetProgramBuilder().CreateRandomProgram(50)
      );
      REQUIRE(hw.GetProgram().size() == 50);
      world.AssignNewEnvIO(hw.GetCPUState());
      // Run organism's hardware for 50 steps
      hw.RunCPUStep(50);
      // We don't know anything about the output buffer here.
      // Just want to make sure we don't get any errors.
    }
  }

  // TODO - test each add inst function
  // TODO - test rectifier?
  WHEN("creating an AND program") {
    auto& prog_builder = world.GetProgramBuilder();
    // Build an AND program
    program_t program;
    prog_builder.AddStartAnchor(program);
    prog_builder.AddTask_And(program);
    hw.Reset();
    // Set program of organism to something else
    hw.SetProgram(program);
    world.AssignNewEnvIO(hw.GetCPUState());

    // Run organism's hardware
    hw.RunCPUStep(program.size());
    auto& output_buffer = hw.GetCPUState().GetOutputBuffer();
    CheckTaskProfile(
      world,
      hw,
      {},
      {"NOT","NAND","OR_NOT","AND","OR","AND_NOT","NOR","XOR","EQU"}
    );
    // There should be nothing in the output buffer, and no tasks performed.
    world.ProcessHostOutputBuffer(sgp_host);
    REQUIRE(output_buffer.size() == 0);
    CheckTaskProfile(
      world,
      hw,
      {"AND"},
      {"NOT","NAND","OR_NOT","OR","AND_NOT","NOR","XOR","EQU"}
    );
  }

  WHEN("creating an OrNot program") {
    auto& prog_builder = world.GetProgramBuilder();
    // Build an OrNot program
    program_t program;
    prog_builder.AddStartAnchor(program);
    prog_builder.AddTask_OrNot(program);
    hw.Reset();
    // Set program of organism to something else
    hw.SetProgram(program);
    world.AssignNewEnvIO(hw.GetCPUState());

    // Run organism's hardware
    hw.RunCPUStep(program.size());
    auto& output_buffer = hw.GetCPUState().GetOutputBuffer();
    CheckTaskProfile(
      world,
      hw,
      {},
      {"NOT","NAND","OR_NOT","AND","OR","AND_NOT","NOR","XOR","EQU"}
    );
    // There should be nothing in the output buffer, and no tasks performed.
    world.ProcessHostOutputBuffer(sgp_host);
    REQUIRE(output_buffer.size() == 0);
    CheckTaskProfile(
      world,
      hw,
      {"OR_NOT"},
      {"NOT","NAND","AND","OR","AND_NOT","NOR","XOR","EQU"}
    );
  }

  WHEN("creating an Or program") {
    auto& prog_builder = world.GetProgramBuilder();
    // Build an OR program
    program_t program;
    prog_builder.AddStartAnchor(program);
    prog_builder.AddTask_Or(program);
    hw.Reset();
    // Set program of organism to something else
    hw.SetProgram(program);
    world.AssignNewEnvIO(hw.GetCPUState());

    // Run organism's hardware
    hw.RunCPUStep(program.size());
    auto& output_buffer = hw.GetCPUState().GetOutputBuffer();
    CheckTaskProfile(
      world,
      hw,
      {},
      {"NOT","NAND","OR_NOT","AND","OR","AND_NOT","NOR","XOR","EQU"}
    );
    // There should be nothing in the output buffer, and no tasks performed.
    world.ProcessHostOutputBuffer(sgp_host);
    REQUIRE(output_buffer.size() == 0);
    CheckTaskProfile(
      world,
      hw,
      {"OR"},
      {"NOT","NAND","AND","OR_NOT","AND_NOT","NOR","XOR","EQU"}
    );
  }

  WHEN("creating an AndNot program") {
    auto& prog_builder = world.GetProgramBuilder();
    // Build an AndNot program
    program_t program;
    prog_builder.AddStartAnchor(program);
    prog_builder.AddTask_AndNot(program);
    hw.Reset();
    // Set program of organism to something else
    hw.SetProgram(program);
    world.AssignNewEnvIO(hw.GetCPUState());

    // Run organism's hardware
    hw.RunCPUStep(program.size());
    auto& output_buffer = hw.GetCPUState().GetOutputBuffer();
    CheckTaskProfile(
      world,
      hw,
      {},
      {"NOT","NAND","OR_NOT","AND","OR","AND_NOT","NOR","XOR","EQU"}
    );
    // There should be nothing in the output buffer, and no tasks performed.
    world.ProcessHostOutputBuffer(sgp_host);
    REQUIRE(output_buffer.size() == 0);
    CheckTaskProfile(
      world,
      hw,
      {"AND_NOT"},
      {"NOT","NAND","AND","OR_NOT","OR","NOR","XOR","EQU"}
    );
  }

  WHEN("creating a Nor program") {
    auto& prog_builder = world.GetProgramBuilder();
    // Build an Nor program
    program_t program;
    prog_builder.AddStartAnchor(program);
    prog_builder.AddTask_Nor(program);
    hw.Reset();
    // Set program of organism to something else
    hw.SetProgram(program);
    world.AssignNewEnvIO(hw.GetCPUState());

    // Run organism's hardware
    hw.RunCPUStep(program.size());
    auto& output_buffer = hw.GetCPUState().GetOutputBuffer();
    CheckTaskProfile(
      world,
      hw,
      {},
      {"NOT","NAND","OR_NOT","AND","OR","AND_NOT","NOR","XOR","EQU"}
    );
    // There should be nothing in the output buffer, and no tasks performed.
    world.ProcessHostOutputBuffer(sgp_host);
    REQUIRE(output_buffer.size() == 0);
    CheckTaskProfile(
      world,
      hw,
      {"NOR"},
      {"NOT","NAND","AND","OR_NOT","OR","AND_NOT","XOR","EQU"}
    );
  }

  WHEN("creating a Xor program") {
    auto& prog_builder = world.GetProgramBuilder();
    // Build an Xor program
    program_t program;
    prog_builder.AddStartAnchor(program);
    prog_builder.AddTask_Xor(program);
    hw.Reset();
    // Set program of organism to something else
    hw.SetProgram(program);
    world.AssignNewEnvIO(hw.GetCPUState());

    // Run organism's hardware
    hw.RunCPUStep(program.size());
    auto& output_buffer = hw.GetCPUState().GetOutputBuffer();
    CheckTaskProfile(
      world,
      hw,
      {},
      {"NOT","NAND","OR_NOT","AND","OR","AND_NOT","NOR","XOR","EQU"}
    );
    // There should be nothing in the output buffer, and no tasks performed.
    world.ProcessHostOutputBuffer(sgp_host);
    REQUIRE(output_buffer.size() == 0);
    CheckTaskProfile(
      world,
      hw,
      {"XOR"},
      {"NOT","NAND","AND","OR_NOT","OR","AND_NOT","NOR","EQU"}
    );
  }

  WHEN("creating an Equ program") {
    auto& prog_builder = world.GetProgramBuilder();
    // Build an Xor program
    program_t program;
    prog_builder.AddStartAnchor(program);
    prog_builder.AddTask_Equ(program);
    hw.Reset();
    // Set program of organism to something else
    hw.SetProgram(program);
    world.AssignNewEnvIO(hw.GetCPUState());

    // Run organism's hardware
    hw.RunCPUStep(program.size());
    auto& output_buffer = hw.GetCPUState().GetOutputBuffer();
    CheckTaskProfile(
      world,
      hw,
      {},
      {"NOT","NAND","OR_NOT","AND","OR","AND_NOT","NOR","XOR","EQU"}
    );
    // There should be nothing in the output buffer, and no tasks performed.
    world.ProcessHostOutputBuffer(sgp_host);
    REQUIRE(output_buffer.size() == 0);
    CheckTaskProfile(
      world,
      hw,
      {"EQU"},
      {"NOT","NAND","AND","OR_NOT","OR","AND_NOT","NOR","XOR"}
    );
  }

  // TODO - test adding disabled instructions => Make sure they fail to be added
}