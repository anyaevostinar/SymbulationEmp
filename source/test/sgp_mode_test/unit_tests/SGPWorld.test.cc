#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorld.cc"
#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"
//#include "../../../sgp_mode/SGPWorldData.cc"
#include "../../../catch/catch.hpp"

/**
 * This file is dedicated to unit tests for SGPWorld
 */



TEST_CASE("Update only hosts test", "[refactor]") {

  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;

  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.GRID_X(2);
  config.GRID_Y(2);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/functional_tests/hardware-test-env.json");


  world_t world(random, &config);
  world.Resize(2,2);

  emp::Ptr<sgp_host_t> uninfected_host = emp::NewPtr<sgp_host_t>(&random, &world, &config);

  world.AddOrgAt(uninfected_host, 0);
  // TODO: this doesn't work if you add at position 1 instead, is that a problem?

  THEN("Organisms can be added to the world") {
    REQUIRE(world.GetNumOrgs() == 1);
    REQUIRE(world.GetOrgPtr(0)->GetAge() == 0);
  }



  for (int i = 0; i < 10; i++) {
    world.Update();
  }

  THEN("Organisms persist and are managed by the world") {
    REQUIRE(world.GetNumOrgs() == 1);
    REQUIRE(world.GetOrgPtr(0)->GetAge() == 10);
  }
}



// TEST_CASE("Host Setup", "[sgp][sgp-unit]") {
//   emp::Random random(1);
//   SymConfigSGP config;
//   config.SEED(2);
//   config.MUTATION_RATE(0.0);
//   config.MUTATION_SIZE(0.000);
//   config.TRACK_PARENT_TASKS(PARENTONLY);
//   config.VT_TASK_MATCH(1);
//   config.HOST_ONLY_FIRST_TASK_CREDIT(1);
//   config.SYM_ONLY_FIRST_TASK_CREDIT(1);
//   config.HOST_REPRO_RES(10000);

//   //world.SetupHosts requires a pointer for the number of hosts in the world
//   unsigned long setupCount = 1;
//   WHEN("INTERACTION_MECHANISM Config is set to SGP hosts"){
//     config.INTERACTION_MECHANISM(0);
//     SGPWorld world(random, &config, LogicTasks);
//     world.SetupHosts(&setupCount);
//     THEN("The world contains a SGPHost"){
//       emp::Ptr<Organism> host =  world.GetOrgPtr(0);
//       REQUIRE(host->GetName() == "SGPHost");
//     }
//   }

//   WHEN("INTERACTION_MECHANISM Config is set to Health hosts"){
//     config.INTERACTION_MECHANISM(1);
//     SGPWorld world(random, &config, LogicTasks);
//     world.SetupHosts(&setupCount);
//     THEN("The world contains a HealtHost"){
//       emp::Ptr<Organism> host =  world.GetOrgPtr(0);
//       REQUIRE(host->GetName() == "HealthHost");
//     }
//   }

//   WHEN("INTERACTION_MECHANISM Config is set to Stress hosts"){
//     config.INTERACTION_MECHANISM(2);
//     SGPWorld world(random, &config, LogicTasks);
//     world.SetupHosts(&setupCount);
//     THEN("The world contains a StressHost"){
//       emp::Ptr<Organism> host =  world.GetOrgPtr(0);
//       REQUIRE(host->GetName() == "StressHost");
//     }
//   }

//   WHEN("INTERACTION_MECHANISM Config is set to Nutrient Mode"){
//     config.INTERACTION_MECHANISM(3);
//     SGPWorld world(random, &config, LogicTasks);
//     world.SetupHosts(&setupCount);
//     THEN("The world contains a SGPHost, Nutrient mode does not have it's own SGPHost subclass"){
//       emp::Ptr<Organism> host =  world.GetOrgPtr(0);
//       REQUIRE(host->GetName() == "SGPHost");
//     }
//   }

//   WHEN("INTERACTION_MECHANISM Config is set to an option that does not exist"){
//     config.INTERACTION_MECHANISM(4);
//     SGPWorld world(random, &config, LogicTasks);
//     THEN("An exception should be thrown"){
//       REQUIRE_THROWS(world.SetupHosts(&setupCount));
//     }
//   }
// }

// TEST_CASE("TaskMatchCheck Unit Test", "[sgp][sgp-unit]") {
//   GIVEN("Two task sets"){
//     emp::Random random(1);
//     SymConfigSGP config;
//     config.SEED(2);

