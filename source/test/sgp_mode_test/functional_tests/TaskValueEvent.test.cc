#include "emp/math/Random.hpp"

#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorld.cc"
#include "../../../sgp_mode/SGPWorldSetup.cc"
#include "../../../sgp_mode/SGPWorldData.cc"

#include "../../../catch/catch.hpp"

using world_t = sgpmode::SGPWorld;
using cpu_state_t = sgpmode::CPUState<world_t>;
using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
using program_t = typename world_t::sgp_prog_t;
using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;

// write tests hard code for multiple events, one add, one mul, one change in current json file
// check organisms are getting the changes when they perform new task
// write new json to alter multiple tasks
// write new json for host and sym only

TEST_CASE("TaskValueEvent One Time Events", "[sgp][events]") {
  GIVEN("onetime-events.json"){
    emp::Random random(2);
    sgpmode::SymConfigSGP config;
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.EVENTS_CFG_PATH("source/test/sgp_mode_test/task_value_json_files/onetime-events.json");

    // Fill in any generic configs here
    int num_updates = 3;
    config.UPDATES(num_updates);
    config.SYM_ONLY_FIRST_TASK_CREDIT(1);
    config.HOST_ONLY_FIRST_TASK_CREDIT(1);
    config.POP_SIZE(1);
    config.CYCLES_PER_UPDATE(52);

    config.HOST_REPRO_RES(10000);
    config.SYM_HORIZ_TRANS_RES(10000);
    config.SYM_VERT_TRANS_RES(10000);


    WHEN("One time add, mul, and change task-value events are loaded"){
      world_t world(random, &config);
      world.Setup();
      auto& builder = world.GetProgramBuilder();

      // create host with NAND operation
      emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, builder.CreateNandProgram(50));

      // create symbiont with NAND operation
      emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config, builder.CreateNandProgram(50));

      // add host and sym 
      world.AddOrgAt(host, 0);
      host->AddSymbiont(symbiont);

      // get NAND Task id
      const size_t nand_task_id = world.GetTaskEnv().GetTaskSet().GetID("NAND");

      THEN("Events are processed"){
        for(int i = 0; i <= num_updates; i++){
          world.Update();
          switch(i){
            case 0:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
              // REQUIRE(symbiont->GetPoints() == 5);
              // REQUIRE(host->GetPoints() == 5);
              continue;
            case 1:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 15);
              // CHECK(symbiont->GetPoints() == 20);
              // CHECK(host->GetPoints() == 20);
              continue;
            case 2:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 30);
              // CHECK(symbiont->GetPoints() == 50);
              // CHECK(host->GetPoints() == 50);
              continue;
            case 3:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 100);
              // CHECK(symbiont->GetPoints() == 150);
              // CHECK(host->GetPoints() == 150);
              continue;
          }
        }
      }
    }
  }
}


TEST_CASE("TaskValueEvent Multiple Reoccuring Events", "[sgp][events]") {
  GIVEN("reoccur-events.json"){
    emp::Random random(11);
    sgpmode::SymConfigSGP config;
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.EVENTS_CFG_PATH("source/test/sgp_mode_test/task_value_json_files/reoccur-events.json");
    // Fill in any generic configs here
    int num_updates = 6;
    config.UPDATES(num_updates);

    WHEN("Reoccuring add and mul task-value events are loaded"){      
      world_t world(random, &config);
      world.Setup();
      auto& builder = world.GetProgramBuilder();

      // get NAND Task id
      const size_t nand_task_id = world.GetTaskEnv().GetTaskSet().GetID("NAND");

      THEN("Events are processed"){
        for(int i = 0; i <= num_updates; i++){
          world.Update();
          switch(i){
            case 0:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
              continue;
            case 1:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 10);
              continue;
            case 2:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 30);
              continue;
            case 3:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 35);
              continue;
            case 4:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 70);
              continue;
            case 5:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 70);
              continue;
            case 6:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 70);
              continue;
          }
        }
      }
    }
  }
}

