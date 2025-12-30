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


TEST_CASE("When DONATION_STEAL_INST is 1 then Symbiont with 'Steal' instruction properly takes CPU cycles from HealthHost", "[sgp][sgp-functional]"){
 
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

TEST_CASE("When DONATION_STEAL_INST is 1 then Symbiont with 'Donate' instruction properly gives CPU cycles to HealthHost", "[sgp][sgp-functional]"){
 
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
  //Creates a symbiont that only does Nand operations
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