//     SGPWorld world(random, &config, LogicTasks);
//     emp::BitSet<CPU_BITSET_LENGTH> host_tasks_performed = emp::BitSet<CPU_BITSET_LENGTH>(false);
//     emp::BitSet<CPU_BITSET_LENGTH> sym_tasks_performed = emp::BitSet<CPU_BITSET_LENGTH>(false);

//     WHEN("The two task sets share no completed tasks"){
//       host_tasks_performed.Set(1);
//       sym_tasks_performed.Set(2);
      
//       THEN("TaskMatchCheck returns false") {
//         REQUIRE(!world.TaskMatchCheck(sym_tasks_performed, host_tasks_performed));
//       }
//     }

//     WHEN("The two task sets share a completed task"){
//       host_tasks_performed.Set(1);
//       sym_tasks_performed.Set(1);
      
//       THEN("TaskMatchCheck returns true") {
//         REQUIRE(world.TaskMatchCheck(sym_tasks_performed, host_tasks_performed));
//       }
//     }
//   }
// }

// TEST_CASE("TaskMatchCheck for parents", "[sgp][sgp-unit]") {
//   GIVEN("A host and a symbiont"){
//     emp::Random random(1);
//     SymConfigSGP config;
//     config.SEED(2);
//     config.MUTATION_RATE(0.0);
//     config.MUTATION_SIZE(0.000);
//     config.TRACK_PARENT_TASKS(PARENTONLY);
//     config.VT_TASK_MATCH(1);
//     config.HOST_ONLY_FIRST_TASK_CREDIT(0);
//     config.SYM_ONLY_FIRST_TASK_CREDIT(0);
//     config.HOST_REPRO_RES(10000);

//     SGPWorld world(random, &config, LogicTasks);

//     //Creates a host that only does NOT operations
//     emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));

//     //Creates a symbiont that only does NOT operations
//     emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(100));

//     //Adds host to world and sym to host.
//     world.AddOrgAt(host, 0);
//     host->AddSymbiont(sym);
    
//     WHEN("TRACK_PARENT_TASKS is parent only"){
//       WHEN("The host's and symbiont's parents have both performed NOT"){
//         host->GetCPU().state.parent_tasks_performed->Set(0);
//         sym->GetCPU().state.parent_tasks_performed->Set(0);

//         THEN("TaskMatchCheck returns true when Host task set and Symbiont task set are the arguments"){
//           REQUIRE(world.TaskMatchCheck(world.fun_get_task_profile(sym), world.fun_get_task_profile(host)));
//         }
//       }
      
//       WHEN("The host and symbiont have both performed NOT"){
//         host->GetCPU().state.tasks_performed->Set(0);
//         sym->GetCPU().state.tasks_performed->Set(0);
//         WHEN("The host's parent has performed NAND and the symbiont's parent has performed EQU"){
//           host->GetCPU().state.parent_tasks_performed->Set(1);
//           sym->GetCPU().state.parent_tasks_performed->Set(8);

//           THEN("TaskMatchCheck returns false when Host task set and Symbiont task set are the arguments"){
//             REQUIRE(!world.TaskMatchCheck(world.fun_get_task_profile(sym), world.fun_get_task_profile(host)));
//           }
//         }
//       }
//       WHEN("The host and symbiont have performed no tasks"){
//         WHEN("The host's parent has performed NAND and the symbiont's parent has performed EQU"){
//           host->GetCPU().state.parent_tasks_performed->Set(1);
//           sym->GetCPU().state.parent_tasks_performed->Set(8);

//           THEN("TaskMatchCheck returns false when Host task set and Symbiont task set are the arguments"){
//             REQUIRE(!world.TaskMatchCheck(world.fun_get_task_profile(sym), world.fun_get_task_profile(host)));
//           }
//         }
//       }
//     }
//     WHEN("TRACK_PARENT_TASKS is parent or child"){
//       config.TRACK_PARENT_TASKS(CURRENTORPARENT);
//       world.SetupTaskProfileFun();
//       WHEN("The host and symbiont have both performed NOT"){
//         host->GetCPU().state.tasks_performed->Set(0);
//         sym->GetCPU().state.tasks_performed->Set(0);
//         WHEN("The host's parent has performed NAND and the symbiont's parent has performed EQU"){
//           host->GetCPU().state.parent_tasks_performed->Set(1);
//           sym->GetCPU().state.parent_tasks_performed->Set(8);

