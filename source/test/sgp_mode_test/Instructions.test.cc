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

TEST_CASE("Test instructions", "[sgp]") {
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

//     SECTION("Test Donate instruction") {
//       program_t program;
//       prog_builder.AddStartAnchor(program);
//       prog_builder.AddInst(program, "Donate", 0, 1); // Donate from register 0 to register 1
//       hw.Reset();
//       hw.SetProgram(program);
//       world.AssignNewEnvIO(hw.GetCPUState());

//       hw.SetRegisters({10, 20, 30, 40, 50, 60, 70, 80}); // Initial register values
//       hw.RunCPUStep(1); // Anchor
//       hw.RunCPUStep(1); // Donate from register 0 to register 1

//         // Verify donation occurred (registers updated as expected)
//       REQUIRE(CheckRegisterContents(hw, {10, 20, 30, 40, 50, 60, 70, 80}));
//     }

//     SECTION("Test Steal instruction") {
//       program_t program;
//       prog_builder.AddStartAnchor(program);
//       prog_builder.AddInst(program, "Steal", 0, 1); // Steal from register 1 into register 0
//       hw.Reset();
//       hw.SetProgram(program);
//       world.AssignNewEnvIO(hw.GetCPUState());

//       hw.SetRegisters({10, 20, 30, 40, 50, 60, 70, 80}); // Initial register values
//       hw.RunCPUStep(1); // Anchor
//       hw.RunCPUStep(1); // Steal from register 1 to register 0

//         // Verify steal occurred (registers updated as expected)
//       REQUIRE(CheckRegisterContents(hw, {20, 20, 30, 40, 50, 60, 70, 80}));
//     }

//     SECTION("Test Infect instruction") {
//       program_t program;
//       prog_builder.AddStartAnchor(program);
//       prog_builder.AddInst(program, "Infect", 0, 1); // Infect from register 0 to register 1
//       hw.Reset();
//       hw.SetProgram(program);
//       world.AssignNewEnvIO(hw.GetCPUState());

//       hw.SetRegisters({10, 20, 30, 40, 50, 60, 70, 80}); // Initial register values
//       hw.RunCPUStep(1); // Anchor
//       hw.RunCPUStep(1); // Infect from register 0 to register 1

//         // Verify infection occurred (registers updated as expected)
//       REQUIRE(CheckRegisterContents(hw, {10, 10, 30, 40, 50, 60, 70, 80}));
//     }
}

