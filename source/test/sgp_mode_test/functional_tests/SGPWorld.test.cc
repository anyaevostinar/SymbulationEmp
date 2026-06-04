#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"

/**
 * This file is dedicated to testing SGPWorld functionality 
 */

// TODO - refactor task match checks into compatibiliity mode checks
//        (test all compatibility modes)
TEST_CASE("Baseline function", "[sgp][sgp-functional]") {
  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.FREE_LIVING_SYMS(1);
  config.GRID_X(2);
  config.GRID_Y(2);
  config.POP_SIZE(0);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  
  sgpmode::SGPWorld world(random, &config);
  world.Setup();
  world.Resize(2,2);

  emp::Ptr<sgpmode::SGPHost<hw_spec_t>> infected_host = emp::NewPtr<sgpmode::SGPHost<hw_spec_t>>(&random, &world, &config);
  emp::Ptr<sgpmode::SGPHost<hw_spec_t>> uninfected_host = emp::NewPtr<sgpmode::SGPHost<hw_spec_t>>(&random, &world, &config);
  emp::Ptr<sgpmode::SGPSymbiont<hw_spec_t>> hosted_symbiont = emp::NewPtr<sgpmode::SGPSymbiont<hw_spec_t>> (&random, &world, &config);
  emp::Ptr<sgpmode::SGPSymbiont<hw_spec_t>> free_symbiont = emp::NewPtr<sgpmode::SGPSymbiont<hw_spec_t>>(&random, &world, &config);

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
