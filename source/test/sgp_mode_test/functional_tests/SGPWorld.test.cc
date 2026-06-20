#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"

/**
 * This file is dedicated to testing SGPWorld functionality
 */

// TODO - refactor task match checks into compatibiliity mode checks
//        (test all compatibility modes)
TEST_CASE("A world containing a single infected host and its symbiont is updated correctly", "[sgp][sgp-functional]") {
  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.FREE_LIVING_SYMS(1);
  config.WORLD_WIDTH(2);
  config.WORLD_HEIGHT(2);
  config.INIT_POP_SIZE(0);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");

  sgpmode::SGPWorld world(random, &config);
  world.Setup();
  world.Resize(2,2);

  auto& prog_builder = world.GetProgramBuilder();

  emp::Ptr<sgpmode::SGPHost<hw_spec_t>> infected_host = emp::NewPtr<sgpmode::SGPHost<hw_spec_t>>(&random, &world, &config, prog_builder.CreateNotProgram(100));
  emp::Ptr<sgpmode::SGPSymbiont<hw_spec_t>> hosted_symbiont = emp::NewPtr<sgpmode::SGPSymbiont<hw_spec_t>> (&random, &world, &config, prog_builder.CreateNotProgram(100));

  infected_host->AddSymbiont(hosted_symbiont);
  world.AddOrgAt(infected_host, 0);

  THEN("An infected host can be added to the world") {
    REQUIRE(world.GetNumOrgs() == 1);
  }

  for (int i = 0; i < 10; i++) {
    world.Update();
  }

  THEN("An infected host persists and is managed by the world") {
    REQUIRE(world.GetNumOrgs() == 1);
  }
}

TEST_CASE("A world containing a single uninfected host is updated correctly", "[sgp][sgp-functional]") {
  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.FREE_LIVING_SYMS(1);
  config.WORLD_WIDTH(2);
  config.WORLD_HEIGHT(2);
  config.INIT_POP_SIZE(0);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");

  sgpmode::SGPWorld world(random, &config);
  world.Setup();
  world.Resize(2,2);

  auto& prog_builder = world.GetProgramBuilder();

  emp::Ptr<sgpmode::SGPHost<hw_spec_t>> uninfected_host = emp::NewPtr<sgpmode::SGPHost<hw_spec_t>>(&random, &world, &config, prog_builder.CreateNotProgram(100));

  world.AddOrgAt(uninfected_host, 1);

  THEN("An uninfected host can be added to the world") {
    REQUIRE(world.GetNumOrgs() == 1);
  }

  for (int i = 0; i < 10; i++) {
    world.Update();
  }

  THEN("An uninfected host persists and is managed by the world") {
    REQUIRE(world.GetNumOrgs() == 1);
  }
}

/* TODO need update CollectCurrentUpdateData to support free living symbionts before uncommenting this test

TEST_CASE("A world containing a single free living symbiont is updated correctly", "[sgp][sgp-functional]") {
  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.FREE_LIVING_SYMS(1);
  config.WORLD_WIDTH(2);
  config.WORLD_HEIGHT(2);
  config.INIT_POP_SIZE(0);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");

  sgpmode::SGPWorld world(random, &config);
  world.Setup();
  world.Resize(2,2);

  auto& prog_builder = world.GetProgramBuilder();
  emp::Ptr<sgpmode::SGPSymbiont<hw_spec_t>> free_symbiont = emp::NewPtr<sgpmode::SGPSymbiont<hw_spec_t>>(&random, &world, &config, prog_builder.CreateNotProgram(100));

  world.AddOrgAt(free_symbiont, emp::WorldPosition(0, 0));

  THEN("A free living symbiont can be added to the world") {
    REQUIRE(world.GetNumOrgs() == 1);
  }

  THEN("A free living symbiont can run one CPU step"){
    free_symbiont->GetHardware().RunCPUStep(1);
  }

  THEN("A free living symbiont can run two CPU steps"){
    free_symbiont->GetHardware().RunCPUStep(1);
    free_symbiont->GetHardware().RunCPUStep(1);
  }

  for (int i = 0; i < 10; i++) {
    world.Update();
  }

  THEN("A free living symbiont persists and is managed by the world") {
    REQUIRE(world.GetNumOrgs() == 1);
  }
}
*/