//           THEN("TaskMatchCheck returns true when Host task set and Symbiont task set are the arguments"){
//             REQUIRE(world.TaskMatchCheck(world.fun_get_task_profile(sym), world.fun_get_task_profile(host)));
//           }
//         }
//       }
//       WHEN("The host and symbiont have performed no tasks"){
//         WHEN("The host's parent has performed NAND and the symbiont's parent has performed EQU"){
//           host->GetCPU().state.parent_tasks_performed->Set(1);
//           sym->GetCPU().state.parent_tasks_performed->Set(8);

//           THEN("TaskMatchCheck returns false when Host task set and Symbiont task set are the arguments"){
//             REQUIRE(!world.TaskMatchCheck(world.fun_get_task_profile(sym), world.fun_get_task_profile(host)));
//           }
//         }
//       }
//     }
//   }
// }

// TEST_CASE("SGP SymDoBirth", "[sgp][sgp-unit]") {
//   GIVEN("A target host and an incoming symbiont"){
//     emp::Random random(1);
//     SymConfigSGP config;
//     config.SEED(2);
//     config.INTERACTION_MECHANISM(STRESS);
//     config.SYMBIONT_TYPE(PARASITE);
//     config.MUTATION_RATE(0.0);
//     config.MUTATION_SIZE(0.000);
//     config.TRACK_PARENT_TASKS(PARENTONLY);
//     SGPWorld world(random, &config, LogicTasks);
//     world.Resize(2, 2);

//     emp::Ptr<StressHost> source_host = emp::NewPtr<StressHost>(&random, &world, &config);
//     emp::Ptr<StressHost> target_host = emp::NewPtr<StressHost>(&random, &world, &config);

//     emp::Ptr<SGPSymbiont> symbiont_parent = emp::NewPtr<SGPSymbiont>(&random, &world, &config);
//     emp::Ptr<SGPSymbiont> target_symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config);
    
//     source_host->AddSymbiont(symbiont_parent);
//     target_host->AddSymbiont(target_symbiont);

//     emp::WorldPosition parent_pos = emp::WorldPosition(1,0);
//     emp::Ptr<Organism> symbiont_offspring = symbiont_parent->Reproduce();
    
//     // TRACK_PARENT_TASKS is on, so comparison for symbiont_offspring's injection is occurring between
//     // symbiont_parent's parent tasks (symbiont_offspring's grandparent tasks), target_host's parent tasks, and target_symbiont's parent tasks
//     target_host->GetCPU().state.parent_tasks_performed->Set(0);
//     target_host->GetCPU().state.parent_tasks_performed->Set(1);
//     symbiont_parent->GetCPU().state.parent_tasks_performed->Set(0);
//     target_symbiont->GetCPU().state.parent_tasks_performed->Set(0);

//     world.AddOrgAt(source_host, 0);
//     world.AddOrgAt(target_host, 1);

//     WHEN("Preferential ousting is on and the target host has a symbiont") {
//       config.OUSTING(1);
//       config.PREFERENTIAL_OUSTING(BETTERMATCH);
//       WHEN("The incoming symbiont has a better match"){
//         symbiont_parent->GetCPU().state.parent_tasks_performed->Set(1);
//         world.SymDoBirth(symbiont_offspring, parent_pos);
//         THEN("The incoming symbiont successfully ousts"){
//           REQUIRE(target_host->HasSym());
//           REQUIRE(target_host->GetSymbionts().at(0) == symbiont_offspring); 
//           REQUIRE(world.GetGraveyard().size() == 1);
//           REQUIRE(world.GetGraveyard().at(0).DynamicCast<SGPSymbiont>() == target_symbiont);
//         }
//         world.GetGraveyard().at(0).Delete();
//       }

//       WHEN("The incoming symbiont has a worse match"){
//         target_symbiont->GetCPU().state.parent_tasks_performed->Set(1);
//         world.SymDoBirth(symbiont_offspring, parent_pos);
//         THEN("The incoming symbiont does not oust") {
//           REQUIRE(target_host->GetSymbionts().at(0).DynamicCast<SGPSymbiont>() == target_symbiont);
//         }
//       }
//     }
//   }
// }

// TEST_CASE("PreferentialOustingAllowed", "[sgp][sgp-unit]"){
//   // pref ousting settings
//   // 0 = no preferential ousting, 
//   // 1 = the incoming symbiont must have an equal or better match than the current symbiont in order to oust
//   // 2 = the incoming symbiont must have a strictly better match than the current symbiont in order to oust
//   GIVEN("A host infected with a symbiont"){
//     emp::Random random(89);
//     SymConfigSGP config;
//     config.OUSTING(1);
//     SGPWorld world(random, &config, LogicTasks);
    
