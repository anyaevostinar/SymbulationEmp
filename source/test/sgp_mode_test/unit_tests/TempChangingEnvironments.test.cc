#include "emp/math/Random.hpp"

#include "../../../sgp_mode/hardware/SGPHardware.h"
#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorld.cc"
#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"
#include "../../../sgp_mode/SGPWorldData.cc"
#include "../../../sgp_mode/ProgramBuilder.h"



#include "../../../catch/catch.hpp"


TEST_CASE("Temporally changing environment test", "[sgp]") {
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
  using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;
  using program_t = typename world_t::sgp_prog_t;

  // set up configs
  sgpmode::SymConfigSGP config;
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.FILE_PATH("SGPSymbiont_test_output");
  config.SEED(89);
  config.POP_SIZE(0);
  config.GRID_X(2);
  config.GRID_Y(5);
  config.HOST_MIN_CYCLES_BEFORE_REPRO(1000);
  config.HOST_MIN_CYCLES_BEFORE_REPRO(1000);
  
  config.ENABLE_TEMP_CHANGING_ENVIRONMENT(1);
  int change_interval = 1;
  int total_updates = 4;
  config.TEMP_CHANGING_ENVIRONMENT_INTERVAL(change_interval);
  config.UPDATES(total_updates);
  config.FREE_LIVING_SYMS(1);

  // initialize world
  emp::Random random(config.SEED());
  world_t world(random, &config);
  world.Setup();
  auto& builder = world.GetProgramBuilder();

  // Group 1: NOT, AND, OR (start rewarded)
  // Group 2: NAND, AND-NOT, OR-NOT (start punished)
  size_t not_task_id = world.GetTaskEnv().GetTaskSet().GetID("NOT");
  size_t or_not_task_id = world.GetTaskEnv().GetTaskSet().GetID("OR_NOT");
  size_t nand_task_id = world.GetTaskEnv().GetTaskSet().GetID("NAND");

  WHEN("Task rewards change over time and organisms are static (they can't sense whether tasks are rewarded or punished)") {
    WHEN("Organisms only complete NOT"){
      config.CYCLES_PER_UPDATE(4);
      // NOT-only symbiont
      program_t sym_NOT_program;
      builder.AddStartAnchor(sym_NOT_program);
      for (int i = 0; i < 100; i++) {
        builder.AddTask_Not(sym_NOT_program);
      }
      sym_NOT_program.resize(100 - 1);
      emp::Ptr<sgp_sym_t> symbiont_not_only = emp::NewPtr<sgp_sym_t>(&random, &world, &config, sym_NOT_program);

      // NOT-only host
      program_t host_NOT_program;
      builder.AddStartAnchor(host_NOT_program);
      for (int i = 0; i < 100; i++) {
        builder.AddTask_Not(host_NOT_program);
      }
      host_NOT_program.resize(100 - 1);
      emp::Ptr<sgp_host_t> host_not_only = emp::NewPtr<sgp_host_t>(&random, &world, &config, host_NOT_program);

      // add organisms to world
      host_not_only->AddSymbiont(symbiont_not_only);
      world.AddOrgAt(host_not_only, emp::WorldPosition(1, 0));

      // run
      world.Update();
      size_t host_not_count = world.GetHostTaskSuccesses().at(not_task_id);
      size_t sym_not_count = world.GetSymTaskSuccesses().at(not_task_id);

      THEN("Organisms initially gain points for completing their tasks") {
        REQUIRE(host_not_count == 1);
        REQUIRE(host_not_only->GetPoints() == 5);
        REQUIRE(sym_not_count == 1);
        REQUIRE(symbiont_not_only->GetPoints() == 5);
      }

      // event update
      world.Update();
      host_not_count += world.GetHostTaskSuccesses().at(not_task_id);
      sym_not_count += world.GetSymTaskSuccesses().at(not_task_id);

      THEN("After the environment changes, organisms gain points for completing their tasks") {
        REQUIRE(host_not_count == 2);
        REQUIRE(host_not_only->GetPoints() == 0);
        REQUIRE(sym_not_count == 2);
        REQUIRE(symbiont_not_only->GetPoints() == 0);
      }
    }

    WHEN("Organisms complete ORN and NAND"){
      config.CYCLES_PER_UPDATE(8);
      // NAND-only symbiont
      program_t sym_NAND_program;
      builder.AddStartAnchor(sym_NAND_program);
      for (int i = 0; i < 100; i++) {
        builder.AddTask_Nand(sym_NAND_program);
      }
      sym_NAND_program.resize(100 - 1);
      emp::Ptr<sgp_sym_t> symbiont_nand_only = emp::NewPtr<sgp_sym_t>(&random, &world, &config, sym_NAND_program);

      // OR-NOT-only host
      program_t host_ORNOT_program;
      builder.AddStartAnchor(host_ORNOT_program);
      for (int i = 0; i < 100; i++) {
        builder.AddTask_OrNot(host_ORNOT_program);
      }
      host_ORNOT_program.resize(100 - 1);
      emp::Ptr<sgp_host_t> host_orn_only = emp::NewPtr<sgp_host_t>(&random, &world, &config, host_ORNOT_program);

      // add organisms to world
      host_orn_only->AddSymbiont(symbiont_nand_only);
      world.AddOrgAt(host_orn_only, emp::WorldPosition(0, 0));

      // run
      world.Update();
      size_t host_orn_count = world.GetHostTaskSuccesses().at(or_not_task_id);
      size_t sym_nand_count = world.GetSymTaskSuccesses().at(nand_task_id);

      THEN("Organisms initially lose points for completing their tasks") {
        REQUIRE(host_orn_count == 1);
        REQUIRE(host_orn_only->GetPoints() == -5);
        REQUIRE(sym_nand_count == 1);
        REQUIRE(symbiont_nand_only->GetPoints() == -5);
      }

      // event update
      world.Update();
      host_orn_count += world.GetHostTaskSuccesses().at(or_not_task_id);
      sym_nand_count += world.GetSymTaskSuccesses().at(nand_task_id);

      THEN("After the environment changes, organisms gain points for completing their tasks") {
        REQUIRE(host_orn_count == 3);
        REQUIRE(host_orn_only->GetPoints() == 5);
        REQUIRE(sym_nand_count == 3);
        REQUIRE(symbiont_nand_only->GetPoints() == 5);
      }
    }
  }
}