#include "../../sgp_mode/GenomeLibrary.h"
#include "../../sgp_mode/CPU.h"
#include "../../sgp_mode/HealthHost.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPWorldSetup.cc"
#include "../../sgp_mode/SGPConfigSetup.h"
#include "../../sgp_mode/SGPHost.cc"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPSymbiont.h"
#include "../../default_mode/WorldSetup.cc"
#include "../../default_mode/DataNodes.h"
#include "../../sgp_mode/SGPDataNodes.h"


TEST_CASE("Health host with symbiont loses/gains cycle 50% of time", "[sgp]") {
  emp::Random random(10);
  
  //TODO: The random number seed doesn't seem to be working, different values for the same seed

  SymConfigSGP config;
  config.SEED(10);
  config.INTERACTION_MECHANISM(HEALTH);
  config.LIMITED_RES_TOTAL(10);
  config.LIMITED_RES_INFLOW(500);
  config.VERTICAL_TRANSMISSION(0);
  config.HOST_REPRO_RES(100);
  config.SYM_HORIZ_TRANS_RES(10);

  config.SYNERGY(1);
  config.OUSTING(1);

  SGPWorld world(random, &config, LogicTasks);

  emp::Ptr<HealthHost> host = emp::NewPtr<HealthHost>(&random, &world, &config, CreateNotProgram(100));
  
  WHEN("Parasites are present"){
    config.SYMBIONT_TYPE(PARASITE);
    config.START_MOI(1);

    emp::Ptr<SGPSymbiont> parasite_symbiont = emp::NewPtr<SGPSymbiont> (&random, &world, &config);

    host->AddSymbiont(parasite_symbiont);
    
    world.AddOrgAt(host, 0);

    int total_times_skipped_cycle = 0;
    int repeats = 25;
    for (int i = 0; i < repeats; i++) {
      size_t initial_stack_location = host->GetCPU().GetCPUPointer().GetCore(0).GetProgramCounter();
      
      host->Process(0);
      size_t new_stack_location = host->GetCPU().GetCPUPointer().GetCore(0).GetProgramCounter();
      
      if(initial_stack_location == new_stack_location) {
        total_times_skipped_cycle++;
      }
    }
    
    REQUIRE((double)total_times_skipped_cycle/repeats <= 0.55);
    REQUIRE((double)total_times_skipped_cycle/repeats >= 0.45);
  }

  WHEN("Symbionts are not present"){
    config.START_MOI(0);
    world.AddOrgAt(host, 0);

    int total_times_skipped_cycle = 0;
    int repeats = 25;
    for (int i = 0; i < repeats; i++) {
      size_t initial_stack_location = host->GetCPU().GetCPUPointer().GetCore(0).GetProgramCounter();
      
      host->Process(0);
      size_t new_stack_location = host->GetCPU().GetCPUPointer().GetCore(0).GetProgramCounter();
      
      if(initial_stack_location == new_stack_location) {
        total_times_skipped_cycle++;
      }
    }

    int total_times_gained_cycles = 0;
    for (int i = 0; i < repeats; i++) {
      size_t initial_stack_location = host->GetCPU().GetCPUPointer().GetCore(0).GetProgramCounter();
      
      host->Process(0);
      size_t new_stack_location = host->GetCPU().GetCPUPointer().GetCore(0).GetProgramCounter();
      
      if(new_stack_location - initial_stack_location == 8) {
        total_times_gained_cycles++;
      }
    }
    
    REQUIRE((double)total_times_skipped_cycle/repeats == 0);
    REQUIRE((double)total_times_gained_cycles/repeats == 0);
  }

  WHEN("Mutualists are present"){
    config.START_MOI(1);
    config.SYMBIONT_TYPE(MUTUALIST);

    emp::Ptr<SGPSymbiont> mutualist_symbiont = emp::NewPtr<SGPSymbiont> (&random, &world, &config);

    host->AddSymbiont(mutualist_symbiont);
    

    world.AddOrgAt(host, 0);

    int total_times_gained_cycles = 0;
    int repeats = 25;
    for (int i = 0; i < repeats; i++) {
      size_t initial_stack_location = host->GetCPU().GetCPUPointer().GetCore(0).GetProgramCounter();
      
      host->Process(0);
      size_t new_stack_location = host->GetCPU().GetCPUPointer().GetCore(0).GetProgramCounter();
      
      if(new_stack_location - initial_stack_location == 8) {
        total_times_gained_cycles++;
      }
    }
    
    REQUIRE((double)total_times_gained_cycles/repeats <= 0.55);
    REQUIRE((double)total_times_gained_cycles/repeats >= 0.45);
  }
}