//     emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(PROGRAM_LENGTH));
//     emp::Ptr<SGPSymbiont> target_symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(PROGRAM_LENGTH));
//     emp::Ptr<SGPSymbiont> incoming_symbiont_parent = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(PROGRAM_LENGTH));
//     host->AddSymbiont(target_symbiont);

//     // host self:   000001101
//     // host parent: 100000100
//     host->GetCPU().state.tasks_performed->Set(8);
//     host->GetCPU().state.tasks_performed->Set(6);
//     host->GetCPU().state.tasks_performed->Set(5);
//     host->GetCPU().state.parent_tasks_performed->Set(0);
//     host->GetCPU().state.parent_tasks_performed->Set(6);

//     // hosted sym self:   000001001
//     // hosted sym parent: 100000100
//     target_symbiont->GetCPU().state.tasks_performed->Set(8);
//     target_symbiont->GetCPU().state.tasks_performed->Set(5);
//     target_symbiont->GetCPU().state.parent_tasks_performed->Set(6);
//     target_symbiont->GetCPU().state.parent_tasks_performed->Set(0);

//     WHEN("An ousting symbiont has worse task match with a host than the existing symbiont does"){
//       // host self:   000001101
//       // host parent: 100000100

//       // hosted sym self:   000001001 // match 2/3
//       // hosted sym parent: 100000100 // match 2/2

//       // incoming sym self:   000000001 // match 1/3
//       // incoming sym parent: 000000100 // match 1/2
//       incoming_symbiont_parent->GetCPU().state.tasks_performed->Set(8);
//       incoming_symbiont_parent->GetCPU().state.parent_tasks_performed->Set(6);

//       WHEN("Preferential ousting is off"){ // sanity check that setting is toggleable
//         config.PREFERENTIAL_OUSTING(OFF);
//         THEN("Ousting succeeds"){
//           REQUIRE(world.PreferentialOustingAllowed(world.fun_get_task_profile(incoming_symbiont_parent), host) == true);
//         }
//       }

//       WHEN("Preferential ousting is on"){
//         WHEN("Parental tasks are used"){
//           config.TRACK_PARENT_TASKS(PARENTONLY);
//           world.SetupTaskProfileFun();

//           WHEN("Same or better task match is required"){
//             config.PREFERENTIAL_OUSTING(EQUALMATCH);
//             THEN("Ousting fails"){
//               REQUIRE(world.PreferentialOustingAllowed(world.fun_get_task_profile(incoming_symbiont_parent), host) == false);
//             }
//           }

//           WHEN("Strictly better task match is required"){
//             config.PREFERENTIAL_OUSTING(BETTERMATCH);
//             THEN("Ousting fails"){
//               REQUIRE(world.PreferentialOustingAllowed(world.fun_get_task_profile(incoming_symbiont_parent), host) == false);
//             }
//           }
//         }

//         WHEN("Parental tasks are not used"){
//           config.TRACK_PARENT_TASKS(CURRENTONLY);
//           world.SetupTaskProfileFun();

//           WHEN("Same or better task match is required"){
//             config.PREFERENTIAL_OUSTING(EQUALMATCH);
//             THEN("Ousting fails"){
//               REQUIRE(world.PreferentialOustingAllowed(world.fun_get_task_profile(incoming_symbiont_parent), host) == false);
//             }
//           }

//           WHEN("Strictly better task match is required"){
//             config.PREFERENTIAL_OUSTING(BETTERMATCH);
//             THEN("Ousting fails"){
//               REQUIRE(world.PreferentialOustingAllowed(world.fun_get_task_profile(incoming_symbiont_parent), host) == false);
//             }
//           }
//         }
//       }
      
//     }

//     WHEN("An ousting symbiont has equal task match with a host than the existing symbiont does"){
//       // host self:   000001101
//       // host parent: 100000100

//       // hosted sym self:   000001001 // match 2/3
//       // hosted sym parent: 100000100 // match 2/2

//       // incoming sym self:   000000101 // match 2/3
//       // incoming sym parent: 100000100 // match 2/2
//       incoming_symbiont_parent->GetCPU().state.tasks_performed->Set(8);
//       incoming_symbiont_parent->GetCPU().state.tasks_performed->Set(6);
//       incoming_symbiont_parent->GetCPU().state.parent_tasks_performed->Set(0);
//       incoming_symbiont_parent->GetCPU().state.parent_tasks_performed->Set(6);

