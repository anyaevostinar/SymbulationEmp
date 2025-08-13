#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPWorldSetup.cc"

#include "../../catch/catch.hpp"

TEST_CASE("When only first task credit is on","[sgp]"){
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

TEST_CASE("Task integration scoring and marking", "[sgp]") {
  emp::Random random(1);
  SymConfigSGP config;
  config.SEED(1);
  config.MUTATION_RATE(0.0);
  config.MUTATION_SIZE(0.002);
  config.TRACK_PARENT_TASKS(1);
  config.VT_TASK_MATCH(1);
  SGPWorld world(random, &config, LogicTasks);
  
  // multiple tasks
  ProgramBuilder multi_task_program;
  multi_task_program.AddNot();
  multi_task_program.AddNand();
  multi_task_program.AddAnd();

  emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, multi_task_program.Build(100));
  world.AddOrgAt(host, 0);

  WHEN("OnlyFirstTaskCredit is OFF") {
    config.ONLY_FIRST_TASK_CREDIT(0);

    THEN("Should receive score when task completed") {
      int initial_points = host->GetPoints();
      host->GetCPU().RunCPUStep(0, 100);
      
      // Get points for all tasks
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

    int initial_points = host->GetPoints();
    host->GetCPU().RunCPUStep(0, 100);

    THEN("Should receive score when task completed if it is first task") {
      
      // Points for first task only
      REQUIRE(host->GetPoints() == initial_points + 5);
    }

    THEN("Should not receive score when task completed if it is not first task") {
      int points_after_first = host->GetPoints();
      
      // Second run (should not get points)
      host->GetCPU().RunCPUStep(0, 100);
      points_after_first += 5;
      REQUIRE(host->GetPoints() == points_after_first);
    }

    THEN("Should mark task completed when it is first task") {
      
      // exactly one task is marked complete
      int tasks_completed = 0;
      for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
        tasks_completed += host->GetCPU().state.tasks_performed->Get(i);
      }
      REQUIRE(tasks_completed == 1);
    }

    THEN("Should not mark task completed when it is not first task") {
      // First run
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

TEST_CASE("IsOnlyTask functionality", "[sgp]") {
  emp::Random random(1);
  SymConfigSGP config;
  config.SEED(1);
  config.MUTATION_RATE(0.0);
  config.MUTATION_SIZE(0.002);
  config.TRACK_PARENT_TASKS(1);
  config.VT_TASK_MATCH(1);
  config.ONLY_FIRST_TASK_CREDIT(1);

  SGPWorld world(random, &config, LogicTasks);

  ProgramBuilder multi_task_program;
  multi_task_program.AddNot();
  multi_task_program.AddNand();
    
  emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, multi_task_program.Build(100));
  world.AddOrgAt(host, 0);

  WHEN("No tasks have been completed yet") {

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

TEST_CASE("ProcessOutput Functionality", "[proc]"){
  emp::Random random(1);
  SymConfigSGP config;
  config.SEED(1);
  config.MUTATION_RATE(0.0);
  config.MUTATION_SIZE(0.002);
  config.TRACK_PARENT_TASKS(1);
  config.VT_TASK_MATCH(1);
  config.ONLY_FIRST_TASK_CREDIT(0);

  SGPWorld world(random, &config, LogicTasks);

  ProgramBuilder not_program;
  not_program.AddNot();

  emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, not_program.Build(100));
  world.AddOrgAt(host, 0);
  GIVEN("Two Inputs"){
    //These two inputs were chosen because their is no output for any opeartion that is the same as another output
    host->GetCPU().state.input_buf.push(734856699);
    host->GetCPU().state.input_buf.push(1177728054);

  

    WHEN("ProcessOutput is run on an organism with those inputs, The NOT of the first input, and ONLY_FIRST_TASK_CREDIT is 0"){
      //The result of applying the NOT bitwise opeartions to the binary form of 734856699
      long not_output = 3560110596;
      
        host->GetCPU().state.world->GetTaskSet().ProcessOutput(host->GetCPU().state,not_output,0);
        THEN("Organism should have 5 points"){
          REQUIRE(host->GetPoints() == 5);
        }
        THEN("Organism should be marked as having performed NOT"){
        for (int i = 0; i < 9; i++){
          if(i == 0){
          REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 1);
          }
          else{
          REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 0);
          }
          }
        }
        THEN("World data should have marked 1 completion for NOT"){
          for (auto data : world.GetTaskSet()) {
              if(data.task.name == "NOT"){
          
              REQUIRE(data.n_succeeds_host == 1);
              }
              else{
                REQUIRE(data.n_succeeds_host == 0);
              }
          }
        }


      
    }

    WHEN("ProcessOutput is run on an organism with those inputs, The NAND of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
      //The result of applying the NAND bitwise opeartions to the binary form of 734856699 and 1177728054
      long nand_output = 4261411789;
      host->GetCPU().state.world->GetTaskSet().ProcessOutput(host->GetCPU().state,nand_output,0);
        THEN("Organism should have 5 points"){
          REQUIRE(host->GetPoints() == 5);
        }
        THEN("Organism should be marked as having performed NAND"){
        for (int i = 0; i < 9; i++){
          if(i == 1){
          REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 1);
          }
          else{
          REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 0);
          }
          }
        }
        THEN("World data should have marked 1 completion for NAND"){
          for (auto data : world.GetTaskSet()) {
              if(data.task.name == "NAND"){
          
              REQUIRE(data.n_succeeds_host == 1);
              }
              else{
                REQUIRE(data.n_succeeds_host == 0);
              }
          }
        }
    }

    WHEN("ProcessOutput is run on an organism with those inputs, The AND of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
      //The result of applying the AND bitwise opeartions to the binary form of 734856699 and 1177728054
      long and_output = 33555506;
      host->GetCPU().state.world->GetTaskSet().ProcessOutput(host->GetCPU().state,and_output,0);
        THEN("Organism should have 5 points"){
          REQUIRE(host->GetPoints() == 5);
        }
        THEN("Organism should be marked as having performed AND"){
        for (int i = 0; i < 9; i++){
          if(i == 2){
          REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 1);
          }
          else{
          REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 0);
          }
          }
        }
        THEN("World data should have marked 1 completion for AND"){
          for (auto data : world.GetTaskSet()) {
              if(data.task.name == "AND"){
          
              REQUIRE(data.n_succeeds_host == 1);
              }
              else{
                REQUIRE(data.n_succeeds_host == 0);
              }
          }
        }
    }

    WHEN("ProcessOutput is run on an organism with those inputs, The ORN of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
      //The result of applying the ORN bitwise opeartions to the binary form of 734856699 and 1177728054
      long orn_output = 3150794747;
      host->GetCPU().state.world->GetTaskSet().ProcessOutput(host->GetCPU().state,orn_output,0);
        THEN("Organism should have 5 points"){
          REQUIRE(host->GetPoints() == 5);
        }
        THEN("Organism should be marked as having performed ORN"){
        for (int i = 0; i < 9; i++){
          if(i == 3){
         REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 1);
          }
          else{
          REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 0);
          }
          }
        }
        THEN("World data should have marked 1 completion for ORN"){
          for (auto data : world.GetTaskSet()) {
              if(data.task.name == "ORN"){
          
              REQUIRE(data.n_succeeds_host == 1);
              }
              else{
                REQUIRE(data.n_succeeds_host == 0);
              }
          }
        }
    }

    WHEN("ProcessOutput is run on an organism with those inputs, The OR of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
      //The result of applying the OR bitwise opeartions to the binary form of 734856699 and 1177728054
      long or_output = 1879029247;
      host->GetCPU().state.world->GetTaskSet().ProcessOutput(host->GetCPU().state,or_output,0);
        THEN("Organism should have 5 points"){
          REQUIRE(host->GetPoints() == 5);
        }
        THEN("Organism should be marked as having performed OR"){
        for (int i = 0; i < 9; i++){
          if(i == 4){
          REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 1);
          }
          else{
          REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 0);
          }
          }
        }
        THEN("World data should have marked 1 completion for OR"){
          for (auto data : world.GetTaskSet()) {
              if(data.task.name == "OR"){
          
              REQUIRE(data.n_succeeds_host == 1);
              }
              else{
                REQUIRE(data.n_succeeds_host == 0);
              }
          }
        }
    }

    WHEN("ProcessOutput is run on an organism with those inputs, The ANDN of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
      //The result of applying the ANDN bitwise opeartions to the binary form of 734856699 and 1177728054
      long andn_output = 701301193;
      host->GetCPU().state.world->GetTaskSet().ProcessOutput(host->GetCPU().state,andn_output,0);
        THEN("Organism should have 5 points"){
          REQUIRE(host->GetPoints() == 5);
        }
        THEN("Organism should be marked as having performed ANDN"){
        for (int i = 0; i < 9; i++){
          if(i == 5){
          REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 1);
          }
          else{
          REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 0);
          }
          }
        }
        THEN("World data should have marked 1 completion for ANDN"){
          for (auto data : world.GetTaskSet()) {
              if(data.task.name == "ANDN"){
          
              REQUIRE(data.n_succeeds_host == 1);
              }
              else{
                REQUIRE(data.n_succeeds_host == 0);
              }
          }
        }
    }

    WHEN("ProcessOutput is run on an organism with those inputs, The NOR of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
      //The result of applying the NOR bitwise opeartions to the binary form of 734856699 and 1177728054
      long nor_output = 2415938048;
      host->GetCPU().state.world->GetTaskSet().ProcessOutput(host->GetCPU().state,nor_output,0);
        THEN("Organism should have 5 points"){
          REQUIRE(host->GetPoints() == 5);
        }
        THEN("Organism should be marked as having performed NOR"){
        for (int i = 0; i < 9; i++){
          if(i == 6){
          REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 1);
          }
          else{
          REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 0);
          }
          }
        }
        THEN("World data should have marked 1 completion for NOR"){
          for (auto data : world.GetTaskSet()) {
              if(data.task.name == "NOR"){
          
              REQUIRE(data.n_succeeds_host == 1);
              }
              else{
                REQUIRE(data.n_succeeds_host == 0);
              }
          }
        }
    }

    WHEN("ProcessOutput is run on an organism with those inputs, The XOR of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
      //The result of applying the XOR bitwise opeartions to the binary form of 734856699 and 1177728054
      long xor_output = 1845473741;
      host->GetCPU().state.world->GetTaskSet().ProcessOutput(host->GetCPU().state,xor_output,0);
        THEN("Organism should have 5 points"){
          REQUIRE(host->GetPoints() == 5);
        }
        THEN("Organism should be marked as having performed XOR"){
        for (int i = 0; i < 9; i++){
          if(i == 7){
          REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 1);
          }
          else{
          REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 0);
          }
          }
        }
        THEN("World data should have marked 1 completion for XOR"){
          for (auto data : world.GetTaskSet()) {
              if(data.task.name == "XOR"){
          
              REQUIRE(data.n_succeeds_host == 1);
              }
              else{
                REQUIRE(data.n_succeeds_host == 0);
              }
          }
        }
    }

    WHEN("ProcessOutput is run on an organism with those inputs, The EQU of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
      //The result of applying the EQU bitwise opeartions to the binary form of 734856699 and 1177728054
      long equ_output = 2449493554;
      host->GetCPU().state.world->GetTaskSet().ProcessOutput(host->GetCPU().state,equ_output,0);
        THEN("Organism should have 5 points"){
          REQUIRE(host->GetPoints() == 5);
        }
        THEN("Organism should be marked as having performed EQU"){
        for (int i = 0; i < 9; i++){
          if(i == 8){
          REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 1);
          }
          else{
          REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 0);
          }
          }
        }
        THEN("World data should have marked 1 completion for EQU"){
          for (auto data : world.GetTaskSet()) {
              if(data.task.name == "EQU"){
          
              REQUIRE(data.n_succeeds_host == 1);
              }
              else{
                REQUIRE(data.n_succeeds_host == 0);
              }
          }
        }
    }
  }
}

