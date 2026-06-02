#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"


/**
 * This file is for testing the various aspects of symbiont reproduction, 
 * including horizontal and vertical transmission (not just when Reproduce instruction is called).
 */
using world_t = sgpmode::SGPWorld;
using cpu_state_t = sgpmode::CPUState<world_t>;
using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
using program_t = typename world_t::sgp_prog_t;
using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;

TEST_CASE("SGPSymbiont Reproduce", "[sgp][sgp-functional]") {
  GIVEN("An SGPWorld with a symbiont only able to perform NOT"){
  emp::Random random(31);
    size_t not_id = 1;
    sgpmode::SymConfigSGP config;
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.TASK_PROFILE_MODE("parent-all");
    world_t world(random, &config);
    world.Setup();
    auto& prog_builder = world.GetProgramBuilder();
    emp::Ptr<sgp_sym_t> sym_parent = emp::NewPtr<sgp_sym_t>(&random, &world, &config, prog_builder.CreateNotProgram(100));

    WHEN("The Symbiont reproduces"){
      emp::Ptr<sgp_sym_t> sym_baby = (sym_parent->Reproduce()).DynamicCast<sgp_sym_t>();
      THEN("Symbiont child increases its lineage reproduction count") {
        REQUIRE(sym_parent->GetReproCount() == sym_baby->GetReproCount() - 1);
      }

      WHEN("Parental task tracking is set to parent-all and parent does NOT before reproduction") {
        emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateNotProgram(100));
        
        world.AddOrgAt(host, 0);
        host->AddSymbiont(sym_parent);
        for (int i = 0; i < 50; i++) {
          world.Update();
        }

        emp::Ptr<sgp_sym_t> sym_baby = (sym_parent->Reproduce()).DynamicCast<sgp_sym_t>();
        world.AssignNewEnvIO(sym_baby->GetHardware().GetCPUState());

        THEN("All tasks are tracked correctly for three generations") {
          
          REQUIRE(sym_parent->GetHardware().GetCPUState().GetParentTasksPerformed().None());

          REQUIRE(sym_parent->GetHardware().GetCPUState().GetTasksPerformed().Get(not_id));
          REQUIRE(sym_parent->GetHardware().GetCPUState().GetTasksPerformed().CountOnes() == 1);

          REQUIRE(sym_baby->GetHardware().GetCPUState().GetParentTasksPerformed().Get(not_id));
          REQUIRE(sym_baby->GetHardware().GetCPUState().GetParentTasksPerformed().CountOnes() == 1);
        }

        THEN("The symbiont child tracks any gains or loses in task completions") {
          // in this second generation, we expect the symbiont to gain the NOT task
          // since first-gen organisms are marked as having parents who have completed no tasks
          REQUIRE(sym_baby->GetHardware().GetCPUState().GetLineageTaskLossCount(not_id) == 0);
          REQUIRE(sym_baby->GetHardware().GetCPUState().GetLineageTaskGainCount(not_id) == 1);
          size_t num_tasks = sym_baby->GetHardware().GetCPUState().GetLineageTaskGain().size();
          for (int i=0; i<num_tasks; i++) {
            if (i != not_id) {
              REQUIRE(sym_baby->GetHardware().GetCPUState().GetLineageTaskGainCount(i) == 0);  
              REQUIRE(sym_baby->GetHardware().GetCPUState().GetLineageTaskLossCount(i) == 0);   
            }
          }
        }

        THEN("The symbiont child tracks how its tasks compare to its parent's partner's tasks") {
          REQUIRE(sym_baby->GetHardware().GetCPUState().GetLineageTaskConvergeToPartner(not_id) == 0);
          REQUIRE(sym_baby->GetHardware().GetCPUState().GetLineageTaskDivergeFromPartner(not_id) == 1);

          size_t num_tasks = sym_baby->GetHardware().GetCPUState().GetLineageTaskGain().size();
          for (int i=0; i<num_tasks; i++) {
            if (i != not_id) {
              REQUIRE(sym_baby->GetHardware().GetCPUState().GetLineageTaskGainCount(i) == 0);  
              REQUIRE(sym_baby->GetHardware().GetCPUState().GetLineageTaskLossCount(i) == 0);   
            }
          }
        }
        
      }
      sym_baby.Delete();
    }
  }
}