TEST_CASE("TaskValueEvent Reoccuring Multiply Event", "[sgp][events]") {
  GIVEN("multiply-reoccur-events.json"){
    emp::Random random(7);
    sgpmode::SymConfigSGP config;
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    // Fill in any generic configs here
    int num_updates = 5;
    config.UPDATES(num_updates);
    config.SYM_ONLY_FIRST_TASK_CREDIT(1);
    config.HOST_ONLY_FIRST_TASK_CREDIT(1);
    config.POP_SIZE(1);
    config.CYCLES_PER_UPDATE(52);

    config.HOST_REPRO_RES(10000);
    config.SYM_HORIZ_TRANS_RES(10000);
    config.SYM_VERT_TRANS_RES(10000);

    WHEN("Reoccuring mul task-value events are loaded"){
      config.EVENTS_CFG_PATH("source/test/sgp_mode_test/task_value_json_files/multiply-reoccur-events.json");
      
      world_t world(random, &config);
      world.Setup();
      auto& builder = world.GetProgramBuilder();

      // create host with NAND operation
      emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, builder.CreateNandProgram(50));

      // create symbiont with NAND operation
      emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config, builder.CreateNandProgram(50));

      // add host and sym 
      world.AddOrgAt(host, 0);
      host->AddSymbiont(symbiont);

      // get NAND Task id
      const size_t nand_task_id = world.GetTaskEnv().GetTaskSet().GetID("NAND");

      THEN("Events are processed"){
        for(int i = 0; i <= num_updates; i++){
          world.Update();
          switch(i){
            case 0:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
              // CHECK(host->GetPoints() == 5);
              continue;
            case 1:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == -5);
              // CHECK(host->GetPoints() == 0);
              continue;
            case 2:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == -5);
              // CHECK(host->GetPoints() == -5);
              continue;
            case 3:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
              // CHECK(host->GetPoints() == 0);
              continue;
            case 4:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
              // CHECK(host->GetPoints() == 5);
              continue;
            case 5:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
              // CHECK(host->GetPoints() == 10);
              continue;
          }
        }
      }
    }
  }
}

TEST_CASE("TaskValueEvent Reoccuring Add Event", "[sgp][events]") {
  GIVEN("add-reoccur-events.json"){
    emp::Random random(44);
    sgpmode::SymConfigSGP config;
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.EVENTS_CFG_PATH("source/test/sgp_mode_test/task_value_json_files/add-reoccur-events.json");

    // Fill in any generic configs here
    int num_updates = 3;
    config.UPDATES(num_updates);

    WHEN("Reoccuring add task-value events are loaded"){      
      world_t world(random, &config);
      world.Setup();
      auto& builder = world.GetProgramBuilder();

      // get NAND Task id
      const size_t nand_task_id = world.GetTaskEnv().GetTaskSet().GetID("NAND");

      THEN("Events are processed"){
        for(int i = 0; i <= num_updates; i++){
          world.Update();
          switch(i){
            case 0:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
              continue;
            case 1:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 10);
              continue;
            case 2:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 15);
              continue;
            case 3:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 15);
              continue;
          }
        }
      }
    }
  }
}

TEST_CASE("TaskValueEvent Reoccuring Change Event", "[sgp][events]") {
  GIVEN("change-reoccur-events.json"){
    emp::Random random(60);
    sgpmode::SymConfigSGP config;
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.EVENTS_CFG_PATH("source/test/sgp_mode_test/task_value_json_files/change-reoccur-events.json");

    // Fill in any generic configs here
    int num_updates = 3;
    config.UPDATES(num_updates);

    WHEN("Reoccuring change task-value event is loaded"){      
      world_t world(random, &config);
      world.Setup();
      auto& builder = world.GetProgramBuilder();

      // get NAND Task id
      const size_t nand_task_id = world.GetTaskEnv().GetTaskSet().GetID("NAND");

      THEN("Events are processed"){
        for(int i = 0; i <= num_updates; i++){
          world.Update();
          switch(i){
            case 0:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
              continue;
            case 1:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 25);
              continue;
            case 2:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 25);
              continue;
            case 3:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 25);
              continue;
          }
        }
      }
    }
  }
}

