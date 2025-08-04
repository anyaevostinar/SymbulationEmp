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
//TODO: tasks -> Tasks for folder name and test name (Task.integration.test.cc)
// MOVE program builder outside of WHEN to cut down replications
// Move CPU cycles from THEN to WHEN
TEST_CASE("Task completion scoring and marking", "[tasks]") {
  emp::Random random(1);
  SymConfigSGP config;
  config.SEED(1);
  config.MUTATION_RATE(0.0);
  config.MUTATION_SIZE(0.002);
  config.TRACK_PARENT_TASKS(1);
  config.VT_TASK_MATCH(1);

  WHEN("OnlyFirstTaskCredit is OFF") {
    config.ONLY_FIRST_TASK_CREDIT(0);
    SGPWorld world(random, &config, LogicTasks);

    // multiple tasks
    ProgramBuilder multi_task_program;
    multi_task_program.AddNot();
    multi_task_program.AddNand();
    multi_task_program.AddAnd();

    emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, multi_task_program.Build(100));
    world.AddOrgAt(host, 0);

    THEN("Should receive score when task completed") {
      int initial_points = host->GetPoints();
      host->GetCPU().RunCPUStep(0, 100);
      
      // Get points for alltasks
      REQUIRE(host->GetPoints() > initial_points);
    }

    THEN("Should mark task when task completed") {
      host->GetCPU().RunCPUStep(0, 100);
      
      // Check that tasks marked complete
      int tasks_completed = 0;
      for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
        tasks_completed += host->GetCPU().state.tasks_performed->Get(i);
      }
      REQUIRE(tasks_completed > 0);
    }
  }

  WHEN("OnlyFirstTaskCredit is ON") {
    config.ONLY_FIRST_TASK_CREDIT(1);
    SGPWorld world(random, &config, LogicTasks);

    // Program with multiple tasks
    ProgramBuilder multi_task_program;
    multi_task_program.AddNot();
    multi_task_program.AddNand();
    multi_task_program.AddAnd();

    emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, multi_task_program.Build(100));
    world.AddOrgAt(host, 0);

    THEN("Should receive score when task completed if it is first task") {
      int initial_points = host->GetPoints();
      host->GetCPU().RunCPUStep(0, 100);
      
      // Points for first task only
      REQUIRE(host->GetPoints() == initial_points + 5);
    }

    THEN("Should not receive score when task completed if it is not first task") {
      // First for first task
      host->GetCPU().RunCPUStep(0, 100);
      int points_after_first = host->GetPoints(); //change points after task --> 10
      
      // Second run (should not get points)
      host->GetCPU().RunCPUStep(0, 100);
      REQUIRE(host->GetPoints() == points_after_first);
    }

    THEN("Should mark task completed when it is first task") {
      host->GetCPU().RunCPUStep(0, 100);
      
      // exactly one task is marked complete
      int tasks_completed = 0;
      for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
        tasks_completed += host->GetCPU().state.tasks_performed->Get(i);
      }
      REQUIRE(tasks_completed == 1);
    }

    THEN("Should not mark task completed when it is not first task") {
      // First run
      host->GetCPU().RunCPUStep(0, 100);
      int tasks_completed_first = 0;
      for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
        tasks_completed_first += host->GetCPU().state.tasks_performed->Get(i);
      }
      
      // Second run (should not mark any tasks)
      host->GetCPU().RunCPUStep(0, 100);
      int tasks_completed_second = 0;
      for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
        tasks_completed_second += host->GetCPU().state.tasks_performed->Get(i);
      }
      
      REQUIRE(tasks_completed_second == tasks_completed_first);
    }
  }
}

