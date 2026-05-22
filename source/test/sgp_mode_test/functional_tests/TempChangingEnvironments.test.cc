#include "emp/math/Random.hpp"

#include "../../../sgp_mode/hardware/SGPHardware.h"
#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorld.cc"
#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"
#include "../../../sgp_mode/SGPWorldData.cc"
#include "../../../sgp_mode/ProgramBuilder.h"

#include "../../../catch/catch.hpp"

/**
 * This file is dedicated to organism point gain / loss in temporally changing environments 
 */

using world_t = sgpmode::SGPWorld;
using cpu_state_t = sgpmode::CPUState<world_t>;
using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;
using program_t = typename world_t::sgp_prog_t;

TEST_CASE("Hosts start with a rewarded task in a temporally changing environment"){
  // set up configs
  sgpmode::SymConfigSGP config;
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.FILE_PATH("TempChangingEnv_test_output");
  config.SEED(89);
  config.POP_SIZE(0);
  config.GRID_X(2);
  config.GRID_Y(2);
  config.CYCLES_PER_UPDATE(4);
  config.ENABLE_TEMP_CHANGING_ENVIRONMENT(1);
  config.TEMP_CHANGING_ENVIRONMENT_INTERVAL(1);

  // initialize world
  emp::Random random(config.SEED());
  world_t world(random, &config);
  world.Setup();
  auto& builder = world.GetProgramBuilder();

  // Group 1: NOT, AND, OR (start rewarded)
  size_t not_task_id = world.GetTaskEnv().GetTaskSet().GetID("NOT");
  
  WHEN("A host can do only NOT at the start of the experiment"){
    // NOT-only host
    program_t host_NOT_program;
    builder.AddStartAnchor(host_NOT_program);
    for (int i = 0; i < 5; i++) {
      builder.AddTask_NotIO(host_NOT_program);
    }
    host_NOT_program.resize(20 - 1);
    emp::Ptr<sgp_host_t> host_not_only = emp::NewPtr<sgp_host_t>(&random, &world, &config, host_NOT_program);

    // add organisms to world
    world.AddOrgAt(host_not_only, emp::WorldPosition(1, 0));

    // run
    world.Update();
    size_t host_not_count = world.GetHostTaskSuccesses().at(not_task_id);

    THEN("The host initially gains points for completing its task") {
      REQUIRE(host_not_count == 1);
      REQUIRE(host_not_only->GetPoints() == 5);
    }

    // event update
    world.Update();
    host_not_count += world.GetHostTaskSuccesses().at(not_task_id);

    THEN("After the environment changes, the host loses points for completing its task") {
      REQUIRE(host_not_count == 2);
      REQUIRE(host_not_only->GetPoints() == 0);
    }
  }
}

TEST_CASE("Symbionts start with a rewarded task in a temporally changing environment"){
  // set up configs
  sgpmode::SymConfigSGP config;
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.FILE_PATH("TempChangingEnv_test_output");
  config.SEED(89);
  config.POP_SIZE(0);
  config.GRID_X(2);
  config.GRID_Y(2);
  config.CYCLES_PER_UPDATE(4);
  config.ENABLE_TEMP_CHANGING_ENVIRONMENT(1);
  config.TEMP_CHANGING_ENVIRONMENT_INTERVAL(1);

  // initialize world
  emp::Random random(config.SEED());
  world_t world(random, &config);
  world.Setup();
  auto& builder = world.GetProgramBuilder();

  // Group 1: NOT, AND, OR (start rewarded)
  size_t not_task_id = world.GetTaskEnv().GetTaskSet().GetID("NOT");

  WHEN("A symbiont can do only NOT at the start of the experiment"){
    // NOT-only symbiont
    program_t sym_NOT_program;
    builder.AddStartAnchor(sym_NOT_program);
    for (int i = 0; i < 5; i++) {
      builder.AddTask_NotIO(sym_NOT_program);
    }
    sym_NOT_program.resize(20 - 1);
    emp::Ptr<sgp_sym_t> symbiont_not_only = emp::NewPtr<sgp_sym_t>(&random, &world, &config, sym_NOT_program);
    emp::Ptr<sgp_host_t> host_not_only = emp::NewPtr<sgp_host_t>(&random, &world, &config);

    // add organisms to world
    host_not_only->AddSymbiont(symbiont_not_only);
    world.AddOrgAt(host_not_only, emp::WorldPosition(1, 0));

    // run
    world.Update();
    size_t sym_not_count = world.GetSymTaskSuccesses().at(not_task_id);

    THEN("The symbiont initially gains points for completing its task") {
      REQUIRE(sym_not_count == 1);
      REQUIRE(symbiont_not_only->GetPoints() == 5);
    }

    // event update
    world.Update();
    sym_not_count += world.GetSymTaskSuccesses().at(not_task_id);

    THEN("After the environment changes, the symbiont loses points for completing its task") {
      REQUIRE(sym_not_count == 2);
      REQUIRE(symbiont_not_only->GetPoints() == 0);
    }
  }
}

