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
  auto& registers = hardware.GetCPU().GetActiveCore().registers;
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

  // BOOKMARK
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

  // SECTION("Test Push instruction") {

  // }

  // SECTION("Test Pop instruction") {

  // }

  // SECTION("Test SwapStack instruction") {

  // }

  // SECTION("Test Swap instruction") {

  // }

  // SECTION("Test Reproduce instruction") {

  // }

  // SECTION("Test IO instruction") {

  // }

  // SECTION("Test JumpIfNEq instruction") {

  // }

  // SECTION("Test JumpIfLess instruction") {

  // }

  // SECTION("Test JumpIfEq instruction") {

  // }

  // SECTION("Test Donate instruction") {

  // }

  // SECTION("Test Steal instruction") {

  // }

  // SECTION("Test Infect instruction") {

  // }

}