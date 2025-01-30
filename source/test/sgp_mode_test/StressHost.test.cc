#include "../../sgp_mode/StressHost.h"

TEST_CASE("Extinction event", "[sgp]") {
  emp::Random random(61);
  SymConfigSGP config;
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

  SGPWorld world(random, &config, LogicTasks);

  WHEN("There are stress symbionts in the world"){
    config.START_MOI(1);
    world.SetupHosts(&world_size);
    for (size_t i = 0; i < config.EXTINCTION_FREQUENCY() - 1; i++) world.Update();
    REQUIRE(world.GetNumOrgs() == world_size);
    WHEN("Stress symbionts are mutualists"){
      config.STRESS_TYPE("mutualist");
      world.Update();
      THEN("Hosts are less likely to die during the extinction event") {
        REQUIRE(world.GetNumOrgs() < world_size * (1 - mutualist_death_chance) + 10);
        REQUIRE(world.GetNumOrgs() > world_size * (1 - mutualist_death_chance) - 10);
      }
    }
    WHEN("Stress symbionts are parasites") {
      config.STRESS_TYPE("parasite");
      world.Update();
      THEN("Hosts are more likely to die during the extinction event") {
        REQUIRE(world.GetNumOrgs() < world_size * (1 - parasite_death_chance) + 10);
        REQUIRE(world.GetNumOrgs() > world_size * (1 - parasite_death_chance) - 10);
      }
    }
    WHEN("Stress symbionts are neutrals"){
      config.STRESS_TYPE("neutral");
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