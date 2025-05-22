#include "emp/math/Random.hpp"

#include "../../sgp_mode/hardware/SGPHardware.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPWorld.cc"
#include "../../sgp_mode/SGPWorldSetup.cc"
#include "../../sgp_mode/SGPWorldData.cc"
#include "../../sgp_mode/ProgramBuilder.h"

#include "../../catch/catch.hpp"

#include <array>

namespace inst_tests_internal {

using hw_spec_t = sgpmode::SGPHardwareSpec<
  sgpmode::Library,
  sgpmode::CPUState<sgpmode::SGPWorld>,
  sgpmode::SGPWorld
>;
constexpr size_t num_registers = hw_spec_t::num_registers;

}

bool CheckRegisterContents(
  sgpmode::SGPWorld::sgp_hw_t& hardware,
  const emp::vector<uint32_t>& req_register_values
) {
  // auto& registers = hardware.GetCPU().GetActiveCore().registers;
  emp_assert(req_register_values.size() <= registers.size());
  for (size_t reg_i = 0; reg_i < req_register_values.size(); ++reg_i) {
    // NOTE - All instructions in symbulation cast sgp-lite's float register values
    //        to uint32_t
    // if (GetRegisterAs<uint32_t>(registers, reg_i) != req_register_values[reg_i]) {
    // if ((registers[reg_i]) != req_register_values[reg_i]) {
    if (hardware.GetRegister(reg_i) != req_register_values[reg_i]) {
      return false;
    }
  }
  return true;
}

void PrintRegisterContents(
  sgpmode::SGPWorld::sgp_hw_t& hardware
) {
  auto& registers = hardware.GetCPU().GetActiveCore().registers;
  for (size_t reg_i = 0; reg_i < registers.size(); ++reg_i) {
    if (reg_i) std::cout << " ";
    std::cout << "[" << reg_i << ":" << registers[reg_i] << "," << hardware.Reg(reg_i) << "," << hardware.GetRegister(reg_i) << "]";
  }
  std::cout << std::endl;
}

