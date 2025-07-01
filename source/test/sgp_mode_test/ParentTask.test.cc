#include "../../sgp_mode/GenomeLibrary.h"
#include "../../sgp_mode/CPU.h"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPHost.cc"
#include "../../sgp_mode/HealthHost.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPWorldSetup.cc"
#include "../../sgp_mode/SGPDataNodes.h"

#include "../../catch/catch.hpp"


TEST_CASE("Task Match properly returns whether a host/host's parnet and symbiont/symbiont's parent share any tasks", "[sgp]"){
  
  emp::Random random(1);
  SymConfigSGP config;
  config.RANDOM_ANCESTOR(false);
  config.SEED(2);
  config.ORGANISM_TYPE(HEALTH);
  config.STRESS_TYPE(PARASITE);
  config.MUTATION_RATE(0.0);
  config.MUTATION_SIZE(0.002);
  config.TRACK_PARENT_TASKS(1);
  config.VT_TASK_MATCH(1);
  config.ONLY_FIRST_TASK_CREDIT(1);
  config.HOST_REPRO_RES(10000);

  SGPWorld world(random, &config, LogicTasks);


  //Creates a host that only does NOT operations
  emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));
  //Creates a symbiont that does both Not and Nand operations
  emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(100));


  emp::Ptr<SGPSymbiont> sym_baby = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(100));
  emp::Ptr<SGPHost> host_baby = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));


  //Adds host to world and sym to host.
  world.AddOrgAt(host, 0);
  world.AddOrgAt(host_baby, 1);
  host->AddSymbiont(sym);
  host_baby->AddSymbiont(sym_baby);

  WHEN("Host and Symbiont have both performed NOT"){
    host->GetCPU().state.tasks_performed->Set(0);
    sym->GetCPU().state.tasks_performed->Set(0);

    THEN("TaskMatchCheck returns true when Host and Symbiont are the arguments"){
      REQUIRE(world.TaskMatchCheck(sym, host));
    }
    
    host_baby->GetCPU().state.parent_tasks_performed->Import(*(host->GetCPU().state.tasks_performed));
    sym_baby->GetCPU().state.parent_tasks_performed->Import(*(sym->GetCPU().state.tasks_performed));

    THEN("TaskMatchCheck returns true when the child of Host and the child of Symbiont are the arguments"){
      REQUIRE(world.TaskMatchCheck(sym_baby, host_baby));
    }

  }
  WHEN("Host has performed NOT and Symbiont has performed EQU"){
    host->GetCPU().state.tasks_performed->Set(1);
    sym->GetCPU().state.tasks_performed->Set(8);
    THEN("TaskMatchCheck returns false when Host and Symbiont are the arguments"){
      REQUIRE(!world.TaskMatchCheck(sym, host));
    }

    host_baby->GetCPU().state.parent_tasks_performed->Import(*(host->GetCPU().state.tasks_performed));
    sym_baby->GetCPU().state.parent_tasks_performed->Import(*(sym->GetCPU().state.tasks_performed));

    THEN("TaskMatchCheck returns false when the child of Host and the child of Symbiont are the arguments"){
      REQUIRE(!world.TaskMatchCheck(sym_baby, host_baby));
    }
  }



}