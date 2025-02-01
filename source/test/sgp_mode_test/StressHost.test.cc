#include "../../sgp_mode/StressHost.h"

TEST_CASE("Extinction event", "[sgp]") {
  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.ORGANISM_TYPE("stress");
  config.EXTINCTION_FREQUENCY(10);
  config.GRID_X(10);
  config.GRID_Y(10);
  size_t world_size = config.GRID_X() * config.GRID_Y();

  double parasite_death_chance = 0.5;
  double mutualist_death_chance = 0.125;
  double base_death_chance = 0.25;
  config.PARASITE_DEATH_CHANCE(parasite_death_chance);
  config.MUTUALIST_DEATH_CHANCE(mutualist_death_chance);
  config.BASE_DEATH_CHANCE(base_death_chance);

  sgpmode::SGPWorld world(random, &config, sgpmode::LogicTasks);
  world.SetupScheduler();

  WHEN("There are stress symbionts in the world"){
    config.START_MOI(1);
    // world.SetupHosts(&world_size);
    world.Setup();
    for (size_t i = 0; i < config.EXTINCTION_FREQUENCY() - 1; i++) world.Update();
    REQUIRE(world.GetNumOrgs() == world_size);
    WHEN("Stress symbionts are mutualists") {
      config.STRESS_TYPE("mutualist");
      world.SetupOrgMode(); // TODO - This is a little funky. Come back and refactor.
      world.Update();
      THEN("Hosts are less likely to die during the extinction event") {
        REQUIRE(world.GetNumOrgs() < world_size * (1 - mutualist_death_chance) + 10);
        REQUIRE(world.GetNumOrgs() > world_size * (1 - mutualist_death_chance) - 10);
      }
    }
    WHEN("Stress symbionts are parasites") {
      config.STRESS_TYPE("parasite");
      world.SetupOrgMode();
      world.Update();
      THEN("Hosts are more likely to die during the extinction event") {
        REQUIRE(world.GetNumOrgs() < world_size * (1 - parasite_death_chance) + 10);
        REQUIRE(world.GetNumOrgs() > world_size * (1 - parasite_death_chance) - 10);
      }
    }
    WHEN("Stress symbionts are neutrals"){
      config.STRESS_TYPE("neutral");
      world.SetupOrgMode();
      world.Update();
      THEN("Hosts die according to the default extinction probability during the extinction event") {
        REQUIRE(world.GetNumOrgs() < world_size * (1 - base_death_chance) + 10);
        REQUIRE(world.GetNumOrgs() > world_size * (1 - base_death_chance) - 10);
      }
    }
  }
  WHEN("There are no stress symbionts in the world") {
    config.START_MOI(0);
    world.SetupOrgMode();
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
  config.ORGANISM_TYPE(STRESS);
  config.START_MOI(0);
  config.EXTINCTION_FREQUENCY(100000);
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
    THEN("Stress hosts do not accrue mutations late in an experiment") {
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
    THEN("Stress hosts accrue more mutations late in an experiment") {
      REQUIRE(world.GetNumOrgs() == world_size);
      auto it = world.GetTaskSet().begin();
      ++it;
      REQUIRE((*it).n_succeeds_host > 30);
      REQUIRE((*world.GetTaskSet().begin()).n_succeeds_host > no_mut_NOT_rate * 2);
    }
  }
}