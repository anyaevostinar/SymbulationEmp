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

TEST_CASE("SymFindHost can handle transfering Stress Symbiont during stress event", "[sgp]") {
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

  WHEN("An ousting symbiont has worse task match with a host than the existing symbiont does"){
    WHEN("Preferential ousting is off"){ // sanity check that setting is toggleable
      config.PREFERENTIAL_OUSTING(0);
      THEN("Ousting succeeds"){
        REQUIRE(world.PreferentialOustingAllowed(incoming_symbiont_parent, host) == true);
      }
    }
    WHEN("Preferential ousting is on & same or better task match is permitted"){
      config.PREFERENTIAL_OUSTING(1);
      THEN("Ousting fails"){
        REQUIRE(world.PreferentialOustingAllowed(incoming_symbiont_parent, host) == false);
      }
    }
  }
  WHEN("An ousting symbiont has equal task match with a host than the existing symbiont does"){
    WHEN("Strictly better task match is required"){
      config.PREFERENTIAL_OUSTING(1);
      THEN("Ousting fails"){
        REQUIRE(world.PreferentialOustingAllowed(incoming_symbiont_parent, host) == false);
      }
    }
    WHEN("Same or better task match is permitted"){
      config.PREFERENTIAL_OUSTING(2);
      THEN("Ousting succeeds"){
        REQUIRE(world.PreferentialOustingAllowed(incoming_symbiont_parent, host) == true);
      }
    }
  }
  WHEN("An ousting symbiont has better task match with a host than the existing symbiont does"){
    WHEN("Strictly better task match is required"){
      config.PREFERENTIAL_OUSTING(1);
      THEN("Ousting succeeds"){
        REQUIRE(world.PreferentialOustingAllowed(incoming_symbiont_parent, host) == true);
      }
    }
    WHEN("Same or better task match is permitted"){
      config.PREFERENTIAL_OUSTING(2);
      THEN("Ousting succeeds"){
        REQUIRE(world.PreferentialOustingAllowed(incoming_symbiont_parent, host) == true);
      }
    }
  }
  
  incoming_symbiont_parent.Delete();
  host.Delete();
}
