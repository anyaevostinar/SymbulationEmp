#include "emp/math/Random.hpp"

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
#include "../../../sgp_mode/events/EventManager.h"

#include "../../../catch/catch.hpp"

using world_t = sgpmode::SGPWorld;
using cpu_state_t = sgpmode::CPUState<world_t>;
using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
using program_t = typename world_t::sgp_prog_t;
using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;


TEST_CASE("TaskValueEvent One Time Events", "[sgp][events]") {
  GIVEN("onetime-events.json") {
    emp::Random random(2);
    sgpmode::SymConfigSGP config;
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.EVENTS_CFG_PATH("source/test/sgp_mode_test/task-value-events-cfg/onetime-events.json");
    test_utils::SetWellMixed(config, 10, 0);
    config.TASK_IO_BANK_SIZE(10);
    config.UPDATES(100);
    config.SYM_ONLY_FIRST_TASK_CREDIT(1);
    config.HOST_ONLY_FIRST_TASK_CREDIT(1);
    config.INIT_POP_SIZE(1);
    config.CYCLES_PER_UPDATE(52);

    config.HOST_REPRO_RES(10000);
    config.SYM_HORIZ_TRANS_RES(10000);
    config.SYM_VERT_TRANS_RES(10000);

    WHEN("One time add, mul, and change task-value events are loaded") {
      world_t world(random, &config);
      world.Setup();
      auto& builder = world.GetProgramBuilder();
      auto& event_manager = world.GetEventManager();
      REQUIRE(event_manager.GetOneTimeEvents().size() == 3);
      REQUIRE(event_manager.GetRecurringEvents().size() == 0);
      // Events are reverse sorted by update.
      auto& one_time_events = event_manager.GetOneTimeEvents();
      REQUIRE(one_time_events[0]->GetEventType() == "task_value");
      REQUIRE(one_time_events[0]->GetStartUpdate() == 3);
      REQUIRE(one_time_events[1]->GetEventType() == "task_value");
      REQUIRE(one_time_events[1]->GetStartUpdate() == 2);
      REQUIRE(one_time_events[2]->GetEventType() == "task_value");
      REQUIRE(one_time_events[2]->GetStartUpdate() == 1);

      // get NAND Task id
      const size_t nand_task_id = world.GetTaskEnv().GetTaskSet().GetID("NAND");

      // create host with NAND operation
      emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, builder.CreateNandProgram(50));
      // create symbiont with NAND operation
      emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config, builder.CreateNandProgram(50));
      // add host and sym
      world.AddOrgAt(host, 0);
      host->AddSymbiont(symbiont);

      THEN("Events are processed") {
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
        world.Update(); // Should trigger any update-0 events. (none)
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
        REQUIRE(event_manager.GetOneTimeEvents().size() == 3);
        REQUIRE(event_manager.GetRecurringEvents().size() == 0);

        world.Update(); // Should trigger any update-1 events. (+10)
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 15);
        REQUIRE(event_manager.GetOneTimeEvents().size() == 2);
        REQUIRE(event_manager.GetRecurringEvents().size() == 0);

        world.Update(); // Should trigger any update-2 events. (*2)
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 30);
        REQUIRE(event_manager.GetOneTimeEvents().size() == 1);
        REQUIRE(event_manager.GetRecurringEvents().size() == 0);

        world.Update(); // Should trigger any update-3 events. (=100)
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 100);
        REQUIRE(event_manager.GetOneTimeEvents().size() == 0);
        REQUIRE(event_manager.GetRecurringEvents().size() == 0);

        world.Update(); // Should trigger any update-4 events. (none)
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 100);
      }
    }
  }
}

TEST_CASE("TaskValueEvent Multiple Reoccuring Events", "[sgp][events]") {
  GIVEN("reoccur-events.json") {
    emp::Random random(11);
    sgpmode::SymConfigSGP config;
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.EVENTS_CFG_PATH("source/test/sgp_mode_test/task-value-events-cfg/reoccur-events.json");
    config.UPDATES(100);
    test_utils::SetWellMixed(config, 10, 0);
    config.TASK_IO_BANK_SIZE(10);

    WHEN("Reoccuring add and mul task-value events are loaded") {
      world_t world(random, &config);
      world.Setup();

      auto& event_manager = world.GetEventManager();
      REQUIRE(event_manager.GetOneTimeEvents().size() == 0);
      REQUIRE(event_manager.GetRecurringEvents().size() == 2);
      // Events are reverse sorted by start update
      auto& recurring_events = event_manager.GetRecurringEvents();
      REQUIRE(recurring_events[0]->GetEventType() == "task_value");
      REQUIRE(recurring_events[0]->GetStartUpdate() == 2);
      REQUIRE(recurring_events[0]->GetEndUpdate() == 4);
      REQUIRE(recurring_events[0]->GetFrequency() == 2);
      REQUIRE(recurring_events[1]->GetEventType() == "task_value");
      REQUIRE(recurring_events[1]->GetStartUpdate() == 1);
      REQUIRE(recurring_events[1]->GetEndUpdate() == 5);
      REQUIRE(recurring_events[1]->GetFrequency() == 2);

      // get NAND Task id
      const size_t nand_task_id = world.GetTaskEnv().GetTaskSet().GetID("NAND");

      THEN("Events are processed") {

        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
        REQUIRE(event_manager.GetOneTimeEvents().size() == 0);
        REQUIRE(event_manager.GetRecurringEvents().size() == 2);

        // +5 event triggers on updates 1, 3, 5
        // x2 event triggers on updates 2, 4

        world.Update(); // Trigger any update-0 events (none)
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
        REQUIRE(recurring_events.size() == 2);

        world.Update(); // Trigger any update-1 events (+5)
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 10);
        REQUIRE(recurring_events.size() == 2);

        world.Update(); // Trigger any update-2 events (x2)
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 20);
        REQUIRE(recurring_events.size() == 2);

        world.Update(); // Trigger any update-3 events (+5)
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 25);
        REQUIRE(recurring_events.size() == 2);

        world.Update(); // Trigger any update-4 events (x2)
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 50);
        REQUIRE(recurring_events.size() == 1);

        world.Update(); // Trigger any update-5 events (+5)
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 55);
        REQUIRE(recurring_events.size() == 0);

        world.Update(); // Trigger any update-6 events (none)
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 55);
        REQUIRE(recurring_events.size() == 0);
      }
    }
  }
}


