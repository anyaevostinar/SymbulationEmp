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


TEST_CASE("Health hosts evolve", "[sgp][integration]") {
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
      //There can never be exactly no_mut_NAND_rate NAND tasks completed as it will ocassionally guess NOT
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
      //There can never be exactly no_mut_NAND_rate NAND tasks completed as it will ocassionally guess NOT
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
      REQUIRE((*world.GetTaskSet().begin()).n_succeeds_host >= no_mut_NAND_rate * 5);
      ++it;
      REQUIRE((*it).n_succeeds_host > 3000);
      
    }
  }
} 


TEST_CASE("Organisms, without mutation will only receive credit for NOT operations", "[sgp]") {
     
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