TEST_CASE("SGPSymbiont Vertical Transmission", "[sgp][sgp-functional]"){
  //Note: Catch automatically reruns everything from the top of the test case for each GIVEN and WHEN, so the world/orgs get reset
  emp::Random random(51);
  sgpmode::SymConfigSGP config;
  config.GRID_X(10);
  config.GRID_Y(10);
  config.HOST_REPRO_RES(0);
  config.SYM_VERT_TRANS_RES(0);
  config.HORIZ_TRANS(0);
  config.DATA_INT(26);
  config.VERTICAL_TRANSMISSION(1);
  config.POP_SIZE(0);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  GIVEN("A host infected with a symbiont in the world when self-all task mode and task match required for vt"){
    config.TASK_PROFILE_MODE("self-all");
    config.VT_TASK_MATCH(true);
    world_t world(random, &config);
    world.Setup();
    auto& prog_builder = world.GetProgramBuilder();

    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));
    emp::Ptr<sgp_sym_t> sym = emp::NewPtr<sgp_sym_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));

    world.AddOrgAt(host, 0);
    host->AddSymbiont(sym);
    REQUIRE(world.GetNumOrgs() == 1);
    WHEN("Host and Symbiont share a matching task and host reproduces"){
      sym->GetHardware().GetCPUState().MarkTaskPerformed(0);
      host->GetHardware().GetCPUState().MarkTaskPerformed(0);
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont vertically transmits"){
        REQUIRE(world.GetVerticalTransmissionAttemptCount().GetCount() == 1);
        REQUIRE(world.GetVerticalTransmissionSuccessCount().GetCount() == 1);
      }
    }
    WHEN("Host and Symbiont share a matching parent task, but not self task, and host reproduces"){
      sym->GetHardware().GetCPUState().SetParentTaskPerformed(0);
      host->GetHardware().GetCPUState().SetParentTaskPerformed(0);
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont tries but fails to vertically transmit"){
        REQUIRE(world.GetVerticalTransmissionAttemptCount().GetCount() == 1);
        REQUIRE(world.GetVerticalTransmissionSuccessCount().GetCount() == 0);
      }
    }

    WHEN("Host and Symbiont do not share a matching task and host reproduces"){
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont tries, but does not vertically transmit"){
        REQUIRE(world.GetVerticalTransmissionAttemptCount().GetCount() == 1);
        REQUIRE(world.GetVerticalTransmissionSuccessCount().GetCount() == 0);
      }
    }
  }

  GIVEN("A host infected with a symbiont in the world when parent-all task mode and task match required for vt"){
    config.TASK_PROFILE_MODE("parent-all");
    config.VT_TASK_MATCH(true);
    world_t world(random, &config);
    world.Setup();
    auto& prog_builder = world.GetProgramBuilder();

    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));
    emp::Ptr<sgp_sym_t> sym = emp::NewPtr<sgp_sym_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));

    world.AddOrgAt(host, 0);
    host->AddSymbiont(sym);
    REQUIRE(world.GetNumOrgs() == 1);
    WHEN("Host and Symbiont share a matching task and host reproduces"){
      sym->GetHardware().GetCPUState().MarkTaskPerformed(0);
      host->GetHardware().GetCPUState().MarkTaskPerformed(0);
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont tries, but fails to, vertically transmit"){
        REQUIRE(world.GetVerticalTransmissionAttemptCount().GetCount() == 1);
        REQUIRE(world.GetVerticalTransmissionSuccessCount().GetCount() == 0);
      }
    }
    WHEN("Host and Symbiont share a matching parent task, but not self task, and host reproduces"){
      sym->GetHardware().GetCPUState().SetParentTaskPerformed(0);
      host->GetHardware().GetCPUState().SetParentTaskPerformed(0);
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont vertically transmits"){
        REQUIRE(world.GetVerticalTransmissionAttemptCount().GetCount() == 1);
        REQUIRE(world.GetVerticalTransmissionSuccessCount().GetCount() == 1);
      }
    }

    WHEN("Host and Symbiont do not share a matching task and host reproduces"){
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont tries, but does not vertically transmit"){
        REQUIRE(world.GetVerticalTransmissionAttemptCount().GetCount() == 1);
        REQUIRE(world.GetVerticalTransmissionSuccessCount().GetCount() == 0);
      }
    }
  }

  GIVEN("A host infected with a symbiont in the world when VT task match is off"){
    config.TASK_PROFILE_MODE("self-all");
    config.VT_TASK_MATCH(false);
    world_t world(random, &config);
    world.Setup();
    auto& prog_builder = world.GetProgramBuilder();

    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));
    emp::Ptr<sgp_sym_t> sym = emp::NewPtr<sgp_sym_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));

    world.AddOrgAt(host, 0);
    host->AddSymbiont(sym);
    REQUIRE(world.GetNumOrgs() == 1);
    WHEN("Host and Symbiont share a matching task and host reproduces"){
      sym->GetHardware().GetCPUState().MarkTaskPerformed(0);
      host->GetHardware().GetCPUState().MarkTaskPerformed(0);
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont vertically transmits"){
        REQUIRE(world.GetVerticalTransmissionAttemptCount().GetCount() == 1);
        REQUIRE(world.GetVerticalTransmissionSuccessCount().GetCount() == 1);
      }
    }

    WHEN("Host and Symbiont do not share a matching task and host reproduces"){
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont vertically transmits"){
        REQUIRE(world.GetVerticalTransmissionAttemptCount().GetCount() == 1);
        REQUIRE(world.GetVerticalTransmissionSuccessCount().GetCount() == 1);
      }
    }
  }

}
 
