#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorld.cc"
#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"
#include "../../../sgp_mode/SGPWorldData.cc"
#include "../../../catch/catch.hpp"

/**
 * This file is dedicated to unit tests for SGPWorld
 */

using world_t = sgpmode::SGPWorld;
using cpu_state_t = sgpmode::CPUState<world_t>;
using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;
using program_t = typename world_t::sgp_prog_t;


TEST_CASE("Update only hosts test", "[sgp]") {


  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.GRID_X(2);
  config.GRID_Y(2);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.POP_SIZE(0);


  world_t world(random, &config);
  world.Resize(2,2);
  world.Setup();
  auto& builder = world.GetProgramBuilder();

  emp::Ptr<sgp_host_t> uninfected_host = emp::NewPtr<sgp_host_t>(&random, &world, &config, builder.CreateReproProgram(100));

  world.AddOrgAt(uninfected_host, 0);
  // TODO: this doesn't work if you add at position 1 instead, is that a problem?

  THEN("Organisms can be added to the world") {
    REQUIRE(world.GetNumOrgs() == 1);
    REQUIRE(world.GetOrgPtr(0)->GetAge() == 0);
  }



  for (int i = 0; i < 10; i++) {
    world.Update();
  }

  THEN("Organisms persist and are managed by the world") {
    REQUIRE(world.GetNumOrgs() == 1);
    REQUIRE(world.GetOrgPtr(0)->GetAge() == 10);
  }
}



TEST_CASE("Host Setup", "[sgp][sgp-unit]") {
  emp::Random random(1);
  sgpmode::SymConfigSGP config;
  config.SEED(2);
  config.MUTATION_RATE(0.0);
  config.MUTATION_SIZE(0.000);
  //config.TRACK_PARENT_TASKS(PARENTONLY);
  config.VT_TASK_MATCH(1);
  config.HOST_ONLY_FIRST_TASK_CREDIT(1);
  config.SYM_ONLY_FIRST_TASK_CREDIT(1);
  config.HOST_REPRO_RES(10000);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");

  //world.SetupHosts requires a pointer for the number of hosts in the world
  unsigned long setupCount = 1;
  //TODO?
}

TEST_CASE("Ousting is permitted", "[sgp]") {
  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.GRID_X(2);
  config.GRID_Y(2);
  config.OUSTING(1);
  config.SYM_LIMIT(1);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");

  world_t world(random, &config);
  world.Setup();
  world.Resize(2, 2);

  emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config);
  emp::Ptr<sgp_sym_t> old_symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config);
  emp::Ptr<sgp_sym_t> new_symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config);
  WHEN("Symbiont is added to host that has a symbiont") {
    host->AddSymbiont(old_symbiont);
    world.AddOrgAt(host, 0);

    REQUIRE(host->GetSymbionts().size() == 1);
    REQUIRE(world.GetGraveyard().size() == 0);
    host->AddSymbiont(new_symbiont);
    THEN("Less matching symbiont is ousted to graveyard") {
        REQUIRE(host->GetSymbionts().size() == 1);
        REQUIRE(world.GetGraveyard().size() == 1);
    }
  }
  world.CleanupGraveyard();
}

TEST_CASE("Setup with an empty population"){
  sgpmode::SymConfigSGP config;
  config.SYM_LIMIT(1);
  config.GRID_X(2);
  config.GRID_Y(2);
  config.SEED(234);
  config.POP_SIZE(0);

  emp::Random random(config.SEED());
  world_t world(random, &config);
    

  WHEN("POP_SIZE is 0 and setup is called"){
    world.Setup();

    THEN("The world is sized correctly"){
      REQUIRE(world.GetSize() == 4);
    }
    THEN("The world is empty"){
      REQUIRE(world.GetNumOrgs() == 0);
    }
  }
}

