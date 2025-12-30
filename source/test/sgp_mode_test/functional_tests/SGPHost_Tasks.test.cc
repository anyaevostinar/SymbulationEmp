#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPHost.cc"
#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"
#include "../../../sgp_mode/SGPDataNodes.h"


TEST_CASE("When HOST_ONLY_FIRST_TASK_CREDIT is 1, the most tasks a Host can receive credit for is 1", "[sgp][sgp-functional]"){

  GIVEN("An SGPworld with HOST_ONLY_FIRST_TASK_CREDIT on"){
    emp::Random random(1);
    SymConfigSGP config;
    config.SEED(1);
    config.MUTATION_RATE(0.0);
    config.MUTATION_SIZE(0.002);
    config.TRACK_PARENT_TASKS(1);
    config.VT_TASK_MATCH(1);
    config.HOST_ONLY_FIRST_TASK_CREDIT(1);

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
        }
      }
    }

    WHEN("Hosts are able to complete all tasks"){
      
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

      ProgramBuilder empty_program;

      //Creates a host that can do all tasks
      emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, program.Build(100));
      
      //Creates a symbiont that can not do any tasks
      emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, empty_program.Build(100));

      //Adds host to world and sym to host.
      world.AddOrgAt(host, 0);
      host->AddSymbiont(sym);


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

    WHEN("Hosts are unable to complete any tasks"){
      
      //Empty Builder
      ProgramBuilder program;
      //Creates a host that cannot do any tasks
      emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, program.Build(100));
      
      //Creates a symbiont that cannot do any tasks
      emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, program.Build(100));

      //Adds host to world and sym to host.
      world.AddOrgAt(host, 0);
      host->AddSymbiont(sym);

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
}

TEST_CASE("When HOST_ONLY_FIRST_TASK_CREDIT is 0, hosts receive credit for all tasks they complete", "[sgp][sgp-functional]"){

  GIVEN("An SGPWorld with HOST_ONLY_FIRST_TASK_CREDIT off"){
    emp::Random random(1);
    SymConfigSGP config;
    config.SEED(2);
    config.MUTATION_RATE(0.0);
    config.MUTATION_SIZE(0.002);
    config.TRACK_PARENT_TASKS(1);
    config.VT_TASK_MATCH(1);
    config.HOST_ONLY_FIRST_TASK_CREDIT(0);

    SGPWorld world(random, &config, LogicTasks);


    WHEN("Host are able to complete both NOT and NAND tasks"){
      ProgramBuilder program;
      program.AddNot();
      program.AddNand();

      ProgramBuilder empty_program;

      //Creates a host that can do both Not and NAND operations
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
        THEN("Host should receive credit for completing 2 tasks"){
          REQUIRE(tasks_completed == 2);
        }
      }
    }

    WHEN("Hosts are able to complete all tasks"){

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

      ProgramBuilder empty_program;

      //Creates a host that can do all tasks
      emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, program.Build(100));
      
      //Creates a symbiont that can not do any tasks
      emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, empty_program.Build(100));

      //Adds host to world and sym to host.
      world.AddOrgAt(host, 0);
      host->AddSymbiont(sym);

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

    WHEN("Host are unable to complete any tasks"){
      //Empty Builder
      ProgramBuilder program;
      //Creates a host that cannot do any tasks
      emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, program.Build(100));
      
      //Creates a symbiont that cannot do any tasks
      emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, program.Build(100));

      //Adds host to world and sym to host.
      world.AddOrgAt(host, 0);
      host->AddSymbiont(sym);

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
}