TEST_CASE("Hosts start with a punished task in a temporally changing environment", "[sgp]") {
  // set up configs
  sgpmode::SymConfigSGP config;
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.FILE_PATH("TempChangingEnv_test_output");
  config.SEED(89);
  config.POP_SIZE(0);
  config.GRID_X(2);
  config.GRID_Y(2);
  config.CYCLES_PER_UPDATE(8);
  config.ENABLE_TEMP_CHANGING_ENVIRONMENT(1);
  config.TEMP_CHANGING_ENVIRONMENT_INTERVAL(1);

  // initialize world
  emp::Random random(config.SEED());
  world_t world(random, &config);
  world.Setup();
  auto& builder = world.GetProgramBuilder();

  // Group 2: NAND, AND-NOT, OR-NOT (start punished)
  size_t or_not_task_id = world.GetTaskEnv().GetTaskSet().GetID("OR_NOT");

  WHEN("A host can do only OR_NOT at the start of the experiment"){
    // OR-NOT-only host
    program_t host_ORNOT_program;
    builder.AddStartAnchor(host_ORNOT_program);
    for (int i = 0; i < 5; i++) {
      builder.AddTask_OrNotIO(host_ORNOT_program);
    }
    host_ORNOT_program.resize(20 - 1);
    emp::Ptr<sgp_host_t> host_orn_only = emp::NewPtr<sgp_host_t>(&random, &world, &config, host_ORNOT_program);

    // add organisms to world
    world.AddOrgAt(host_orn_only, emp::WorldPosition(0, 0));

    // run
    world.Update();
    size_t host_orn_count = world.GetHostTaskSuccesses().at(or_not_task_id);

    THEN("The host initially loses points for completing its task") {
      REQUIRE(host_orn_count == 1);
      REQUIRE(host_orn_only->GetPoints() == -5);
    }

    // event update
    world.Update();
    host_orn_count += world.GetHostTaskSuccesses().at(or_not_task_id);

    THEN("After the environment changes, the host gains points for completing its task") {
      REQUIRE(host_orn_count == 3);
      REQUIRE(host_orn_only->GetPoints() == 5);
    }
  }
}


TEST_CASE("Symbionts start with a punished task in a temporally changing environment", "[sgp]") {
  // set up configs
  sgpmode::SymConfigSGP config;
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.FILE_PATH("TempChangingEnv_test_output");
  config.SEED(89);
  config.POP_SIZE(0);
  config.GRID_X(2);
  config.GRID_Y(2);
  config.CYCLES_PER_UPDATE(8);
  config.ENABLE_TEMP_CHANGING_ENVIRONMENT(1);
  config.TEMP_CHANGING_ENVIRONMENT_INTERVAL(1);

  // initialize world
  emp::Random random(config.SEED());
  world_t world(random, &config);
  world.Setup();
  auto& builder = world.GetProgramBuilder();

  // Group 2: NAND, AND-NOT, OR-NOT (start punished)
  size_t nand_task_id = world.GetTaskEnv().GetTaskSet().GetID("NAND");

  WHEN("A symbiont can do only NAND at the start of the experiment"){
    // NAND-only symbiont
    program_t sym_NAND_program;
    builder.AddStartAnchor(sym_NAND_program);
    for (int i = 0; i < 5; i++) {
      builder.AddTask_NandIO(sym_NAND_program);
    }
    sym_NAND_program.resize(20 - 1);
    emp::Ptr<sgp_sym_t> symbiont_nand_only = emp::NewPtr<sgp_sym_t>(&random, &world, &config, sym_NAND_program);
    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, builder.CreateNotProgram(100));

    // add organisms to world
    host->AddSymbiont(symbiont_nand_only);
    world.AddOrgAt(host, emp::WorldPosition(0, 0));

    // run
    world.Update();
    size_t sym_nand_count = world.GetSymTaskSuccesses().at(nand_task_id);

    THEN("The symbiont initially loses points for completing its task") {
      REQUIRE(sym_nand_count == 1);
      REQUIRE(symbiont_nand_only->GetPoints() == -5);
    }

    // event update
    world.Update();
    sym_nand_count += world.GetSymTaskSuccesses().at(nand_task_id);

    THEN("After the environment changes, the symbiont gains points for completing its tasks") {
      REQUIRE(sym_nand_count == 3);
      REQUIRE(symbiont_nand_only->GetPoints() == 5);
    }
  } 
}