#include "../../sgp_mode/StressHost.h"

TEST_CASE("Extinction event probabilities", "[sgp]") {
  emp::Random random(61);
  SymConfigSGP config;
  config.INTERACTION_MECHANISM(STRESS);
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

TEST_CASE("Parasites transfer during stress event", "[sgp]") {
  emp::Random random(61);
  SymConfigSGP config;
  config.INTERACTION_MECHANISM(STRESS);
  config.EXTINCTION_FREQUENCY(1);
  config.GRID_X(10);
  config.GRID_Y(10);
  size_t world_size = config.GRID_X() * config.GRID_Y();
  SGPWorld world(random, &config, LogicTasks);

  WHEN("There are stress parasites with 100% kill rate in the world"){
    config.START_MOI(1);
    double parasite_death_chance = 1.0; 
    config.PARASITE_DEATH_CHANCE(parasite_death_chance);
    config.BASE_DEATH_CHANCE(0);
    config.SYMBIONT_TYPE(PARASITE);
    

    //50 empty hosts that "do" NAND
    for(int i=0; i<50; i++) {
      emp::Ptr<StressHost> host = emp::NewPtr<StressHost>(&random, &world, &config, CreateReproProgram(100));
      host->GetCPU().state.tasks_performed->Set(1); //"Does" NAND
      world.AddOrgAt(host, i);
    }

    //50 infected hosts that "do" NOT, infected with "NAND" parasites
    for(int i = 50; i<100; i++) {
      emp::Ptr<StressHost> host = emp::NewPtr<StressHost>(&random, &world, &config, CreateNotProgram(100));
      host->GetCPU().state.tasks_performed->Set(0);
      emp::Ptr<SGPSymbiont> symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateReproProgram(100));
      symbiont->GetCPU().state.tasks_performed->Set(1);
      host->AddSymbiont(symbiont);
      world.AddOrgAt(host,i);
    }



    for (size_t i = 0; i < config.EXTINCTION_FREQUENCY() - 1; i++) world.Update();
    REQUIRE(world.GetNumOrgs() == world_size);
      
    world.Update();

    THEN("Half the hosts died but the remaining got infected by fleeing parasites") {        
      REQUIRE(world.GetNumOrgs() == 50);
      int surviving_syms = 0;
      for(int i = 0; i < 50; i ++) {
        if(world.GetPop()[i]->HasSym()) {
          surviving_syms += 1;
        }
      }
      REQUIRE(surviving_syms > 0);
    }
  }
}