//       WHEN("Preferential ousting is on"){
//         WHEN("Parental tasks are used"){
//           config.TRACK_PARENT_TASKS(PARENTONLY);
//           world.SetupTaskProfileFun();

//           WHEN("Same or better task match is required"){
//             config.PREFERENTIAL_OUSTING(EQUALMATCH);
//             THEN("Ousting succeeds"){
//               REQUIRE(world.PreferentialOustingAllowed(world.fun_get_task_profile(incoming_symbiont_parent), host) == true);
//             }
//           }

//           WHEN("Strictly better task match is required"){
//             config.PREFERENTIAL_OUSTING(BETTERMATCH);
//             THEN("Ousting fails"){
//               REQUIRE(world.PreferentialOustingAllowed(world.fun_get_task_profile(incoming_symbiont_parent), host) == false);
//             }
//           }
//         }

//         WHEN("Parental tasks are not used"){
//           config.TRACK_PARENT_TASKS(CURRENTONLY);
//           world.SetupTaskProfileFun();

//           WHEN("Same or better task match is required"){
//             config.PREFERENTIAL_OUSTING(EQUALMATCH);
//             THEN("Ousting succeeds"){
//               REQUIRE(world.PreferentialOustingAllowed(world.fun_get_task_profile(incoming_symbiont_parent), host) == true);
//             }
//           }

//           WHEN("Strictly better task match is required"){
//             config.PREFERENTIAL_OUSTING(BETTERMATCH);
//             THEN("Ousting fails"){
//               REQUIRE(world.PreferentialOustingAllowed(world.fun_get_task_profile(incoming_symbiont_parent), host) == false);
//             }
//           }
//         }
//       }
//     }

//     WHEN("An ousting symbiont has better task match with a host than the existing symbiont does"){
//       // host self:   000001101
//       // host parent: 101000100

//       // hosted sym self:   000001001 // match 2/3
//       // hosted sym parent: 100000100 // match 2/3

//       // incoming sym self:   000001101 // match 3/3
//       // incoming sym parent: 101000100 // match 3/3
//       host->GetCPU().state.parent_tasks_performed->Set(2);
//       incoming_symbiont_parent->GetCPU().state.tasks_performed->Set(8);
//       incoming_symbiont_parent->GetCPU().state.tasks_performed->Set(6);
//       incoming_symbiont_parent->GetCPU().state.tasks_performed->Set(5);
//       incoming_symbiont_parent->GetCPU().state.parent_tasks_performed->Set(0);
//       incoming_symbiont_parent->GetCPU().state.parent_tasks_performed->Set(2);
//       incoming_symbiont_parent->GetCPU().state.parent_tasks_performed->Set(6);
      
//       WHEN("Preferential ousting is on"){
//         WHEN("Parental tasks are used"){
//           config.TRACK_PARENT_TASKS(PARENTONLY);
//           world.SetupTaskProfileFun();

//           WHEN("Same or better task match is required"){
//             config.PREFERENTIAL_OUSTING(EQUALMATCH);
//             THEN("Ousting succeeds"){
//               REQUIRE(world.PreferentialOustingAllowed(world.fun_get_task_profile(incoming_symbiont_parent), host) == true);
//             }
//           }
//           WHEN("Strictly better task match is required"){
//             config.PREFERENTIAL_OUSTING(BETTERMATCH);
//             THEN("Ousting succeeds"){
//               REQUIRE(world.PreferentialOustingAllowed(world.fun_get_task_profile(incoming_symbiont_parent), host) == true);
//             }
//           }
//         }

//         WHEN("Parental tasks are not used"){
//           config.TRACK_PARENT_TASKS(CURRENTONLY);
//           world.SetupTaskProfileFun();

//           WHEN("Same or better task match is required"){
//             config.PREFERENTIAL_OUSTING(EQUALMATCH);
//             THEN("Ousting succeeds"){
//               REQUIRE(world.PreferentialOustingAllowed(world.fun_get_task_profile(incoming_symbiont_parent), host) == true);
//             }
//           }

//           WHEN("Strictly better task match is required"){
//             config.PREFERENTIAL_OUSTING(BETTERMATCH);
//             THEN("Ousting succeeds"){
//               REQUIRE(world.PreferentialOustingAllowed(world.fun_get_task_profile(incoming_symbiont_parent), host) == true);
//             }
//           }
//         }
//       }
//     }
//     incoming_symbiont_parent.Delete();
//     host.Delete();
//   }
// }