TEST_CASE("WhichTaskDone Functionality", "[sgp]"){
  emp::Random random(1);
  SymConfigSGP config;
  config.SEED(1);
  config.MUTATION_RATE(0.0);
  config.MUTATION_SIZE(0.002);
  config.TRACK_PARENT_TASKS(1);
  config.VT_TASK_MATCH(1);
  config.ONLY_FIRST_TASK_CREDIT(0);

  SGPWorld world(random, &config, LogicTasks);

  ProgramBuilder not_program;
  not_program.AddNot();

  emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, not_program.Build(100));
  world.AddOrgAt(host, 0);
  GIVEN("Two Inputs"){
    //These two inputs were chosen because their is no output for any opeartion that is the same as another output
    host->GetCPU().state.input_buf.push(734856699);
    host->GetCPU().state.input_buf.push(1177728054);

  

    WHEN("WhichTaskDone is run on an organism with those inputs, The NOT of the first input, and ONLY_FIRST_TASK_CREDIT is 0"){
      //The result of applying the NOT bitwise opeartions to the binary form of 734856699
      long not_output = 3560110596;
      THEN("WhichTaskDone should return a 0"){
        int task_id = host->GetCPU().state.world->GetTaskSet().WhichTaskDone(host->GetCPU().state,not_output,0);
        REQUIRE(task_id == 0);
      }
    }

    WHEN("WhichTaskDone is run on an organism with those inputs, The NAND of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
      //The result of applying the NAND bitwise opeartions to the binary form of 734856699 and 1177728054
      long nand_output = 4261411789;
      THEN("WhichTaskDone should return a 1"){
        int task_id = host->GetCPU().state.world->GetTaskSet().WhichTaskDone(host->GetCPU().state,nand_output,0);
        REQUIRE(task_id == 1);
      }
    }

    WHEN("WhichTaskDone is run on an organism with those inputs, The AND of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
      //The result of applying the AND bitwise opeartions to the binary form of 734856699 and 1177728054
      long and_output = 33555506;
      THEN("WhichTaskDone should return a 2"){
        int task_id = host->GetCPU().state.world->GetTaskSet().WhichTaskDone(host->GetCPU().state,and_output,0);
        REQUIRE(task_id == 2);
      }
    }

    WHEN("WhichTaskDone is run on an organism with those inputs, The ORN of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
      //The result of applying the ORN bitwise opeartions to the binary form of 734856699 and 1177728054
      long orn_output = 3150794747;
      THEN("WhichTaskDone should return a 3"){
        int task_id = host->GetCPU().state.world->GetTaskSet().WhichTaskDone(host->GetCPU().state,orn_output,0);
        REQUIRE(task_id == 3);
      }
    }

    WHEN("WhichTaskDone is run on an organism with those inputs, The OR of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
      //The result of applying the OR bitwise opeartions to the binary form of 734856699 and 1177728054
      long or_output = 1879029247;
      THEN("WhichTaskDone should return a 4"){
        int task_id = host->GetCPU().state.world->GetTaskSet().WhichTaskDone(host->GetCPU().state,or_output,0);
        REQUIRE(task_id == 4);
      }
    }

    WHEN("WhichTaskDone is run on an organism with those inputs, The ANDN of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
      //The result of applying the ANDN bitwise opeartions to the binary form of 734856699 and 1177728054
      long andn_output = 701301193;
      THEN("WhichTaskDone should return a 5"){
        int task_id = host->GetCPU().state.world->GetTaskSet().WhichTaskDone(host->GetCPU().state,andn_output,0);
        REQUIRE(task_id == 5);
      }
    }

    WHEN("WhichTaskDone is run on an organism with those inputs, The NOR of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
      //The result of applying the NOR bitwise opeartions to the binary form of 734856699 and 1177728054
      long nor_output = 2415938048;
      THEN("WhichTaskDone should return a 6"){
        int task_id = host->GetCPU().state.world->GetTaskSet().WhichTaskDone(host->GetCPU().state,nor_output,0);
        REQUIRE(task_id == 6);
      }
    }

    WHEN("WhichTaskDone is run on an organism with those inputs, The XOR of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
      //The result of applying the XOR bitwise opeartions to the binary form of 734856699 and 1177728054
      long xor_output = 1845473741;
      THEN("WhichTaskDone should return a 7"){
        int task_id = host->GetCPU().state.world->GetTaskSet().WhichTaskDone(host->GetCPU().state,xor_output,0);
        REQUIRE(task_id == 7);
      }
    }

    WHEN("WhichTaskDone is run on an organism with those inputs, The EQU of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
      //The result of applying the EQU bitwise opeartions to the binary form of 734856699 and 1177728054
      long equ_output = 2449493554;
      THEN("WhichTaskDone should return a 8"){
        int task_id = host->GetCPU().state.world->GetTaskSet().WhichTaskDone(host->GetCPU().state,equ_output,0);
        REQUIRE(task_id == 8);
      }
    }
  }
}

TEST_CASE("Task completion edge cases", "[sgp]") { // IsSolved Edge casses documentation
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