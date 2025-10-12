#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPWorldSetup.cc"

#include "../../catch/catch.hpp"

TEST_CASE("GetDominantInfo", "[sgp]") {
  emp::Random random(61);
  SymConfigSGP config;
  config.DOMINANT_COUNT(10);

  SGPWorld world(random, &config, TaskSet{});

  SGPHost host1(&random, &world, &config);
  SGPHost host2(&random, &world, &config);
  // Make sure they have different genomes
  host1.Mutate();
  host2.Mutate();

  WHEN("The world contains 2 of one org and 1 of another") {
    // One copy of host1 and two of host2
    world.AddOrgAt(emp::NewPtr<SGPHost>(host1), 0);
    world.AddOrgAt(emp::NewPtr<SGPHost>(host2), 1);
    world.AddOrgAt(emp::NewPtr<SGPHost>(host2), 2);

    CHECK(world.GetNumOrgs() == 3);

    THEN("The first org is dominant") {
      auto dominant = world.GetDominantInfo();
      CHECK(dominant.size() == 2);

      CHECK(*dominant[0].first == host2);
      CHECK(*dominant[0].first != host1);
      CHECK(dominant[0].second == 2);

      CHECK(*dominant[1].first == host1);
      CHECK(*dominant[1].first != host2);
      CHECK(dominant[1].second == 1);
    }
  }
}

TEST_CASE("Baseline function", "[sgp]") {
  emp::Random random(61);
  SymConfigSGP config;
  config.GRID_X(2);
  config.GRID_Y(2);


  SGPWorld world(random, &config, LogicTasks);
  world.Resize(2,2);

  emp::Ptr<SGPHost> infected_host = emp::NewPtr<SGPHost>(&random, &world, &config);
  emp::Ptr<SGPHost> uninfected_host = emp::NewPtr<SGPHost>(&random, &world, &config);
  emp::Ptr<SGPSymbiont> hosted_symbiont = emp::NewPtr<SGPSymbiont> (&random, &world, &config);

  infected_host->AddSymbiont(hosted_symbiont);
  world.AddOrgAt(infected_host, 0);
  world.AddOrgAt(uninfected_host, 1);
  
  THEN("Organisms can be added to the world") {
    REQUIRE(world.GetNumOrgs() == 2);
  }

  for (int i = 0; i < 10; i++) {
    world.Update();
  }

  THEN("Organisms persist and are managed by the world") {
    REQUIRE(world.GetNumOrgs() == 2);
  }
}

TEST_CASE("Host Setup", "[sgp]") {
   emp::Random random(1);
  SymConfigSGP config;
  config.SEED(2);
  config.MUTATION_RATE(0.0);
  config.MUTATION_SIZE(0.000);
  config.TRACK_PARENT_TASKS(1);
  config.VT_TASK_MATCH(1);
  config.ONLY_FIRST_TASK_CREDIT(1);
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

      THEN("TaskMatchCheck returns true") {
        REQUIRE(world.TaskMatchCheck(sym, host));
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
        REQUIRE(world.TaskMatchCheck(sym, host));
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
        REQUIRE(!world.TaskMatchCheck(sym, host));
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
        REQUIRE(world.TaskMatchCheck(sym, host));
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
        REQUIRE(!world.TaskMatchCheck(sym, host));
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
        REQUIRE(world.TaskMatchCheck(sym, host));
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
    config.ONLY_FIRST_TASK_CREDIT(0);
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

    WHEN("Host and Symbiont have both performed NOT"){
      host->GetCPU().state.tasks_performed->Set(0);
      sym->GetCPU().state.tasks_performed->Set(0);

      THEN("TaskMatchCheck returns true when Host and Symbiont are the arguments"){
        REQUIRE(world.TaskMatchCheck(sym, host));
      }

    }
    WHEN("Host has performed NOT and Symbiont has performed EQU"){
      host->GetCPU().state.tasks_performed->Set(1);
      sym->GetCPU().state.tasks_performed->Set(8);

      THEN("TaskMatchCheck returns false when Host and Symbiont are the arguments"){
        REQUIRE(!world.TaskMatchCheck(sym, host));
      }
    
    }

  }
}

