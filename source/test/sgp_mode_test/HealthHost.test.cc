#include "../../sgp_mode/HealthHost.h"

//Tests to write:
// Health host with parasite loses cycle 50% of time
// Health host with mutualist gains cycle 50% of time
  // in signalgp-lite/include/sgpl/hardware/Cpu.hpp there is GetCore(0)
  // in signalgp-lite/include/sgpl/hardware/Core.hpp there is GetProgramCounter()
  // that hopefully will show whether organism has advanced program counter
// Also test with health host with NOT, give it just barely enough CPUs to finish, check whether it manages to complete NOT
// Trickier for mutualist, check just before enough CPUs and it should manage to finish

TEST_CASE("Health hosts evolve more ORN with parasites than without", "[sgp-integration]") {
  emp::Random random(10);
  //TODO: The random number seed doesn't seem to be working, different values for the same seed

  SymConfigSGP config;
  config.SEED(10);
  config.ORGANISM_TYPE(HEALTH);
  config.STRESS_TYPE(PARASITE);
  config.LIMITED_RES_TOTAL(10);
  config.LIMITED_RES_INFLOW(500);
  config.VERTICAL_TRANSMISSION(0);
  config.HOST_REPRO_RES(100);
  config.SYM_HORIZ_TRANS_RES(10);
  config.THREAD_COUNT(1);
  config.TASK_TYPE(1);
  config.DONATION_STEAL_INST(0);
  config.LIMITED_TASK_RESET_INTERVAL(20);

  config.OUSTING(1); //TODO: test removing the HealthHost ousting code


  size_t world_size = config.GRID_X() * config.GRID_Y();
  SGPWorld world(random, &config, LogicTasks);


  size_t run_updates = 20000;
  WHEN("There are parasites") {
    config.START_MOI(1);
    
    world.Setup();
  
    REQUIRE(world.GetNumOrgs() == world_size);
    for (size_t i = 0; i < run_updates; i++) {
      if (i % 1000 == 0) {
        world.GetTaskSet().ResetTaskData();
      }
      world.Update();
    }
    //std::cout << "Random: " << random.GetSeed() << std::endl;
    //std::cout << "Random number: " << random.GetUInt() << std::endl;
    auto it = world.GetTaskSet().begin();
    THEN("Parasites do some NOT") {
      REQUIRE((*it).n_succeeds_sym > 0);
    }
    THEN("Health hosts evolve to do 30 or more ORN tasks") {
      REQUIRE(world.GetNumOrgs() == world_size);
      //advance iterator to the ORN task
      for (size_t i = 0; i < 3; i++) {
        ++it;
      }
      REQUIRE((*it).task.name == "ORN");
      REQUIRE((*it).n_succeeds_host > 100);
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
    //std::cout << "Random: " << random.GetSeed() << std::endl;
    //std::cout << "Random number: " << random.GetUInt() << std::endl;
    auto it = world.GetTaskSet().begin();
    THEN("Non-existant parasites do no NOT") {
      REQUIRE((*it).n_succeeds_sym == 0);
    }
    THEN("Health hosts evolve to do fewer than 10 ORN tasks") {
      REQUIRE(world.GetNumOrgs() == world_size);
      //advance iterator to the ORN task
      for (size_t i = 0; i < 3; i++) {
        ++it;
      }
      REQUIRE((*it).task.name == "ORN");
      REQUIRE((*it).n_succeeds_host < 100);
    }
  }

}

TEST_CASE("Health hosts evolve", "[sgp-integration]") {
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
