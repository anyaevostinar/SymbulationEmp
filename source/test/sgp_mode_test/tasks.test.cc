#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPWorldSetup.cc"

#include "../../catch/catch.hpp"

TEST_CASE("When only first task credit is on","[ony]"){
    emp::Random random(1);
    SymConfigSGP config;
    config.SEED(1);
    config.MUTATION_RATE(0.0);
    config.MUTATION_SIZE(0.002);
    config.TRACK_PARENT_TASKS(1);
    config.VT_TASK_MATCH(1);
    config.ONLY_FIRST_TASK_CREDIT(1);

    SGPWorld world(random, &config, LogicTasks);


    WHEN("Hosts are able to complete both NOT and NAND tasks"){
      
      //Builds program that does both NOT and NAND operations
      ProgramBuilder program;
      program.AddNot();
      program.AddNand();
      ProgramBuilder empty_program;

      //Creates a host that does both Not and Nand operations
      emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, program.Build(100));
      
      //Creates a symbiont that can not do any tasks
      emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, empty_program.Build(100));

      //Adds host to world and sym to host.
      world.AddOrgAt(host, 0);
      host->AddSymbiont(sym);


      WHEN("Host completes both NOT and NAND tasks"){
      
        host->GetCPU().RunCPUStep(0, 100);
        
        int tasks_completed = 0;
        for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
          tasks_completed += host->GetCPU().state.tasks_performed->Get(i);
        }
        THEN("Host should only get credit for completing 1 task"){
          REQUIRE(tasks_completed == 1);
            REQUIRE(host->GetPoints() == 5);
          
        }
      }
    }
}