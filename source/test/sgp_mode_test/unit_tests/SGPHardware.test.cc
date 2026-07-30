#include "emp/math/Random.hpp"

#include "../../../sgp_mode/hardware/SGPHardware.h"
#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorld.cc"
#include "../../../sgp_mode/SGPWorldSetup.cc"
#include "../../../sgp_mode/SGPWorldData.cc"
#include "../../../sgp_mode/ProgramBuilder.h"

#include "../../../catch/catch.hpp"

#include <array>
#include <string>

TEST_CASE("Test Printing Simple Instructions", "[sgp]"){
    using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
  using program_t = typename world_t::sgp_prog_t;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
  using tag_t = typename hw_spec_t::tag_t;

  sgpmode::SymConfigSGP config;
  config.CYCLES_PER_UPDATE(0);
  config.HOST_REPRO_RES(1);
  config.SEED(61);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.FILE_PATH("hardware_test_output");
  config.POP_SIZE(1);
  config.START_MOI(0);
  config.TASK_IO_UNIQUE_OUTPUT(true);

  emp::Random random(config.SEED());
  world_t world(random, &config);
  world.Setup();
  auto& prog_builder = world.GetProgramBuilder();

  auto& org = world.GetOrg(0);
  auto& sgp_host = static_cast<sgp_host_t&>(org);
  hardware_t& hw = sgp_host.GetHardware();

  std::ostringstream output;

  WHEN("Program contains Nop Instruction"){
    program_t program;
    prog_builder.AddStartAnchor(program);
    prog_builder.AddInst(program, "Nop-0", 0);
    hw.Reset();
    hw.SetProgram(program);

    hw.PrintCode(output);

    THEN("Global Anchor and Nop Instruction should be printed"){
        REQUIRE(output.str() == "AA:\n    nop-0       \n");
    }
  }
  WHEN("Program contains Increment Instruction"){
    program_t program;
    prog_builder.AddStartAnchor(program);
    prog_builder.AddInst(program, "Increment", 0);
    hw.Reset();
    hw.SetProgram(program);

    hw.PrintCode(output);

    THEN("Global Anchor and Increment Instruction should be printed"){
        REQUIRE(output.str() == "AA:\n    increment   r0\n");
    }
  }
  WHEN("Program contains Decrement Instruction"){
    program_t program;
    prog_builder.AddStartAnchor(program);
    prog_builder.AddInst(program, "Decrement", 0);
    hw.Reset();
    hw.SetProgram(program);

    hw.PrintCode(output);

    THEN("Global Anchor and Increment Instruction should be printed"){
        REQUIRE(output.str() == "AA:\n    decrement   r0\n");
    }
  }

  WHEN("Program contains Nand Instruction"){
    program_t program;
    prog_builder.AddStartAnchor(program);
    prog_builder.AddInst(program, "Nand", 0, 1, 0);
    hw.Reset();
    hw.SetProgram(program);

    hw.PrintCode(output);

    THEN("Global Anchor and Nand Instruction should be printed"){
        REQUIRE(output.str() == "AA:\n    nand        r0, r1, r0\n");
    }
  }
}

TEST_CASE("Test Printing Complex Instructions", "[sgp]"){
    using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
  using program_t = typename world_t::sgp_prog_t;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
  using tag_t = typename hw_spec_t::tag_t;

  sgpmode::SymConfigSGP config;
  config.CYCLES_PER_UPDATE(0);
  config.HOST_REPRO_RES(1);
  config.SEED(61);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.FILE_PATH("hardware_test_output");
  config.POP_SIZE(1);
  config.START_MOI(0);
  config.TASK_IO_UNIQUE_OUTPUT(true);

  emp::Random random(config.SEED());
  world_t world(random, &config);
  world.Setup();
  auto& prog_builder = world.GetProgramBuilder();

  auto& org = world.GetOrg(0);
  auto& sgp_host = static_cast<sgp_host_t&>(org);
  hardware_t& hw = sgp_host.GetHardware();

  std::ostringstream output;

  WHEN("Program contains JumpIfNEq Instruction"){
    program_t program;
    tag_t start_tag(prog_builder.GetStartTag());
    tag_t tag1("0000000000000000000000000000000000000000000000000000000000000001");

    prog_builder.AddStartAnchor(program);
    prog_builder.AddInst(program, "Nop-0", 0);
    prog_builder.AddInst(program, "Nop-0", 0);
    prog_builder.AddInst(program, "Nop-0", 0);
    prog_builder.AddInst(program, "Global Anchor", tag1);
    prog_builder.AddInst(program, "Nop-0", 0);
    prog_builder.AddInst(program, "Nop-0", 0);
    prog_builder.AddInst(program, "JumpIfNEq", 0, 1, 0, tag1);
    hw.Reset();
    hw.SetProgram(program);

    hw.PrintCode(output);

    THEN("Global Anchor and Nop Instruction should be printed"){
        REQUIRE(output.str() == "AA:\n    nop-0       \n    nop-0       \n    nop-0       \nAB:\n    nop-0       \n    nop-0       \n    jumpifneq   r0, r1, AB\n");
    }
  }

  WHEN("Program contains JumpIfEq Instruction"){
    program_t program;
    tag_t start_tag(prog_builder.GetStartTag());
    tag_t tag1("0000000000000000000000000000000000000000000000000000000000000001");

    prog_builder.AddStartAnchor(program);
    prog_builder.AddInst(program, "Nop-0", 0);
    prog_builder.AddInst(program, "Nop-0", 0);
    prog_builder.AddInst(program, "Nop-0", 0);
    prog_builder.AddInst(program, "Global Anchor", tag1);
    prog_builder.AddInst(program, "Nop-0", 0);
    prog_builder.AddInst(program, "Nop-0", 0);
    prog_builder.AddInst(program, "JumpIfEq", 0, 1, 0, tag1);
    hw.Reset();
    hw.SetProgram(program);

    hw.PrintCode(output);

    THEN("Global Anchor and Nop Instruction should be printed"){
        REQUIRE(output.str() == "AA:\n    nop-0       \n    nop-0       \n    nop-0       \nAB:\n    nop-0       \n    nop-0       \n    jumpifeq    r0, r1, AB\n");
    }
  }

  WHEN("Program contains JumpIfLess Instruction"){
    program_t program;
    tag_t start_tag(prog_builder.GetStartTag());
    tag_t tag1("0000000000000000000000000000000000000000000000000000000000000010");

    prog_builder.AddStartAnchor(program);
    prog_builder.AddInst(program, "Nop-0", 0);
    prog_builder.AddInst(program, "Nop-0", 0);
    prog_builder.AddInst(program, "Nop-0", 0);
    prog_builder.AddInst(program, "Global Anchor", tag1);
    prog_builder.AddInst(program, "Nop-0", 0);
    prog_builder.AddInst(program, "Nop-0", 0);
    prog_builder.AddInst(program, "JumpIfLess", 0, 1, 0, tag1);
    hw.Reset();
    hw.SetProgram(program);

    hw.PrintCode(output);

    THEN("Global Anchor and Nop Instruction should be printed"){
        REQUIRE(output.str() == "AA:\n    nop-0       \n    nop-0       \n    nop-0       \nAB:\n    nop-0       \n    nop-0       \n    jumpifless  r0, r1, AB\n");
    }
  }
  
}