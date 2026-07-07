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


TEST_CASE("TaskValueEvent One Time Events", "[sgp][events]") {
  GIVEN("onetime-events.json"){
    emp::Random random(2);
    sgpmode::SymConfigSGP config;
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.EVENTS_CFG_PATH("source/test/sgp_mode_test/task_value_json_files/onetime-events.json");

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


    WHEN("One time add, mul, and change task-value events are loaded"){
      world_t world(random, &config);
      world.Setup();
      auto& builder = world.GetProgramBuilder();

      // get NAND Task id
      const size_t nand_task_id = world.GetTaskEnv().GetTaskSet().GetID("NAND");

      THEN("Events are processed"){
        switch(world.GetUpdate()){
          case 0: {
            // create host with NAND operation
            emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, builder.CreateNandProgram(50));
            // create symbiont with NAND operation
            emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config, builder.CreateNandProgram(50));
            // add host and sym 
            world.AddOrgAt(host, 0);
            host->AddSymbiont(symbiont);
            
            CHECK(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
            CHECK(symbiont->GetPoints() == 0);
            CHECK(host->GetPoints() == 0);
          }

          case 1: {
            // create host with NAND operation
            emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, builder.CreateNandProgram(50));
            // create symbiont with NAND operation
            emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config, builder.CreateNandProgram(50));
            // add host and sym 
            world.AddOrgAt(host, 0);
            host->AddSymbiont(symbiont);

            world.Update();
            CHECK(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
            CHECK(symbiont->GetPoints() == 5);
            CHECK(host->GetPoints() == 5);
          }

          case 2: {
            // create host with NAND operation
            emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, builder.CreateNandProgram(50));
            // create symbiont with NAND operation
            emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config, builder.CreateNandProgram(50));
            // add host and sym 
            world.AddOrgAt(host, 0);
            host->AddSymbiont(symbiont);

            world.Update();
            CHECK(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 15);
            CHECK(symbiont->GetPoints() == 15);
            CHECK(host->GetPoints() == 15);
          } 

          case 3: {
            // create host with NAND operation
            emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, builder.CreateNandProgram(50));
            // create symbiont with NAND operation
            emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config, builder.CreateNandProgram(50));
            // add host and sym 
            world.AddOrgAt(host, 0);
            host->AddSymbiont(symbiont);

            world.Update();
            CHECK(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 30);
            CHECK(symbiont->GetPoints() == 30); // 80?
            CHECK(host->GetPoints() == 30); // 80?
          }

          case 4: {
            // create host with NAND operation
            emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, builder.CreateNandProgram(50));
            // create symbiont with NAND operation
            emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config, builder.CreateNandProgram(50));
            // add host and sym 
            world.AddOrgAt(host, 0);
            host->AddSymbiont(symbiont);

            world.Update();
            CHECK(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 100);
            CHECK(symbiont->GetPoints() == 100); // 180?
            CHECK(host->GetPoints() == 100); // 180? 
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
    int num_updates = 7;
    config.UPDATES(num_updates);

    WHEN("Reoccuring add and mul task-value events are loaded"){      
      world_t world(random, &config);
      world.Setup();
      auto& builder = world.GetProgramBuilder();

      // get NAND Task id
      const size_t nand_task_id = world.GetTaskEnv().GetTaskSet().GetID("NAND");

      THEN("Events are processed"){
        switch(world.GetUpdate()){
          case 0: {
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(nand_task_id).task_value == 5);
          }
          case 1: {
            world.Update();
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(nand_task_id).task_value == 5);
          }
          case 2: {
            world.Update();
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 10);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(nand_task_id).task_value == 10);
          }
          case 3: {
            world.Update();
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 30);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(nand_task_id).task_value == 30);
          }
          case 4: {
            world.Update();
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 35);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(nand_task_id).task_value == 35);
          }
          case 5: {
            world.Update();
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 70);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(nand_task_id).task_value == 70);
          }
          case 6: {
            world.Update();
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 70);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(nand_task_id).task_value == 70);
          }
          case 7: {
            world.Update();
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 70);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(nand_task_id).task_value == 70);
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
    int num_updates = 6;
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
        switch(world.GetUpdate()){
          case 0: {
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
          }
          case 1: {
            world.Update();
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
          }
          case 2: {
            world.Update();
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == -5);
          }
          case 3: {
            world.Update();
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == -5);
          }
          case 4: {
            world.Update();
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
          }
          case 5: {
            world.Update();
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
          }
          case 6: {
            world.Update();
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
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
    int num_updates = 4;
    config.UPDATES(num_updates);

    WHEN("Reoccuring add task-value events are loaded"){      
      world_t world(random, &config);
      world.Setup();
      auto& builder = world.GetProgramBuilder();

      // get NAND Task id
      const size_t nand_task_id = world.GetTaskEnv().GetTaskSet().GetID("NAND");

      THEN("Events are processed"){
        switch(world.GetUpdate()){
          case 0: {
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
          }
          case 1: {
            world.Update();
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
          }
          case 2: {
            world.Update();
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 10);
          }
          case 3: {
            world.Update();
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 15);
          }
          case 4: {
            world.Update();
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 15);
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
    int num_updates = 4;
    config.UPDATES(num_updates);

    WHEN("Reoccuring change task-value event is loaded"){      
      world_t world(random, &config);
      world.Setup();
      auto& builder = world.GetProgramBuilder();

      // get NAND Task id
      const size_t nand_task_id = world.GetTaskEnv().GetTaskSet().GetID("NAND");

      THEN("Events are processed"){
        switch(world.GetUpdate()) {
          case 0: {
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(nand_task_id).task_value == 5);
          }
          case 1: {
            world.Update();
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(nand_task_id).task_value == 5);
          }
          case 2: {
            world.Update();
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 25);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(nand_task_id).task_value == 25);
          }
          case 3: {
            world.Update();
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 25);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(nand_task_id).task_value == 25);
          }
          case 4: {
            world.Update();
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 25);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(nand_task_id).task_value == 25);
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
    int num_updates = 4;
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
        switch(world.GetUpdate()){
          case 0:{
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(nand_task_id).task_value == 5);
            REQUIRE(host->GetPoints() == 0);
            REQUIRE(symbiont->GetPoints() == 0);
          }
          case 1: {
            // create host with NAND operation
            emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, builder.CreateNandProgram(50));
            // create symbiont with NAND operation
            emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config, builder.CreateNandProgram(50));
            // add host and sym 
            world.AddOrgAt(host, 0);
            host->AddSymbiont(symbiont);
            world.Update();

            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(nand_task_id).task_value == 5);
            REQUIRE(host->GetPoints() == 5);
            REQUIRE(symbiont->GetPoints() == 5);
          }
          case 2: {
            // create host with NAND operation
            emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, builder.CreateNandProgram(50));
            // create symbiont with NAND operation
            emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config, builder.CreateNandProgram(50));
            // add host and sym 
            world.AddOrgAt(host, 0);
            host->AddSymbiont(symbiont);
            world.Update();

            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 50);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(nand_task_id).task_value == 100);
            REQUIRE(host->GetPoints() == 50);
            REQUIRE(symbiont->GetPoints() == 100);
          }
          case 3: {
            // create host with NAND operation
            emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, builder.CreateNandProgram(50));
            // create symbiont with NAND operation
            emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config, builder.CreateNandProgram(50));
            // add host and sym 
            world.AddOrgAt(host, 0);
            host->AddSymbiont(symbiont);
            world.Update();  

            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 50);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(nand_task_id).task_value == 100);
            // incorrect output
            CHECK(host->GetPoints() == 50); 
            CHECK(symbiont->GetPoints() == 100); 
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
    int num_updates = 2;
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
        switch(world.GetUpdate()){
          case 1: {
            world.Update();
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(not_task_id).task_value == 5);
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(and_task_id).task_value == 5);
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(or_task_id).task_value == 5);
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(xor_task_id).task_value == 5);
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nor_task_id).task_value == 5);
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(equ_task_id).task_value == 5);
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(andnot_task_id).task_value == 5);
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(ornot_task_id).task_value == 5);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(nand_task_id).task_value == 5);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(not_task_id).task_value == 5);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(and_task_id).task_value == 5);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(or_task_id).task_value == 5);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(xor_task_id).task_value == 5);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(nor_task_id).task_value == 5);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(equ_task_id).task_value == 5);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(andnot_task_id).task_value == 5);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(ornot_task_id).task_value == 5);
          }
          case 2: {
            world.Update();
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 100);
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(not_task_id).task_value == 100);
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(and_task_id).task_value == 100);
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(or_task_id).task_value == 100);
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(xor_task_id).task_value == 100);
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(nor_task_id).task_value == 100);
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(equ_task_id).task_value == 100);
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(andnot_task_id).task_value == 100);
            REQUIRE(world.GetTaskEnv().GetHostTaskReq(ornot_task_id).task_value == 100);
              
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(nand_task_id).task_value == 100);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(not_task_id).task_value == 100);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(and_task_id).task_value == 100);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(or_task_id).task_value == 100);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(xor_task_id).task_value == 100);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(nor_task_id).task_value == 100);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(equ_task_id).task_value == 100);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(andnot_task_id).task_value == 100);
            REQUIRE(world.GetTaskEnv().GetSymTaskReq(ornot_task_id).task_value == 100);
          }
        }
      }
    }
  }
}



