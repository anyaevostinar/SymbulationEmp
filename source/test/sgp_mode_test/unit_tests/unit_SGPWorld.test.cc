#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"

#include "../../../catch/catch.hpp"

TEST_CASE("Host Setup", "[sgp]") {
   emp::Random random(1);
  SymConfigSGP config;
  config.SEED(2);
  config.MUTATION_RATE(0.0);
  config.MUTATION_SIZE(0.000);
  config.TRACK_PARENT_TASKS(1);
  config.VT_TASK_MATCH(1);
  config.HOST_ONLY_FIRST_TASK_CREDIT(1);
  config.SYM_ONLY_FIRST_TASK_CREDIT(1);
  config.HOST_REPRO_RES(10000);



  //world.SetupHosts requires a pointer for the number of hosts in the world
  unsigned long setupCount = 1;
  WHEN("INTERACTION_MECHANISM Config is set to SGP hosts"){
    config.INTERACTION_MECHANISM(0);
    SGPWorld world(random, &config, LogicTasks);
    world.SetupHosts(&setupCount);
    THEN("The world contains a SGPHost"){
    emp::Ptr<Organism> host =  world.GetOrgPtr(0);

    REQUIRE(host->GetName() == "SGPHost");
    
    }
  }
  WHEN("INTERACTION_MECHANISM Config is set to Health hosts"){
    config.INTERACTION_MECHANISM(1);
    SGPWorld world(random, &config, LogicTasks);
    world.SetupHosts(&setupCount);
    THEN("The world contains a HealtHost"){
    emp::Ptr<Organism> host =  world.GetOrgPtr(0);
     REQUIRE(host->GetName() == "HealthHost");
    
    }
  }

  WHEN("INTERACTION_MECHANISM Config is set to Stress hosts"){
    config.INTERACTION_MECHANISM(2);
    SGPWorld world(random, &config, LogicTasks);
    world.SetupHosts(&setupCount);
    THEN("The world contains a StressHost"){
    emp::Ptr<Organism> host =  world.GetOrgPtr(0);
     REQUIRE(host->GetName() == "StressHost");
    
    }
  }

  
  WHEN("INTERACTION_MECHANISM Config is set to an option that does not exist"){
    config.INTERACTION_MECHANISM(4);
    SGPWorld world(random, &config, LogicTasks);
    THEN("An exception should be thrown"){
      REQUIRE_THROWS(world.SetupHosts(&setupCount));
    }
  }
  
}

