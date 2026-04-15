#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPWorldSetup.cc"
#include "../../sgp_mode/utils.h"

#include "../../catch/catch.hpp"

// TODO - refactor task match checks into compatibiliity mode checks
//        (test all compatibility modes)
TEST_CASE("GetDominantInfo", "[sgp]") {
  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.DOMINANT_COUNT(10);

  sgpmode::SGPWorld world(random, &config, sgpmode::TaskSet{});

  sgpmode::SGPHost host1(&random, &world, &config);
  sgpmode::SGPHost host2(&random, &world, &config);
  // Make sure they have different genomes
  host1.Mutate();
  host2.Mutate();

  WHEN("The world contains 2 of one org and 1 of another") {
    // One copy of host1 and two of host2
    world.AddOrgAt(emp::NewPtr<sgpmode::SGPHost>(host1), 0);
    world.AddOrgAt(emp::NewPtr<sgpmode::SGPHost>(host2), 1);
    world.AddOrgAt(emp::NewPtr<sgpmode::SGPHost>(host2), 2);

    CHECK(world.GetNumOrgs() == 3);

    THEN("The first org is dominant") {
      auto dominant = world.GetDominantInfo();
      CHECK(dominant.size() == 2);

      CHECK(*dominant[0].first == host2);
      CHECK(*dominant[0].first != host1);
      CHECK(dominant[0].second == 2);

      CHECK(*dominant[1].first == host1);
      CHECK(*dominant[1].first != host2);
      CHECK(dominant[1].second == 1);
    }
  }
}

TEST_CASE("Baseline function", "[sgp]") {
  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.FREE_LIVING_SYMS(1);
  config.GRID_X(2);
  config.GRID_Y(2);


  sgpmode::SGPWorld world(random, &config, sgpmode::LogicTasks);
  world.SetupOrgMode();
  world.SetupScheduler();
  world.SetupHostReproduction();
  world.SetupSymReproduction();
  world.SetupHostSymInteractions();
  world.Resize(2,2);

  emp::Ptr<sgpmode::SGPHost> infected_host = emp::NewPtr<sgpmode::SGPHost>(&random, &world, &config);
  emp::Ptr<sgpmode::SGPHost> uninfected_host = emp::NewPtr<sgpmode::SGPHost>(&random, &world, &config);
  emp::Ptr<sgpmode::SGPSymbiont> hosted_symbiont = emp::NewPtr<sgpmode::SGPSymbiont> (&random, &world, &config);
  emp::Ptr<sgpmode::SGPSymbiont> free_symbiont = emp::NewPtr<sgpmode::SGPSymbiont>(&random, &world, &config);

  infected_host->AddSymbiont(hosted_symbiont);
  world.AddOrgAt(infected_host, 0);
  world.AddOrgAt(uninfected_host, 1);
  world.AddOrgAt(free_symbiont, emp::WorldPosition(0, 0));

  THEN("Organisms can be added to the world") {
    REQUIRE(world.GetNumOrgs() == 3);
  }

  for (int i = 0; i < 10; i++) {
    world.Update();
  }

  THEN("Organisms persist and are managed by the world") {
    REQUIRE(world.GetNumOrgs() == 3);
  }
}

TEST_CASE("TaskMatchCheck", "[sgp]") {
  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.SYM_LIMIT(2);
  sgpmode::SGPWorld world(random, &config, sgpmode::LogicTasks);
  world.SetupOrgMode();
  world.SetupScheduler();
  world.SetupHostReproduction();
  world.SetupSymReproduction();
  world.SetupHostSymInteractions();

  sgpmode::ProgramBuilder builder;
  builder.AddNand();

  emp::Ptr<sgpmode::SGPHost> NOT_host = emp::NewPtr<sgpmode::SGPHost>(&random, &world, &config, sgpmode::CreateNotProgram(100));
  emp::Ptr<sgpmode::SGPSymbiont> NOT_symbiont = emp::NewPtr<sgpmode::SGPSymbiont>(&random, &world, &config, sgpmode::CreateNotProgram(100));
  emp::Ptr<sgpmode::SGPSymbiont> NAND_symbiont = emp::NewPtr<sgpmode::SGPSymbiont>(&random, &world, &config, builder.Build(100));

  NOT_host->AddSymbiont(NOT_symbiont);
  NOT_host->AddSymbiont(NAND_symbiont);
  world.AddOrgAt(NOT_host, 0);


  bool not_not_matched = false;
  bool not_nand_matched = false;
  for (int i = 0; i < 100; i++) {
    world.Update();

    not_not_matched = sgpmode::utils::AnyMatch(
      *(NOT_symbiont->GetCPU().state.tasks_performed),
      *(NOT_host->GetCPU().state.tasks_performed)
    );
    not_nand_matched = sgpmode::utils::AnyMatch(
      *(NAND_symbiont->GetCPU().state.tasks_performed),
      *(NOT_host->GetCPU().state.tasks_performed)
    );
  }

  WHEN("A host and symbiont can both do at least one same task") {
    THEN("TaskMatchCheck returns true") {
      REQUIRE(not_not_matched == true);
    }
  }
  WHEN("A host and symbiont have no tasks they can both do") {
    THEN("TaskMatchCheck returns false") {
      REQUIRE(not_nand_matched == false);
    }
  }
}

TEST_CASE("Ousting is permitted", "[sgp]") {
  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.GRID_X(2);
  config.GRID_Y(2);
  config.OUSTING(1);
  config.SYM_LIMIT(1);

  sgpmode::SGPWorld world(random, &config, sgpmode::LogicTasks);
  world.SetupOrgMode();
  world.SetupScheduler();
  world.SetupHostReproduction();
  world.SetupSymReproduction();
  world.SetupHostSymInteractions();
  world.Resize(2, 2);

  emp::Ptr<sgpmode::SGPHost> host = emp::NewPtr<sgpmode::SGPHost>(&random, &world, &config);
  emp::Ptr<sgpmode::SGPSymbiont> old_symbiont = emp::NewPtr<sgpmode::SGPSymbiont>(&random, &world, &config);
  emp::Ptr<sgpmode::SGPSymbiont> new_symbiont = emp::NewPtr<sgpmode::SGPSymbiont>(&random, &world, &config);

  host->AddSymbiont(old_symbiont);
  world.AddOrgAt(host, 0);

  REQUIRE(host->GetSymbionts().size() == 1);
  REQUIRE(world.GetGraveyard().size() == 0);

  host->AddSymbiont(new_symbiont);

  REQUIRE(host->GetSymbionts().size() == 1);
  REQUIRE(world.GetGraveyard().size() == 1);

  world.Update(); // clean up the graveyard

  REQUIRE(world.GetGraveyard().size() == 0);
}
