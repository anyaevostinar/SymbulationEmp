#include "emp/math/Random.hpp"

#include "../../sgp_mode/hardware/SGPHardware.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPWorld.cc"
#include "../../sgp_mode/SGPWorldSetup.cc"
#include "../../sgp_mode/SGPWorldData.cc"
#include "../../sgp_mode/ProgramBuilder.h"

#include "../../catch/catch.hpp"

bool CheckRegisterContents(
  sgpmode::SGPWorld::sgp_hw_t& hardware,
  const emp::vector<uint32_t>& req_register_values
) {
  auto& registers = hardware.GetCPU().GetActiveCore().registers;
  emp_assert(req_register_values.size() <= registers.size());
  for (size_t reg_i = 0; reg_i < req_register_values.size(); ++reg_i) {
    // NOTE - All instructions in symbulation cast sgp-lite's float register values
    //        to uint32_t
    if ((uint32_t)(registers[reg_i]) != req_register_values[reg_i]) return false;
  }
  return true;
}

void PrintRegisterContents(
  sgpmode::SGPWorld::sgp_hw_t& hardware
) {
  auto& registers = hardware.GetCPU().GetActiveCore().registers;
  for (size_t reg_i = 0; reg_i < registers.size(); ++reg_i) {
    if (reg_i) std::cout << " ";
    std::cout << "[" << reg_i << ":" << registers[reg_i] << "," << (uint32_t)(registers[reg_i]) << "]";
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

  // World should have one organism inside.
  REQUIRE(world.IsOccupied(0));
  auto& org = world.GetOrg(0);
  auto& sgp_host = static_cast<sgp_host_t&>(org);
  hardware_t& hw = sgp_host.GetHardware();
  REQUIRE(hw.GetCPUState().GetNumTasks() == 9);

  SECTION("Test Increment instruction") {
    auto& prog_builder = world.GetProgramBuilder();
    // Build an OR program
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
    REQUIRE(CheckRegisterContents(hw, {0, 0, 0, 0, 0, 0, 0}));
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
  // BOOKMARK
  // TODO - ask about intention with casting
  // SECTION("Test Decrement instruction") {

  // }

  // SECTION("Test ShiftLeft instruction") {

  // }

  // SECTION("Test ShiftRight instruction") {

  // }

  // SECTION("Test Add instruction") {

  // }

  // SECTION("Test Subtract instruction") {

  // }

  // SECTION("Test Nand instruction") {

  // }

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

  // SECTION("Test global::Anchor instruction") {

  // }

}