TEST_CASE("Test non-interactive instructions", "[sgp]") {
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
  using program_t = typename world_t::sgp_prog_t;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
  using tag_t = typename hw_spec_t::tag_t;

  sgpmode::SymConfigSGP config;
  config.CYCLES_PER_UPDATE(0);
  config.RANDOM_ANCESTOR(false);
  config.HOST_REPRO_RES(1);
  config.SEED(61);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.FILE_PATH("Instructions_test_output");
  config.POP_SIZE(1);
  config.START_MOI(0);
  config.TASK_IO_UNIQUE_OUTPUT(true);

  emp::Random random(config.SEED());
  world_t world(random, &config);
  world.Setup();
  auto& prog_builder = world.GetProgramBuilder();

  // World should have one organism inside.
  REQUIRE(world.IsOccupied(0));
  auto& org = world.GetOrg(0);
  auto& sgp_host = static_cast<sgp_host_t&>(org);
  hardware_t& hw = sgp_host.GetHardware();
  REQUIRE(hw.GetCPUState().GetNumTasks() == 9);

  SECTION("Test Increment instruction") {
    program_t program;
    prog_builder.AddStartAnchor(program);
    prog_builder.AddInst(program, "Increment", 0);
    prog_builder.AddInst(program, "Increment", 0);
    prog_builder.AddInst(program, "Increment", 1);
    prog_builder.AddInst(program, "Increment", 7);
    hw.Reset();
    // Set program of organism to something else
    hw.SetProgram(program);
    world.AssignNewEnvIO(hw.GetCPUState());
    hw.RunCPUStep(1); // Anchor
    REQUIRE(CheckRegisterContents(hw, {0, 0, 0, 0, 0, 0, 0, 0}));
    // PrintRegisterContents(hw);
    hw.RunCPUStep(1); // Increment 0
    // PrintRegisterContents(hw);
    REQUIRE(CheckRegisterContents(hw, {1, 0, 0, 0, 0, 0, 0, 0}));
    hw.RunCPUStep(1); // Increment 0
    REQUIRE(CheckRegisterContents(hw, {2, 0, 0, 0, 0, 0, 0, 0}));
    hw.RunCPUStep(1); // Increment 1
    // PrintRegisterContents(hw);
    REQUIRE(CheckRegisterContents(hw, {2, 1, 0, 0, 0, 0, 0, 0}));
    hw.RunCPUStep(1); // Increment 7
    REQUIRE(CheckRegisterContents(hw, {2, 1, 0, 0, 0, 0, 0, 1}));
  }

  SECTION("Test Decrement instruction") {
    program_t program;
    prog_builder.AddStartAnchor(program);
    prog_builder.AddInst(program, "Decrement", 0);
    prog_builder.AddInst(program, "Decrement", 0);
    prog_builder.AddInst(program, "Decrement", 1);
    prog_builder.AddInst(program, "Decrement", 7);
    hw.Reset();
    // Set program of organism to something else
    hw.SetProgram(program);
    world.AssignNewEnvIO(hw.GetCPUState());
    hw.SetRegisters({100, 100, 100, 100, 100, 100, 100, 100});
    // PrintRegisterContents(hw);
    hw.RunCPUStep(1); // Anchor
    REQUIRE(CheckRegisterContents(hw, {100, 100, 100, 100, 100, 100, 100, 100}));
    // PrintRegisterContents(hw);
    hw.RunCPUStep(1); // Decrement 0
    // PrintRegisterContents(hw);
    REQUIRE(CheckRegisterContents(hw, {99, 100, 100, 100, 100, 100, 100, 100}));
    // PrintRegisterContents(hw);
    hw.RunCPUStep(1); // Decrement 0
    REQUIRE(CheckRegisterContents(hw, {98, 100, 100, 100, 100, 100, 100, 100}));
    hw.RunCPUStep(1); // Decrement 1
    // PrintRegisterContents(hw);
    REQUIRE(CheckRegisterContents(hw, {98, 99, 100, 100, 100, 100, 100, 100}));
    hw.RunCPUStep(1); // Decrement 7
    REQUIRE(CheckRegisterContents(hw, {98, 99, 100, 100, 100, 100, 100, 99}));
  }

  SECTION("Test ShiftLeft instruction") {
    program_t program;
    prog_builder.AddStartAnchor(program);
    prog_builder.AddInst(program, "ShiftLeft", 0);
    prog_builder.AddInst(program, "ShiftLeft", 1);
    hw.Reset();
    hw.SetProgram(program);
    // Set program of organism to something else
    world.AssignNewEnvIO(hw.GetCPUState());
    hw.SetRegisters({4, 1});
    hw.RunCPUStep(1); // Anchor
    hw.RunCPUStep(1);
    REQUIRE(CheckRegisterContents(hw, {8, 1, 0, 0, 0, 0, 0, 0}));
    hw.RunCPUStep(1);
    REQUIRE(CheckRegisterContents(hw, {8, 2, 0, 0, 0, 0, 0, 0}));
  }

  SECTION("Test ShiftRight instruction") {
    program_t program;
    prog_builder.AddStartAnchor(program);
    prog_builder.AddInst(program, "ShiftRight", 0);
    prog_builder.AddInst(program, "ShiftRight", 1);
    prog_builder.AddInst(program, "ShiftRight", 2);
    prog_builder.AddInst(program, "ShiftRight", 2);
    hw.Reset();
    hw.SetProgram(program);
    // Set program of organism to something else
    world.AssignNewEnvIO(hw.GetCPUState());
    hw.SetRegisters({4, 1, 128});
    hw.RunCPUStep(1); // Anchor
    hw.RunCPUStep(1);
    REQUIRE(CheckRegisterContents(hw, {2, 1, 128, 0, 0, 0, 0, 0}));
    hw.RunCPUStep(1);
    REQUIRE(CheckRegisterContents(hw, {2, 0, 128, 0, 0, 0, 0, 0}));
    hw.RunCPUStep(2);
    REQUIRE(CheckRegisterContents(hw, {2, 0, 32, 0, 0, 0, 0, 0}));
  }

  SECTION("Test Add instruction") {
    program_t program;
    prog_builder.AddStartAnchor(program);
    prog_builder.AddInst(program, "Add", 0, 1, 2);
    prog_builder.AddInst(program, "Add", 0, 0, 0);
    hw.Reset();
    hw.SetProgram(program);
    // Set program of organism to something else
    world.AssignNewEnvIO(hw.GetCPUState());
    hw.SetRegisters({0, 10, 15});
    hw.RunCPUStep(1); // Anchor
    hw.RunCPUStep(1);
    REQUIRE(CheckRegisterContents(hw, {25, 10, 15, 0, 0, 0, 0, 0}));
    hw.RunCPUStep(1);
    REQUIRE(CheckRegisterContents(hw, {50, 10, 15, 0, 0, 0, 0, 0}));
  }

  SECTION("Test Subtract instruction") {
    program_t program;
    prog_builder.AddStartAnchor(program);
    prog_builder.AddInst(program, "Subtract", 0, 1, 2);
    prog_builder.AddInst(program, "Subtract", 0, 0, 0);
    hw.Reset();
    hw.SetProgram(program);
    // Set program of organism to something else
    world.AssignNewEnvIO(hw.GetCPUState());
    hw.SetRegisters({0, 15, 10});
    hw.RunCPUStep(1); // Anchor
    hw.RunCPUStep(1);
    REQUIRE(CheckRegisterContents(hw, {5, 15, 10, 0, 0, 0, 0, 0}));
    hw.RunCPUStep(1);
    REQUIRE(CheckRegisterContents(hw, {0, 15, 10, 0, 0, 0, 0, 0}));
  }

  SECTION("Test Nand instruction") {
    program_t program;
    prog_builder.AddStartAnchor(program);
    prog_builder.AddInst(program, "Nand", 2, 0, 1);
    // prog_builder.AddInst(program, "Nand", 2, 0, 0);
    hw.Reset();
    hw.SetProgram(program);
    // Set program of organism to something else
    world.AssignNewEnvIO(hw.GetCPUState());
    hw.SetRegisters({7, 5, 0});
    hw.RunCPUStep(1); // Anchor
    hw.RunCPUStep(1);
    const uint32_t result = ~(7 & 5);
    REQUIRE(CheckRegisterContents(hw, {7, 5, result, 0, 0, 0, 0, 0}));
  }

  SECTION("Test Push instruction") {
    program_t program;
    prog_builder.AddStartAnchor(program);
    prog_builder.AddInst(program, "Push", 0); // Push value from register 0 onto the stack
    prog_builder.AddInst(program, "Push", 1); // Push value from register 1 onto the stack

    hw.Reset();
    hw.SetProgram(program);
    world.AssignNewEnvIO(hw.GetCPUState());

    hw.SetRegisters({10, 20, 30}); // Initial register values
    hw.RunCPUStep(1); // Anchor
    hw.RunCPUStep(1); // Push 10 onto stack
    REQUIRE(hw.GetCPUState().GetStacks().GetTop().value() == 10);
    hw.RunCPUStep(1); // Push 20 onto stack
    REQUIRE(hw.GetCPUState().GetStacks().GetTop().value() == 20);

    // After pushing, the stack should have 10 and 20 as the top two values.
    // Verify stack state (depending on how stack is represented)
    REQUIRE(CheckRegisterContents(hw, {10, 20, 30, 0, 0, 0, 0, 0}));
    auto& stacks =  hw.GetCPUState().GetStacks();
    // check the contents
    REQUIRE(stacks.GetActiveStack()[0] == 10);
    REQUIRE(stacks.GetActiveStack()[1] == 20);
  }

  SECTION("Test Pop instruction") {
    program_t program;
    prog_builder.AddStartAnchor(program);
    prog_builder.AddInst(program, "Push", 0); // Push value from register 0 onto the stack
    prog_builder.AddInst(program, "Push", 1); // Push value from register 1 onto the stack

    prog_builder.AddInst(program, "Pop", 2); // Pop value from stack into register 2
    prog_builder.AddInst(program, "Pop", 3); // Pop value from stack into register 3
    prog_builder.AddInst(program, "Pop", 4); // Pop value from stack into register 4 (stack is empty, reg[4] = 0)

    hw.Reset();
    hw.SetProgram(program);
    world.AssignNewEnvIO(hw.GetCPUState());
    hw.SetRegisters({10, 20, 30, 40, 50, 60, 70, 80});

    hw.RunCPUStep(1); // Anchor
    hw.RunCPUStep(1); // Push
    hw.RunCPUStep(1); // Push
    hw.RunCPUStep(1); // Pop
    hw.RunCPUStep(1); // Pop
    hw.RunCPUStep(1); // Pop

    REQUIRE(CheckRegisterContents(hw, {10, 20, 20, 10, 0, 60, 70, 80}));
  }

  SECTION("Test SwapStack instruction") {
    program_t program;
    prog_builder.AddStartAnchor(program);
    prog_builder.AddInst(program, "Push", 0); // Push value from register 0 onto the stack
    prog_builder.AddInst(program, "SwapStack"); // Switch to stack 1
    prog_builder.AddInst(program, "Push", 1); // Push value from register 1 onto the stack
    prog_builder.AddInst(program, "SwapStack"); // Switch to stack 0
    prog_builder.AddInst(program, "Push", 2); // Push value from register 0 onto the stack
    // Stack state: [ [reg[0], reg[2]], [reg[1]] ]
    prog_builder.AddInst(program, "Pop", 3);
    prog_builder.AddInst(program, "Pop", 4);
    prog_builder.AddInst(program, "Pop", 5);
    prog_builder.AddInst(program, "SwapStack");
    prog_builder.AddInst(program, "Pop", 6);

    hw.Reset();
    hw.SetProgram(program);
    world.AssignNewEnvIO(hw.GetCPUState());
    hw.SetRegisters({10, 20, 30, 40, 50, 60, 70, 80});

    hw.RunCPUStep(1); // Anchor
    hw.RunCPUStep(1); // Push
    hw.RunCPUStep(1); // Swap stack
    hw.RunCPUStep(1); // Push
    hw.RunCPUStep(1); // Swap stack
    hw.RunCPUStep(1); // Push
    hw.RunCPUStep(1); // Pop
    hw.RunCPUStep(1); // Pop
    hw.RunCPUStep(1); // Pop
    hw.RunCPUStep(1); // Swap stack
    hw.RunCPUStep(1); // Pop
    REQUIRE(CheckRegisterContents(hw, {10, 20, 30, 30, 10, 0, 20, 80}));
  }

  SECTION("Test Swap instruction") {
    program_t program;
    prog_builder.AddStartAnchor(program);
    prog_builder.AddInst(program, "Swap", 0, 1);
    hw.Reset();
    hw.SetProgram(program);
    world.AssignNewEnvIO(hw.GetCPUState());
    hw.SetRegisters({10, 20});
    hw.RunCPUStep(1); // Anchor
    hw.RunCPUStep(1); // Swap stack values
    // After swapping, the stack should have 20 and 10.
    REQUIRE(CheckRegisterContents(hw, {20, 10, 0, 0, 0, 0, 0, 0}));
  }

  SECTION("Test Reproduce instruction") {
    // Reproduce just raises flag on cpu state that organism wants to attempt to
    // reproduce. (the world handle's actual reproduction process)
    program_t program;
    prog_builder.AddStartAnchor(program);
    prog_builder.AddInst(program, "Reproduce");
    prog_builder.AddInst(program, "Reproduce");

    hw.Reset();
    hw.SetProgram(program);
    world.AssignNewEnvIO(hw.GetCPUState());
    hw.GetCPUState().SetLocation({0}); // Need to set valid location
    REQUIRE(hw.GetCPUState().GetLocation().IsValid());

    REQUIRE(!hw.GetCPUState().ReproInProgress());
    REQUIRE(!hw.GetCPUState().ReproAttempt());

    hw.RunCPUStep(1); // Anchor
    hw.RunCPUStep(1); // Reproduce
    REQUIRE(!hw.GetCPUState().ReproInProgress());
    REQUIRE(hw.GetCPUState().ReproAttempt());
    hw.RunCPUStep(1); // Reproduce
    REQUIRE(!hw.GetCPUState().ReproInProgress());
    REQUIRE(hw.GetCPUState().ReproAttempt());
  }

  SECTION("Test IO instruction") {
    program_t program;
    prog_builder.AddStartAnchor(program);
    prog_builder.AddInst(program, "IO", 0);
    prog_builder.AddInst(program, "IO", 1);
    prog_builder.AddInst(program, "IO", 2);

    hw.Reset();
    hw.SetProgram(program);
    world.AssignNewEnvIO(hw.GetCPUState());

    auto& inputs = hw.GetCPUState().GetInputBuffer();

    hw.SetRegisters({10, 20, 30, 40, 50, 60, 70, 80}); // Initial register values
    hw.RunCPUStep(1); // Anchor
    // Check output buffer as we go
    REQUIRE(hw.GetCPUState().GetOutputBuffer().size() == 0);
    hw.RunCPUStep(1); // Execute IO operation
    REQUIRE(hw.GetCPUState().GetOutputBuffer().size() == 1);
    REQUIRE(hw.GetCPUState().GetOutputBuffer()[0] == 10);
    hw.RunCPUStep(1);
    REQUIRE(hw.GetCPUState().GetOutputBuffer().size() == 2);
    REQUIRE(hw.GetCPUState().GetOutputBuffer()[1] == 20);
    hw.RunCPUStep(1);
    REQUIRE(hw.GetCPUState().GetOutputBuffer().size() == 3);
    REQUIRE(hw.GetCPUState().GetOutputBuffer()[2] == 30);
    // Check that inputs from input buffer were placed into registers as appropriate
    REQUIRE(CheckRegisterContents(hw, {inputs[0], inputs[1], inputs[2], 40, 50, 60, 70, 80}));
  }

  SECTION("Test JumpIfNEq instruction") {
    program_t program;
    tag_t start_tag(prog_builder.GetStartTag());
    tag_t tag1("0000000000000000000000000000000000000000000000000000000000000001");
    // tag_t tag2("0100000000000000000000000000000000000000000000000000000000000010");

    prog_builder.AddStartAnchor(program);
    prog_builder.AddInst(program, "Decrement", 2);
    prog_builder.AddInst(program, "Increment", 0);
    prog_builder.AddInst(program, "JumpIfNEq", 0, 0, 0, start_tag); // This jump should fail
    prog_builder.AddInst(program, "Increment", 0);
    prog_builder.AddInst(program, "JumpIfNEq", 0, 1, 0, tag1); // This jump should succeed
    prog_builder.AddInst(program, "Increment", 0);
    prog_builder.AddInst(program, "Increment", 0);
    prog_builder.AddInst(program, "Increment", 0);
    prog_builder.AddInst(program, "Global Anchor", tag1);
    prog_builder.AddInst(program, "Increment", 1);

    // hw.GetCPU().GetActiveCore().GetProgramCounter();
    hw.Reset();
    hw.SetProgram(program);
    world.AssignNewEnvIO(hw.GetCPUState());

    hw.SetRegisters({0, 0, 10}); // Initial register values
    hw.RunCPUStep(1); // Anchor
    hw.RunCPUStep(1); // Decrement
    hw.RunCPUStep(1); // Increment
    hw.RunCPUStep(1); // JumpIfNEq
    hw.RunCPUStep(1); // Increment
    hw.RunCPUStep(1); // JumpIfNEq
    hw.RunCPUStep(1); // Increment 1
    REQUIRE(CheckRegisterContents(hw, {2, 1, 9}));
  }

  SECTION("Test JumpIfLess instruction") {
    program_t program;
    tag_t start_tag(prog_builder.GetStartTag());
    tag_t tag1("0000000000000000000000000000000000000000000000000000000000000001");
    // tag_t tag2("0100000000000000000000000000000000000000000000000000000000000010");

    prog_builder.AddStartAnchor(program);
    prog_builder.AddInst(program, "Decrement", 2);
    prog_builder.AddInst(program, "Increment", 0);
    prog_builder.AddInst(program, "JumpIfLess", 4, 3, 0, start_tag); // This jump should fail
    prog_builder.AddInst(program, "Increment", 0);
    prog_builder.AddInst(program, "JumpIfLess", 3, 4, 0, tag1); // This jump should succeed
    prog_builder.AddInst(program, "Increment", 0);
    prog_builder.AddInst(program, "Increment", 0);
    prog_builder.AddInst(program, "Increment", 0);
    prog_builder.AddInst(program, "Global Anchor", tag1);
    prog_builder.AddInst(program, "Increment", 1);

    hw.Reset();
    hw.SetProgram(program);
    world.AssignNewEnvIO(hw.GetCPUState());

    hw.SetRegisters({0, 0, 10, 20, 30}); // Initial register values
    hw.RunCPUStep(1); // Anchor
    hw.RunCPUStep(1); // Decrement
    hw.RunCPUStep(1); // Increment
    hw.RunCPUStep(1); // JumpIfLess
    hw.RunCPUStep(1); // Increment
    hw.RunCPUStep(1); // JumpIfLess
    hw.RunCPUStep(1); // Increment 1
    REQUIRE(CheckRegisterContents(hw, {2, 1, 9}));
  }

  SECTION("Test JumpIfEq instruction") {
    program_t program;
    tag_t start_tag(prog_builder.GetStartTag());
    tag_t tag1("0000000000000000000000000000000000000000000000000000000000000001");
    // tag_t tag2("0100000000000000000000000000000000000000000000000000000000000010");

    prog_builder.AddStartAnchor(program);
    prog_builder.AddInst(program, "Decrement", 2);
    prog_builder.AddInst(program, "Increment", 0);
    prog_builder.AddInst(program, "JumpIfEq", 4, 3, 0, start_tag); // This jump should fail
    prog_builder.AddInst(program, "Increment", 0);
    prog_builder.AddInst(program, "JumpIfEq", 3, 3, 0, tag1); // This jump should succeed
    prog_builder.AddInst(program, "Increment", 0);
    prog_builder.AddInst(program, "Increment", 0);
    prog_builder.AddInst(program, "Increment", 0);
    prog_builder.AddInst(program, "Global Anchor", tag1);
    prog_builder.AddInst(program, "Increment", 1);

    hw.Reset();
    hw.SetProgram(program);
    world.AssignNewEnvIO(hw.GetCPUState());

    hw.SetRegisters({0, 0, 10, 20, 30}); // Initial register values
    hw.RunCPUStep(1); // Anchor
    hw.RunCPUStep(1); // Decrement
    hw.RunCPUStep(1); // Increment
    hw.RunCPUStep(1); // JumpIfEq
    hw.RunCPUStep(1); // Increment
    hw.RunCPUStep(1); // JumpIfEq
    hw.RunCPUStep(1); // Increment 1
    REQUIRE(CheckRegisterContents(hw, {2, 1, 9}));
  }
}


