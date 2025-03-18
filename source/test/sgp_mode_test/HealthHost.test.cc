#include "../../sgp_mode/HealthHost.h"

//TODO: test case: with parasites, health hosts at 20k updates evolve more than 30 ORN and 200 NAND
//Whereas health hosts without parasites evolve at 20k updates evolve less than 10 ORN and 100 NAND
//random seed 10

TEST_CASE("Health hosts evolve more ORN with parasites than without", "[sgp]") {
  emp::Random random(10);
  SymConfigSGP config;
  config.ORGANISM_TYPE(HEALTH);
  config.STRESS_TYPE(PARASITE);


  size_t world_size = config.GRID_X() * config.GRID_Y();
  SGPWorld world(random, &config, LogicTasks);


  size_t run_updates = 20000;
  WHEN("There are parasites") {
    config.START_MOI(0);
    
    world.SetupHosts(&world_size);
  
    REQUIRE(world.GetNumOrgs() == world_size);
    for (size_t i = 0; i < run_updates; i++) {
      if (i % 100 == 0) {
        world.GetTaskSet().ResetTaskData();
      }
      world.Update();
    }
    auto it = world.GetTaskSet().begin();
    THEN("Parasites do some NOT") {
      REQUIRE((*it).n_succeeds_sym > 0);
    }
    THEN("Health hosts evolve to do 30 or more ORN tasks") {
      REQUIRE(world.GetNumOrgs() == world_size);
      //advance iterator to the ORN task
      for (size_t i = 0; i < 4; i++) {
        std::cout << "advancing" << std::endl;
        std::cout << (*it).task.name << (*it).n_succeeds_host << std::endl;
        ++it;
      }
      std::cout << (*it).task.name << (*it).n_succeeds_host << std::endl;
      REQUIRE((*it).n_succeeds_host > 30);
    }
  }

}

TEST_CASE("Health hosts evolve", "[sgp]") {
  emp::Random random(32);
  SymConfigSGP config;
  config.ORGANISM_TYPE(HEALTH);
  config.START_MOI(0);
  config.GRID_X(10);
  config.GRID_Y(100);
  config.HOST_REPRO_RES(20);
  size_t world_size = config.GRID_X() * config.GRID_Y();

  SGPWorld world(random, &config, LogicTasks);
  world.SetupHosts(&world_size);

  REQUIRE(world.GetNumOrgs() == world_size);

  size_t no_mut_NOT_rate = 600000;

  size_t run_updates = 15000;

  WHEN("Mutation size is 0") {
    config.MUTATION_SIZE(0); //  chance
    for (size_t i = 0; i < run_updates; i++) {
      world.Update();
    }
    THEN("Health hosts do not accrue mutations late in an experiment") {
      REQUIRE(world.GetNumOrgs() == world_size);
      auto it = world.GetTaskSet().begin();
      ++it;
      REQUIRE((*it).n_succeeds_host == 0);
      REQUIRE((*world.GetTaskSet().begin()).n_succeeds_host == no_mut_NOT_rate);
    }
  }

  WHEN("Mutation size is greater than 0") {
    config.MUTATION_SIZE(0.0002);
    for (size_t i = 0; i < run_updates; i++) {
      world.Update();
    }
    THEN("Health hosts accrue more mutations late in an experiment") {
      REQUIRE(world.GetNumOrgs() == world_size);
      auto it = world.GetTaskSet().begin();
      ++it;
      REQUIRE((*it).n_succeeds_host > 30);
      REQUIRE((*world.GetTaskSet().begin()).n_succeeds_host > no_mut_NOT_rate * 2);
    }
  }
} 
