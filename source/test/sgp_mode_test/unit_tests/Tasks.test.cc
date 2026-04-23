#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"
#include "../../../sgp_mode/CPUState.h"

#include "../../../catch/catch.hpp"

/**
 * This file is dedicated to unit tests for Tasks
 */

// TEST_CASE("Only first task credit for hosts vs. symbionts","[sgp][sgp-unit]"){
//   GIVEN("A host and symbiont can theoretically do both NOT and NAND tasks"){
//     emp::Random random(1);
//     SymConfigSGP config;
//     config.SEED(1);
//     config.MUTATION_RATE(0.0);
//     config.MUTATION_SIZE(0.002);
//     config.TRACK_PARENT_TASKS(PARENTONLY);
//     config.VT_TASK_MATCH(1);

//     SGPWorld world(random, &config, LogicTasks);

//     //Builds program that does both NOT and NAND operations
//     ProgramBuilder program;
//     program.AddNot();
//     program.AddNand();
      
//     //Creates a host that does both Not and Nand operations
//     emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, program.Build(100));
      
//     //Creates a symbiont that can not do any tasks
//     emp::Ptr<SGPSymbiont> symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config, program.Build(100));

//     //Adds host to world and sym to host.
//     world.AddOrgAt(host, 0);
//     host->AddSymbiont(symbiont);
  
//     WHEN("Only first task credit for hosts is on"){
//       config.HOST_ONLY_FIRST_TASK_CREDIT(1);
  
//       WHEN("Only first task credit for symbionts is off"){
//         config.SYM_ONLY_FIRST_TASK_CREDIT(0);
//         host->GetCPU().RunCPUStep(0, 100);
//         symbiont->GetCPU().RunCPUStep(0, 100);
//         THEN("The host only gets credit for NOT"){
//           int tasks_completed = 0;
//           for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
//             tasks_completed += host->GetCPU().state.tasks_performed->Get(i);
//           }
//           REQUIRE(tasks_completed == 1);
//           REQUIRE(host->GetPoints() == 5);
//         }
//         THEN("The symbiont gets credit for both NOT and NAND"){
//           int tasks_completed = 0;
//           for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
//             tasks_completed += symbiont->GetCPU().state.tasks_performed->Get(i);
//           }
//           REQUIRE(tasks_completed == 2);
//           REQUIRE(symbiont->GetPoints() == 10);
//         }
//       }

//       WHEN("Only first task credit for symbionts is on"){
//         config.SYM_ONLY_FIRST_TASK_CREDIT(1);
//         host->GetCPU().RunCPUStep(0, 100);
//         symbiont->GetCPU().RunCPUStep(0, 100);
//         THEN("The host only gets credit for NOT"){
//           int tasks_completed = 0;
//           for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
//             tasks_completed += host->GetCPU().state.tasks_performed->Get(i);
//           }
//           REQUIRE(tasks_completed == 1);
//           REQUIRE(host->GetPoints() == 5);
//         }
//         THEN("The symbiont only gets credit for NOT"){
//           int tasks_completed = 0;
//           for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
//             tasks_completed += symbiont->GetCPU().state.tasks_performed->Get(i);
//           }
//           REQUIRE(tasks_completed == 1);
//           REQUIRE(symbiont->GetPoints() == 5);
//         }
//       }
//     }
 
//     WHEN("Only first task credit for hosts is off"){
//       config.HOST_ONLY_FIRST_TASK_CREDIT(0);
//       WHEN("Only first task credit for symbionts is on"){
//         config.SYM_ONLY_FIRST_TASK_CREDIT(1);
//         host->GetCPU().RunCPUStep(0, 100);
//         symbiont->GetCPU().RunCPUStep(0, 100);
//         THEN("The host gets credit for both NOT and NAND"){
//           int tasks_completed = 0;
//           for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
//             tasks_completed += host->GetCPU().state.tasks_performed->Get(i);
//           }
//           REQUIRE(tasks_completed == 2);
//           REQUIRE(host->GetPoints() == 10);
//         }
//         THEN("The symbiont only gets credit for NOT"){
//           int tasks_completed = 0;
//           for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
//             tasks_completed += symbiont->GetCPU().state.tasks_performed->Get(i);
//           }
//           REQUIRE(tasks_completed == 1);
//           REQUIRE(symbiont->GetPoints() == 5);
//         }
//       }

