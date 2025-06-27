#include "../../sgp_mode/GenomeLibrary.h"
#include "../../sgp_mode/CPU.h"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPHost.cc"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPWorldSetup.cc"
#include "../../sgp_mode/SGPDataNodes.h"

#include "../../catch/catch.hpp"



TEST_CASE("When ONLY_FIRST_TASK_CREDIT is 1, the most tasks an organism can recieve credit for is 1", "[sgp]"){

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

  SGPWorld world(random, &config, LogicTasks);


  WHEN("Symbiont and Host are able to complete both NOT and NAND tasks"){
    
    //Builds program that does both NOT and NAND operations
    ProgramBuilder program;
    program.AddNot();
    program.AddNand();

    //Creates a host that does both Not and Nand operations
    emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, program.Build(100));
    
    //Creates a symbiont that does both Not and Nand operations
    emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, program.Build(100));

    //Adds host to world and sym to host.
    world.AddOrgAt(host, 0);
    host->AddSymbiont(sym);


    WHEN("Symbiont completes both NOT and NAND tasks"){
    
      sym->GetCPU().RunCPUStep(0, 100);
      
        int tasks_completed = 0;
      for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
        tasks_completed += sym->GetCPU().state.tasks_performed->Get(i);
      }
      THEN("Symbiont should only get credit for completing 1 task"){
      REQUIRE(tasks_completed == 1);
      }
    }

    WHEN("Host completes both NOT and NAND tasks"){
    
      host->GetCPU().RunCPUStep(0, 100);
      
        int tasks_completed = 0;
      for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
        tasks_completed += host->GetCPU().state.tasks_performed->Get(i);
      }
      THEN("Host should only get credit for completing 1 task"){
      REQUIRE(tasks_completed == 1);
      }
    }
  }

  WHEN("Symbiont and Host are able to complete all tasks"){
    
    //Builds program that does all tasks
    ProgramBuilder program;
    program.AddNot();
    program.AddNand();
    program.AddAnd();
    program.AddOrn();
    program.AddOr();
    program.AddAndn();
    program.AddNor();
    program.AddXor();
    program.AddEqu();

    //Creates a host that can do all tasks
    emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, program.Build(100));
    
    //Creates a symbiont that can do all tasks
    emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, program.Build(100));

    //Adds host to world and sym to host.
    world.AddOrgAt(host, 0);
    host->AddSymbiont(sym);


    WHEN("Symbiont completes all tasks"){
    
      sym->GetCPU().RunCPUStep(0, 100);
      
        int tasks_completed = 0;
      for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
        tasks_completed += sym->GetCPU().state.tasks_performed->Get(i);
      }
      THEN("Symbiont should only get credit for completing 1 task"){
      REQUIRE(tasks_completed == 1);
      }
    }

    WHEN("Host completes all tasks"){
    
      host->GetCPU().RunCPUStep(0, 100);
      
        int tasks_completed = 0;
      for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
        tasks_completed += host->GetCPU().state.tasks_performed->Get(i);
      }
      THEN("Host should only get credit for completing 1 task"){
      REQUIRE(tasks_completed == 1);
      }
    }
  }

  WHEN("Symbiont and Host are unable to complete any tasks"){
    
    //Empty Builder
    ProgramBuilder program;
    //Creates a host that cannot do any tasks
    emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, program.Build(100));
    
    //Creates a symbiont that cannot do any tasks
    emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, program.Build(100));

    //Adds host to world and sym to host.
    world.AddOrgAt(host, 0);
    host->AddSymbiont(sym);


    WHEN("Symbiont completes no tasks"){
    
      sym->GetCPU().RunCPUStep(0, 100);
      
        int tasks_completed = 0;
      for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
        tasks_completed += sym->GetCPU().state.tasks_performed->Get(i);
      }
      THEN("Symbiont should not get credit for any tasks"){
      REQUIRE(tasks_completed == 0);
      }
    }

    WHEN("Host completes no tasks"){
    
      host->GetCPU().RunCPUStep(0, 100);
      
        int tasks_completed = 0;
      for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
        tasks_completed += host->GetCPU().state.tasks_performed->Get(i);
      }
      THEN("Host should not get credit for any tasks"){
      REQUIRE(tasks_completed == 0);
      }
    }
  }
  

}