TEST_CASE("Test host-symbiont interactive instructions", "[sgp]") {
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
  using program_t = typename world_t::sgp_prog_t;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
  using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;
  using tag_t = typename hw_spec_t::tag_t;

  sgpmode::SymConfigSGP config;
  config.CYCLES_PER_UPDATE(0);
  config.RANDOM_ANCESTOR(false);
  config.HOST_REPRO_RES(1);
  config.SEED(61);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.FILE_PATH("Instructions_test_output");
  config.POP_SIZE(1);
  config.START_MOI(0);
  config.TASK_IO_UNIQUE_OUTPUT(true);
  config.SYM_DONATE_PROP(0.5);
  config.DONATE_PENALTY(0.25);

  emp::Random random(config.SEED());
  world_t world(random, &config);
  world.Setup();
  auto& prog_builder = world.GetProgramBuilder();

  // World should have one organism inside.
  REQUIRE(world.IsOccupied(0));
  auto& org = world.GetOrg(0);
  auto& sgp_host = static_cast<sgp_host_t&>(org);
  hardware_t& hw = sgp_host.GetHardware();
  REQUIRE(hw.GetCPUState().GetNumTasks() == 9);

  SECTION("Test Donate instruction") {
    program_t program;
    prog_builder.AddStartAnchor(program);
    prog_builder.AddInst(program, "Donate");
    hw.Reset();
    hw.SetProgram(program);
    world.AssignNewEnvIO(hw.GetCPUState());
    // Sym donate prop: 0.5
    // Donate penalty: 0.25
    WHEN("Host with runs donate with no symbiont") {
      // Nothing happens
      REQUIRE(sgp_host.IsHost());
      REQUIRE(!sgp_host.HasSym());
      sgp_host.SetPoints(100);
      // Run host program
      hw.RunCPUStep(1); // Anchor
      hw.RunCPUStep(1); // Donate
      REQUIRE(sgp_host.GetPoints() == 100);
    }

    WHEN("Host runs donate with symbiont") {
      // Inject symbiont
      program_t sym_program;
      prog_builder.AddStartAnchor(sym_program);
      prog_builder.AddInst(sym_program, "Donate");
      sgp_host.AddSymbiont(
        emp::NewPtr<sgp_sym_t>(&random, &world, &config, sym_program)
      );
      REQUIRE(sgp_host.HasSym());
      sgp_sym_t& sgp_sym = *static_cast<sgp_sym_t*>(sgp_host.GetSymbionts()[0].Raw());
      sgp_host.SetPoints(100);
      sgp_sym.SetPoints(100);
      // Run host program
      hw.RunCPUStep(1);
      hw.RunCPUStep(1);
      REQUIRE(sgp_host.GetPoints() == 100);
      REQUIRE(sgp_sym.GetPoints() == 100);
    }

    WHEN("Symbiont runs donate instruction with no host") {
      program_t sym_program;
      prog_builder.AddStartAnchor(sym_program);
      prog_builder.AddInst(sym_program, "Donate");
      sgp_sym_t sgp_sym(
        &random, &world, &config, sym_program
      );
      sgp_sym.SetPoints(100);
      REQUIRE(!sgp_sym.IsHost());
      REQUIRE(!sgp_sym.GetHardware().GetCPUState().HasHost());
      sgp_sym.GetHardware().RunCPUStep(1);
      sgp_sym.GetHardware().RunCPUStep(1);
      REQUIRE(sgp_sym.GetPoints() == 100);
    }

    WHEN("Symbiont runs donate instruction with a host") {
      // Inject symbiont
      program_t sym_program;
      prog_builder.AddStartAnchor(sym_program);
      prog_builder.AddInst(sym_program, "Donate");
      sgp_host.AddSymbiont(
        emp::NewPtr<sgp_sym_t>(&random, &world, &config, sym_program)
      );
      REQUIRE(sgp_host.HasSym());
      sgp_sym_t& sgp_sym = *static_cast<sgp_sym_t*>(sgp_host.GetSymbionts()[0].Raw());
      REQUIRE(!sgp_sym.IsHost());
      REQUIRE(sgp_sym.GetHardware().GetCPUState().HasHost());
      double init_host_points = 100;
      double init_sym_points = 50;
      sgp_host.SetPoints(init_host_points);
      sgp_sym.SetPoints(init_sym_points);

      sgp_sym.GetHardware().RunCPUStep(1);
      sgp_sym.GetHardware().RunCPUStep(1);

      // Symbiont donates min(sym_points, (sym_points + host_points)*donate prop )
      double to_donate = emp::Min(
        init_sym_points,
        (init_sym_points + init_host_points)*config.SYM_DONATE_PROP()
      );
      double donate_value = to_donate * (1.0 - config.DONATE_PENALTY());
      REQUIRE(sgp_sym.GetPoints() == init_sym_points - to_donate);
      REQUIRE(sgp_host.GetPoints() == init_host_points + donate_value);
    }
  }

  SECTION("Test Steal instruction") {
    program_t program;
    prog_builder.AddStartAnchor(program);
    prog_builder.AddInst(program, "Steal");
    hw.Reset();
    hw.SetProgram(program);
    world.AssignNewEnvIO(hw.GetCPUState());

    WHEN("Host with runs steal with no symbiont") {
      // Nothing happens
      REQUIRE(sgp_host.IsHost());
      REQUIRE(!sgp_host.HasSym());
      sgp_host.SetPoints(100);
      // Run host program
      hw.RunCPUStep(1); // Anchor
      hw.RunCPUStep(1); // Steal
      REQUIRE(sgp_host.GetPoints() == 100);
    }

    WHEN("Host runs steal with symbiont") {
      // Inject symbiont
      program_t sym_program;
      prog_builder.AddStartAnchor(sym_program);
      prog_builder.AddInst(sym_program, "Steal");
      sgp_host.AddSymbiont(
        emp::NewPtr<sgp_sym_t>(&random, &world, &config, sym_program)
      );
      REQUIRE(sgp_host.HasSym());
      sgp_sym_t& sgp_sym = *static_cast<sgp_sym_t*>(sgp_host.GetSymbionts()[0].Raw());
      sgp_host.SetPoints(100);
      sgp_sym.SetPoints(100);
      // Run host program
      hw.RunCPUStep(1);
      hw.RunCPUStep(1);
      REQUIRE(sgp_host.GetPoints() == 100);
      REQUIRE(sgp_sym.GetPoints() == 100);
    }

    WHEN("Symbiont runs steal instruction with no host") {
      program_t sym_program;
      prog_builder.AddStartAnchor(sym_program);
      prog_builder.AddInst(sym_program, "Steal");
      sgp_sym_t sgp_sym(
        &random, &world, &config, sym_program
      );
      sgp_sym.SetPoints(100);
      REQUIRE(!sgp_sym.IsHost());
      REQUIRE(!sgp_sym.GetHardware().GetCPUState().HasHost());
      sgp_sym.GetHardware().RunCPUStep(1);
      sgp_sym.GetHardware().RunCPUStep(1);
      // Nothing happens
      REQUIRE(sgp_sym.GetPoints() == 100);
    }

    WHEN("Symbiont runs steal instruction with a host") {
      // Inject symbiont
      program_t sym_program;
      prog_builder.AddStartAnchor(sym_program);
      prog_builder.AddInst(sym_program, "Steal");
      sgp_host.AddSymbiont(
        emp::NewPtr<sgp_sym_t>(&random, &world, &config, sym_program)
      );
      REQUIRE(sgp_host.HasSym());
      sgp_sym_t& sgp_sym = *static_cast<sgp_sym_t*>(sgp_host.GetSymbionts()[0].Raw());
      REQUIRE(!sgp_sym.IsHost());
      REQUIRE(sgp_sym.GetHardware().GetCPUState().HasHost());
      double init_host_points = 100;
      double init_sym_points = 50;
      sgp_host.SetPoints(init_host_points);
      sgp_sym.SetPoints(init_sym_points);

      sgp_sym.GetHardware().RunCPUStep(1);
      sgp_sym.GetHardware().RunCPUStep(1);

      double to_steal = emp::Min(
        init_host_points,
        (init_sym_points + init_host_points) * config.SYM_STEAL_PROP()
      );
      double steal_value = to_steal * (1.0 - config.STEAL_PENALTY());
      REQUIRE(sgp_host.GetPoints() == init_host_points - to_steal);
      REQUIRE(sgp_sym.GetPoints() == init_sym_points + steal_value);
    }
  }
}