//       WHEN("Only first task credit for symbionts is off"){
//         config.SYM_ONLY_FIRST_TASK_CREDIT(0);
//         host->GetCPU().RunCPUStep(0, 100);
//         symbiont->GetCPU().RunCPUStep(0, 100);
//         THEN("The host gets credit for both NOT and NAND"){
//           int tasks_completed = 0;
//           for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
//             tasks_completed += host->GetCPU().state.tasks_performed->Get(i);
//           }
//           REQUIRE(tasks_completed == 2);
//           REQUIRE(host->GetPoints() == 10);
//         }
//         THEN("The symbiont gets credit for both NOT and NAND"){
//           int tasks_completed = 0;
//           for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
//             tasks_completed += symbiont->GetCPU().state.tasks_performed->Get(i);
//           }
//           REQUIRE(tasks_completed == 2);
//           REQUIRE(symbiont->GetPoints() == 10);
//         }
//       }
//     }
//   }
  
// }

// TODO: The below are good ideas but don't fit the new structure of the tasks, can they be refactored to be useful?

// TEST_CASE("IsOnlyTask functionality", "[sgp][sgp-unit]") {
//   GIVEN("a TaskSet"){
//     emp::Random random(1);
//     SymConfigSGP config;
//     SGPWorld world(random, &config, LogicTasks);

//     Organism org;
//     CPUState state(&org,&world);

//     WHEN("No tasks have been completed yet") {
//       THEN("IsOnlyTask should return true for first task") {
//         // Simulate completing the first task (NOT)
//         state.tasks_performed->Set(0); // Mark NOT as completed
        
//         // Create a non const copy of LogicTasks for testing
//         TaskSet test_tasks = LogicTasks;
//         bool is_only = test_tasks.IsOnlyTask(state, 0);
//         REQUIRE(is_only == true);
//       }
//     }

//     WHEN("One task has already been completed") {
//       THEN("IsOnlyTask should return false for second task") {
//         // Mark first task as completed
//         state.tasks_performed->Set(0); // NOT completed
        
//         // Create a non const copy of LogicTasks for testing
//         TaskSet test_tasks = LogicTasks;
//         // Check if second task NAND is the only task
//         bool is_only = test_tasks.IsOnlyTask(state, 1);
//         REQUIRE(is_only == false);
//       }

//       THEN("IsOnlyTask should return true for the first completed task") {
//         // Mark first task as completed
//         state.tasks_performed->Set(0); // NOT completed
        
//         // Create a non const copy of LogicTasks for testing
//         TaskSet test_tasks = LogicTasks;
//         // Check if first task is the only task
//         bool is_only = test_tasks.IsOnlyTask(state, 0);
//         REQUIRE(is_only == true);
//       }
//     }

//     WHEN("Multiple tasks have been completed") {
//       THEN("IsOnlyTask should return false for any task") {
//         // Mark multiple tasks
//         state.tasks_performed->Set(0);
//         state.tasks_performed->Set(1);
        
//         // Create a non const copy of LogicTasks for testing
//         TaskSet test_tasks = LogicTasks;
//         // Check if any task is the only task
//         bool is_only_not = test_tasks.IsOnlyTask(state, 0);
//         bool is_only_nand = test_tasks.IsOnlyTask(state, 1);
//         bool is_only_and = test_tasks.IsOnlyTask(state, 2);
        
//         REQUIRE(is_only_not == false);
//         REQUIRE(is_only_nand == false);
//         REQUIRE(is_only_and == false);
//       }
//     }
//   }
// }

// TEST_CASE("WhichTaskDone Functionality", "[sgp][sgp-unit]"){
//   emp::Random random(1);
//   SymConfigSGP config;
//   SGPWorld world(random, &config, LogicTasks);

//   Organism org;
//   CPUState state(&org,&world);

//   TaskSet test_tasks = LogicTasks;
//   GIVEN("Two Inputs"){
//     //These two inputs were chosen because their is no output for any opeartion that is the same as another output
//     state.input_buf.push(734856699);
//     state.input_buf.push(1177728054);

//     WHEN("WhichTaskDone is run on an organism with those inputs, The NOT of the first input, and ONLY_FIRST_TASK_CREDIT is 0"){
//       //The result of applying the NOT bitwise opeartions to the binary form of 734856699
//       long not_output = 3560110596;
//       THEN("WhichTaskDone should return a 0"){
//         int task_id = test_tasks.WhichTaskDone(state,not_output,0);
//         REQUIRE(task_id == 0);
//       }
//     }