TEST_CASE("TaskMaskCheck Unit Test", "[sgp]") {

  emp::Random random(1);
  SymConfigSGP config;
  config.SEED(2);


  SGPWorld world(random, &config, LogicTasks);
  ProgramBuilder program;

  GIVEN("A world in nutrient mode") {
    config.INTERACTION_MECHANISM(NUTRIENT);

    WHEN("Host and symbiont do different tasks") {
      //Creates a host that is marked as having done NOT
      emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, program.Build(100));
      host->GetCPU().state.tasks_performed->Set(1);

      //Creates a symbiont that is marked as having done NAND
      emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, program.Build(100));
      sym->GetCPU().state.tasks_performed->Set(2); 

      THEN("TaskMatchCheck returns false") {
        REQUIRE(!world.TaskMatchCheck(world.fun_get_task_profile(sym), world.fun_get_task_profile(host)));
      }

      host.Delete();
      sym.Delete();
    }

    WHEN("Host and symbiont do the same task") {
      //Creates a host that is marked as having done NOT
      emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, program.Build(100));
      host->GetCPU().state.tasks_performed->Set(1);

      //Creates a symbiont that is marked as having done NOT
      emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, program.Build(100));
      sym->GetCPU().state.tasks_performed->Set(1); 

      THEN("TaskMatchCheck returns true") {
        REQUIRE(world.TaskMatchCheck(world.fun_get_task_profile(sym), world.fun_get_task_profile(host)));
      }

      host.Delete();
      sym.Delete();
    }
  }

  GIVEN("A world in health mode") {
    config.INTERACTION_MECHANISM(HEALTH);

    WHEN("Host and symbiont do different tasks") {
      //Creates a host that is marked as having done NOT
      emp::Ptr<HealthHost> host = emp::NewPtr<HealthHost>(&random, &world, &config, program.Build(100));
      host->GetCPU().state.tasks_performed->Set(1);

      //Creates a symbiont that is marked as having done NAND
      emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, program.Build(100));
      sym->GetCPU().state.tasks_performed->Set(2); 

      THEN("TaskMatchCheck returns false") {
        REQUIRE(!world.TaskMatchCheck(world.fun_get_task_profile(sym), world.fun_get_task_profile(host)));
      }

      host.Delete();
      sym.Delete();
    }

    WHEN("Host and symbiont do the same task") {
      //Creates a host that is marked as having done NOT
      emp::Ptr<HealthHost> host = emp::NewPtr<HealthHost>(&random, &world, &config, program.Build(100));
      host->GetCPU().state.tasks_performed->Set(1);

      //Creates a symbiont that is marked as having done NOT
      emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, program.Build(100));
      sym->GetCPU().state.tasks_performed->Set(1); 

      THEN("TaskMatchCheck returns true") {
        REQUIRE(world.TaskMatchCheck(world.fun_get_task_profile(sym), world.fun_get_task_profile(host)));
      }

      host.Delete();
      sym.Delete();
    }
  }

  GIVEN("A world in stress mode") {
    config.INTERACTION_MECHANISM(STRESS);

    WHEN("Host and symbiont do different tasks") {
      //Creates a host that is marked as having done NOT
      ProgramBuilder program;
      emp::Ptr<StressHost> host = emp::NewPtr<StressHost>(&random, &world, &config, program.Build(100));
      host->GetCPU().state.tasks_performed->Set(1);

      //Creates a symbiont that is marked as having done NAND
      emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, program.Build(100));
      sym->GetCPU().state.tasks_performed->Set(2); 

      THEN("TaskMatchCheck returns false") {
        REQUIRE(!world.TaskMatchCheck(world.fun_get_task_profile(sym), world.fun_get_task_profile(host)));
      }

      host.Delete();
      sym.Delete();
    }

    WHEN("Host and symbiont do the same task") {
      //Creates a host that is marked as having done NOT
      ProgramBuilder program;
      emp::Ptr<StressHost> host = emp::NewPtr<StressHost>(&random, &world, &config, program.Build(100));
      host->GetCPU().state.tasks_performed->Set(1);

      //Creates a symbiont that is marked as having done NOT
      emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, program.Build(100));
      sym->GetCPU().state.tasks_performed->Set(1); 

      THEN("TaskMatchCheck returns true") {
        REQUIRE(world.TaskMatchCheck(world.fun_get_task_profile(sym), world.fun_get_task_profile(host)));
      }

      host.Delete();
      sym.Delete();
    }
  }
}

TEST_CASE("TaskMatchCheck for parents", "[sgp]") {
  
  GIVEN("An SGPWorld with no mutation"){
    emp::Random random(1);
    SymConfigSGP config;
    config.SEED(2);
    config.MUTATION_RATE(0.0);
    config.MUTATION_SIZE(0.000);
    config.TRACK_PARENT_TASKS(1);
    config.VT_TASK_MATCH(1);
    config.HOST_ONLY_FIRST_TASK_CREDIT(0);
    config.SYM_ONLY_FIRST_TASK_CREDIT(0);
    config.HOST_REPRO_RES(10000);

    SGPWorld world(random, &config, LogicTasks);


    //Creates a host that only does NOT operations
    emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));

    //Creates a symbiont that only does NOT operations
    emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(100));


    emp::Ptr<SGPSymbiont> sym_baby = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(100));
    emp::Ptr<SGPHost> host_baby = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));


    //Adds host to world and sym to host.
    world.AddOrgAt(host, 0);
    world.AddOrgAt(host_baby, 1);
    host->AddSymbiont(sym);
    host_baby->AddSymbiont(sym_baby);

    WHEN("Host and Symbiont parents have both performed NOT"){
      host->GetCPU().state.parent_tasks_performed->Set(0);
      sym->GetCPU().state.parent_tasks_performed->Set(0);

      THEN("TaskMatchCheck returns true when Host task set and Symbiont task set are the arguments"){
        REQUIRE(world.TaskMatchCheck(world.fun_get_task_profile(sym), world.fun_get_task_profile(host)));
      }

    }
    WHEN("Host parent has performed NOT and Symbiont parent has performed EQU"){
      host->GetCPU().state.parent_tasks_performed->Set(1);
      sym->GetCPU().state.parent_tasks_performed->Set(8);

      THEN("TaskMatchCheck returns false when Host task set and Symbiont task set are the arguments"){
        REQUIRE(!world.TaskMatchCheck(world.fun_get_task_profile(sym), world.fun_get_task_profile(host)));
      }
    
    }

  }
}