// ***** CREATE NAND TEST  *****
TEST_CASE("Checking if NAND op is done for more than two updates", "[sgp][events]") {
  GIVEN("onetime-events.json"){
    emp::Random random(2);
    sgpmode::SymConfigSGP config;
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.EVENTS_CFG_PATH("source/test/sgp_mode_test/task_value_json_files/onetime-events.json");

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
        switch(world.GetUpdate()){
          case 0: {            
            CHECK(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);

            CHECK(host->GetHardware().GetCPUState().GetTaskPerformanceCount(nand_task_id) == 0);
            CHECK(symbiont->GetHardware().GetCPUState().GetTaskPerformanceCount(nand_task_id) == 0);

            CHECK(symbiont->GetPoints() == 0);
            CHECK(host->GetPoints() == 0);
          }

          case 1: {
            world.Update();
            CHECK(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);

            CHECK(host->GetHardware().GetCPUState().GetTaskPerformanceCount(nand_task_id) == 1);
            CHECK(symbiont->GetHardware().GetCPUState().GetTaskPerformanceCount(nand_task_id) == 1);

            CHECK(symbiont->GetPoints() == 5);
            CHECK(host->GetPoints() == 5);
          }

          case 2: {
            world.Update();
            CHECK(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 15);

            CHECK(host->GetHardware().GetCPUState().GetTaskPerformanceCount(nand_task_id) == 2);
            CHECK(symbiont->GetHardware().GetCPUState().GetTaskPerformanceCount(nand_task_id) == 2);

            CHECK(symbiont->GetPoints() == 20);
            CHECK(host->GetPoints() == 20);
          } 

          case 3: {
            world.Update();
            CHECK(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 30);

            CHECK(host->GetHardware().GetCPUState().GetTaskPerformanceCount(nand_task_id) == 4);
            CHECK(symbiont->GetHardware().GetCPUState().GetTaskPerformanceCount(nand_task_id) == 4);

            CHECK(symbiont->GetPoints() == 80); 
            CHECK(host->GetPoints() == 80); 
          }

          case 4: {
            world.Update();
            CHECK(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 100);

            CHECK(host->GetHardware().GetCPUState().GetTaskPerformanceCount(nand_task_id) == 5);
            CHECK(symbiont->GetHardware().GetCPUState().GetTaskPerformanceCount(nand_task_id) == 5);

            CHECK(symbiont->GetPoints() == 180);
            CHECK(host->GetPoints() == 180);
          }        
        }
      }
    }
  }
}