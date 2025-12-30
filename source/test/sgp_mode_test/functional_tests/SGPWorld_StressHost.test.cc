#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"

#include "../../../catch/catch.hpp"

TEST_CASE("Stress parasites can reproduce for free when their host is killed in an extinction event", "[sgp][sgp-functional]") {
  GIVEN("Stress is on, parasites are present, and an extinction event occurs") {
    emp::Random random(61);
    SymConfigSGP config;

    config.SYM_LIMIT(2);
    config.EXTINCTION_FREQUENCY(1);
    config.PARASITE_NUM_OFFSPRING_ON_STRESS_INTERACTION(3);
    config.TRACK_PARENT_TASKS(1);
    config.INTERACTION_MECHANISM(STRESS);
    config.SYMBIONT_TYPE(1);


    SGPWorld world(random, &config, LogicTasks);

    emp::Ptr<StressHost> host = emp::NewPtr<StressHost>(&random, &world, &config);
    emp::Ptr<SGPSymbiont> matching_symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config);
    emp::Ptr<SGPSymbiont> non_matching_symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config);

    host->AddSymbiont(matching_symbiont);
    host->AddSymbiont(non_matching_symbiont);
    
    REQUIRE(host->GetSymbionts().size() == 2);

    host->GetCPU().state.parent_tasks_performed->Set(1);
    matching_symbiont->GetCPU().state.tasks_performed->Set(0); // this gets used to create the parent task set for the escapees
    matching_symbiont->GetCPU().state.parent_tasks_performed->Set(1); // this gets used to determine escapee infection ability
    
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
        REQUIRE(world.symbiont_stress_escapee_offspring.size() == 0);
      }
    }
  }
}

TEST_CASE("ProcessStressEscapeeOffspring", "[sgp][sgp-functional]") {
  WHEN("There are symbionts in the queue") {
    emp::Random random(19);
    SymConfigSGP config;
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
    config.HOST_MIN_CYCLES_BEFORE_REPRO(100);
    config.GRID_X(10);
    config.GRID_Y(10);

    SGPWorld world(random, &config, LogicTasks);
    world.Resize(10);

    emp::Ptr<StressHost> host = emp::NewPtr<StressHost>(&random, &world, &config);
    emp::Ptr<StressHost> host_2 = emp::NewPtr<StressHost>(&random, &world, &config);
    emp::Ptr<StressHost> host_3 = emp::NewPtr<StressHost>(&random, &world, &config);
    emp::Ptr<SGPSymbiont> symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config);

    host->AddSymbiont(symbiont);

    world.AddOrgAt(host, 0);
    world.AddOrgAt(host_2, 1);
    world.AddOrgAt(host_3, 2);

    REQUIRE(world.GetNumOrgs() == 3);
    
    WHEN("Preferential ousting is off") {
      host->GetCPU().state.parent_tasks_performed->Set(0); 
      host_2->GetCPU().state.parent_tasks_performed->Set(0); // vulnerable (to infection) surviving host
      host_3->GetCPU().state.parent_tasks_performed->Set(1); // non-vulnerable surviving host
      symbiont->GetCPU().state.parent_tasks_performed->Set(0);
      symbiont->GetCPU().state.tasks_performed->Set(1);

      
      world.Update();
      REQUIRE(world.GetNumOrgs() == 2);
      THEN("Symbionts are placed or deleted") {
        REQUIRE(world.IsOccupied(0) == false);
        REQUIRE(world.IsOccupied(1) == true); // host_2 should have survived
        REQUIRE(world.IsOccupied(2) == true); // host_3 should have survived

        REQUIRE(host_2->GetSymbionts().size() > 0); // with six offspring, let's expect the vulnerable host to be infected 
        REQUIRE(host_2->GetSymbionts().at(0).DynamicCast<SGPSymbiont>()->GetCPU().state.parent_tasks_performed->Get(1) == true);
        REQUIRE(host_3->GetSymbionts().size() == 0); // and the non-matching host to be spared  
      }
      THEN("The queue is left empty") {
        REQUIRE(world.symbiont_stress_escapee_offspring.size() == 0);
      }
    }
    
    WHEN("Preferential ousting is on") {
      
      config.OUSTING(1);
      config.PREFERENTIAL_OUSTING(2);
      config.PARASITE_DEATH_CHANCE(0.8);

      emp::Ptr<StressHost> host_4 = emp::NewPtr<StressHost>(&random, &world, &config);
      world.AddOrgAt(host_4, 3);

      emp::Ptr<SGPSymbiont> symbiont_3 = emp::NewPtr<SGPSymbiont>(&random, &world, &config);
      host_3->AddSymbiont(symbiont_3);
      emp::Ptr<SGPSymbiont> symbiont_4 = emp::NewPtr<SGPSymbiont>(&random, &world, &config);
      host_4->AddSymbiont(symbiont_4);

      // most can do one common task 
      host->GetCPU().state.parent_tasks_performed->Set(6);   // infected but not parasitized
      host_2->GetCPU().state.parent_tasks_performed->Set(0); // uninfected
      host_3->GetCPU().state.parent_tasks_performed->Set(0); // parasitized
      host_4->GetCPU().state.parent_tasks_performed->Set(0); // parasitized

      symbiont->GetCPU().state.parent_tasks_performed->Set(0);
      
      symbiont_3->GetCPU().state.parent_tasks_performed->Set(0);
      symbiont_3->GetCPU().state.tasks_performed->Set(5);    // used as a tag to identify self (it doesn't get ousted by an escapee offspring)
      
      symbiont_4->GetCPU().state.parent_tasks_performed->Set(0);
      symbiont_4->GetCPU().state.parent_tasks_performed->Set(6);
      symbiont_4->GetCPU().state.tasks_performed->Set(2);    // used as a tag to identify escapee offspring

      world.Update();
      REQUIRE(world.GetNumOrgs() > 1);
      THEN("Host at position 0 survives and has its symbiont ousted by an escapee offspring from position 3") { // host
        // strictly better ousting is required, host and symbiont have 0/1 match and host and symbiont_4 have 1/1 match
        REQUIRE(world.IsOccupied(0));
        REQUIRE(host->HasSym());
        REQUIRE(host->GetSymbionts().at(0).DynamicCast<SGPSymbiont>()->GetCPU().state.parent_tasks_performed->Get(2) == true);
      }
      THEN("Host at position 1 survives and is infected") { // host_2
        REQUIRE(world.IsOccupied(1));
        REQUIRE(host_2->HasSym() );
        REQUIRE(host_2->GetSymbionts().at(0).DynamicCast<SGPSymbiont>()->GetCPU().state.parent_tasks_performed->Get(2) == true);
      }
      THEN("Host at position 2 survives and does not have its symbiont ousted") { // host_3
        // strictly better ousting is required, host_3 and symbiont_3 have the best possible match 1/1
        REQUIRE(world.IsOccupied(2));
        REQUIRE(host_3->HasSym());
        REQUIRE(host_3->GetSymbionts().at(0).DynamicCast<SGPSymbiont>()->GetCPU().state.tasks_performed->Get(5) == true);
      }
      THEN("Host at position 3 dies") { // host_4
        REQUIRE(world.IsOccupied(3) == false);
      }
      THEN("The queue is left empty") {
        REQUIRE(world.symbiont_stress_escapee_offspring.size() == 0);
      }
    }
  }
}

