#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPWorld.h"

#include "../../catch/catch.hpp"

TEST_CASE("Limited task reset", "[sgp]") {
  emp::Random random(61);
  SymConfigBase config;
  const size_t reset_interval = 12;
  config.CYCLES_PER_UPDATE(25);
  config.LIMITED_TASK_RESET_INTERVAL(reset_interval);
  config.TASK_TYPE(1);

  SGPWorld world(random, &config, TaskSet{emp::NewPtr<InputTask>(NOT)});
  SGPHost host(&random, &world, &config);
  world.AddOrgAt(emp::NewPtr<SGPHost>(host), 0);

  for (size_t i = 0; i < reset_interval - 1; i++) {
    world.Update();
  }
  REQUIRE((*world.GetTaskSet().begin()).n_succeeds_host == 1);
  for (size_t i = 0; i < reset_interval; i++) {
    world.Update();
  }
  REQUIRE((*world.GetTaskSet().begin()).n_succeeds_host == 2);
}