TEST_CASE("FindHostForHorizontalTrans when task matching is not required for horizontal transmission and there is a nearby matching host", "[sgp][sgp-unit]") {
  GIVEN("A host infected with a symbiont"){
    sgpmode::SymConfigSGP config;
    config.SYM_LIMIT(1);
    config.GRID_X(2);
    config.GRID_Y(2);
    config.SEED(11);
    config.POP_SIZE(0);
    config.FIND_NEIGHBOR_HOST_ATTEMPTS(1);
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.HORIZONTAL_TRANSMISSION_COMPATIBILITY_MODE("always");
    config.TASK_PROFILE_COMPATIBILITY_MODE("task-any-match");

    emp::Random random(config.SEED());
    world_t world(random, &config);
    world.Setup();
    
    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config);
    emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config);
    host->AddSymbiont(symbiont);

    world.AddOrgAt(host, 0);
    size_t source_id = symbiont->GetLocation().GetPopID();

    WHEN("There exists a nearby host") {
      emp::WorldPosition neighbor_position = emp::WorldPosition(1,0);
      emp::Ptr<sgp_host_t> neighbor_host = emp::NewPtr<sgp_host_t>(&random, &world, &config);
      world.AddOrgAt(neighbor_host, neighbor_position);

      WHEN("The nearby host has matching tasks with the incoming symbiont") {
        neighbor_host->GetHardware().GetCPUState().MarkTaskPerformed(8);
        symbiont->GetHardware().GetCPUState().MarkTaskPerformed(8);

        WHEN("Task matching is not required for horizontal transmission") {
          auto pos_found = world.FindHostForHorizontalTrans(source_id, symbiont);
          THEN("The position of the nearby, matching host is returned"){
            REQUIRE(pos_found);
            REQUIRE(world.IsOccupied(*pos_found));
            REQUIRE(pos_found->GetIndex() == neighbor_position.GetIndex());
            REQUIRE(pos_found->GetPopID() == neighbor_position.GetPopID());
          }
        }
      }
    }
  }
}

TEST_CASE("FindHostForHorizontalTrans when task matching is required for horizontal transmission and there is a nearby matching host", "[sgp][sgp-unit]") {
  GIVEN("A host infected with a symbiont"){
    sgpmode::SymConfigSGP config;
    config.SYM_LIMIT(1);
    config.GRID_X(2);
    config.GRID_Y(2);
    config.SEED(11);
    config.POP_SIZE(0);
    config.FIND_NEIGHBOR_HOST_ATTEMPTS(1);
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.HORIZONTAL_TRANSMISSION_COMPATIBILITY_MODE("task-profile-compatible");
    config.TASK_PROFILE_COMPATIBILITY_MODE("task-any-match");

    emp::Random random(config.SEED());
    world_t world(random, &config);
    world.Setup();

    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config);
    emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config);
    host->AddSymbiont(symbiont);

    world.AddOrgAt(host, 0);
    size_t source_id = symbiont->GetLocation().GetPopID();

    WHEN("There exists a nearby host") {
      emp::WorldPosition neighbor_position = emp::WorldPosition(1,0);
      emp::Ptr<sgp_host_t> neighbor_host = emp::NewPtr<sgp_host_t>(&random, &world, &config);
      world.AddOrgAt(neighbor_host, neighbor_position);

      WHEN("The nearby host has matching tasks with the incoming symbiont") {
        neighbor_host->GetHardware().GetCPUState().MarkTaskPerformed(8);
        symbiont->GetHardware().GetCPUState().MarkTaskPerformed(8);

        WHEN("Task matching is required for horizontal transmission") {
          auto pos_found = world.FindHostForHorizontalTrans(source_id, symbiont);
          THEN("The position of the nearby, matching host is returned"){
            REQUIRE(pos_found.has_value() == true);
            REQUIRE(pos_found->GetIndex() == neighbor_position.GetIndex());
            REQUIRE(pos_found->GetPopID() == neighbor_position.GetPopID());
          }
        }
      }
    }
  }
}