//     WHEN("WhichTaskDone is run on an organism with those inputs, The NAND of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
//       //The result of applying the NAND bitwise opeartions to the binary form of 734856699 and 1177728054
//       long nand_output = 4261411789;
//       THEN("WhichTaskDone should return a 1"){
//         int task_id = test_tasks.WhichTaskDone(state,nand_output,0);
//         REQUIRE(task_id == 1);
//       }
//     }

//     WHEN("WhichTaskDone is run on an organism with those inputs, The AND of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
//       //The result of applying the AND bitwise opeartions to the binary form of 734856699 and 1177728054
//       long and_output = 33555506;
//       THEN("WhichTaskDone should return a 2"){
//         int task_id = test_tasks.WhichTaskDone(state,and_output,0);
//         REQUIRE(task_id == 2);
//       }
//     }

//     WHEN("WhichTaskDone is run on an organism with those inputs, The ORN of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
//       //The result of applying the ORN bitwise opeartions to the binary form of 734856699 and 1177728054
//       long orn_output = 3150794747;
//       THEN("WhichTaskDone should return a 3"){
//         int task_id = test_tasks.WhichTaskDone(state,orn_output,0);
//         REQUIRE(task_id == 3);
//       }
//     }

//     WHEN("WhichTaskDone is run on an organism with those inputs, The OR of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
//       //The result of applying the OR bitwise opeartions to the binary form of 734856699 and 1177728054
//       long or_output = 1879029247;
//       THEN("WhichTaskDone should return a 4"){
//         int task_id = test_tasks.WhichTaskDone(state,or_output,0);
//         REQUIRE(task_id == 4);
//       }
//     }

//     WHEN("WhichTaskDone is run on an organism with those inputs, The ANDN of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
//       //The result of applying the ANDN bitwise opeartions to the binary form of 734856699 and 1177728054
//       long andn_output = 701301193;
//       THEN("WhichTaskDone should return a 5"){
//         int task_id = test_tasks.WhichTaskDone(state,andn_output,0);
//         REQUIRE(task_id == 5);
//       }
//     }

//     WHEN("WhichTaskDone is run on an organism with those inputs, The NOR of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
//       //The result of applying the NOR bitwise opeartions to the binary form of 734856699 and 1177728054
//       long nor_output = 2415938048;
//       THEN("WhichTaskDone should return a 6"){
//         int task_id = test_tasks.WhichTaskDone(state,nor_output,0);
//         REQUIRE(task_id == 6);
//       }
//     }

//     WHEN("WhichTaskDone is run on an organism with those inputs, The XOR of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
//       //The result of applying the XOR bitwise opeartions to the binary form of 734856699 and 1177728054
//       long xor_output = 1845473741;
//       THEN("WhichTaskDone should return a 7"){
//         int task_id = test_tasks.WhichTaskDone(state,xor_output,0);
//         REQUIRE(task_id == 7);
//       }
//     }

//     WHEN("WhichTaskDone is run on an organism with those inputs, The EQU of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
//       //The result of applying the EQU bitwise opeartions to the binary form of 734856699 and 1177728054
//       long equ_output = 2449493554;
//       THEN("WhichTaskDone should return a 8"){
//         int task_id = test_tasks.WhichTaskDone(state,equ_output,0);
//         REQUIRE(task_id == 8);
//       }
//     }
//   }
// }

// TEST_CASE("Task completion edge cases", "[sgp][sgp-unit]") { // IsSolved Edge casses documentation
//   GIVEN("A Task"){
//     emp::Random random(1);
//     SymConfigSGP config;
//     SGPWorld world(random, &config, LogicTasks);

//     Organism org;
//     CPUState state(&org,&world);
//     WHEN("Testing output values of 0 and 1 as a solution for the task") {
     
//       THEN("Outputs of 0 and 1 should not be considered valid task completions") {
//         // Create a NOT task to test with
//         Task not_task("NOT", 1, 5.0, [](auto &x) { return ~x[0]; });
        
//         // Test with output 0
//         bool is_solved_0 = not_task.IsSolved(state, 0);
//         REQUIRE(is_solved_0 == false);
        
//         // Test with output 1
//         bool is_solved_1 = not_task.IsSolved(state, 1);
//         REQUIRE(is_solved_1 == false);
//       }
//     }
//   }
// }