TEST_CASE("IsOnlyTask functionality", "[tasks]") {
  emp::Random random(1);
  SymConfigSGP config;
  config.SEED(1);
  config.MUTATION_RATE(0.0);
  config.MUTATION_SIZE(0.002);
  config.TRACK_PARENT_TASKS(1);
  config.VT_TASK_MATCH(1);
  config.ONLY_FIRST_TASK_CREDIT(1);

  SGPWorld world(random, &config, LogicTasks);

  WHEN("No tasks have been completed yet") {
    ProgramBuilder multi_task_program;
    multi_task_program.AddNot();
    multi_task_program.AddNand();
    
    emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, multi_task_program.Build(100));
    world.AddOrgAt(host, 0);

    THEN("IsOnlyTask should return true for first task") {
      // Simulate completing the first task (NOT)
      host->GetCPU().state.tasks_performed->Set(0); // Mark NOT as completed
      
      // Create a non const copy of LogicTasks for testing
      TaskSet test_tasks = LogicTasks;
      bool is_only = test_tasks.IsOnlyTask(host->GetCPU().state, 0);
      REQUIRE(is_only == true);
    }
  }

  WHEN("One task has already been completed") {
    ProgramBuilder multi_task_program;
    multi_task_program.AddNot();
    multi_task_program.AddNand();
    
    emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, multi_task_program.Build(100));
    world.AddOrgAt(host, 0);

    THEN("IsOnlyTask should return false for second task") {
      // Mark first task as completed
      host->GetCPU().state.tasks_performed->Set(0); // NOT completed
      
      // Create a non const copy of LogicTasks for testing
      TaskSet test_tasks = LogicTasks;
      // Check if second task NAND is the only task
      bool is_only = test_tasks.IsOnlyTask(host->GetCPU().state, 1);
      REQUIRE(is_only == false);
    }

    THEN("IsOnlyTask should return true for the first completed task") {
      // Mark first task as completed
      host->GetCPU().state.tasks_performed->Set(0); // NOT completed
      
      // Create a non const copy of LogicTasks for testing
      TaskSet test_tasks = LogicTasks;
      // Check if first task is the only task
      bool is_only = test_tasks.IsOnlyTask(host->GetCPU().state, 0);
      REQUIRE(is_only == true);
    }
  }

  WHEN("Multiple tasks have been completed") {
    ProgramBuilder multi_task_program;
    multi_task_program.AddNot();
    multi_task_program.AddNand();
    multi_task_program.AddAnd();
    
    emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, multi_task_program.Build(100));
    world.AddOrgAt(host, 0);

    THEN("IsOnlyTask should return false for any task") {
      // Mark multiple tasks
      host->GetCPU().state.tasks_performed->Set(0);
      host->GetCPU().state.tasks_performed->Set(1);
      
      // Create a non const copy of LogicTasks for testing
      TaskSet test_tasks = LogicTasks;
      // Check if any task is the only task
      bool is_only_not = test_tasks.IsOnlyTask(host->GetCPU().state, 0);
      bool is_only_nand = test_tasks.IsOnlyTask(host->GetCPU().state, 1);
      bool is_only_and = test_tasks.IsOnlyTask(host->GetCPU().state, 2);
      
      REQUIRE(is_only_not == false);
      REQUIRE(is_only_nand == false);
      REQUIRE(is_only_and == false);
    }
  }
}

TEST_CASE("Task completion edge cases", "[tasks]") { // IsSolved Edge casses documentation
  emp::Random random(1);
  SymConfigSGP config;
  config.SEED(1);
  config.MUTATION_RATE(0.0);
  config.MUTATION_SIZE(0.002);
  config.TRACK_PARENT_TASKS(1);
  config.VT_TASK_MATCH(1);
  config.ONLY_FIRST_TASK_CREDIT(0);

  SGPWorld world(random, &config, LogicTasks);

  WHEN("Testing output values of 0 and 1") {
    ProgramBuilder not_program;
    not_program.AddNot();
    
    emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, not_program.Build(100));
    world.AddOrgAt(host, 0);

    THEN("Outputs of 0 and 1 should not be considered valid task completions") {
      // Create a NOT task to test with
      Task not_task("NOT", 1, 5.0, [](auto &x) { return ~x[0]; });
      
      // Test with output 0
      bool is_solved_0 = not_task.IsSolved(host->GetCPU().state, 0);
      REQUIRE(is_solved_0 == false);
      
      // Test with output 1
      bool is_solved_1 = not_task.IsSolved(host->GetCPU().state, 1);
      REQUIRE(is_solved_1 == false);
    }
  }
}