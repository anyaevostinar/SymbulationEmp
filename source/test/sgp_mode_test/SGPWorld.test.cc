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
  config.FREE_LIVING_SYMS(1);
  config.GRID_X(2);
  config.GRID_Y(2);


  SGPWorld world(random, &config, LogicTasks);
  world.Resize(2,2);

  emp::Ptr<SGPHost> infected_host = emp::NewPtr<SGPHost>(&random, &world, &config);
  emp::Ptr<SGPHost> uninfected_host = emp::NewPtr<SGPHost>(&random, &world, &config);
  emp::Ptr<SGPSymbiont> hosted_symbiont = emp::NewPtr<SGPSymbiont> (&random, &world, &config);
  emp::Ptr<SGPSymbiont> free_symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config);

  infected_host->AddSymbiont(hosted_symbiont);
  world.AddOrgAt(infected_host, 0);
  world.AddOrgAt(uninfected_host, 1);
  world.AddOrgAt(free_symbiont, emp::WorldPosition(0, 0));
  
  THEN("Organisms can be added to the world") {
    REQUIRE(world.GetNumOrgs() == 3);
  }

  for (int i = 0; i < 10; i++) {
    world.Update();
  }

  THEN("Organisms persist and are managed by the world") {
    REQUIRE(world.GetNumOrgs() == 3);
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
  WHEN("ORGANISM_TYPE Config is set to SGP hosts"){
    config.ORGANISM_TYPE(0);
    SGPWorld world(random, &config, LogicTasks);
    world.SetupHosts(&setupCount);
    THEN("The world contains a SGPHost"){
    emp::Ptr<SGPHost> host =  (world.GetOrgPtr(0)).Cast<SGPHost>();

    REQUIRE(host->GetName() == "SGPHost");
    
    }
  }
  WHEN("ORGANISM_TYPE Config is set to Health hosts"){
    config.ORGANISM_TYPE(1);
    SGPWorld world(random, &config, LogicTasks);
    world.SetupHosts(&setupCount);
    THEN("The world contains a HealtHost"){
    emp::Ptr<HealthHost> host = (world.GetOrgPtr(0)).Cast<HealthHost>();
     REQUIRE(host->GetName() == "HealthHost");
    
    }
  }

  WHEN("ORGANISM_TYPE Config is set to Stress hosts"){
    config.ORGANISM_TYPE(2);
    SGPWorld world(random, &config, LogicTasks);
    world.SetupHosts(&setupCount);
    THEN("The world contains a StressHost"){
    emp::Ptr<StressHost> host =  (world.GetOrgPtr(0)).Cast<StressHost>();
     REQUIRE(host->GetName() == "StressHost");
    
    }
  }

  
  WHEN("ORGANISM_TYPE Config is set to an option that does not exist"){
    config.ORGANISM_TYPE(3);
    SGPWorld world(random, &config, LogicTasks);
    world.SetupHosts(&setupCount);
    THEN("The world should have no organisms"){
      REQUIRE(world.GetNumOrgs() == 0);
    }
  }
  
}

TEST_CASE("TaskMatchCheck", "[sgp]") {
  
  emp::Random random(61);
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
  //Creates a symbiont that does both Not and Nand operations
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
    
    host_baby->GetCPU().state.parent_tasks_performed->Import(*(host->GetCPU().state.tasks_performed));
    sym_baby->GetCPU().state.parent_tasks_performed->Import(*(sym->GetCPU().state.tasks_performed));

    THEN("TaskMatchCheck returns true when the child of Host and the child of Symbiont are the arguments"){
      REQUIRE(world.TaskMatchCheck(sym_baby, host_baby));
    }

  }
  WHEN("Host has performed NOT and Symbiont has performed EQU"){
    host->GetCPU().state.tasks_performed->Set(1);
    sym->GetCPU().state.tasks_performed->Set(8);
    THEN("TaskMatchCheck returns false when Host and Symbiont are the arguments"){
      REQUIRE(!world.TaskMatchCheck(sym, host));
    }

    host_baby->GetCPU().state.parent_tasks_performed->Import(*(host->GetCPU().state.tasks_performed));
    sym_baby->GetCPU().state.parent_tasks_performed->Import(*(sym->GetCPU().state.tasks_performed));

    THEN("TaskMatchCheck returns false when the child of Host and the child of Symbiont are the arguments"){
      REQUIRE(!world.TaskMatchCheck(sym_baby, host_baby));
    }
  }

}

TEST_CASE("Organisms, without mutation will only recieve credit for NOT operations", "[sgp]") {
     
  emp::Random random(1);
  SymConfigSGP config;
  config.SEED(2);
  config.ORGANISM_TYPE(HEALTH);
  config.STRESS_TYPE(PARASITE);
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
  int all_ones_binary = 4294967295;
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