TEST_CASE("TaskMatchCheck when HOST_ONLY_FIRST_TASK_CREDIT and SYM_ONLY_FIRST_TASK_CREDIT is 1", "[sgp]") {
  GIVEN("An SGPWorld where HOST_ONLY_FIRST_TASK_CREDIT and SYM_ONLY_FIRST_TASK_CREDIT is on and there is no mutation"){
    emp::Random random(1);
    SymConfigSGP config;
    config.SEED(2);
    config.MUTATION_RATE(0.0);
    config.MUTATION_SIZE(0.000);
    config.TRACK_PARENT_TASKS(1);
    config.VT_TASK_MATCH(1);
    config.HOST_ONLY_FIRST_TASK_CREDIT(1);
    config.SYM_ONLY_FIRST_TASK_CREDIT(1);
    config.HOST_REPRO_RES(10000);

    SGPWorld world(random, &config, LogicTasks);


    //Creates a host that only does NOT operations
    emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));

    //Creates a symbiont that only does NOT operations
    emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(100));


    emp::Ptr<SGPSymbiont> sym_baby = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(100));
    emp::Ptr<SGPHost> host_baby = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));


    //Adds host to world and sym to host.
    world.AddOrgAt(host, 0);
    world.AddOrgAt(host_baby, 1);
    host->AddSymbiont(sym);
    host_baby->AddSymbiont(sym_baby);

    WHEN("Parent Host and Symbiont have both performed NOT"){
      host->GetCPU().state.tasks_performed->Set(0);
      sym->GetCPU().state.tasks_performed->Set(0);

      host_baby->GetCPU().state.parent_tasks_performed->Import(*(host->GetCPU().state.tasks_performed));
      sym_baby->GetCPU().state.parent_tasks_performed->Import(*(sym->GetCPU().state.tasks_performed));

      THEN("TaskMatchCheck returns true when the task set of the host child and the task set of the symbiont child are the arguments"){
        REQUIRE(world.TaskMatchCheck(world.fun_get_task_profile(sym_baby), world.fun_get_task_profile(host_baby)));
      }

    }
    WHEN("Parent Host has performed NOT and Parent Symbiont has performed EQU"){
      host->GetCPU().state.tasks_performed->Set(1);
      sym->GetCPU().state.tasks_performed->Set(8);

      host_baby->GetCPU().state.parent_tasks_performed->Import(*(host->GetCPU().state.tasks_performed));
      sym_baby->GetCPU().state.parent_tasks_performed->Import(*(sym->GetCPU().state.tasks_performed));

      THEN("TaskMatchCheck returns false when the task set of the host child and the task set of the symbiont child are the arguments"){
        REQUIRE(!world.TaskMatchCheck(world.fun_get_task_profile(sym_baby), world.fun_get_task_profile(host_baby)));
      }
    }

  }
}
TEST_CASE("SGP SymDoBirth", "[sgp]") {
  emp::Random random(1);
  SymConfigSGP config;
  config.SEED(2);
  config.INTERACTION_MECHANISM(STRESS);
  config.SYMBIONT_TYPE(PARASITE);
  config.MUTATION_RATE(0.0);
  config.MUTATION_SIZE(0.000);
  config.TRACK_PARENT_TASKS(1);
  SGPWorld world(random, &config, LogicTasks);
  world.Resize(2, 2);

  emp::Ptr<StressHost> source_host = emp::NewPtr<StressHost>(&random, &world, &config);
  emp::Ptr<StressHost> target_host = emp::NewPtr<StressHost>(&random, &world, &config);

  emp::Ptr<SGPSymbiont> symbiont_parent = emp::NewPtr<SGPSymbiont>(&random, &world, &config);
  emp::Ptr<SGPSymbiont> target_symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config);
  
  source_host->AddSymbiont(symbiont_parent);
  target_host->AddSymbiont(target_symbiont);

  emp::WorldPosition parent_pos = emp::WorldPosition(1,0);
  emp::Ptr<Organism> symbiont_offspring = symbiont_parent->Reproduce();
  
  // TRACK_PARENT_TASKS is on, so comparison for symbiont_offspring's injection is occurring between
  // symbiont_parent's parent tasks (symbiont_offspring's grandparent tasks), target_host's parent tasks, and target_symbiont's parent tasks
  target_host->GetCPU().state.parent_tasks_performed->Set(0);
  target_host->GetCPU().state.parent_tasks_performed->Set(1);
  symbiont_parent->GetCPU().state.parent_tasks_performed->Set(0);
  target_symbiont->GetCPU().state.parent_tasks_performed->Set(0);

  world.AddOrgAt(source_host, 0);
  world.AddOrgAt(target_host, 1);

  WHEN("Preferential ousting is on and the target host has a symbiont") {
    config.OUSTING(1);
    config.PREFERENTIAL_OUSTING(2);
    WHEN("The incoming symbiont has a better match"){
      symbiont_parent->GetCPU().state.parent_tasks_performed->Set(1);
      world.SymDoBirth(symbiont_offspring, parent_pos);
      THEN("The incoming symbiont successfully ousts"){
        REQUIRE(target_host->HasSym());
        REQUIRE(target_host->GetSymbionts().at(0) == symbiont_offspring); 
        REQUIRE(world.GetGraveyard().size() == 1);
        REQUIRE(world.GetGraveyard().at(0).DynamicCast<SGPSymbiont>() == target_symbiont);
      }
      world.GetGraveyard().at(0).Delete();
    }
    WHEN("The incoming symbiont has a worse match"){
      target_symbiont->GetCPU().state.parent_tasks_performed->Set(1);
      world.SymDoBirth(symbiont_offspring, parent_pos);
      THEN("The incoming symbiont does not oust") {
        REQUIRE(target_host->GetSymbionts().at(0).DynamicCast<SGPSymbiont>() == target_symbiont);
      }
    }
  }
}

