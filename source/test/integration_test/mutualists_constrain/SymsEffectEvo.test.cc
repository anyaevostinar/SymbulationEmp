#include "../../../sgp_mode/GenomeLibrary.h"
#include "../../../sgp_mode/CPU.h"
#include "../../../sgp_mode/HealthHost.h"
#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"
#include "../../../sgp_mode/SGPConfigSetup.h"
#include "../../../sgp_mode/SGPHost.cc"
#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPSymbiont.h"
#include "../../../default_mode/WorldSetup.cc"
#include "../../../default_mode/DataNodes.h"
#include "../../../sgp_mode/SGPDataNodes.h"
#include "../../../sgp_mode/StressHost.h"


TEST_CASE("Health hosts evolve less NOT with parasites than without", "[sgp][integration]") {
  emp::Random random(10);
  //TODO: The random number seed doesn't seem to be working, different values for the same seed

  SymConfigSGP config;
  config.SYNERGY(1);
  config.SEED(10);  
  config.INTERACTION_MECHANISM(1); //Health hosts
  config.SYMBIONT_TYPE(1); //Parasites
  config.LIMITED_RES_TOTAL(10);
  config.LIMITED_RES_INFLOW(500);
  config.VERTICAL_TRANSMISSION(0);
  config.HOST_REPRO_RES(100);
  config.SYM_HORIZ_TRANS_RES(0);
 

  config.OUSTING(1);


  size_t world_size = config.GRID_X() * config.GRID_Y();
  SGPWorld world(random, &config, LogicTasks);


  size_t run_updates = 8000;
  WHEN("There are parasites") {
    config.START_MOI(1);
    
    world.Setup();
  
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
    THEN("Health hosts evolve to do less than 8k NOT tasks") {
      REQUIRE(world.GetNumOrgs() == world_size);
      REQUIRE(world.GetTaskSet().NumTasks() == 9);
      REQUIRE((*it).task.name == "NOT");
      REQUIRE((*it).n_succeeds_host < 8000);
    }
  }
  WHEN("There are no parasites") {
    config.START_MOI(0);
    
    world.Setup();
  
    REQUIRE(world.GetNumOrgs() == world_size);
    //TODO: Use RunExperiment(false)
    for (size_t i = 0; i < run_updates; i++) {
      if (i % 1000 == 0) {
        world.GetTaskSet().ResetTaskData();
      }
      world.Update();
    }
    auto it = world.GetTaskSet().begin();
    THEN("Non-existant parasites do no NOT") {
      REQUIRE((*it).n_succeeds_sym == 0);
    }
    THEN("Health hosts keep doing NOT tasks") {
      REQUIRE(world.GetNumOrgs() == world_size);
      REQUIRE(world.GetTaskSet().NumTasks() == 9);
      REQUIRE((*it).task.name == "NOT");
      REQUIRE((*it).n_succeeds_host > 9000);
    }
  }

}