#include "emp/math/Random.hpp"

#include "../../sgp_mode/hardware/SGPHardware.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPWorld.cc"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPWorldSetup.cc"
#include "../../sgp_mode/SGPWorldData.cc"
#include "../../sgp_mode/ProgramBuilder.h"

#include "../../catch/catch.hpp"

TEST_CASE("Stress event", "[sgp]") {
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;

  sgpmode::SymConfigSGP config;
  config.CYCLES_PER_UPDATE(4);
  config.RANDOM_ANCESTOR(false);
  config.HOST_REPRO_RES(1000); // No repro for these tests
  config.SYM_HORIZ_TRANS_RES(1000);
  config.SYM_VERT_TRANS_RES(1000);
  config.SEED(61);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.FILE_PATH("SGPStressMode_test_output");
  config.POP_SIZE(100);
  config.TASK_IO_UNIQUE_OUTPUT(true);
  // Zero out mutation rates
  config.MUTATION_RATE(0);
  config.MUTATION_SIZE(0);
  config.SGP_MUT_PER_BIT_RATE(0);
  // Stress settings
  config.ENABLE_STRESS(true);
  config.STRESS_FREQUENCY(10);
  config.PARASITE_DEATH_CHANCE(0.75);
  config.MUTUALIST_DEATH_CHANCE(0.25);
  config.BASE_DEATH_CHANCE(0.5);
  // Initialize random number generator
  emp::Random random(config.SEED());
  const size_t world_size = config.POP_SIZE();

  WHEN("Stress symbionts are mutualists") {
    config.STRESS_TYPE("mutualist");
    config.START_MOI(1);
    world_t world(random, &config);
    world.Setup();
    // Run world up until extinction frequency
    for (size_t i = 0; i < config.STRESS_FREQUENCY(); ++i) {
      world.Update();
    }
    // Everyone should be alive, world should be same size as when started
    REQUIRE(world.GetNumOrgs() == world_size);
    // Next update should trigger stress event
    world.Update();
    THEN("Hosts are less likely to die during the stress event") {
      REQUIRE(world.GetNumOrgs() < (world_size * (1 - config.MUTUALIST_DEATH_CHANCE())) + 15);
      REQUIRE(world.GetNumOrgs() > (world_size * (1 - config.MUTUALIST_DEATH_CHANCE())) - 15);
    }
  }

  WHEN("Stress symbionts are parasites") {
    config.STRESS_TYPE("parasite");
    config.START_MOI(1);
    world_t world(random, &config);
    world.Setup();
    // Run world up until extinction frequency
    for (size_t i = 0; i < config.STRESS_FREQUENCY(); ++i) {
      world.Update();
    }
    // Everyone should be alive, world should be same size as when started
    REQUIRE(world.GetNumOrgs() == world_size);
    // Next update should trigger stress event
    world.Update();
    THEN("Hosts are more likely to die during the stress event") {
      REQUIRE(world.GetNumOrgs() < (world_size * (1 - config.PARASITE_DEATH_CHANCE())) + 15);
      REQUIRE(world.GetNumOrgs() > (world_size * (1 - config.PARASITE_DEATH_CHANCE())) - 15);
    }
  }

  WHEN("Stress symbionts are nuetral") {
    config.STRESS_TYPE("neutral");
    config.START_MOI(1);
    world_t world(random, &config);
    world.Setup();
    // Run world up until extinction frequency
    for (size_t i = 0; i < config.STRESS_FREQUENCY(); ++i) {
      world.Update();
    }
    // Everyone should be alive, world should be same size as when started
    REQUIRE(world.GetNumOrgs() == world_size);
    // Next update should trigger stress event
    world.Update();
    THEN("Hosts die according to default probability") {
      REQUIRE(world.GetNumOrgs() < (world_size * (1 - config.BASE_DEATH_CHANCE())) + 15);
      REQUIRE(world.GetNumOrgs() > (world_size * (1 - config.BASE_DEATH_CHANCE())) - 15);
    }
  }

  WHEN("There are no stress symbionts in the world") {
    config.STRESS_TYPE("parasite"); // Set to parasite for high contrast with base death chance
    config.START_MOI(0);  // No symbionts on initialization
    world_t world(random, &config);
    world.Setup();
    // Run world up until extinction frequency
    for (size_t i = 0; i < config.STRESS_FREQUENCY(); ++i) {
      world.Update();
    }
    // Everyone should be alive, world should be same size as when started
    REQUIRE(world.GetNumOrgs() == world_size);
    // Next update should trigger stress event
    world.Update();
    THEN("Hosts die according to default probability") {
      REQUIRE(world.GetNumOrgs() < (world_size * (1 - config.BASE_DEATH_CHANCE())) + 15);
      REQUIRE(world.GetNumOrgs() > (world_size * (1 - config.BASE_DEATH_CHANCE())) - 15);
    }
  }

}

// TODO (future): Evolution test