TEST_CASE("TaskValueEvent Host/Sym Only Events", "[sgp][events]") {
  GIVEN("hostsym-only-events.json"){
    emp::Random random(19);
    sgpmode::SymConfigSGP config;
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.EVENTS_CFG_PATH("source/test/sgp_mode_test/task_value_json_files/hostsym-only-events.json");
    // Fill in any generic configs here
    int num_updates = 2;
    config.UPDATES(num_updates);
    config.SYM_ONLY_FIRST_TASK_CREDIT(1);
    config.HOST_ONLY_FIRST_TASK_CREDIT(1);
    config.POP_SIZE(1);
    config.CYCLES_PER_UPDATE(52);

    config.HOST_REPRO_RES(10000);
    config.SYM_HORIZ_TRANS_RES(10000);
    config.SYM_VERT_TRANS_RES(10000);

    WHEN("Host only and sym only events are loaded"){      
      world_t world(random, &config);
      world.Setup();
      auto& builder = world.GetProgramBuilder();

      // create host with NAND operation
      emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, builder.CreateNandProgram(50));

      // create symbiont with NAND operation
      emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config, builder.CreateNandProgram(50));

      // add host and sym 
      world.AddOrgAt(host, 0);
      host->AddSymbiont(symbiont);

      // get NAND Task id
      const size_t nand_task_id = world.GetTaskEnv().GetTaskSet().GetID("NAND");

      THEN("Events are processed"){
        for(int i = 0; i <= num_updates; i++){
          world.Update();
          switch(i){
            case 0:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
              REQUIRE(host->GetPoints() == 5);
              REQUIRE(symbiont->GetPoints() == 5);
              continue;
            case 1:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 50);
              REQUIRE(host->GetPoints() == 55);
              REQUIRE(symbiont->GetPoints() == 105);
              continue;
            case 2:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 50);
              // incorrect output
              // CHECK(symbiont->GetPoints() == 205); // 105
              // CHECK(host->GetPoints() == 105); // 55
              continue;
          }
        }
      }
    }
  }
}

TEST_CASE("TaskValueEvent Mulitple Task Names Events", "[sgp][events]") {
  GIVEN("multiple-tasks-events.json"){
    emp::Random random(1);
    sgpmode::SymConfigSGP config;
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.EVENTS_CFG_PATH("source/test/sgp_mode_test/task_value_json_files/multiple-tasks-events.json");
    // Fill in any generic configs here
    int num_updates = 1;
    config.UPDATES(num_updates);

    WHEN("Multiple tasks are in a single event"){      
      world_t world(random, &config);
      world.Setup();
      auto& builder = world.GetProgramBuilder();

      // get NAND Task id
      const size_t nand_task_id = world.GetTaskEnv().GetTaskSet().GetID("NAND");
      const size_t not_task_id = world.GetTaskEnv().GetTaskSet().GetID("NOT");
      const size_t or_task_id = world.GetTaskEnv().GetTaskSet().GetID("OR");
      const size_t and_task_id = world.GetTaskEnv().GetTaskSet().GetID("AND");
      const size_t xor_task_id = world.GetTaskEnv().GetTaskSet().GetID("XOR");
      const size_t equ_task_id = world.GetTaskEnv().GetTaskSet().GetID("EQU");
      const size_t nor_task_id = world.GetTaskEnv().GetTaskSet().GetID("NOR");
      const size_t andnot_task_id = world.GetTaskEnv().GetTaskSet().GetID("AND_NOT");
      const size_t ornot_task_id = world.GetTaskEnv().GetTaskSet().GetID("OR_NOT");

      THEN("Events are processed"){
        for(int i = 0; i <= num_updates; i++){
          world.Update();
          switch(i){
            case 0:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(not_task_id).task_value == 5);
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(and_task_id).task_value == 5);
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(or_task_id).task_value == 5);
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(xor_task_id).task_value == 5);
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nor_task_id).task_value == 5);
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(equ_task_id).task_value == 5);
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(andnot_task_id).task_value == 5);
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(ornot_task_id).task_value == 5);
              continue;
            case 1:
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 100);
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(not_task_id).task_value == 100);
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(and_task_id).task_value == 100);
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(or_task_id).task_value == 100);
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(xor_task_id).task_value == 100);
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(nor_task_id).task_value == 100);
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(equ_task_id).task_value == 100);
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(andnot_task_id).task_value == 100);
              REQUIRE(world.GetTaskEnv().GetHostTaskReq(ornot_task_id).task_value == 100);
              continue;
          }
        }
      }
    }
  }
}
