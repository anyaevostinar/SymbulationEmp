#include "../../test_utils.h"

#include "../../../default_mode/SymWorld.h"
#include "../../../default_mode/WorldSetup.cc"
#include "../../../default_mode/DataNodes.h"
#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorld.cc"
#include "../../../sgp_mode/SGPWorldSetup.cc"
#include "../../../sgp_mode/SGPWorldData.cc"
#include "../../../sgp_mode/SGPW_InteractionMechanismSetup.cc"
#include "../../../sgp_mode/SGPW_TaskProfileSetup.cc"
#include "../../../sgp_mode/ProgramBuilder.h"
#include "../../../catch/catch.hpp"

using world_t = sgpmode::SGPWorld;
using cpu_state_t = sgpmode::CPUState<world_t>;
using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
using program_t = typename world_t::sgp_prog_t;
using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;
using tag_t = typename hw_spec_t::tag_t;

TEST_CASE("Test host SenseTask instruction after a rewarded task", "[sgp]"){
  sgpmode::SymConfigSGP config;
  config.CYCLES_PER_UPDATE(0);
  config.SEED(61);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.FILE_PATH("Instructions_test_output");
  config.START_MOI(0);
  config.TASK_IO_UNIQUE_OUTPUT(true);
  config.TASK_IO_BANK_SIZE(10);
  config.ENABLE_TEMP_CHANGING_ENVIRONMENT(true);
  config.TEMP_CHANGING_ENVIRONMENT_ORG_TYPE("plastic-both");
  test_utils::SetWellMixed(config, 1, 1);

  emp::Random random(config.SEED());
  world_t world(random, &config);
  world.Setup();
  auto& prog_builder = world.GetProgramBuilder();

  auto& org = world.GetOrg(0);
  auto& sgp_host = static_cast<sgp_host_t&>(org);
  hardware_t& host_hw = sgp_host.GetHardware();

  size_t not_task_id = world.GetTaskEnv().GetTaskSet().GetID("NOT");

  // setup correct reward/punishment values for update 0
  world.Update();

  WHEN("A host runs a task which is rewarded and then the SenseTask instruction") {
    program_t host_program;
    prog_builder.AddStartAnchor(host_program);
    prog_builder.AddInst(host_program, "IO", 0);
    prog_builder.AddTask_Not(host_program);
    prog_builder.AddInst(host_program, "SenseTask", 0, 1);
    host_hw.Reset();
    host_hw.SetProgram(host_program);
    world.AssignNewEnvIO(host_hw.GetCPUState());

    // NOT is currently rewarded.
    REQUIRE(world.GetTaskEnv().GetHostTaskReq(not_task_id).task_value > 0);

    // Initial register values
    host_hw.SetRegisters({3, 2, 5});

    // Run host program
    host_hw.RunCPUStep(4);

    THEN("SenseTask puts a 1 into register 1"){
      REQUIRE(host_hw.GetRegister(1) == 1);
    }
  }
}

TEST_CASE("Test host SenseTask instruction after a punished task", "[sgp]"){
  sgpmode::SymConfigSGP config;
  config.CYCLES_PER_UPDATE(0);
  config.SEED(61);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.FILE_PATH("Instructions_test_output");
  config.START_MOI(0);
  config.TASK_IO_UNIQUE_OUTPUT(true);
  config.ENABLE_TEMP_CHANGING_ENVIRONMENT(true);
  config.TEMP_CHANGING_ENVIRONMENT_ORG_TYPE("plastic-both");
  config.TASK_IO_BANK_SIZE(10);
  test_utils::SetWellMixed(config, 1, 1);

  emp::Random random(config.SEED());
  world_t world(random, &config);
  world.Setup();
  auto& prog_builder = world.GetProgramBuilder();
  auto& org = world.GetOrg(0);
  auto& sgp_host = static_cast<sgp_host_t&>(org);
  hardware_t& host_hw = sgp_host.GetHardware();

  size_t nand_task_id = world.GetTaskEnv().GetTaskSet().GetID("NAND");

  // setup correct reward/punishment values for update 0
  world.Update();

  WHEN("A host runs a task which is punished and then the SenseTask instruction") {
    program_t host_program;
    prog_builder.AddStartAnchor(host_program);
    prog_builder.AddInst(host_program, "IO", 0);
    prog_builder.AddInst(host_program, "IO", 1);
    prog_builder.AddTask_Nand(host_program);
    prog_builder.AddInst(host_program, "SenseTask", 0, 1);
    host_hw.Reset();
    host_hw.SetProgram(host_program);
    world.AssignNewEnvIO(host_hw.GetCPUState());

    // NAND is currently punished.
    REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value < 0);

    // Initial register values
    host_hw.SetRegisters({7, 12, 9});

    // Run host program
    host_hw.RunCPUStep(5);

    THEN("SenseTask puts a 0 into register 1"){
      REQUIRE(host_hw.GetRegister(1) == 0);
    }
  }
}

