#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPWorldSetup.cc"
#include "../../sgp_mode/SGPDataNodes.h"

TEST_CASE("SGPSymbiont Reproduce", "[sgp]") {
  emp::Random random(31);
  SymConfigSGP config;
  SGPWorld world(random, &config, LogicTasks);
  emp::Ptr<SGPSymbiont> sym_parent = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(100));

  THEN("Symbiont child increases its lineage reproduction count") {
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

TEST_CASE("SGPSymbiont CheckTaskInteraction in nutrient mode", "[sgp]") {
  emp::Random random(42);
  SymConfigSGP config;
  config.ORGANISM_TYPE(NUTRIENT);
  config.NUTRIENT_DONATE_STEAL_PROP(0.5);
  SGPWorld world(random, &config, LogicTasks);

  ProgramBuilder builder;
  builder.AddNot(); //First task
  emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, builder.Build(100));
  emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, builder.Build(100));

  world.AddOrgAt(host, 0);
  host->AddSymbiont(sym);

  host->GetCPU().state.tasks_performed->Set(0);
  // starting points
  host->SetPoints(10.0);
  double sym_score = 8.0;

  WHEN("Parasite steals from host") {
    config.STRESS_TYPE(PARASITE);
    double result = sym->CheckTaskInteraction(sym_score, 0);

    double expected_steal = config.NUTRIENT_DONATE_STEAL_PROP() * sym_score;
    double expected_actual = emp::Min(10.0, expected_steal);

    REQUIRE(result == expected_actual);
    REQUIRE(host->GetPoints() == 10.0 - expected_actual);
  }

  WHEN("Mutualist donates to host") {
    config.STRESS_TYPE(MUTUALIST); 
    host->SetPoints(10.0); 
    double result = sym->CheckTaskInteraction(sym_score, 0);

    double expected_donation = config.NUTRIENT_DONATE_STEAL_PROP() * sym_score;
    double expected_score_remain = sym_score - expected_donation;

    REQUIRE(result == expected_score_remain);
    REQUIRE(host->GetPoints() == 10.0 + expected_donation);
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

TEST_CASE("Symbiont comparison operators", "[sgp]") {
  emp::Random random(31);
	SymConfigSGP config;
	SGPWorld world(random, &config, LogicTasks);
	emp::Ptr<SGPSymbiont> sym_parent = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(100));

    emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));
    config.TRACK_PARENT_TASKS(1);
    world.AddOrgAt(host, 0);
    host->AddSymbiont(sym_parent);

    for (int i = 0; i < 25; i++) {
      world.Update();
    }
    emp::Ptr<SGPSymbiont> clone1 = emp::NewPtr<SGPSymbiont>(*sym_parent);
    emp::Ptr<SGPSymbiont> clone2 = emp::NewPtr<SGPSymbiont>(*sym_parent);
    emp::Ptr<SGPSymbiont> different = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(99)); // For comparing

    REQUIRE(*sym_parent == *clone1);
    REQUIRE(*clone1 == *clone2);

    REQUIRE_FALSE(*sym_parent == *different);

    // Can't assert true/false without knowing bitcode ordering,
    // assert that bitcode ordering is well-defined
    bool lt = *sym_parent < *different || *different < *sym_parent;
    REQUIRE(lt);
    
    clone1.Delete();
    clone2.Delete();
    different.Delete();
  }

TEST_CASE("SGPSymbiont destructor cleans up shared pointers and in-progress reproduction", "[sgp]") {
  emp::Random random(31);
	SymConfigSGP config;
	SGPWorld world(random, &config, LogicTasks);
  emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(100));
  sym->GetCPU().state.in_progress_repro = 3;
  world.to_reproduce.resize(5); 
  world.to_reproduce[3].second = emp::WorldPosition(1, 2); 

  REQUIRE(world.to_reproduce[3].second.IsValid());

  WHEN("Symbionts is destroyed") {
    sym.Delete(); 
    
    THEN("Reproduction queue is invalidated after symbiont is destroyed") {
      REQUIRE_FALSE(world.to_reproduce[3].second.IsValid());
    }
  }
}