TEST_CASE("TaskMatchCheck when ONLY_FIRST_TASK_CREDIT is 1", "[sgp]") {

  GIVEN("An SGPWorld where ONLY_FIRST_TASK_CREDIT is on and there is no mutation"){
    emp::Random random(1);
    SymConfigSGP config;
    config.SEED(2);
    config.MUTATION_RATE(0.0);
    config.MUTATION_SIZE(0.000);
    config.TRACK_PARENT_TASKS(1);
    config.VT_TASK_MATCH(1);
    config.ONLY_FIRST_TASK_CREDIT(1);
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

      THEN("TaskMatchCheck returns true when the child of Host and the child of Symbiont are the arguments"){
        REQUIRE(world.TaskMatchCheck(sym_baby, host_baby));
      }

    }
    WHEN("Parent Host has performed NOT and Parent Symbiont has performed EQU"){
      host->GetCPU().state.tasks_performed->Set(1);
      sym->GetCPU().state.tasks_performed->Set(8);

      host_baby->GetCPU().state.parent_tasks_performed->Import(*(host->GetCPU().state.tasks_performed));
      sym_baby->GetCPU().state.parent_tasks_performed->Import(*(sym->GetCPU().state.tasks_performed));

      THEN("TaskMatchCheck returns false when the child of Host and the child of Symbiont are the arguments"){
        REQUIRE(!world.TaskMatchCheck(sym_baby, host_baby));
      }
    }

  }
}

TEST_CASE("Organisms, without mutation will only receive credit for NOT operations", "[sgp]") {
     
  GIVEN("An SGPWorld with no mutation"){
    emp::Random random(1);
    SymConfigSGP config;
    config.SEED(2);
    config.INTERACTION_MECHANISM(HEALTH);
    config.SYMBIONT_TYPE(PARASITE);
    config.MUTATION_RATE(0.0);
    config.MUTATION_SIZE(0.000);
    SGPWorld world(random, &config, LogicTasks);

    // Mock Organism to check reproduction
    class TestOrg : public Organism {
    public:
      bool IsHost() override { return true; }
      void AddPoints(double p) override {}
      double GetPoints() override { return 0; }
    };

    TestOrg organism;

    // NOT builder
    ProgramBuilder builder;
    (builder.AddNot)();
    CPU cpu(&organism, &world, builder.Build(100));
    
    
    
    cpu.RunCPUStep(0, 100);
    
    //The result of a AND bitwise operations when one of the inputs, in binary, is all ones will be the other input
    long all_ones_binary = 4294967295;
    cpu.state.input_buf.push(all_ones_binary);
    cpu.RunCPUStep(0, 100);
    world.Update();

    //Checks both that NOT is being done and no other operations are being done
    for (auto data : world.GetTaskSet()) {
      
        if(data.task.name != "NOT"){
      
        REQUIRE(data.n_succeeds_host == 0);
        }
        else{
          REQUIRE(data.n_succeeds_host > 0);
        }
      
    }


  }
}

TEST_CASE("Ousting is permitted", "[sgp]") {
  emp::Random random(61);
  SymConfigSGP config;
  config.GRID_X(2);
  config.GRID_Y(2);
  config.OUSTING(1);
  config.SYM_LIMIT(1);

  SGPWorld world(random, &config, LogicTasks);
  world.Resize(2, 2);

  emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config);
  emp::Ptr<SGPSymbiont> old_symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config);
  emp::Ptr<SGPSymbiont> new_symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config);

  host->AddSymbiont(old_symbiont);
  world.AddOrgAt(host, 0);

  REQUIRE(host->GetSymbionts().size() == 1);
  REQUIRE(world.GetGraveyard().size() == 0);

  host->AddSymbiont(new_symbiont);

  REQUIRE(host->GetSymbionts().size() == 1);
  REQUIRE(world.GetGraveyard().size() == 1);

  world.Update(); // clean up the graveyard

  REQUIRE(world.GetGraveyard().size() == 0);
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
  
  target_host->GetCPU().state.tasks_performed->Set(0);
  target_host->GetCPU().state.tasks_performed->Set(1);
  symbiont_parent->GetCPU().state.tasks_performed->Set(0);
  target_symbiont->GetCPU().state.tasks_performed->Set(0);

  world.AddOrgAt(source_host, 0);
  world.AddOrgAt(target_host, 1);

  WHEN("Preferential ousting is on and the target host has a symbiont") {
    config.OUSTING(1);
    config.PREFERENTIAL_OUSTING(2);
    WHEN("The incoming symbiont has a better match"){
      symbiont_parent->GetCPU().state.tasks_performed->Set(1);
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
      target_symbiont->GetCPU().state.tasks_performed->Set(1);
      world.SymDoBirth(symbiont_offspring, parent_pos);
      THEN("The incoming symbiont does not oust") {
        REQUIRE(target_host->GetSymbionts().at(0).DynamicCast<SGPSymbiont>() == target_symbiont);
      }
    }
  }
}