TEST_CASE("Preferential ousting", "[sgp]"){
  // pref ousting settings
  // 0 = no preferential ousting, 
  // 1 = the incoming symbiont must have an equal or better match than the current symbiont in order to oust
  // 2 = the incoming symbiont must have a strictly better match than the current symbiont in order to oust

  emp::Random random(89);
  SymConfigSGP config;
  SGPWorld world(random, &config, LogicTasks);
  config.OUSTING(1);

  emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(PROGRAM_LENGTH));
  emp::Ptr<SGPSymbiont> target_symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(PROGRAM_LENGTH));
  emp::Ptr<SGPSymbiont> incoming_symbiont_parent = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(PROGRAM_LENGTH));
  host->AddSymbiont(target_symbiont);

  // host self:   000001101
  // host parent: 100000100
  host->GetCPU().state.tasks_performed->Set(8);
  host->GetCPU().state.tasks_performed->Set(6);
  host->GetCPU().state.tasks_performed->Set(5);
  host->GetCPU().state.parent_tasks_performed->Set(0);
  host->GetCPU().state.parent_tasks_performed->Set(6);

  // hosted sym self:   000001001
  // hosted sym parent: 100000100
  target_symbiont->GetCPU().state.tasks_performed->Set(8);
  target_symbiont->GetCPU().state.tasks_performed->Set(5);
  target_symbiont->GetCPU().state.parent_tasks_performed->Set(6);
  target_symbiont->GetCPU().state.parent_tasks_performed->Set(0);

  WHEN("An ousting symbiont has worse task match with a host than the existing symbiont does"){
    // host self:   000001101
    // host parent: 100000100

    // hosted sym self:   000001001 // match 2/3
    // hosted sym parent: 100000100 // match 2/2

    // incoming sym self:   000000001 // match 1/3
    // incoming sym parent: 000000100 // match 1/2
    incoming_symbiont_parent->GetCPU().state.tasks_performed->Set(8);
    incoming_symbiont_parent->GetCPU().state.parent_tasks_performed->Set(6);

    WHEN("Preferential ousting is off"){ // sanity check that setting is toggleable
      config.PREFERENTIAL_OUSTING(0);
      THEN("Ousting succeeds"){
        REQUIRE(world.PreferentialOustingAllowed(world.fun_get_task_profile(incoming_symbiont_parent), host) == true);
      }
    }
    WHEN("Preferential ousting is on"){
      WHEN("Parental tasks are used"){
        config.TRACK_PARENT_TASKS(1);
        world.SetupTaskProfileFun();

        WHEN("Same or better task match is required"){
          config.PREFERENTIAL_OUSTING(1);
          THEN("Ousting fails"){
            REQUIRE(world.PreferentialOustingAllowed(world.fun_get_task_profile(incoming_symbiont_parent), host) == false);
          }
        }
        WHEN("Strictly better task match is required"){
          config.PREFERENTIAL_OUSTING(2);
          THEN("Ousting fails"){
            REQUIRE(world.PreferentialOustingAllowed(world.fun_get_task_profile(incoming_symbiont_parent), host) == false);
          }
        }
      }
      WHEN("Parental tasks are not used"){
        config.TRACK_PARENT_TASKS(0);
        world.SetupTaskProfileFun();

        WHEN("Same or better task match is required"){
          config.PREFERENTIAL_OUSTING(1);
          THEN("Ousting fails"){
            REQUIRE(world.PreferentialOustingAllowed(world.fun_get_task_profile(incoming_symbiont_parent), host) == false);
          }
        }
        WHEN("Strictly better task match is required"){
          config.PREFERENTIAL_OUSTING(2);
          THEN("Ousting fails"){
            REQUIRE(world.PreferentialOustingAllowed(world.fun_get_task_profile(incoming_symbiont_parent), host) == false);
          }
        }
      }
    }
  }

  
  WHEN("An ousting symbiont has equal task match with a host than the existing symbiont does"){
    // host self:   000001101
    // host parent: 100000100

    // hosted sym self:   000001001 // match 2/3
    // hosted sym parent: 100000100 // match 2/2

    // incoming sym self:   000000101 // match 2/3
    // incoming sym parent: 100000100 // match 2/2
    incoming_symbiont_parent->GetCPU().state.tasks_performed->Set(8);
    incoming_symbiont_parent->GetCPU().state.tasks_performed->Set(6);
    incoming_symbiont_parent->GetCPU().state.parent_tasks_performed->Set(0);
    incoming_symbiont_parent->GetCPU().state.parent_tasks_performed->Set(6);

    WHEN("Preferential ousting is on"){
      WHEN("Parental tasks are used"){
        config.TRACK_PARENT_TASKS(1);
        world.SetupTaskProfileFun();

        WHEN("Same or better task match is required"){
          config.PREFERENTIAL_OUSTING(1);
          THEN("Ousting succeeds"){
            REQUIRE(world.PreferentialOustingAllowed(world.fun_get_task_profile(incoming_symbiont_parent), host) == true);
          }
        }
        WHEN("Strictly better task match is required"){
          config.PREFERENTIAL_OUSTING(2);
          THEN("Ousting fails"){
            REQUIRE(world.PreferentialOustingAllowed(world.fun_get_task_profile(incoming_symbiont_parent), host) == false);
          }
        }
      }
      WHEN("Parental tasks are not used"){
        config.TRACK_PARENT_TASKS(0);
        world.SetupTaskProfileFun();

        WHEN("Same or better task match is required"){
          config.PREFERENTIAL_OUSTING(1);
          THEN("Ousting succeeds"){
            REQUIRE(world.PreferentialOustingAllowed(world.fun_get_task_profile(incoming_symbiont_parent), host) == true);
          }
        }
        WHEN("Strictly better task match is required"){
          config.PREFERENTIAL_OUSTING(2);
          THEN("Ousting fails"){
            REQUIRE(world.PreferentialOustingAllowed(world.fun_get_task_profile(incoming_symbiont_parent), host) == false);
          }
        }
      }
    }
  }

  WHEN("An ousting symbiont has better task match with a host than the existing symbiont does"){
    // host self:   000001101
    // host parent: 101000100

    // hosted sym self:   000001001 // match 2/3
    // hosted sym parent: 100000100 // match 2/3

    // incoming sym self:   000001101 // match 3/3
    // incoming sym parent: 101000100 // match 3/3
    host->GetCPU().state.parent_tasks_performed->Set(2);
    incoming_symbiont_parent->GetCPU().state.tasks_performed->Set(8);
    incoming_symbiont_parent->GetCPU().state.tasks_performed->Set(6);
    incoming_symbiont_parent->GetCPU().state.tasks_performed->Set(5);
    incoming_symbiont_parent->GetCPU().state.parent_tasks_performed->Set(0);
    incoming_symbiont_parent->GetCPU().state.parent_tasks_performed->Set(2);
    incoming_symbiont_parent->GetCPU().state.parent_tasks_performed->Set(6);
    
    WHEN("Preferential ousting is on"){
      WHEN("Parental tasks are used"){
        config.TRACK_PARENT_TASKS(1);
        world.SetupTaskProfileFun();

        WHEN("Same or better task match is required"){
          config.PREFERENTIAL_OUSTING(1);
          THEN("Ousting succeeds"){
            REQUIRE(world.PreferentialOustingAllowed(world.fun_get_task_profile(incoming_symbiont_parent), host) == true);
          }
        }
        WHEN("Strictly better task match is required"){
          config.PREFERENTIAL_OUSTING(2);
          THEN("Ousting succeeds"){
            REQUIRE(world.PreferentialOustingAllowed(world.fun_get_task_profile(incoming_symbiont_parent), host) == true);
          }
        }
      }
      WHEN("Parental tasks are not used"){
        config.TRACK_PARENT_TASKS(0);
        world.SetupTaskProfileFun();

        WHEN("Same or better task match is required"){
          config.PREFERENTIAL_OUSTING(1);
          THEN("Ousting succeeds"){
            REQUIRE(world.PreferentialOustingAllowed(world.fun_get_task_profile(incoming_symbiont_parent), host) == true);
          }
        }
        WHEN("Strictly better task match is required"){
          config.PREFERENTIAL_OUSTING(2);
          THEN("Ousting succeeds"){
            REQUIRE(world.PreferentialOustingAllowed(world.fun_get_task_profile(incoming_symbiont_parent), host) == true);
          }
        }
      }
    }
  }
  incoming_symbiont_parent.Delete();
  host.Delete();
}