TEST_CASE("When ONLY_FIRST_TASK_CREDIT is 0, organisms recieve credit for all tasks they complete", "[sgp]"){

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
  config.ONLY_FIRST_TASK_CREDIT(0);

  SGPWorld world(random, &config, LogicTasks);


  WHEN("Symbiont and Host are able to complete both NOT and NAND tasks"){
    ProgramBuilder program;
    program.AddNot();
    program.AddNand();

    //Creates a host that only does NOT operations
    emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, program.Build(100));
    
    //Creates a symbiont that does both Not and Nand operations
    emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, program.Build(100));

    //Adds host to world and sym to host.
    world.AddOrgAt(host, 0);
    host->AddSymbiont(sym);


    WHEN("Symbiont completes both NOT and NAND tasks"){
    
      sym->GetCPU().RunCPUStep(0, 100);
      
        int tasks_completed = 0;
      for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
        tasks_completed += sym->GetCPU().state.tasks_performed->Get(i);
      }
      THEN("Symbiont should recieve credit for completing 2 tasks"){
      REQUIRE(tasks_completed == 2);
      }
    }

    WHEN("Host completes both NOT and NAND tasks"){
    
      host->GetCPU().RunCPUStep(0, 100);
      
        int tasks_completed = 0;
      for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
        tasks_completed += host->GetCPU().state.tasks_performed->Get(i);
      }
      THEN("Host should recieve credit for completing 2 tasks"){
      REQUIRE(tasks_completed == 2);
      }
    }
  }

  WHEN("Symbiont and Host are able to complete all tasks"){

    //Builds program that completes all tasks
    ProgramBuilder program;
    program.AddNot();
    program.AddNand();
    program.AddAnd();
    program.AddOrn();
    program.AddOr();
    program.AddAndn();
    program.AddNor();
    program.AddXor();
    program.AddEqu();
    //Creates a host that can do all tasks
    emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, program.Build(100));
    
    //Creates a symbiont that can do all tasks
    emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, program.Build(100));

    //Adds host to world and sym to host.
    world.AddOrgAt(host, 0);
    host->AddSymbiont(sym);


    WHEN("Symbiont completes all tasks"){
    
      sym->GetCPU().RunCPUStep(0, 100);
      
        int tasks_completed = 0;
      for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
        tasks_completed += sym->GetCPU().state.tasks_performed->Get(i);
      }
      THEN("Symbiont should get credit for completing all 9 tasks"){
      REQUIRE(tasks_completed == 9);
      }
    }

    WHEN("Host completes all tasks"){
    
      host->GetCPU().RunCPUStep(0, 100);
      
        int tasks_completed = 0;
      for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
        tasks_completed += host->GetCPU().state.tasks_performed->Get(i);
      }
      THEN("Host should get credit for completing all 9 tasks"){
      REQUIRE(tasks_completed == 9);
      }
    }
  }

  WHEN("Symbiont and Host are unable to complete any tasks"){
    //Empty Builder
    ProgramBuilder program;
    //Creates a host that cannot do any tasks
    emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, program.Build(100));
    
    //Creates a symbiont that cannot do any tasks
    emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, program.Build(100));

    //Adds host to world and sym to host.
    world.AddOrgAt(host, 0);
    host->AddSymbiont(sym);


    WHEN("Symbiont completes no tasks"){
    
      sym->GetCPU().RunCPUStep(0, 100);
      
        int tasks_completed = 0;
      for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
        tasks_completed += sym->GetCPU().state.tasks_performed->Get(i);
      }
      THEN("Symbiont should not get credit for any tasks"){
      REQUIRE(tasks_completed == 0);
      }
    }

    WHEN("Host completes no tasks"){
    
      host->GetCPU().RunCPUStep(0, 100);
      
        int tasks_completed = 0;
      for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
        tasks_completed += host->GetCPU().state.tasks_performed->Get(i);
      }
      THEN("Host should not get credit for any tasks"){
      REQUIRE(tasks_completed == 0);
      }
    }
  }
  

}