TEST_CASE("SymFindHost can handle transferring Stress Symbiont during stress event", "[sgp]") {
  GIVEN("An SGPWorld with no mutation"){
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

    emp::Ptr<StressHost> old_host = emp::NewPtr<StressHost>(&random, &world, &config);
    emp::Ptr<StressHost> new_host = emp::NewPtr<StressHost>(&random, &world, &config);
    emp::Ptr<SGPSymbiont> symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config);

    //New host and symbiont have performed NOT
    //(not old host to prevent symbiont from trying to reinfect it)
    new_host->GetCPU().state.tasks_performed->Set(0);
    symbiont->GetCPU().state.tasks_performed->Set(0);

    old_host->AddSymbiont(symbiont);
    world.AddOrgAt(old_host, emp::WorldPosition(0,0));
    world.AddOrgAt(new_host, emp::WorldPosition(1,0));

    REQUIRE(old_host->GetSymbionts().size() == 1);
    REQUIRE(new_host->GetSymbionts().size() == 0);
    REQUIRE(symbiont->GetHost().DynamicCast<StressHost>() == old_host);

    WHEN("SymFindHost is called with an existing symbiont") {
      old_host->RemoveSymbiont(1); //Need to remove manually
      //WorldPosition index is 1-indexed location in host and pop_id is host's location in the world
      emp::WorldPosition location = world.SymFindHost(symbiont, emp::WorldPosition(1,0));
      THEN("That symbiont transfers to a new host successfully") {
        REQUIRE(location.GetIndex() == 1);
        REQUIRE(location.GetPopID() ==1);
        REQUIRE(old_host->GetSymbionts().size() == 0);
        REQUIRE(new_host->GetSymbionts().size() == 1);
        REQUIRE(symbiont->GetHost().DynamicCast<StressHost>() == new_host);
      }
    }
    WHEN("Preferential ousting is on and the target host has a symbiont") {
      config.OUSTING(1);
      config.PREFERENTIAL_OUSTING(2);

      new_host->GetCPU().state.tasks_performed->Set(1);
      emp::Ptr<SGPSymbiont> target_symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config);
      new_host->AddSymbiont(target_symbiont);
      WHEN("The incoming symbiont has a better match"){
        symbiont->GetCPU().state.tasks_performed->Set(1);
        target_symbiont->GetCPU().state.tasks_performed->Set(0);

        old_host->RemoveSymbiont(1);
        emp::WorldPosition location = world.SymFindHost(symbiont, emp::WorldPosition(1, 0));
        THEN("That symbiont transfers to a new host successfully") {
          REQUIRE(location.GetIndex() == 1);
          REQUIRE(location.GetPopID() == 1);
          REQUIRE(old_host->GetSymbionts().size() == 0);
          REQUIRE(new_host->GetSymbionts().size() == 1);
          REQUIRE(symbiont->GetHost().DynamicCast<StressHost>() == new_host);
          REQUIRE(world.GetGraveyard().size() == 1);
          REQUIRE(world.GetGraveyard().at(0).DynamicCast<SGPSymbiont>() == target_symbiont);
        }
        world.GetGraveyard().at(0).Delete();
      }
      WHEN("The incoming symbiont has a worse match") {
        target_symbiont->GetCPU().state.tasks_performed->Set(0);
        target_symbiont->GetCPU().state.tasks_performed->Set(1);

        old_host->RemoveSymbiont(1);
        emp::WorldPosition location = world.SymFindHost(symbiont, emp::WorldPosition(1, 0));
        THEN("That symbiont fails to transfer to a new host") {
          REQUIRE(location.IsValid() == false);
          REQUIRE(old_host->GetSymbionts().size() == 0);
          REQUIRE(new_host->GetSymbionts().size() == 1);
          REQUIRE(target_symbiont->GetHost().DynamicCast<StressHost>() == new_host);
        }
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

  // host w/out parent: 000001101
  // host w/ parent:    100001101
  host->GetCPU().state.tasks_performed->Set(8);
  host->GetCPU().state.tasks_performed->Set(6);
  host->GetCPU().state.tasks_performed->Set(5);
  host->GetCPU().state.parent_tasks_performed->Set(0);
  host->GetCPU().state.parent_tasks_performed->Set(6);

  // hosted sym w/out parent: 000001001
  // hosted sym w/ parent:    100001001
  target_symbiont->GetCPU().state.tasks_performed->Set(8);
  target_symbiont->GetCPU().state.tasks_performed->Set(5);
  target_symbiont->GetCPU().state.parent_tasks_performed->Set(5);
  target_symbiont->GetCPU().state.parent_tasks_performed->Set(0);

  WHEN("An ousting symbiont has worse task match with a host than the existing symbiont does"){
    // incoming sym w/out parent: 000000001
    // hosted sym w/ parent:      000001001
    incoming_symbiont_parent->GetCPU().state.tasks_performed->Set(8);
    incoming_symbiont_parent->GetCPU().state.parent_tasks_performed->Set(5);
    
    WHEN("Preferential ousting is off"){ // sanity check that setting is toggleable
      config.PREFERENTIAL_OUSTING(0);
      THEN("Ousting succeeds"){
        REQUIRE(world.PreferentialOustingAllowed(incoming_symbiont_parent, host) == true);
      }
    }
    WHEN("Preferential ousting is on"){
      WHEN("Parental tasks are used"){
        config.TRACK_PARENT_TASKS(1);
        WHEN("Same or better task match is required"){
          config.PREFERENTIAL_OUSTING(1);
          THEN("Ousting fails"){
            REQUIRE(world.PreferentialOustingAllowed(incoming_symbiont_parent, host) == false);
          }
        }
        WHEN("Strictly better task match is required"){
          config.PREFERENTIAL_OUSTING(2);
          THEN("Ousting fails"){
            REQUIRE(world.PreferentialOustingAllowed(incoming_symbiont_parent, host) == false);
          }
        }
      }
      WHEN("Parental tasks are not used"){
        config.TRACK_PARENT_TASKS(0);
        WHEN("Same or better task match is required"){
          config.PREFERENTIAL_OUSTING(1);
          THEN("Ousting fails"){
            REQUIRE(world.PreferentialOustingAllowed(incoming_symbiont_parent, host) == false);
          }
        }
        WHEN("Strictly better task match is required"){
          config.PREFERENTIAL_OUSTING(2);
          THEN("Ousting fails"){
            REQUIRE(world.PreferentialOustingAllowed(incoming_symbiont_parent, host) == false);
          }
        }
      }
    }
  }

  
  WHEN("An ousting symbiont has equal task match with a host than the existing symbiont does"){
    // host w/out parent: 000001101
    // host w/ parent:    100001101

    // hosted sym w/out parent: 000001001 // match 2/3
    // hosted sym w/ parent:    100001001 // match 3/4

    // incoming sym w/out parent: 000000101 // match 2/3
    // hosted sym w/ parent:      000001101 // match 3/4
    incoming_symbiont_parent->GetCPU().state.tasks_performed->Set(8);
    incoming_symbiont_parent->GetCPU().state.tasks_performed->Set(6);
    incoming_symbiont_parent->GetCPU().state.parent_tasks_performed->Set(8);
    incoming_symbiont_parent->GetCPU().state.parent_tasks_performed->Set(5);
    
    WHEN("Preferential ousting is on"){
      WHEN("Parental tasks are used"){
        config.TRACK_PARENT_TASKS(1);
        WHEN("Same or better task match is required"){
          config.PREFERENTIAL_OUSTING(1);
          THEN("Ousting fails"){
            REQUIRE(world.PreferentialOustingAllowed(incoming_symbiont_parent, host) == true);
          }
        }
        WHEN("Strictly better task match is required"){
          config.PREFERENTIAL_OUSTING(2);
          THEN("Ousting fails"){
            REQUIRE(world.PreferentialOustingAllowed(incoming_symbiont_parent, host) == false);
          }
        }
      }
      WHEN("Parental tasks are not used"){
        config.TRACK_PARENT_TASKS(0);
        WHEN("Same or better task match is required"){
          config.PREFERENTIAL_OUSTING(1);
          THEN("Ousting fails"){
            REQUIRE(world.PreferentialOustingAllowed(incoming_symbiont_parent, host) == true);
          }
        }
        WHEN("Strictly better task match is required"){
          config.PREFERENTIAL_OUSTING(2);
          THEN("Ousting fails"){
            REQUIRE(world.PreferentialOustingAllowed(incoming_symbiont_parent, host) == false);
          }
        }
      }
    }
  }

  WHEN("An ousting symbiont has better task match with a host than the existing symbiont does"){
    // host w/out parent: 000001101
    // host w/ parent:    100001101

    // hosted sym w/out parent: 000001001 // match 2/3
    // hosted sym w/ parent:    100001001 // match 3/4

    // incoming sym w/out parent: 000001101 // match 3/3
    // hosted sym w/ parent:      100001101 // match 4/4
    incoming_symbiont_parent->GetCPU().state.tasks_performed->Set(8);
    incoming_symbiont_parent->GetCPU().state.tasks_performed->Set(6);
    incoming_symbiont_parent->GetCPU().state.tasks_performed->Set(5);
    incoming_symbiont_parent->GetCPU().state.parent_tasks_performed->Set(0);
    
    WHEN("Preferential ousting is on"){
      WHEN("Parental tasks are used"){
        config.TRACK_PARENT_TASKS(1);
        WHEN("Same or better task match is required"){
          config.PREFERENTIAL_OUSTING(1);
          THEN("Ousting fails"){
            REQUIRE(world.PreferentialOustingAllowed(incoming_symbiont_parent, host) == true);
          }
        }
        WHEN("Strictly better task match is required"){
          config.PREFERENTIAL_OUSTING(2);
          THEN("Ousting fails"){
            REQUIRE(world.PreferentialOustingAllowed(incoming_symbiont_parent, host) == true);
          }
        }
      }
      WHEN("Parental tasks are not used"){
        config.TRACK_PARENT_TASKS(0);
        WHEN("Same or better task match is required"){
          config.PREFERENTIAL_OUSTING(1);
          THEN("Ousting fails"){
            REQUIRE(world.PreferentialOustingAllowed(incoming_symbiont_parent, host) == true);
          }
        }
        WHEN("Strictly better task match is required"){
          config.PREFERENTIAL_OUSTING(2);
          THEN("Ousting fails"){
            REQUIRE(world.PreferentialOustingAllowed(incoming_symbiont_parent, host) == true);
          }
        }
      }
    }
  }
  incoming_symbiont_parent.Delete();
  host.Delete();
}

TEST_CASE("Stress parasites can reproduce for free when their host is killed in an extinction event", "[sgp]") {
  GIVEN("Stress is on, parasites are present, and an extinction event occurs") {
    emp::Random random(61);
    SymConfigSGP config;
    SGPWorld world(random, &config, LogicTasks);

    config.SYM_LIMIT(2);
    config.EXTINCTION_FREQUENCY(1);
    config.PARASITE_NUM_OFFSPRING_ON_STRESS_INTERACTION(3);
    config.TRACK_PARENT_TASKS(1);
    config.INTERACTION_MECHANISM(STRESS);
    config.SYMBIONT_TYPE(1);

    emp::Ptr<StressHost> host = emp::NewPtr<StressHost>(&random, &world, &config);
    emp::Ptr<SGPSymbiont> matching_symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config);
    emp::Ptr<SGPSymbiont> non_matching_symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config);

    host->AddSymbiont(matching_symbiont);
    host->AddSymbiont(non_matching_symbiont);
    
    REQUIRE(host->GetSymbionts().size() == 2);

    host->GetCPU().state.tasks_performed->Set(0);
    host->GetCPU().state.tasks_performed->Set(1);
    matching_symbiont->GetCPU().state.tasks_performed->Set(0);
    
    world.AddOrgAt(host, 0);

    WHEN("A host dies"){
      config.BASE_DEATH_CHANCE(1);
      config.PARASITE_DEATH_CHANCE(1);

      host->Process(0);
      REQUIRE(host->GetDead() == true);
      THEN("Matching parasites place offspring in the free reproduction vector") {
        REQUIRE(world.symbiont_stress_escapee_offspring.size() == 3);
        emp::Ptr<SGPSymbiont> symbiont_1 = world.symbiont_stress_escapee_offspring.at(0).escapee_offspring.DynamicCast<SGPSymbiont>();
        emp::Ptr<SGPSymbiont> symbiont_2 = world.symbiont_stress_escapee_offspring.at(1).escapee_offspring.DynamicCast<SGPSymbiont>();
        emp::Ptr<SGPSymbiont> symbiont_3 = world.symbiont_stress_escapee_offspring.at(2).escapee_offspring.DynamicCast<SGPSymbiont>();

        REQUIRE(symbiont_1 != symbiont_2);
        REQUIRE(symbiont_1->GetCPU().state.parent_tasks_performed->CountOnes() == 1);
        REQUIRE(symbiont_1->GetCPU().state.parent_tasks_performed->Get(0) == 1);
        REQUIRE(symbiont_2->GetCPU().state.parent_tasks_performed->CountOnes() == 1);
        REQUIRE(symbiont_2->GetCPU().state.parent_tasks_performed->Get(0) == 1);
        REQUIRE(symbiont_3->GetCPU().state.parent_tasks_performed->CountOnes() == 1);
        REQUIRE(symbiont_3->GetCPU().state.parent_tasks_performed->Get(0) == 1);
      }

      THEN("Non-matching parasites do not place offspring in the free reproduction vector") {
        REQUIRE(world.symbiont_stress_escapee_offspring.size() != 6);
      }
    }
    WHEN("A host does not die") {
      config.BASE_DEATH_CHANCE(0);
      config.PARASITE_DEATH_CHANCE(0);

      REQUIRE(host->GetDead() == false);
      THEN("Its parasites do not place offspring in the free reproduction vector") {
        // it seems that in Alex's version, symbionts can place in the vector even if symbionts don't die?
        REQUIRE(world.symbiont_stress_escapee_offspring.size() == 0);
      }
    }
  }
}