TEST_CASE("TaskValueEvent Reoccuring Multiply Event", "[sgp][events]") {
  GIVEN("multiply-reoccur-events.json") {
    emp::Random random(7);
    sgpmode::SymConfigSGP config;
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.UPDATES(100);
    test_utils::SetWellMixed(config, 10, 0);
    config.TASK_IO_BANK_SIZE(10);
    config.SYM_ONLY_FIRST_TASK_CREDIT(1);
    config.HOST_ONLY_FIRST_TASK_CREDIT(1);
    config.INIT_POP_SIZE(1);
    config.CYCLES_PER_UPDATE(52);
    config.HOST_REPRO_RES(10000);
    config.SYM_HORIZ_TRANS_RES(10000);
    config.SYM_VERT_TRANS_RES(10000);

    WHEN("Reoccuring mul task-value events are loaded") {
      config.EVENTS_CFG_PATH("source/test/sgp_mode_test/task-value-events-cfg/multiply-reoccur-events.json");

      world_t world(random, &config);
      world.Setup();
      auto& builder = world.GetProgramBuilder();
      auto& event_manager = world.GetEventManager();
      auto& recurring_events = event_manager.GetRecurringEvents();
      // create host with NAND operation
      emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, builder.CreateNandProgram(50));
      // create symbiont with NAND operation
      emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config, builder.CreateNandProgram(50));
      // add host and sym
      world.AddOrgAt(host, 0);
      host->AddSymbiont(symbiont);

      // get NAND Task id
      const size_t nand_task_id = world.GetTaskEnv().GetTaskSet().GetID("NAND");

      THEN("Events are processed") {
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
        REQUIRE(recurring_events.size() == 1);
        world.Update(); // Update 0 events
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
        REQUIRE(recurring_events.size() == 1);
        world.Update(); // Update 1 events
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == -5);
        REQUIRE(recurring_events.size() == 1);
        world.Update(); // Update 2 events
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == -5);
        REQUIRE(recurring_events.size() == 1);
        world.Update(); // Update 3 events
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
        REQUIRE(recurring_events.size() == 0);
        world.Update(); // Update 4 events
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
        world.Update(); // Update 5 events
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
      }
    }
  }
}

TEST_CASE("TaskValueEvent Reoccuring Add Event", "[sgp][events]") {
  GIVEN("add-reoccur-events.json") {
    emp::Random random(44);
    sgpmode::SymConfigSGP config;
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.EVENTS_CFG_PATH("source/test/sgp_mode_test/task-value-events-cfg/add-reoccur-events.json");
    config.UPDATES(100);
    test_utils::SetWellMixed(config, 10, 0);
    config.TASK_IO_BANK_SIZE(10);

    WHEN("Reoccuring add task-value events are loaded") {
      world_t world(random, &config);
      world.Setup();
      auto& event_manager = world.GetEventManager();
      auto& recurring_events = event_manager.GetRecurringEvents();

      // get NAND Task id
      const size_t nand_task_id = world.GetTaskEnv().GetTaskSet().GetID("NAND");

      THEN("Events are processed") {
        // 1:2:1: 1, 2
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
        REQUIRE(recurring_events.size() == 1);

        world.Update(); // Update 0 events
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
        REQUIRE(recurring_events.size() == 1);

        world.Update(); // Update 1 events
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 10);
        REQUIRE(recurring_events.size() == 1);

        world.Update(); // Update 2 events
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 15);
        REQUIRE(recurring_events.size() == 0);

        world.Update(); // Update 3 events
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 15);
        REQUIRE(recurring_events.size() == 0);
      }
    }
  }
}