TEST_CASE("SGPSymbiont Vertical Transmission off", "[sgp][sgp-functional]"){
  //Note: Catch automatically reruns everything from the top of the test case for each WHEN, so the world/orgs get reset
  emp::Random random(51);
  sgpmode::SymConfigSGP config;
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.GRID_X(10);
  config.GRID_Y(10);
  config.DATA_INT(26);
  config.HOST_REPRO_RES(0);
  config.SYM_VERT_TRANS_RES(0);
  config.HORIZ_TRANS(0);
  config.POP_SIZE(0);
  config.VERTICAL_TRANSMISSION(0);
  GIVEN("A host infected with a symbiont in the world"){
    world_t world(random, &config);
    world.Setup();
    auto& prog_builder = world.GetProgramBuilder();

    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));
    emp::Ptr<sgp_sym_t> sym = emp::NewPtr<sgp_sym_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));

    world.AddOrgAt(host, 0);
    host->AddSymbiont(sym);
    REQUIRE(world.GetNumOrgs() == 1);
    WHEN("Host and Symbiont share a matching task and matching parent task and host reproduces"){
      sym->GetHardware().GetCPUState().MarkTaskPerformed(0);
      host->GetHardware().GetCPUState().MarkTaskPerformed(0);
      sym->GetHardware().GetCPUState().SetParentTaskPerformed(0);
      host->GetHardware().GetCPUState().SetParentTaskPerformed(0);
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont does not vertically transmit"){
        REQUIRE(world.GetVerticalTransmissionAttemptCount().GetCount() == 0);
        REQUIRE(world.GetVerticalTransmissionSuccessCount().GetCount() == 0);
      }
    }
  }
}


// TEST_CASE("SGPSymbiont Horizontal Transmission", "[sgp][sgp-functional]"){
//   GIVEN("Two hosts one of which is infected with a symbiont"){
//     emp::Random random(42);
//     SymConfigSGP config;
//     config.INTERACTION_MECHANISM(DEFAULT);
//     config.GRID_X(10);
//     config.GRID_Y(10);
//     config.HOST_REPRO_RES(100);
//     config.SYM_HORIZ_TRANS_RES(0);
//     config.VERTICAL_TRANSMISSION(0);

