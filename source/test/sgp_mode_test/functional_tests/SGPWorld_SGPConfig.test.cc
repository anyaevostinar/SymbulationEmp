#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"

#include "../../../catch/catch.hpp"

/**
 * This file contains tests checking that the world behavior is correct after a small number of updates for specific 
 * configuration settings of interest.
 */



TEST_CASE("Baseline function", "[sgp][sgp-functional]") {
  emp::Random random(61);
  SymConfigSGP config;
  config.GRID_X(2);
  config.GRID_Y(2);


  SGPWorld world(random, &config, LogicTasks);
  world.Resize(2,2);

  emp::Ptr<SGPHost> infected_host = emp::NewPtr<SGPHost>(&random, &world, &config);
  emp::Ptr<SGPHost> uninfected_host = emp::NewPtr<SGPHost>(&random, &world, &config);
  emp::Ptr<SGPSymbiont> hosted_symbiont = emp::NewPtr<SGPSymbiont> (&random, &world, &config);

  infected_host->AddSymbiont(hosted_symbiont);
  world.AddOrgAt(infected_host, 0);
  world.AddOrgAt(uninfected_host, 1);
  
  THEN("Organisms can be added to the world") {
    REQUIRE(world.GetNumOrgs() == 2);
  }

  for (int i = 0; i < 10; i++) {
    world.Update();
  }

  THEN("Organisms persist and are managed by the world") {
    REQUIRE(world.GetNumOrgs() == 2);
  }
}

TEST_CASE("Ousting is permitted", "[sgp][sgp-functional]") {
  emp::Random random(61);
  SymConfigSGP config;
  config.GRID_X(2);
  config.GRID_Y(2);
  config.OUSTING(1);
  config.SYM_LIMIT(1);

  SGPWorld world(random, &config, LogicTasks);
  world.Resize(2, 2);

  emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config);
  emp::Ptr<SGPSymbiont> old_symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config);
  emp::Ptr<SGPSymbiont> new_symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config);

  host->AddSymbiont(old_symbiont);
  world.AddOrgAt(host, 0);

  REQUIRE(host->GetSymbionts().size() == 1);
  REQUIRE(world.GetGraveyard().size() == 0);

  host->AddSymbiont(new_symbiont);

  REQUIRE(host->GetSymbionts().size() == 1);
  REQUIRE(world.GetGraveyard().size() == 1);

  world.Update(); // clean up the graveyard

  REQUIRE(world.GetGraveyard().size() == 0);
}

TEST_CASE("Health hosts evolve", "[sgp][sgp-functional]") {
  emp::Random random(32);
  SymConfigSGP config;
  config.INTERACTION_MECHANISM(1); // Health hosts
  config.START_MOI(0);
  config.GRID_X(10);
  config.GRID_Y(100);
  config.HOST_REPRO_RES(20);
  config.SYNERGY(1);
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
    THEN("Health hosts do not accrue mutations late in an experiment") {
      REQUIRE(world.GetNumOrgs() == world_size);
      auto it = world.GetTaskSet().begin();
      ++it;
      //There can never be exactly no_mut_NAND_rate NAND tasks completed as it will occasionally guess NOT
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
    THEN("Health hosts accrue more mutations late in an experiment") {
      REQUIRE(world.GetNumOrgs() == world_size);
      auto it = world.GetTaskSet().begin();
      REQUIRE((*world.GetTaskSet().begin()).n_succeeds_host >= no_mut_NAND_rate * 5);
      ++it;
      REQUIRE((*it).n_succeeds_host > 3000);
    }
  }
} 


TEST_CASE("Stress hosts evolve", "[sgp][sgp-functional]") {
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
      //There can never be exactly no_mut_NAND_rate NAND tasks completed as it will occasionally guess NOT
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
      REQUIRE((*world.GetTaskSet().begin()).n_succeeds_host >= no_mut_NAND_rate * 4);
      ++it;
      REQUIRE((*it).n_succeeds_host > 3000);
      
    }
  }
} 


TEST_CASE("Organisms, without mutation will only receive credit for NOT operations", "[sgp][sgp-functional]") {
     
  GIVEN("An SGPWorld with no mutation"){
    emp::Random random(1);
    SymConfigSGP config;
    config.SEED(2);
    config.INTERACTION_MECHANISM(HEALTH);
    config.SYMBIONT_TYPE(PARASITE);
    config.MUTATION_RATE(0.0);
    config.MUTATION_SIZE(0.000);
    SGPWorld world(random, &config, LogicTasks);

    // Mock Organism to check reproduction
    class TestOrg : public Organism {
    public:
      bool IsHost() override { return true; }
      void AddPoints(double p) override {}
      double GetPoints() override { return 0; }
    };

    TestOrg organism;

    // NOT builder
    ProgramBuilder builder;
    (builder.AddNot)();
    CPU cpu(&organism, &world, builder.Build(100));
    
    
    
    cpu.RunCPUStep(0, 100);
    
    //The result of a AND bitwise operations when one of the inputs, in binary, is all ones will be the other input
    long all_ones_binary = 4294967295;
    cpu.state.input_buf.push(all_ones_binary);
    cpu.RunCPUStep(0, 100);
    world.Update();

    //Checks both that NOT is being done and no other operations are being done
    for (auto data : world.GetTaskSet()) {
      
        if(data.task.name != "NOT"){
      
        REQUIRE(data.n_succeeds_host == 0);
        }
        else{
          REQUIRE(data.n_succeeds_host > 0);
        }
      
    }


  }
}

TEST_CASE("Extinction event probabilities", "[sgp][sgp-integration]") {
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