// TEST_CASE("GetNeighborHost", "[sgp][sgp-unit]") {
//   GIVEN("A host infected with a symbiont"){
//     emp::Random random(13);
//     SymConfigSGP config;
//     config.SYM_LIMIT(1);
//     config.TRACK_PARENT_TASKS(CURRENTONLY);

//     SGPWorld world(random, &config, LogicTasks);
//     world.Resize(2);

//     emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config);
//     emp::Ptr<SGPSymbiont> symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config);
//     host->AddSymbiont(symbiont);

//     size_t source_index = 0;
//     world.AddOrgAt(host, source_index);

//     WHEN("There exists a nearby host") {
//       int neighbor_index = 1;
//       emp::Ptr<SGPHost> neighbor_host = emp::NewPtr<SGPHost>(&random, &world, &config);
//       world.AddOrgAt(neighbor_host, neighbor_index);

//       WHEN("The nearby host has matching tasks with the incoming symbiont") {
//         neighbor_host->GetCPU().state.tasks_performed->Set(8);
//         symbiont->GetCPU().state.tasks_performed->Set(8);

//         WHEN("Task matching is not required for horizontal transmission") {
//           config.HT_TASK_MATCH(0);
//           THEN("The position of the nearby, matching host is returned"){
//             REQUIRE(world.GetNeighborHost(source_index, *symbiont->GetCPU().state.tasks_performed) == neighbor_index);
//           }
//         }

//         WHEN("Task matching is required for horizontal transmission") {
//           config.HT_TASK_MATCH(1);
//           THEN("The position of the nearby, matching host is returned") {
//             REQUIRE(world.GetNeighborHost(source_index, *symbiont->GetCPU().state.tasks_performed) == neighbor_index);
//           }
//         }
//       }

//       WHEN("The nearby host does not have matching tasks with the incoming symbiont") {
//         neighbor_host->GetCPU().state.tasks_performed->Set(6);
//         symbiont->GetCPU().state.tasks_performed->Set(8);

//         WHEN("Task matching is not required for horizontal transmission") {
//           config.HT_TASK_MATCH(0);
//           THEN("The position of the nearby, non-matching host is returned") {
//             REQUIRE(world.GetNeighborHost(source_index, *symbiont->GetCPU().state.tasks_performed) == neighbor_index);
//           }
//         }

//         WHEN("Task matching is required for horizontal transmission") {
//           config.HT_TASK_MATCH(1);
//           THEN("-1 (no neighbor) is returned") {
//             REQUIRE(world.GetNeighborHost(source_index, *symbiont->GetCPU().state.tasks_performed) == -1);
//           }
//         }
//       }
//     }

//     WHEN("There does not exist a nearby host") {
//       THEN("-1 (no neighbor) is returned") {
//         REQUIRE(world.GetNeighborHost(source_index, *symbiont->GetCPU().state.tasks_performed) == -1);
//       }
//     }
//   }
// }

// TEST_CASE("GetDominantInfo", "[sgp][sgp-unit]") {
//   GIVEN("An SGPWorld"){
//     emp::Random random(61);
//     SymConfigSGP config;
//     config.DOMINANT_COUNT(10);

//     SGPWorld world(random, &config, TaskSet{});

//     SGPHost host1(&random, &world, &config);
//     SGPHost host2(&random, &world, &config);
//     // Make sure they have different genomes
//     host1.Mutate();
//     host2.Mutate();

//     WHEN("The world contains 2 of one org and 1 of another") {
//       // One copy of host1 and two of host2
//       world.AddOrgAt(emp::NewPtr<SGPHost>(host1), 0);
//       world.AddOrgAt(emp::NewPtr<SGPHost>(host2), 1);
//       world.AddOrgAt(emp::NewPtr<SGPHost>(host2), 2);

//       CHECK(world.GetNumOrgs() == 3);

//       THEN("The first org is dominant") {
//         auto dominant = world.GetDominantInfo();
//         CHECK(dominant.size() == 2);

//         CHECK(*dominant[0].first == host2);
//         CHECK(*dominant[0].first != host1);
//         CHECK(dominant[0].second == 2);

//         CHECK(*dominant[1].first == host1);
//         CHECK(*dominant[1].first != host2);
//         CHECK(dominant[1].second == 1);
//       }
//     }
//   }
// }