//     SGPWorld world(random, &config, LogicTasks);

//     ProgramBuilder builder;
//     ProgramBuilder symbuilder;
//     symbuilder.AddNot(); //First task
//     emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, builder.Build(100));
//     emp::Ptr<SGPHost> host2 = emp::NewPtr<SGPHost>(&random, &world, &config, builder.Build(100));
//     emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, symbuilder.Build(100));

//     world.AddOrgAt(host, 0);
//     world.AddOrgAt(host2,1);
//     host->AddSymbiont(sym);

//     REQUIRE(world.GetNumOrgs() == 2);

//     WHEN("Horizontal Transmission is on"){
//       config.HORIZ_TRANS(1);
//       WHEN("Horizontal Task Match is on"){
//         config.HT_TASK_MATCH(1);
//         WHEN("Track Parent Tasks is off"){
//           config.TRACK_PARENT_TASKS(CURRENTONLY);
//           //Selects correct task match function based on config
//           //used here to avoid having to create new worlds and organism
//           world.SetupTaskProfileFun();
//           WHEN("Only Host and Symbiont share a matching task"){
            
//             sym->GetCPU().state.tasks_performed->Set(0);
//             host2->GetCPU().state.tasks_performed->Set(0);
//             WHEN("The symbiont runs the reproduce instruction"){
//               for(int i = 0; i < 26; i++){
                
//                 world.Update();
//               }
//               THEN("Symbiont horizontally transmits"){
//                 REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetTotal() == 1);
//                 REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetTotal() == 1);
//               }
//             }
//           }
//           WHEN("Only Host parent and Symbiont parent share a matching task"){
            
//             sym->GetCPU().state.parent_tasks_performed->Set(0);
//             host2->GetCPU().state.parent_tasks_performed->Set(0);
//             WHEN("The symbiont runs the reproduce instruction"){
//               for(int i = 0; i < 26; i++){
                
//                 world.Update();
//               }
//               THEN("Symbiont does not horizontally transmit"){
//                 REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetTotal() == 1);
//                 REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetTotal() == 0);
//               }
//             }
//           }
//           WHEN("Host and Symbiont do not share a matching task"){
            
//             sym->GetCPU().state.tasks_performed->Set(0);
//             WHEN("The symbiont runs the reproduce instruction"){
//               for(int i = 0; i < 26; i++){
                
//                 world.Update();
//               }
//               THEN("Symbiont does not horizontally transmit"){
//                 REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetTotal() == 1);
//                 REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetTotal() == 0);
//               }
//             }
//           }
//         }

//         WHEN("Track Parent Tasks is set to parent only"){
//           config.TRACK_PARENT_TASKS(PARENTONLY);
//           //Selects correct task match function based on config
//           //used here to avoid having to create new worlds and organism
//           world.SetupTaskProfileFun();
//           WHEN("Only Host and Symbiont share a matching task"){

//             sym->GetCPU().state.tasks_performed->Set(0);
//             host2->GetCPU().state.tasks_performed->Set(0);
//             WHEN("The symbiont runs the reproduce instruction"){
//               for(int i = 0; i < 26; i++){
            
//                 world.Update();
//               }
//               THEN("Symbiont does not horizontally transmits"){
//                 REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetTotal() == 1);
//                 REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetTotal() == 0);
//               }
//             }
//           }
//           WHEN("Only Host parent and Symbiont parent share a matching task"){
            
//             sym->GetCPU().state.parent_tasks_performed->Set(0);
//             host2->GetCPU().state.parent_tasks_performed->Set(0);
//             WHEN("The symbiont runs the reproduce instruction"){
//               for(int i = 0; i < 26; i++){
                