TEST_CASE("Task matching required for (stress) symbiotic behavior", "[sgp][sgp-functional]") {
  
  GIVEN("Stress is the interaction mechanism") {
    emp::Random random(68);
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

TEST_CASE("Safe time configuration option", "[sgp][sgp-functional]") {
  emp::Random random(62);
  SymConfigSGP config;
  config.INTERACTION_MECHANISM(STRESS);
  config.EXTINCTION_FREQUENCY(10);
  config.SAFE_TIME(20);
  config.GRID_X(10);
  config.GRID_Y(10);
  size_t world_size = config.GRID_X() * config.GRID_Y();

  double parasite_death_chance = 0.5; 
  double mutualist_death_chance = 0.125; 
  double base_death_chance = 0.25;
  config.PARASITE_DEATH_CHANCE(parasite_death_chance);
  config.MUTUALIST_DEATH_CHANCE(mutualist_death_chance);
  config.BASE_DEATH_CHANCE(base_death_chance);

  SGPWorld world(random, &config, LogicTasks);

  WHEN("There are stress symbionts in the world"){
    config.START_MOI(1);
    world.SetupHosts(&world_size);
    for (size_t i = 0; i < config.EXTINCTION_FREQUENCY() - 1; i++) world.Update();
    REQUIRE(world.GetNumOrgs() == world_size);
    WHEN("Stress symbionts are mutualists and it's during the safe time"){
      config.SYMBIONT_TYPE(MUTUALIST);
      world.Update();
      THEN("No hosts die") {
        REQUIRE(world.GetNumOrgs() == world_size);
      }
    }
    WHEN("Stress symbionts are parasites and it's during the safe time") {
      config.SYMBIONT_TYPE(PARASITE);
      world.Update();
      THEN("No hosts die") {        
        REQUIRE(world.GetNumOrgs() == world_size);
      }
    }
  }
}