TEST_CASE("Test freeliving symbiont instructions", "[sgp]") {
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
  using program_t = typename world_t::sgp_prog_t;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
  using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;
  using tag_t = typename hw_spec_t::tag_t;

  sgpmode::SymConfigSGP config;
  config.CYCLES_PER_UPDATE(0);
  config.RANDOM_ANCESTOR(false);
  config.HOST_REPRO_RES(1);
  config.SEED(61);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.FILE_PATH("Instructions_test_output");
  config.POP_SIZE(1);
  config.START_MOI(0);
  config.TASK_IO_UNIQUE_OUTPUT(true);
  config.SYM_DONATE_PROP(0.5);
  config.DONATE_PENALTY(0.25);
  config.FREE_LIVING_SYMS(true);
  config.SYM_LIMIT(1);
  config.PHAGE_EXCLUDE(false);

  emp::Random random(config.SEED());
  world_t world(random, &config);
  world.Setup();
  auto& prog_builder = world.GetProgramBuilder();

  // World should have one organism inside.
  REQUIRE(world.IsOccupied(0));
  auto& org = world.GetOrg(0);
  auto& sgp_host = static_cast<sgp_host_t&>(org);
  hardware_t& host_hw = sgp_host.GetHardware();
  REQUIRE(host_hw.GetCPUState().GetNumTasks() == 9);

  SECTION("Test Infect instruction") {
    program_t host_program;
    prog_builder.AddStartAnchor(host_program);
    prog_builder.AddInst(host_program, "Infect");
    host_hw.Reset();
    host_hw.SetProgram(host_program);
    world.AssignNewEnvIO(host_hw.GetCPUState());

    WHEN("Host runs the Infect instruction") {
      // Nothing happens
      REQUIRE(sgp_host.IsHost());
      REQUIRE(!sgp_host.HasSym());
      sgp_host.SetPoints(100);
      // Run host program
      host_hw.RunCPUStep(1); // Anchor
      host_hw.RunCPUStep(1); // Infect
      // NOTE - Anything else we want to check here?
      // The main purpose of this test is to make sure no asserts fail when
      //   Infect is run (nothing should happen), things work, etc.
      REQUIRE(sgp_host.GetPoints() == 100);
    }

    WHEN("Freeliving symbiont runs Infect instruction") {
      // Build and inject a freeliving symbiont into the world
      program_t sym_program;
      prog_builder.AddStartAnchor(sym_program);
      prog_builder.AddInst(sym_program, "Infect");
      world.Resize(1); // Resize sym pop to 1
      world.InjectSymbiont(
        emp::NewPtr<sgp_sym_t>(&random, &world, &config, sym_program)
      );
      // Make sure world size is 1
      REQUIRE(world.GetSymPop().size() == 1);
      REQUIRE(world.GetPop().size() == 1);
      // Check sym/host locations (world size is 1)
      REQUIRE(world.IsOccupied(0));
      REQUIRE(world.IsSymPopOccupied(0));
      REQUIRE(!sgp_host.HasSym());
      // static_cast<sgp_host_t&>(org);
      auto& sym_org = *(world.GetSymAt(0));
      sgp_sym_t& sgp_sym = static_cast<sgp_sym_t&>(sym_org);
      // InjectSymbiont doesn't automatically set cpu state location
      sgp_sym.GetHardware().GetCPUState().SetLocation(emp::WorldPosition(0, 0));
      // Run symbiont program
      sgp_sym.GetHardware().RunCPUStep(1); // Run Anchor
      sgp_sym.GetHardware().RunCPUStep(1); // Run Infect
      // And sym pop is no longer occupied
      REQUIRE(!world.IsSymPopOccupied(0));
      // Check that symbiont infects host
      REQUIRE(sgp_host.HasSym());
    }
  }

}
