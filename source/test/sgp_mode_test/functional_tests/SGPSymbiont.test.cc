#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"
#include "../../../sgp_mode/SGPDataNodes.h"

TEST_CASE("SGPSymbiont Reproduce", "[sgp]") {
  emp::Random random(31);
  SymConfigSGP config;
  SGPWorld world(random, &config, LogicTasks);
  emp::Ptr<SGPSymbiont> sym_parent = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(100));

  THEN("Symbiont child increases its lineage reproduction count") {
    emp::Ptr<SGPSymbiont> sym_baby = (sym_parent->Reproduce()).DynamicCast<SGPSymbiont>();
    REQUIRE(sym_parent->GetReproCount() == sym_baby->GetReproCount() - 1);
    sym_baby.Delete();
    sym_parent.Delete();
  }

  WHEN("Parental task tracking is on") {
    emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));
    config.TRACK_PARENT_TASKS(1);
    world.AddOrgAt(host, 0);
    host->AddSymbiont(sym_parent);

    for (int i = 0; i < 25; i++) {
      world.Update();
    }

    emp::Ptr<SGPSymbiont> sym_baby = (sym_parent->Reproduce()).DynamicCast<SGPSymbiont>();

    THEN("Symbiont child inherits its parent's empty task bitset") {
      REQUIRE(sym_parent->GetCPU().state.parent_tasks_performed->None());

      REQUIRE(sym_parent->GetCPU().state.tasks_performed->Get(0));
      REQUIRE(sym_parent->GetCPU().state.tasks_performed->CountOnes() == 1);

      REQUIRE(sym_baby->GetCPU().state.parent_tasks_performed->Get(0));
      REQUIRE(sym_baby->GetCPU().state.parent_tasks_performed->CountOnes() == 1);
    }


    THEN("The symbiont child tracks any gains or loses in task completions") {
      // in this second generation, we expect the symbiont to gain the NOT task
      // since first-gen organisms are marked as having parents who have completed no tasks
      REQUIRE(sym_baby->GetCPU().state.task_change_lose[0] == 0);
      REQUIRE(sym_baby->GetCPU().state.task_change_gain[0] == 1);
      for (unsigned int i = 1; i < CPU_BITSET_LENGTH; i++) {
        REQUIRE(sym_baby->GetCPU().state.task_change_lose[i] == 0);
        REQUIRE(sym_baby->GetCPU().state.task_change_gain[i] == 0);
      }
    }

    THEN("The symbiont child tracks how its tasks compare to its parent's partner's tasks") {
      REQUIRE(sym_baby->GetCPU().state.task_toward_partner[0] == 0);
      REQUIRE(sym_baby->GetCPU().state.task_from_partner[0] == 1);

      for (unsigned int i = 1; i < CPU_BITSET_LENGTH; i++) {
        REQUIRE(sym_baby->GetCPU().state.task_toward_partner[i] == 0);
        REQUIRE(sym_baby->GetCPU().state.task_from_partner[i] == 0);
      }
    }
    sym_baby.Delete();
  }
}

TEST_CASE("Symbiont comparison operators", "[sgp]") {
  emp::Random random(31);
	SymConfigSGP config;
	SGPWorld world(random, &config, LogicTasks);
	emp::Ptr<SGPSymbiont> sym_parent = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(100));

    emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));
    config.TRACK_PARENT_TASKS(1);
    world.AddOrgAt(host, 0);
    host->AddSymbiont(sym_parent);

    for (int i = 0; i < 25; i++) {
      world.Update();
    }
    emp::Ptr<SGPSymbiont> clone1 = emp::NewPtr<SGPSymbiont>(*sym_parent);
    emp::Ptr<SGPSymbiont> clone2 = emp::NewPtr<SGPSymbiont>(*sym_parent);
    emp::Ptr<SGPSymbiont> different = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(99)); // For comparing

    REQUIRE(*sym_parent == *clone1);
    REQUIRE(*clone1 == *clone2);

    REQUIRE_FALSE(*sym_parent == *different);

    // Can't assert true/false without knowing bitcode ordering,
    // assert that bitcode ordering is well-defined
    bool lt = *sym_parent < *different || *different < *sym_parent;
    REQUIRE(lt);
    
    clone1.Delete();
    clone2.Delete();
    different.Delete();
  }

TEST_CASE("SGPSymbiont Vertical Transmission", "[sgp-vt]")