TEST_CASE("TaskValueEvent Reoccuring Change Event", "[sgp][events]") {
  GIVEN("change-reoccur-events.json") {
    emp::Random random(60);
    sgpmode::SymConfigSGP config;
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.EVENTS_CFG_PATH("source/test/sgp_mode_test/task-value-events-cfg/change-reoccur-events.json");
    config.UPDATES(100);
    test_utils::SetWellMixed(config, 10, 0);
    config.TASK_IO_BANK_SIZE(10);

    WHEN("Reoccuring change task-value event is loaded") {
      world_t world(random, &config);
      world.Setup();

      // get NAND Task id
      const size_t nand_task_id = world.GetTaskEnv().GetTaskSet().GetID("NAND");

      THEN("Events are processed") {
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
        REQUIRE(world.GetTaskEnv().GetSymTaskReq(nand_task_id).task_value == 5);
        world.Update();
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
        REQUIRE(world.GetTaskEnv().GetSymTaskReq(nand_task_id).task_value == 5);
        world.Update();
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 25);
        REQUIRE(world.GetTaskEnv().GetSymTaskReq(nand_task_id).task_value == 25);
        world.Update();
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 25);
        REQUIRE(world.GetTaskEnv().GetSymTaskReq(nand_task_id).task_value == 25);
        world.Update();
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 25);
        REQUIRE(world.GetTaskEnv().GetSymTaskReq(nand_task_id).task_value == 25);
      }
    }
  }
}

TEST_CASE("TaskValueEvent Host/Sym Only Events", "[sgp][events]") {
  GIVEN("hostsym-only-events.json") {
    emp::Random random(19);
    sgpmode::SymConfigSGP config;
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.EVENTS_CFG_PATH("source/test/sgp_mode_test/task-value-events-cfg/hostsym-only-events.json");
    // Fill in any generic configs here
    config.UPDATES(100);
    test_utils::SetWellMixed(config, 10, 0);
    config.TASK_IO_BANK_SIZE(10);
    config.SYM_ONLY_FIRST_TASK_CREDIT(1);
    config.HOST_ONLY_FIRST_TASK_CREDIT(1);
    config.INIT_POP_SIZE(1);
    config.CYCLES_PER_UPDATE(52);
    config.HOST_REPRO_RES(10000);
    config.SYM_HORIZ_TRANS_RES(10000);
    config.SYM_VERT_TRANS_RES(10000);

    WHEN("Host only and sym only events are loaded") {
      world_t world(random, &config);
      world.Setup();
      auto& builder = world.GetProgramBuilder();
      auto& event_manager = world.GetEventManager();
      // create host with NAND operation
      emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, builder.CreateNandProgram(50));
      // create symbiont with NAND operation
      emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config, builder.CreateNandProgram(50));
      // add host and sym
      world.AddOrgAt(host, 0);
      host->AddSymbiont(symbiont);

      // get NAND Task id
      const size_t nand_task_id = world.GetTaskEnv().GetTaskSet().GetID("NAND");

      THEN("Events are processed") {
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
        REQUIRE(world.GetTaskEnv().GetSymTaskReq(nand_task_id).task_value == 5);

        world.Update(); // Update 0 events
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 5);
        REQUIRE(world.GetTaskEnv().GetSymTaskReq(nand_task_id).task_value == 5);

        world.Update(); // Update 1 events
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 50);
        REQUIRE(world.GetTaskEnv().GetSymTaskReq(nand_task_id).task_value == 100);

        world.Update(); // Update 2 events
        REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value == 50);
        REQUIRE(world.GetTaskEnv().GetSymTaskReq(nand_task_id).task_value == 100);
      }
    }
  }
}


TEST_CASE("TaskValueEvent Mulitple Task Names Events", "[sgp][events]") {
  GIVEN("multiple-tasks-events.json") {
    emp::Random random(1);
    sgpmode::SymConfigSGP config;
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.EVENTS_CFG_PATH("source/test/sgp_mode_test/task-value-events-cfg/multiple-tasks-events.json");
    config.UPDATES(100);
    test_utils::SetWellMixed(config, 10, 0);
    config.TASK_IO_BANK_SIZE(10);

    WHEN("Multiple tasks are in a single event") {
      world_t world(random, &config);
      world.Setup();
      auto& event_manager = world.GetEventManager();
      // get task ids
      const size_t nand_task_id = world.GetTaskEnv().GetTaskSet().GetID("NAND");
      const size_t not_task_id = world.GetTaskEnv().GetTaskSet().GetID("NOT");
      const size_t or_task_id = world.GetTaskEnv().GetTaskSet().GetID("OR");
      const size_t and_task_id = world.GetTaskEnv().GetTaskSet().GetID("AND");
      const size_t xor_task_id = world.GetTaskEnv().GetTaskSet().GetID("XOR");
      const size_t equ_task_id = world.GetTaskEnv().GetTaskSet().GetID("EQU");
      const size_t nor_task_id = world.GetTaskEnv().GetTaskSet().GetID("NOR");
      const size_t andnot_task_id = world.GetTaskEnv().GetTaskSet().GetID("AND_NOT");
      const size_t ornot_task_id = world.GetTaskEnv().GetTaskSet().GetID("OR_NOT");

      THEN("Events are processed") {
        world.Update(); // Update 0 events
        // Check sym task values
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

        world.Update(); // Update 1 events
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