//                 world.Update();
//               }
//               THEN("Symbiont horizontally transmits"){
//                 REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetTotal() == 1);
//                 REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetTotal() == 1);
//               }
//             }
//           }
//           WHEN("Host and Symbiont do not share a matching task"){
            
//             sym->GetCPU().state.tasks_performed->Set(0);
//             WHEN("The symbiont runs the reproduce instruction"){
//               for(int i = 0; i < 26; i++){
                
//                 world.Update();
//               }
//               THEN("Symbiont does not horizontally transmit"){
//                 REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetTotal() == 1);
//                 REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetTotal() == 0);
//               }
//             }
//           }
//         }

//         WHEN("Track Parent Tasks is set to both parent or child"){
//           config.TRACK_PARENT_TASKS(CURRENTORPARENT);
//           //Selects correct task match function based on config
//           //used here to avoid having to create new worlds and organism
//           world.SetupTaskProfileFun();
//           WHEN("Only Host and Symbiont share a matching task"){

//             sym->GetCPU().state.tasks_performed->Set(0);
//             host2->GetCPU().state.tasks_performed->Set(0); 
//             WHEN("The symbiont runs the reproduce instruction"){
//               for(int i = 0; i < 26; i++){
                
//                 world.Update();
//               }
//               THEN("Symbiont does not horizontally transmits"){
//                 REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetTotal() == 1);
//                 REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetTotal() == 1);
//               }
//             }
//           }
//           WHEN("Only Host parent and Symbiont parent share a matching task"){
            
//             sym->GetCPU().state.parent_or_current_tasks_performed->Set(0);
//             host2->GetCPU().state.parent_or_current_tasks_performed->Set(0);
//             WHEN("The symbiont runs the reproduce instruction"){
//               for(int i = 0; i < 26; i++){
                
//                 world.Update();
//               }
//               THEN("Symbiont horizontally transmits"){
//                 REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetTotal() == 1);
//                 REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetTotal() == 1);
//               }
//             }
//           }
//           WHEN("Host and Symbiont do not share a matching task"){
            
//             sym->GetCPU().state.tasks_performed->Set(0);
//             WHEN("The symbiont runs the reproduce instruction"){
//               for(int i = 0; i < 26; i++){
                
//                 world.Update();
//               }
//               THEN("Symbiont does not horizontally transmit"){
//                 REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetTotal() == 1);
//                 REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetTotal() == 0);
//               }
//             }
//           }
//         }
//       }
//       WHEN("Horizontal Task Match is off"){
//         config.HT_TASK_MATCH(0);
//         WHEN("Host and Symbiont share a matching task"){

//           sym->GetCPU().state.tasks_performed->Set(0);
//           host2->GetCPU().state.tasks_performed->Set(0);
//           WHEN("The symbiont runs the reproduce instruction"){
//             for(int i = 0; i < 26; i++){
              
//               world.Update();
//             }
//             THEN("Symbiont horizontally transmits"){
//               REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetTotal() == 1);
//               REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetTotal() == 1); 
//             }
//           }
//         }
//         WHEN("Host and Symbiont do not share a matching task"){
        
//           sym->GetCPU().state.tasks_performed->Set(0);
//           WHEN("The symbiont runs the reproduce instruction"){
//             for(int i = 0; i < 26; i++){
              
//               world.Update();
//             }
//             THEN("Symbiont horizontally transmits"){
//               REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetTotal() == 1);
//               REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetTotal() == 1);
//             }
//           }
//         }
//       }
//     }
//     WHEN("Horizontal Transmission is off"){
//       config.HORIZ_TRANS(0);
//       config.HT_TASK_MATCH(0);
//       WHEN("The symbiont runs the reproduce instruction"){
//         for(int i = 0; i < 26; i++){
//               world.Update();
//             }
//         THEN("Symbiont does not horizontally transmit"){
//           REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetTotal() == 0);
//           REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetTotal() == 0);
//         }
//       }
//     }
//   }
// }