TEST_CASE("FindHostForHorizontalTrans when task matching is not required for horizontal transmission and there is a nearby non-matching host", "[sgp][sgp-unit]") {
  GIVEN("A host infected with a symbiont"){
    sgpmode::SymConfigSGP config;
    config.SYM_LIMIT(1);
    config.GRID_X(2);
    config.GRID_Y(2);
    config.SEED(11);
    config.POP_SIZE(0);
    config.FIND_NEIGHBOR_HOST_ATTEMPTS(1);
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.HORIZONTAL_TRANSMISSION_COMPATIBILITY_MODE("always");
    config.TASK_PROFILE_COMPATIBILITY_MODE("task-any-match");

    emp::Random random(config.SEED());
    world_t world(random, &config);
    world.Setup();

    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config);
    emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config);
    host->AddSymbiont(symbiont);

    world.AddOrgAt(host, 0);
    size_t source_id = symbiont->GetLocation().GetPopID();

    WHEN("There exists a nearby host") {
      emp::WorldPosition neighbor_position = emp::WorldPosition(1,0);
      emp::Ptr<sgp_host_t> neighbor_host = emp::NewPtr<sgp_host_t>(&random, &world, &config);
      world.AddOrgAt(neighbor_host, neighbor_position);

      WHEN("The nearby host does not have matching tasks with the incoming symbiont") {
        neighbor_host->GetHardware().GetCPUState().MarkTaskPerformed(8);
        symbiont->GetHardware().GetCPUState().MarkTaskPerformed(6);
        
        WHEN("Task matching is not required for horizontal transmission") {
          auto pos_found = world.FindHostForHorizontalTrans(source_id, symbiont);
          THEN("The position of the nearby, non-matching host is returned"){
            REQUIRE(pos_found.has_value() == true);
            REQUIRE(pos_found->GetIndex() == neighbor_position.GetIndex());
            REQUIRE(pos_found->GetPopID() == neighbor_position.GetPopID());
          }
        }
      }
    }
  }
}

TEST_CASE("FindHostForHorizontalTrans when task matching is required for horizontal transmission and there is a nearby non-matching host", "[sgp][sgp-unit]") {
  GIVEN("A host infected with a symbiont"){
    sgpmode::SymConfigSGP config;
    config.SYM_LIMIT(1);
    config.GRID_X(2);
    config.GRID_Y(2);
    config.SEED(11);
    config.POP_SIZE(0);
    config.FIND_NEIGHBOR_HOST_ATTEMPTS(1);
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.HORIZONTAL_TRANSMISSION_COMPATIBILITY_MODE("task-profile-compatible");
    config.TASK_PROFILE_COMPATIBILITY_MODE("task-any-match");

    emp::Random random(config.SEED());
    world_t world(random, &config);
    world.Setup();

    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config);
    emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config);
    host->AddSymbiont(symbiont);

    world.AddOrgAt(host, 0);
    size_t source_id = symbiont->GetLocation().GetPopID();

    WHEN("There exists a nearby host") {
      emp::WorldPosition neighbor_position = emp::WorldPosition(1,0);
      emp::Ptr<sgp_host_t> neighbor_host = emp::NewPtr<sgp_host_t>(&random, &world, &config);
      world.AddOrgAt(neighbor_host, neighbor_position);

      WHEN("The nearby host does not have matching tasks with the incoming symbiont") {
        neighbor_host->GetHardware().GetCPUState().MarkTaskPerformed(8);
        symbiont->GetHardware().GetCPUState().MarkTaskPerformed(6);

        
        WHEN("Task matching is required for horizontal transmission") {
          auto pos_found = world.FindHostForHorizontalTrans(source_id, symbiont);
          THEN("Nothing is returned (no acceptable neighboring host)"){
            REQUIRE(pos_found.has_value() == false);
          }
        }
      }
    }
  }
}

TEST_CASE("FindHostForHorizontalTrans when task matching is not required for horizontal transmission and there is not a nearby host", "[sgp][sgp-unit]") {
  GIVEN("A host infected with a symbiont"){
    sgpmode::SymConfigSGP config;
    config.SYM_LIMIT(1);
    config.GRID_X(2);
    config.GRID_Y(2);
    config.SEED(11);
    config.POP_SIZE(0);
    config.FIND_NEIGHBOR_HOST_ATTEMPTS(1);
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.HORIZONTAL_TRANSMISSION_COMPATIBILITY_MODE("always");
    config.TASK_PROFILE_COMPATIBILITY_MODE("task-any-match");

    emp::Random random(config.SEED());
    world_t world(random, &config);
    world.Setup();

    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config);
    emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config);
    host->AddSymbiont(symbiont);

    world.AddOrgAt(host, 0);
    size_t source_id = symbiont->GetLocation().GetPopID();

    WHEN("There does not exist a nearby host") {
      auto pos_found = world.FindHostForHorizontalTrans(source_id, symbiont);
      THEN("Nothing is returned (no acceptable neighboring host)"){
        REQUIRE(pos_found.has_value() == false);
      }
    }
  }
}