TEST_CASE("GetNeighborHost", "[sgp]") {
  emp::Random random(13);
  SymConfigSGP config;
  config.SYM_LIMIT(1);
  config.TRACK_PARENT_TASKS(0);

  SGPWorld world(random, &config, LogicTasks);
  world.Resize(2);

  emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config);
  emp::Ptr<SGPSymbiont> symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config);
  host->AddSymbiont(symbiont);

  size_t source_index = 0;
  world.AddOrgAt(host, source_index);

  WHEN("There exists a nearby host") {
    int neighbor_index = 1;
    emp::Ptr<SGPHost> neighbor_host = emp::NewPtr<SGPHost>(&random, &world, &config);
    world.AddOrgAt(neighbor_host, neighbor_index);

    WHEN("The nearby host has matching tasks with the incoming symbiont") {
      neighbor_host->GetCPU().state.tasks_performed->Set(8);
      symbiont->GetCPU().state.tasks_performed->Set(8);

      WHEN("Task matching is not required for horizontal transmission") {
        config.HT_TASK_MATCH(0);
        THEN("The position of the nearby, matching host is returned"){
          REQUIRE(world.GetNeighborHost(source_index, *symbiont->GetCPU().state.tasks_performed) == neighbor_index);
        }
      }
      WHEN("Task matching is required for horizontal transmission") {
        config.HT_TASK_MATCH(1);
        THEN("The position of the nearby, matching host is returned") {
          REQUIRE(world.GetNeighborHost(source_index, *symbiont->GetCPU().state.tasks_performed) == neighbor_index);
        }
      }
    }

    WHEN("The nearby host does not have matching tasks with the incoming symbiont") {
      neighbor_host->GetCPU().state.tasks_performed->Set(6);
      symbiont->GetCPU().state.tasks_performed->Set(8);

      WHEN("Task matching is not required for horizontal transmission") {
        config.HT_TASK_MATCH(0);
        THEN("The position of the nearby, non-matching host is returned") {
          REQUIRE(world.GetNeighborHost(source_index, *symbiont->GetCPU().state.tasks_performed) == neighbor_index);
        }
      }
      WHEN("Task matching is required for horizontal transmission") {
        config.HT_TASK_MATCH(1);
        THEN("-1 (no neighbor) is returned") {
          REQUIRE(world.GetNeighborHost(source_index, *symbiont->GetCPU().state.tasks_performed) == -1);
        }
      }
    }
  }
  WHEN("There does not exist a nearby host") {
    THEN("-1 (no neighbor) is returned") {
      REQUIRE(world.GetNeighborHost(source_index, *symbiont->GetCPU().state.tasks_performed) == -1);
    }
  }
}
