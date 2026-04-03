#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPHost.cc"
#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"
#include "../../../sgp_mode/SGPDataNodes.h"

/**
 * This file is dedicated to ensuring that HOST_ONLY_FIRST_TASK_CREDIT works correctly with SGPHost
 */

// using world_t = sgpmode::SGPWorld;
// using cpu_state_t = sgpmode::CPUState<world_t>;
// using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
// using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
// using program_t = typename world_t::sgp_prog_t;
// using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;


// TEST_CASE("When HOST_ONLY_FIRST_TASK_CREDIT is 1, the most tasks a Host can receive credit for is 1", "[sgp][sgp-functional]"){
//   GIVEN("A host in a world where HOST_ONLY_FIRST_TASK_CREDIT is on"){
//     emp::Random random(1);
//     sgpmode::SymConfigSGP config;
//     config.SEED(1);
//     config.MUTATION_RATE(0.0);
//     config.MUTATION_SIZE(0.002);
//     //config.TRACK_PARENT_TASKS(PARENTONLY);
//     config.VT_TASK_MATCH(1);
//     config.HOST_ONLY_FIRST_TASK_CREDIT(1);
//     config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/functional_tests/hardware-test-env.json");

//     world_t world(random, &config);

//     WHEN("the host is able to complete both NOT and NAND tasks"){

//       auto& empty_program = world.GetProgramBuilder().CreateReproProgram(100);

//       //Creates a host that does both Not and Nand operations
//       emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, world.GetProgramBuilder().CreateNotNandProgram(100));
      
//       //Adds host to world
//       world.AddOrgAt(host, 0);

//       WHEN("the host completes both NOT and NAND tasks"){
//         host->GetHardware().GetCPU().RunCPUStep(100);
        
//         int tasks_completed = 0;
//         for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
//           tasks_completed += host->GetHardware().GetCPUState().GetTaskPerformed(i);
//         }
//         THEN("the host should only get credit for completing 1 task"){
//           REQUIRE(tasks_completed == 1);
//         }
//       }
//     }

//     WHEN("the host are able to complete all tasks"){  

//       // TODO next switch to ProgramBuilder.test.cc structure

//       //Builds program that does all tasks
//       auto& prog_builder = world.GetProgramBuilder();
//       program_t program;
//       prog_builder.AddStartAnchor(program);
//       prog_builder.AddTask_Not(program);
//       prog_builder.AddTask_Nand(program);
//       prog_builder.AddTask_And(program);
//       prog_builder.AddTask_Orn(program);
//       prog_builder.AddTask_Or(program);
//       prog_builder.AddTask_Andn(program);
//       prog_builder.AddTask_Nor(program);
//       prog_builder.AddTask_Xor(program);
//       prog_builder.AddTask_Equ(program);

      

//       //Creates a host that can do all tasks
//       emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, program);
      
//       //Adds host to world
//       world.AddOrgAt(host, 0);

//       WHEN("the host completes all tasks"){
//         host->GetCPU().RunCPUStep(0, 100);
        
//         int tasks_completed = 0;
//         for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
//           tasks_completed += host->GetCPU().state.tasks_performed->Get(i);
//         }
//         THEN("the host should only get credit for completing 1 task"){
//           REQUIRE(tasks_completed == 1);
//         }
//       }
//     }

//     WHEN("the host is unable to complete any tasks"){
//       //Empty Builder
//       ProgramBuilder program;
//       //Creates a host that cannot do any tasks
//       emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, program.Build(100));

//       //Adds host to world
//       world.AddOrgAt(host, 0);

//       WHEN("the host completes no tasks"){
//         host->GetCPU().RunCPUStep(0, 100);
        
//         int tasks_completed = 0;
//         for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
//           tasks_completed += host->GetCPU().state.tasks_performed->Get(i);
//         }
//         THEN("the host should not get credit for any tasks"){
//           REQUIRE(tasks_completed == 0);
//         }
//       }
//     }
//   }
// }

// TEST_CASE("When HOST_ONLY_FIRST_TASK_CREDIT is 0, hosts receive credit for all tasks they complete", "[sgp][sgp-functional]"){
//   GIVEN("A host in a world where HOST_ONLY_FIRST_TASK_CREDIT off"){
//     emp::Random random(1);
//     SymConfigSGP config;
//     config.SEED(2);
//     config.MUTATION_RATE(0.0);
//     config.MUTATION_SIZE(0.002);
//     config.TRACK_PARENT_TASKS(PARENTONLY);
//     config.VT_TASK_MATCH(1);
//     config.HOST_ONLY_FIRST_TASK_CREDIT(0);

//     SGPWorld world(random, &config, LogicTasks);

//     WHEN("the host is able to complete both NOT and NAND tasks"){
//       ProgramBuilder program;
//       program.AddNot();
//       program.AddNand();

//       ProgramBuilder empty_program;

//       //Creates a host that can do both Not and NAND operations
//       emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, program.Build(100));
      
//       //Adds host to world
//       world.AddOrgAt(host, 0);

//       WHEN("the host completes both NOT and NAND tasks"){
//         host->GetCPU().RunCPUStep(0, 100);
        
//         int tasks_completed = 0;
//         for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
//           tasks_completed += host->GetCPU().state.tasks_performed->Get(i);
//         }
//         THEN("the host should receive credit for completing 2 tasks"){
//           REQUIRE(tasks_completed == 2);
//         }
//       }
//     }

//     WHEN("the host is able to complete all tasks"){
//       //Builds program that completes all tasks
//       ProgramBuilder program;
//       program.AddNot();
//       program.AddNand();
//       program.AddAnd();
//       program.AddOrn();
//       program.AddOr();
//       program.AddAndn();
//       program.AddNor();
//       program.AddXor();
//       program.AddEqu();

//       ProgramBuilder empty_program;

//       //Creates a host that can do all tasks
//       emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, program.Build(100));
      
//       //Adds host to world and sym to host.
//       world.AddOrgAt(host, 0);

//       WHEN("the host completes all tasks"){
//         host->GetCPU().RunCPUStep(0, 100);
        
//         int tasks_completed = 0;
//         for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
//           tasks_completed += host->GetCPU().state.tasks_performed->Get(i);
//         }
//         THEN("the host should get credit for completing all 9 tasks"){
//           REQUIRE(tasks_completed == 9);
//         }
//       }
//     }

//     WHEN("the host is unable to complete any tasks"){
//       //Empty Builder
//       ProgramBuilder program;
//       //Creates a host that cannot do any tasks
//       emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, program.Build(100));
      
//       //Adds host to world and sym to host.
//       world.AddOrgAt(host, 0);

//       WHEN("the host completes no tasks"){
//         host->GetCPU().RunCPUStep(0, 100);
        
//         int tasks_completed = 0;
//         for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
//           tasks_completed += host->GetCPU().state.tasks_performed->Get(i);
//         }
//         THEN("the host should not get credit for any tasks"){
//           REQUIRE(tasks_completed == 0);
//         }
//       }
//     }
//   }
// }
