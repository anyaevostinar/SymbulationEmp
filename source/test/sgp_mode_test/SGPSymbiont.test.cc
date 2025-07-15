#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPWorldSetup.cc"
#include "../../sgp_mode/SGPDataNodes.h"

TEST_CASE("SGPSymbiont Reproduce", "[sgp]") {
	emp::Random random(31);
	SymConfigSGP config;
	SGPWorld world(random, &config, LogicTasks);
	emp::Ptr<SGPSymbiont> sym_parent = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(100));
	

	THEN("Symbiont child increases its lineage reproduction count"){
		emp::Ptr<SGPSymbiont> sym_baby = (sym_parent->Reproduce()).DynamicCast<SGPSymbiont>();
		REQUIRE(sym_parent->GetReproCount() == sym_baby->GetReproCount() - 1);
		sym_baby.Delete();
    sym_parent.Delete();
	}

  WHEN("Parental task tracking is on") {

    emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));
    config.TRACK_PARENT_TASKS(1);
    world.AddOrgAt(host, 0);
    host->AddSymbiont(sym_parent);

    for (int i = 0; i < 25; i++) {
      world.Update();
    }

    emp::Ptr<SGPSymbiont> sym_baby = (sym_parent->Reproduce()).DynamicCast<SGPSymbiont>();

    THEN("Symbiont child inherits its parent's empty task bitset") {
      REQUIRE(sym_parent->GetCPU().state.parent_tasks_performed->None());

      REQUIRE(sym_parent->GetCPU().state.tasks_performed->Get(0));
      REQUIRE(sym_parent->GetCPU().state.tasks_performed->CountOnes() == 1);

      REQUIRE(sym_baby->GetCPU().state.parent_tasks_performed->Get(0));
      REQUIRE(sym_baby->GetCPU().state.parent_tasks_performed->CountOnes() == 1);
    }

    THEN("The symbiont child tracks any gains or loses in task completions") {
      // in this second generation, we expect the symbiont to gain the NOT task
      // since first-gen organisms are marked as having parents who have completed no tasks
      REQUIRE(sym_baby->GetCPU().state.task_change_lose[0] == 0);
      REQUIRE(sym_baby->GetCPU().state.task_change_gain[0] == 1);
      for (unsigned int i = 1; i < CPU_BITSET_LENGTH; i++) {
        REQUIRE(sym_baby->GetCPU().state.task_change_lose[i] == 0);
        REQUIRE(sym_baby->GetCPU().state.task_change_gain[i] == 0);
      }
    }

    THEN("The symbiont child tracks how its tasks compare to its parent's partner's tasks") {
      REQUIRE(sym_baby->GetCPU().state.task_toward_partner[0] == 0);
      REQUIRE(sym_baby->GetCPU().state.task_from_partner[0] == 1);

      for (unsigned int i = 1; i < CPU_BITSET_LENGTH; i++) {
        REQUIRE(sym_baby->GetCPU().state.task_toward_partner[i] == 0);
        REQUIRE(sym_baby->GetCPU().state.task_from_partner[i] == 0);
      }
    }
    sym_baby.Delete();
  }
}


TEST_CASE("When ONLY_FIRST_TASK_CREDIT is 1, the most tasks a symbiont can receive credit for is 1", "[sgp]"){
  GIVEN("An SGPworld with ONLY_FIRST_TASK_CREDIT on "){
    emp::Random random(1);
    SymConfigSGP config;
    config.SEED(1);
    config.ORGANISM_TYPE(HEALTH);
    config.STRESS_TYPE(PARASITE);
    config.MUTATION_RATE(0.0);
    config.MUTATION_SIZE(0.002);
    config.TRACK_PARENT_TASKS(1);
    config.VT_TASK_MATCH(1);
    config.ONLY_FIRST_TASK_CREDIT(1);

    SGPWorld world(random, &config, LogicTasks);


    WHEN("Symbionts are able to complete both NOT and NAND tasks"){
      
      //Builds program that does both NOT and NAND operations
      ProgramBuilder program;
      program.AddNot();
      program.AddNand();
      ProgramBuilder empty_program;

      //Creates a host that cannot do any tasks
      emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, empty_program.Build(100));
      
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


    }

    WHEN("Symbionts are able to complete all tasks"){
      
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

      //Creates a host that can not do any tasks
      emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, empty_program.Build(100));
      
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

    }

    WHEN("Symbionts are unable to complete any tasks"){
      
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

    }
    

  }
}

TEST_CASE("When ONLY_FIRST_TASK_CREDIT is 0, symbionts receive credit for all tasks they complete", "[sgp]"){
  GIVEN("An SGPworld with ONLY_FIRST_TASK_CREDIT off"){
    emp::Random random(1);
    SymConfigSGP config;
    config.SEED(2);
    config.ORGANISM_TYPE(HEALTH);
    config.STRESS_TYPE(PARASITE);
    config.MUTATION_RATE(0.0);
    config.MUTATION_SIZE(0.002);
    config.TRACK_PARENT_TASKS(1);
    config.VT_TASK_MATCH(1);
    config.ONLY_FIRST_TASK_CREDIT(0);

    SGPWorld world(random, &config, LogicTasks);


    WHEN("Symbionts are able to complete both NOT and NAND tasks"){
      ProgramBuilder program;
      program.AddNot();
      program.AddNand();
      ProgramBuilder empty_program;

      //Creates a host that can not do any tasks
      emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, empty_program.Build(100));
      
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

    }

    WHEN("Symbionts are able to complete all tasks"){

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

      //Creates a host that can not do any tasks
      emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, empty_program.Build(100));
      
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

    }

    WHEN("Symbionts are unable to complete any tasks"){
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

    }
    

  }
}