TEST_CASE("Test symbiont SenseTask instruction after a rewarded task", "[sgp]"){
  sgpmode::SymConfigSGP config;
  config.CYCLES_PER_UPDATE(0);
  config.SEED(61);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.FILE_PATH("Instructions_test_output");
  config.START_MOI(1);
  config.TASK_IO_UNIQUE_OUTPUT(true);
  config.ENABLE_TEMP_CHANGING_ENVIRONMENT(true);
  config.TEMP_CHANGING_ENVIRONMENT_ORG_TYPE("plastic-both");
  config.TASK_IO_BANK_SIZE(10);
  test_utils::SetWellMixed(config, 1, 1);

  emp::Random random(config.SEED());
  world_t world(random, &config);
  world.Setup();
  auto& prog_builder = world.GetProgramBuilder();
  auto& sgp_sym = static_cast<sgp_sym_t&>(*world.GetOrg(0).GetSymbionts().at(0));
  hardware_t& sym_hw = sgp_sym.GetHardware();
  size_t not_task_id = world.GetTaskEnv().GetTaskSet().GetID("NOT");

  // setup correct reward/punishment values for update 0
  world.Update();

  WHEN("A symbiont runs a task which is rewarded and then the SenseTask instruction"){
    program_t sym_program;
    prog_builder.AddStartAnchor(sym_program);
    prog_builder.AddInst(sym_program, "IO", 0);
    prog_builder.AddTask_Not(sym_program);
    prog_builder.AddInst(sym_program, "SenseTask", 0, 1);
    sym_hw.Reset();
    sym_hw.SetProgram(sym_program);
    world.AssignNewEnvIO(sym_hw.GetCPUState());

    // NOT is currently rewarded.
    REQUIRE(world.GetTaskEnv().GetHostTaskReq(not_task_id).task_value > 0);

    // Initial register values
    sym_hw.SetRegisters({3, 2, 5});

    // Run symbiont program
    sym_hw.RunCPUStep(4);

    THEN("SenseTask puts a 1 into register 1"){
      REQUIRE(sym_hw.GetRegister(1) == 1);
    }
  }
}

TEST_CASE("Test symbiont SenseTask instruction after a punished task", "[sgp]"){
  sgpmode::SymConfigSGP config;
  config.CYCLES_PER_UPDATE(0);
  config.SEED(61);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.FILE_PATH("Instructions_test_output");
  config.START_MOI(1);
  config.TASK_IO_UNIQUE_OUTPUT(true);
  config.ENABLE_TEMP_CHANGING_ENVIRONMENT(true);
  config.TEMP_CHANGING_ENVIRONMENT_ORG_TYPE("plastic-both");
  config.TASK_IO_BANK_SIZE(10);
  test_utils::SetWellMixed(config, 1, 1);

  emp::Random random(config.SEED());
  world_t world(random, &config);
  world.Setup();
  auto& prog_builder = world.GetProgramBuilder();
  auto& sgp_sym = static_cast<sgp_sym_t&>(*world.GetOrg(0).GetSymbionts().at(0));
  hardware_t& sym_hw = sgp_sym.GetHardware();
  size_t nand_task_id = world.GetTaskEnv().GetTaskSet().GetID("NAND");

  // setup correct reward/punishment values for update 0
  world.Update();

  WHEN("A symbiont runs a task which is punished and then the SenseTask instruction") {
    program_t sym_program;
    prog_builder.AddStartAnchor(sym_program);
    prog_builder.AddInst(sym_program, "IO", 0);
    prog_builder.AddInst(sym_program, "IO", 1);
    prog_builder.AddTask_Nand(sym_program);
    prog_builder.AddInst(sym_program, "SenseTask", 0, 1);
    sym_hw.Reset();
    sym_hw.SetProgram(sym_program);
    world.AssignNewEnvIO(sym_hw.GetCPUState());

    // NAND is currently punished.
    REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value < 0);

    // Initial register values
    sym_hw.SetRegisters({7, 12, 9});

    // run symbiont program
    sym_hw.RunCPUStep(5);

    THEN("SenseTask puts a 0 into register 1"){
      REQUIRE(sym_hw.GetRegister(1) == 0);
    }
  }
}