{
  emp::Random random(42);
  SymConfigSGP config;
  config.INTERACTION_MECHANISM(DEFAULT);
  config.GRID_X(10);
  config.GRID_Y(10);
  config.HOST_REPRO_RES(1);
  config.SYM_VERT_TRANS_RES(0);
  config.HORIZ_TRANS(0);

  SGPWorld world(random, &config, LogicTasks);
  ProgramBuilder builder;
  ProgramBuilder symbuilder;
  builder.AddNot(); //First task
  emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, builder.Build(100));
  emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, symbuilder.Build(100));

  world.AddOrgAt(host, 0);
  host->AddSymbiont(sym);
  REQUIRE(world.GetNumOrgs() == 1);
  WHEN("Vertical Transmission is on"){
    config.VERTICAL_TRANSMISSION(1);
    WHEN("Vertical Task Match is on"){
      config.VT_TASK_MATCH(1);
      WHEN("Host and Symbiont share a matching task"){
        sym->GetCPU().state.tasks_performed->Set(0);
        host->GetCPU().state.tasks_performed->Set(0);
        
        for(int i = 0; i < 26; i++){
          
          world.Update();
        }

        THEN("Symbiont vertically transmits"){
          REQUIRE(world.GetVerticalTransmissionAttemptCount().GetTotal() == 1);
          REQUIRE(world.GetVerticalTransmissionSuccessCount().GetTotal() == 1);
        }
      }
      WHEN("Host and Symbiont do not share a matching task"){
        
        host->GetCPU().state.tasks_performed->Set(0);
       
        for(int i = 0; i < 26; i++){
          
          world.Update();
        }
        THEN("Symbiont does not vertically transmit"){
          REQUIRE(world.GetVerticalTransmissionAttemptCount().GetTotal() == 0);
          REQUIRE(world.GetVerticalTransmissionSuccessCount().GetTotal() == 0);
        }
      }
    }
    WHEN("Vertical Task Match is off"){
      config.VT_TASK_MATCH(0);
      WHEN("Host and Symbiont share a matching task"){

        sym->GetCPU().state.tasks_performed->Set(0);
        host->GetCPU().state.tasks_performed->Set(0);
        for(int i = 0; i < 26; i++){
          
          world.Update();
        }
        THEN("Symbiont vertically transmits"){
          REQUIRE(world.GetVerticalTransmissionAttemptCount().GetTotal() == 1);
          REQUIRE(world.GetVerticalTransmissionSuccessCount().GetTotal() == 1); 
        }
      }
      WHEN("Host and Symbiont do not share a matching task"){
       
        host->GetCPU().state.tasks_performed->Set(0);
        for(int i = 0; i < 26; i++){
          
          world.Update();
        }
        THEN("Symbiont vertically transmits"){
          REQUIRE(world.GetVerticalTransmissionAttemptCount().GetTotal() == 1);
          REQUIRE(world.GetVerticalTransmissionSuccessCount().GetTotal() == 1);
        }
      }
    }
  }
  WHEN("Vertical Transmission is off"){
    config.VERTICAL_TRANSMISSION(0);
    config.VT_TASK_MATCH(0);
    for(int i = 0; i < 26; i++){
          
          world.Update();
        }
    THEN("Symbiont does not vertically transmit"){
      REQUIRE(world.GetVerticalTransmissionAttemptCount().GetTotal() == 0);
      REQUIRE(world.GetVerticalTransmissionSuccessCount().GetTotal() == 0);
    }
  }

}

TEST_CASE("SGPSymbiont Horizontal Transmission", "[sgp-vt]")

{
  emp::Random random(42);
  SymConfigSGP config;
  config.INTERACTION_MECHANISM(DEFAULT);
  config.GRID_X(10);
  config.GRID_Y(10);
  config.HOST_REPRO_RES(100);
  config.SYM_HORIZ_TRANS_RES(0);
  config.VERTICAL_TRANSMISSION(0);

  SGPWorld world(random, &config, LogicTasks);
  ProgramBuilder builder;
  ProgramBuilder symbuilder;
  symbuilder.AddNot(); //First task
  emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, builder.Build(100));
  emp::Ptr<SGPHost> host2 = emp::NewPtr<SGPHost>(&random, &world, &config, builder.Build(100));
  emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, symbuilder.Build(100));

  world.AddOrgAt(host, 0);
  world.AddOrgAt(host2,1);
  host->AddSymbiont(sym);
  REQUIRE(world.GetNumOrgs() == 2);
  WHEN("Horizontal Transmission is on"){
    config.HORIZ_TRANS(1);
    WHEN("Horizontal Task Match is on"){
      config.HT_TASK_MATCH(1);
      WHEN("Host and Symbiont share a matching task"){
        sym->GetCPU().state.tasks_performed->Set(0);
        host->GetCPU().state.tasks_performed->Set(0);
        
        for(int i = 0; i < 26; i++){
          
          world.Update();
        }

        THEN("Symbiont horizontally transmits"){
          REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetTotal() == 1);
          REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetTotal() == 1);
        }
      }
      WHEN("Host and Symbiont do not share a matching task"){
        
        sym->GetCPU().state.tasks_performed->Set(0);
       
        for(int i = 0; i < 26; i++){
          
          world.Update();
        }
        THEN("Symbiont does not horizontally transmit"){
          REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetTotal() == 1);
          REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetTotal() == 0);
        }
      }
    }
    WHEN("Horizontal Task Match is off"){
      config.HT_TASK_MATCH(0);
      WHEN("Host and Symbiont share a matching task"){

        sym->GetCPU().state.tasks_performed->Set(0);
        host->GetCPU().state.tasks_performed->Set(0);
        for(int i = 0; i < 26; i++){
          
          world.Update();
        }
        THEN("Symbiont horizontally transmits"){
          REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetTotal() == 1);
          REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetTotal() == 1); 
        }
      }
      WHEN("Host and Symbiont do not share a matching task"){
       
        sym->GetCPU().state.tasks_performed->Set(0);
        for(int i = 0; i < 26; i++){
          
          world.Update();
        }
        THEN("Symbiont horizontally transmits"){
          REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetTotal() == 1);
          REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetTotal() == 1);
        }
      }
    }
  }
  WHEN("Horizontal Transmission is off"){
    config.HORIZ_TRANS(0);
    config.HT_TASK_MATCH(0);
    for(int i = 0; i < 26; i++){
          
          world.Update();
        }
    THEN("Symbiont does not horizontally transmit"){
      REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetTotal() == 1);
      REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetTotal() == 0);
    }
  }

}