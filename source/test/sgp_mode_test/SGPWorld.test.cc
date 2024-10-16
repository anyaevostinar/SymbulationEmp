#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPWorldSetup.cc"

#include "../../catch/catch.hpp"

TEST_CASE("GetDominantInfo", "[sgp]") {
  emp::Random random(61);
  SymConfigSGP config;
  config.DOMINANT_COUNT(10);

  SGPWorld world(random, &config, TaskSet{});

  SGPHost host1(&random, &world, &config);
  SGPHost host2(&random, &world, &config);
  // Make sure they have different genomes
  host1.Mutate();
  host2.Mutate();

  WHEN("The world contains 2 of one org and 1 of another") {
    // One copy of host1 and two of host2
    world.AddOrgAt(emp::NewPtr<SGPHost>(host1), 0);
    world.AddOrgAt(emp::NewPtr<SGPHost>(host2), 1);
    world.AddOrgAt(emp::NewPtr<SGPHost>(host2), 2);

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
  SymConfigSGP config;
  config.FREE_LIVING_SYMS(1);
  config.GRID_X(2);
  config.GRID_Y(2);


  SGPWorld world(random, &config, LogicTasks);
  world.Resize(2,2);

  emp::Ptr<SGPHost> infected_host = emp::NewPtr<SGPHost>(&random, &world, &config);
  emp::Ptr<SGPHost> uninfected_host = emp::NewPtr<SGPHost>(&random, &world, &config);
  emp::Ptr<SGPSymbiont> hosted_symbiont = emp::NewPtr<SGPSymbiont> (&random, &world, &config);
  emp::Ptr<SGPSymbiont> free_symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config);

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
  SymConfigSGP config;
  SGPWorld world(random, &config, LogicTasks);
  config.SYM_LIMIT(2);

  ProgramBuilder builder;
  builder.AddNand();

  emp::Ptr<SGPHost> NOT_host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));
  emp::Ptr<SGPSymbiont> NOT_symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(100));
  emp::Ptr<SGPSymbiont> NAND_symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config, builder.Build(100));
  
  NOT_host->AddSymbiont(NOT_symbiont);
  NOT_host->AddSymbiont(NAND_symbiont);
  world.AddOrgAt(NOT_host, 0);


  bool not_not_matched = false;
  bool not_nand_matched = false;
  for (int i = 0; i < 100; i++) {
    world.Update();
    if (world.TaskMatchCheck(NOT_symbiont, NOT_host)) not_not_matched = true;
    if (world.TaskMatchCheck(NAND_symbiont, NOT_host)) not_nand_matched = true;
  }

  WHEN("A host and symbiont can both do at least one same task") {
    THEN("TaskMatchCheck returns true"){
      REQUIRE(not_not_matched == true);
    }
  }
  WHEN("A host and symbiont have no tasks they can both do") {
    THEN("TaskMatchCheck returns false") {
      REQUIRE(not_nand_matched == false);
    }
  }
}