TEST_CASE("ProcessStressEscapeeOffspring", "[sgp]") {
  WHEN("There are symbionts in the queue") {
    emp::Random random(69);
    SymConfigSGP config;
    SGPWorld world(random, &config, LogicTasks);
    world.Resize(10);

    config.SYM_LIMIT(2);
    config.EXTINCTION_FREQUENCY(1);
    config.PARASITE_NUM_OFFSPRING_ON_STRESS_INTERACTION(6);
    config.TRACK_PARENT_TASKS(1);
    config.INTERACTION_MECHANISM(STRESS);
    config.SYMBIONT_TYPE(1);
    config.BASE_DEATH_CHANCE(0);
    config.PARASITE_DEATH_CHANCE(1);
    config.SYM_HORIZ_TRANS_RES(100);
    config.SYM_MIN_CYCLES_BEFORE_REPRO(50);
    
    emp::Ptr<StressHost> host = emp::NewPtr<StressHost>(&random, &world, &config);
    emp::Ptr<StressHost> host_2 = emp::NewPtr<StressHost>(&random, &world, &config);
    emp::Ptr<StressHost> host_3 = emp::NewPtr<StressHost>(&random, &world, &config);
    emp::Ptr<SGPSymbiont> matching_symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config);

    host->AddSymbiont(matching_symbiont);

    host->GetCPU().state.tasks_performed->Set(0);
    host_2->GetCPU().state.tasks_performed->Set(0); // vulnerable (to infection) surviving host
    host_3->GetCPU().state.tasks_performed->Set(1); // non-vulnerable surviving host
    matching_symbiont->GetCPU().state.tasks_performed->Set(0);

    world.AddOrgAt(host, 0);
    world.AddOrgAt(host_2, 1);
    world.AddOrgAt(host_3, 2);

    REQUIRE(world.GetNumOrgs() == 3);

    world.Update();
    REQUIRE(world.GetNumOrgs() == 2);
    THEN("They are placed or deleted") {
      REQUIRE(world.IsOccupied(0) == false);
      REQUIRE(world.IsOccupied(1) == true); // host_2 should have survived
      REQUIRE(world.IsOccupied(2) == true); // host_3 should have survived

      REQUIRE(host_2->GetSymbionts().size() > 0); // with six offspring, let's expect the vulnerable host to be infected 
      REQUIRE(host_2->GetSymbionts().at(0).DynamicCast<SGPSymbiont>()->GetCPU().state.parent_tasks_performed->Get(0) == true);
      REQUIRE(host_3->GetSymbionts().size() == 0); // and the non-matching host to be spared  
    }
    THEN("The queue is left empty") {
      REQUIRE(world.symbiont_stress_escapee_offspring.size() == 0);
    }
  }
}

