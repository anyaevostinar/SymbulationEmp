#include "../../sgp_mode/StressHost.h"

TEST_CASE("Extinction event probabilities", "[sgp]") {
  emp::Random random(62);
  SymConfigSGP config;
  config.INTERACTION_MECHANISM(STRESS);
  config.EXTINCTION_FREQUENCY(26);
  config.GRID_X(10);
  config.GRID_Y(10);
  size_t world_size = config.GRID_X() * config.GRID_Y();

  double parasite_death_chance = 0.5; 
  double mutualist_death_chance = 0.125; 
  double base_death_chance = 0.25;
  config.PARASITE_DEATH_CHANCE(parasite_death_chance);
  config.MUTUALIST_DEATH_CHANCE(mutualist_death_chance);
  config.BASE_DEATH_CHANCE(base_death_chance);

  SGPWorld world(random, &config, LogicTasks);

  WHEN("There are stress symbionts in the world"){
    config.START_MOI(1);
    world.SetupHosts(&world_size);
    for (size_t i = 0; i < config.EXTINCTION_FREQUENCY() - 1; i++) world.Update();
    REQUIRE(world.GetNumOrgs() == world_size);
    WHEN("Stress symbionts are mutualists"){
      config.SYMBIONT_TYPE(MUTUALIST);
      world.Update();
      THEN("Hosts are less likely to die during the extinction event") {
        REQUIRE(world.GetNumOrgs() < world_size * (1 - mutualist_death_chance) + 10);
        REQUIRE(world.GetNumOrgs() > world_size * (1 - mutualist_death_chance) - 10);
      }
    }
    WHEN("Stress symbionts are parasites") {
      config.SYMBIONT_TYPE(PARASITE);
      world.Update();
      THEN("Hosts are more likely to die during the extinction event") {        
        REQUIRE(world.GetNumOrgs() < world_size * (1 - parasite_death_chance) + 10);
        REQUIRE(world.GetNumOrgs() > world_size * (1 - parasite_death_chance) - 10);
      }
    }
    WHEN("Stress symbionts are neutrals"){
      config.SYMBIONT_TYPE(NEUTRAL);
      world.Update();
      THEN("Hosts die according to the default extinction probability during the extinction event") {        
        REQUIRE(world.GetNumOrgs() < world_size * (1 - base_death_chance) + 10);
        REQUIRE(world.GetNumOrgs() > world_size * (1 - base_death_chance) - 10);
      }
    }
  }
  WHEN("There are no stress symbionts in the world") {
    config.START_MOI(0);
    world.SetupHosts(&world_size);
    for (size_t i = 0; i < config.EXTINCTION_FREQUENCY() - 1; i++) world.Update();
    REQUIRE(world.GetNumOrgs() == world_size);
    world.Update();
    THEN("Hosts die according to the default extinction probability during the extinction event") {
      REQUIRE(world.GetNumOrgs() < world_size * (1 - base_death_chance) + 10);
      REQUIRE(world.GetNumOrgs() > world_size * (1 - base_death_chance) - 10);
    }
  }
}

TEST_CASE("Stress hosts evolve", "[sgp]") {
  emp::Random random(32);
  SymConfigSGP config;
  config.INTERACTION_MECHANISM(STRESS);
  config.START_MOI(0);
  config.EXTINCTION_FREQUENCY(100000);
  config.GRID_X(10);
  config.GRID_Y(100);
  config.HOST_REPRO_RES(20);
  size_t world_size = config.GRID_X() * config.GRID_Y();

  SGPWorld world(random, &config, LogicTasks);
  world.SetupHosts(&world_size);

  REQUIRE(world.GetNumOrgs() == world_size);

  size_t no_mut_NAND_rate = 600000;

  size_t run_updates = 15000;

  WHEN("Mutation size is 0") {
    config.MUTATION_SIZE(0); //  chance
    for (size_t i = 0; i < run_updates; i++) {
      world.Update();
    }
    THEN("Stress hosts do not accrue mutations late in an experiment") {
      REQUIRE(world.GetNumOrgs() == world_size);
      auto it = world.GetTaskSet().begin();
      ++it;
      //There can never be exactly no_mut_NAND_rate NAND tasks completed as it will ocassionally guess NOT
      REQUIRE((*it).n_succeeds_host == no_mut_NAND_rate - (*world.GetTaskSet().begin()).n_succeeds_host);
      ++it;
      REQUIRE((*it).n_succeeds_host == 0);
    }
  }

  WHEN("Mutation size is greater than 0") {
    config.MUTATION_SIZE(0.0002);
    for (size_t i = 0; i < run_updates; i++) {
      world.Update();
    }
    THEN("Stress hosts accrue more mutations late in an experiment") {
      REQUIRE(world.GetNumOrgs() == world_size);
      auto it = world.GetTaskSet().begin();
      REQUIRE((*world.GetTaskSet().begin()).n_succeeds_host >= no_mut_NAND_rate * 5);
      ++it;
      REQUIRE((*it).n_succeeds_host > 3000);
      
    }
  }
} 


TEST_CASE("Safe time configuration option", "[sgp]") {
  emp::Random random(62);
  SymConfigSGP config;
  config.INTERACTION_MECHANISM(STRESS);
  config.EXTINCTION_FREQUENCY(10);
  config.SAFE_TIME(20);
  config.GRID_X(10);
  config.GRID_Y(10);
  size_t world_size = config.GRID_X() * config.GRID_Y();

  double parasite_death_chance = 0.5; 
  double mutualist_death_chance = 0.125; 
  double base_death_chance = 0.25;
  config.PARASITE_DEATH_CHANCE(parasite_death_chance);
  config.MUTUALIST_DEATH_CHANCE(mutualist_death_chance);
  config.BASE_DEATH_CHANCE(base_death_chance);

  SGPWorld world(random, &config, LogicTasks);

  WHEN("There are stress symbionts in the world"){
    config.START_MOI(1);
    world.SetupHosts(&world_size);
    for (size_t i = 0; i < config.EXTINCTION_FREQUENCY() - 1; i++) world.Update();
    REQUIRE(world.GetNumOrgs() == world_size);
    WHEN("Stress symbionts are mutualists and it's during the safe time"){
      config.SYMBIONT_TYPE(MUTUALIST);
      world.Update();
      THEN("No hosts die") {
        REQUIRE(world.GetNumOrgs() == world_size);
      }
    }
    WHEN("Stress symbionts are parasites and it's during the safe time") {
      config.SYMBIONT_TYPE(PARASITE);
      world.Update();
      THEN("No hosts die") {        
        REQUIRE(world.GetNumOrgs() == world_size);
      }
    }
  }
}