TEST_CASE("Health hosts evolve less NOT with parasites than without", "[sgp]") {
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
TEST_CASE("When DONATION_STEAL_INST is 1 then Symbiont with 'Steal' instruction properly takes CPU cycles from HealthHost", "[sgp]"){
 
  emp::Random random(1);
  SymConfigSGP config;
  config.SEED(0);
  config.INTERACTION_MECHANISM(HEALTH);
  config.SYMBIONT_TYPE(1);
  config.MUTATION_RATE(0.0);
  config.MUTATION_SIZE(0.00);
  config.TRACK_PARENT_TASKS(1);
  config.VT_TASK_MATCH(1);
  config.HOST_ONLY_FIRST_TASK_CREDIT(1);
  config.SYM_ONLY_FIRST_TASK_CREDIT(1);
  config.HOST_REPRO_RES(10000);
  config.DONATION_STEAL_INST(1);
  config.CPU_TRANSFER_CHANCE(1);



  SGPWorld world(random, &config, LogicTasks);


  //Creates a host that only does NAND operations
  emp::Ptr<HealthHost> host = emp::NewPtr<HealthHost>(&random, &world, &config, CreateNandProgram(100));
  //Creates a symbiont that only does Nand operations
  emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateParasiteNandProgram(100,94));

  //Adds host to world and sym to host.
  world.AddOrgAt(host, 0);
  host->AddSymbiont(sym);
    
  WHEN("A symbiont performs a Steal instruction"){
    sym->GetCPU().RunCPUStep(0, 100);
    (*(sym->GetCPU().state.tasks_performed))[1] = 0;
    THEN("The host should be set to lose 4 cycles to the symbiont"){
      REQUIRE(host->GetCyclesGiven() == -1);
    }

    for (size_t i = 0; i < 25; i++) {
      world.Update();
    }
    THEN("The symbiont should complete its task in 25 updates"){
      REQUIRE(sym->GetCPU().state.tasks_performed->Get(1) == true);
    }

    world.Update();
    THEN("The host should be unable to complete its task in 25 updates"){
      REQUIRE(host->GetCPU().state.tasks_performed->Get(1) == false);
    }
  }

}

TEST_CASE("When DONATION_STEAL_INST is 1 then Symbiont with 'Donate' instruction properly gives CPU cycles to HealthHost", "[sgp]"){
 
  emp::Random random(1);
  SymConfigSGP config;
  config.SEED(0);
  config.INTERACTION_MECHANISM(HEALTH);
  config.SYMBIONT_TYPE(0);
  config.MUTATION_RATE(0.0);
  config.MUTATION_SIZE(0.00);
  config.TRACK_PARENT_TASKS(1);
  config.VT_TASK_MATCH(1);
  config.HOST_ONLY_FIRST_TASK_CREDIT(1);
  config.SYM_ONLY_FIRST_TASK_CREDIT(1);
  config.DONATION_STEAL_INST(1);
  config.CPU_TRANSFER_CHANCE(1);
 
  config.HOST_REPRO_RES(10000);

  SGPWorld world(random, &config, LogicTasks);




  //Creates a host that only does NAND operations
  emp::Ptr<HealthHost> host = emp::NewPtr<HealthHost>(&random, &world, &config, CreateNandProgram(100));
  //Creates a symbiont that omly does Nand operations
  emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateMutualistNandProgram(100, 94));

  //Adds host to world and sym to host.
  world.AddOrgAt(host, 0);
  host->AddSymbiont(sym);
    
  WHEN("A symbiont performs a Donate instruction"){
    sym->GetCPU().RunCPUStep(0, 100);
    (*(sym->GetCPU().state.tasks_performed))[1] = 0;
    THEN("The host should be set to gain 4 cycles from the symbiont"){
      REQUIRE(host->GetCyclesGiven() == 1);
    }

    for (size_t i = 0; i < 24; i++) {
      world.Update();
      
    }
    THEN("The host should complete its task one update early"){
      REQUIRE(host->GetCPU().state.tasks_performed->Get(1) == true);
    }

    world.Update();
    THEN("The symbiont should be unable to complete its task in 25 updates"){
      REQUIRE(sym->GetCPU().state.tasks_performed->Get(1) == false);
    }
  }
}