TEST_CASE("Task matching required for (stress) symbiotic behavior", "[sgp]") {
  
  GIVEN("Stress is the interaction mechanism") {
    emp::Random random(69);
    SymConfigSGP config;
    SGPWorld world(random, &config, LogicTasks);
    world.Resize(10);
    config.EXTINCTION_FREQUENCY(1);
    config.INTERACTION_MECHANISM(STRESS);
    config.SYM_LIMIT(2);

    // create organisms
    emp::Ptr<StressHost> matching_host = emp::NewPtr<StressHost>(&random, &world, &config, CreateNotProgram(PROGRAM_LENGTH));
    emp::Ptr<SGPSymbiont> matching_symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(PROGRAM_LENGTH));
    emp::Ptr<SGPSymbiont> other_symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(PROGRAM_LENGTH));
    
    emp::Ptr<StressHost> non_matching_host = emp::NewPtr<StressHost>(&random, &world, &config, CreateNotProgram(PROGRAM_LENGTH));
    emp::Ptr<SGPSymbiont> non_matching_symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(PROGRAM_LENGTH));

    // set task profiles
    matching_host->GetCPU().state.tasks_performed->Set(8);
    matching_host->GetCPU().state.tasks_performed->Set(0);
    matching_symbiont->GetCPU().state.tasks_performed->Set(0);

    non_matching_host->GetCPU().state.tasks_performed->Set(8);
    non_matching_symbiont->GetCPU().state.tasks_performed->Set(0);

    // place organisms
    size_t matching_pos = 0;
    size_t non_matching_pos = 1;
    matching_host->AddSymbiont(matching_symbiont);
    matching_host->AddSymbiont(other_symbiont);
    non_matching_host->AddSymbiont(non_matching_symbiont);

    world.AddOrgAt(matching_host, matching_pos);
    world.AddOrgAt(non_matching_host, non_matching_pos);

    // check placement
    REQUIRE(world.GetNumOrgs() == 2);
    REQUIRE(matching_host->GetSymbionts().size() == 2);

    WHEN("Task matching is required for symbiotic behavior") {
      config.TASK_MATCH_FOR_SYMBIOTIC_BEHAVIOR(1);
      WHEN("Symbionts are parasitic") {
        config.BASE_DEATH_CHANCE(0);
        config.PARASITE_DEATH_CHANCE(1);
        config.SYMBIONT_TYPE(PARASITE);

        world.Update();
        REQUIRE(world.GetNumOrgs() == 1);

        THEN("Hosts with matching parasites are assigned the parasite death chance") {
          REQUIRE(world.IsOccupied(matching_pos) == false);
        }
        THEN("Hosts with non-matching parasites are assigned the base death chance") {
          REQUIRE(world.IsOccupied(non_matching_pos) == true);
        }
      }

      WHEN("Symbionts are mutualistic") {
        config.BASE_DEATH_CHANCE(1);
        config.MUTUALIST_DEATH_CHANCE(0);
        config.SYMBIONT_TYPE(MUTUALIST);

        world.Update();
        REQUIRE(world.GetNumOrgs() == 1);

        THEN("Hosts with matching mutualists are assigned the mutualist death chance") {
          REQUIRE(world.IsOccupied(matching_pos) == true);
        }
        THEN("Hosts with non-matching mutualists are assigned the base death chance") {
          REQUIRE(world.IsOccupied(non_matching_pos) == false);
        }
      }
    }
  }
}
