#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPHost.cc"
#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"
#include "../../../sgp_mode/SGPDataNodes.h"

TEST_CASE("SGPHost Reproduce parental task tracking", "[sgp]") {
  emp::Random random(31);
  SymConfigSGP config;
  SGPWorld world(random, &config, LogicTasks);
  
  WHEN("Parental task tracking is on") {
    config.HOST_REPRO_RES(5000);
    config.MUTATION_RATE(0);
    config.MUTATION_SIZE(0);
    config.TRACK_PARENT_TASKS(1);
    WHEN("A host can only perform NOT") {
      WHEN("It is of the first generation (does not have parents)") {
        emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));
        emp::Ptr<emp::BitSet<CPU_BITSET_LENGTH>> parent_tasks = host->GetCPU().state.parent_tasks_performed;
        emp::Ptr<emp::BitSet<CPU_BITSET_LENGTH>> host_tasks = host->GetCPU().state.tasks_performed;

        world.AddOrgAt(host, 0);

        THEN("Its own tasks are initially all marked as uncompleted") {
          REQUIRE(host_tasks->None());
        }
        THEN("Its parent's tasks are all marked as uncompleted") {
          REQUIRE(parent_tasks->None());
        }
        for (int i = 0; i < 25; i++) {
          world.Update();
        }
        THEN("After running for several updates, it is only tracked as completing NOT") {
          // NOT id is 0 
          REQUIRE(host_tasks->Get(0));
          REQUIRE(host_tasks->CountOnes() == 1);

          host_tasks->Toggle(1, CPU_BITSET_LENGTH);
          REQUIRE(host_tasks->All());
        }
      }
      WHEN("It has a parent who could only perform NOT") {
        emp::Ptr<SGPHost> parent_host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));
        world.AddOrgAt(parent_host, 0);
        for (int i = 0; i < 25; i++) {
          world.Update();
        }
        emp::Ptr<SGPHost> host = (parent_host->Reproduce()).DynamicCast<SGPHost>();

        world.AddOrgAt(host, 0);
        REQUIRE(world.GetNumOrgs() == 1);
        
        emp::Ptr<emp::BitSet<CPU_BITSET_LENGTH>> host_tasks = host->GetCPU().state.tasks_performed;
        emp::Ptr<emp::BitSet<CPU_BITSET_LENGTH>> parent_tasks = host->GetCPU().state.parent_tasks_performed;
        
        THEN("Its own tasks are initially all marked as uncompleted") {
          REQUIRE(host_tasks->None());
        }

        for (int i = 0; i < 25; i++) {
          world.Update();
        }
        
        THEN("Only NOT is recorded are successful for its parent") {
          REQUIRE(parent_tasks->Get(0));
          REQUIRE(parent_tasks->CountOnes() == 1);
          parent_tasks->Toggle(1, CPU_BITSET_LENGTH);
          REQUIRE(parent_tasks->All());
        }

        THEN("It is only tracked as completing NOT") {
          REQUIRE(host_tasks->Any());
        }
      }
    }
  }
}

TEST_CASE("SGPHost Reproduce", "[sgp]") {
  emp::Random random(31);
  SymConfigSGP config;
  SGPWorld world(random, &config, LogicTasks);
  emp::Ptr<SGPHost> host_parent = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));
  world.AddOrgAt(host_parent, 0);

  THEN("Host child increases its lineage reproduction count") {
    emp::Ptr<SGPHost> host_baby = (host_parent->Reproduce()).DynamicCast<SGPHost>();
    REQUIRE(host_parent->GetReproCount() == host_baby->GetReproCount() - 1);
    host_baby.Delete();
  }

  WHEN("Parental task tracking is on") {
    config.TRACK_PARENT_TASKS(1);

    for (int i = 0; i < 25; i++) {
      world.Update();
    }
    
    THEN("Host child inherits its parent's completed task bitset") {
      emp::Ptr<SGPHost> host_baby = (host_parent->Reproduce()).DynamicCast<SGPHost>();
      REQUIRE(host_parent->GetCPU().state.parent_tasks_performed->None());

      REQUIRE(host_parent->GetCPU().state.tasks_performed->Get(0));
      REQUIRE(host_parent->GetCPU().state.tasks_performed->CountOnes() == 1);

      REQUIRE(host_baby->GetCPU().state.parent_tasks_performed->Get(0));
      REQUIRE(host_baby->GetCPU().state.parent_tasks_performed->CountOnes() == 1);
      host_baby.Delete();
    }

    THEN("The host child tracks any gains or loses in task completions") {
      // gen 1 (host_parent) parent tasks: all ones 
      // gen 2 (host_gen2) parent tasks: only NOT (gains nothing, loses everything but not) : checks task-specific loss
      // gen 3 (host_gen3) parent tasks: only NAND (gains NAND, loses NOT) : checks task-specific gain
      // gen 4 (host_gen4) parent tasks: NOT and EQU (gains NOT and EQU, loses NAND) : checks loss of >1 
      // gen 5 (host_gen5) parent tasks: NOT and NAND (gains NAND, loses EQU) : checks gain of >1

      enum TaskIndices {NOT_i = 0, NAND_i = 1, EQU_i = 8};
      for (unsigned int i = 0; i < CPU_BITSET_LENGTH; i++) {
         host_parent->GetCPU().state.parent_tasks_performed->Set(i);
      }
      emp::Ptr<SGPHost> host_gen2 = (host_parent->Reproduce()).DynamicCast<SGPHost>();
      REQUIRE(host_gen2->GetCPU().state.task_change_lose[NOT_i] == 0);
      REQUIRE(host_gen2->GetCPU().state.task_change_gain[NOT_i] == 0);
      for (unsigned int i = 1; i < CPU_BITSET_LENGTH; i++) {
        REQUIRE(host_gen2->GetCPU().state.task_change_lose[i] == 1);
        REQUIRE(host_gen2->GetCPU().state.task_change_gain[i] == 0);
      }


      host_gen2->GetCPU().state.tasks_performed->Set(NAND_i);
      emp::Ptr<SGPHost> host_gen3 = (host_gen2->Reproduce()).DynamicCast<SGPHost>();
      for (unsigned int i = 0; i < CPU_BITSET_LENGTH; i++) {
        if(i == NAND_i){ // gains NAND this gen, lost it last gen
          REQUIRE(host_gen3->GetCPU().state.task_change_lose[i] == 1);
          REQUIRE(host_gen3->GetCPU().state.task_change_gain[i] == 1);
        }
        else{
          REQUIRE(host_gen3->GetCPU().state.task_change_lose[i] == 1);
          REQUIRE(host_gen3->GetCPU().state.task_change_gain[i] == 0);
        }
      }

      host_gen3->GetCPU().state.tasks_performed->Set(NOT_i);
      host_gen3->GetCPU().state.tasks_performed->Set(EQU_i);
      emp::Ptr<SGPHost> host_gen4 = (host_gen3->Reproduce()).DynamicCast<SGPHost>();
      for (unsigned int i = 0; i < CPU_BITSET_LENGTH; i++) {
        if(i == NOT_i || i == EQU_i){ // gains NOT & EQU this gen, lost them previously
          REQUIRE(host_gen4->GetCPU().state.task_change_lose[i] == 1);
          REQUIRE(host_gen4->GetCPU().state.task_change_gain[i] == 1);
        } 
        else if(i == NAND_i){ // lost NAND this gen, lost it and gained it once each previously
          REQUIRE(host_gen4->GetCPU().state.task_change_lose[i] == 2);
          REQUIRE(host_gen4->GetCPU().state.task_change_gain[i] == 1);
        }
        else{
          REQUIRE(host_gen4->GetCPU().state.task_change_lose[i] == 1);
          REQUIRE(host_gen4->GetCPU().state.task_change_gain[i] == 0);
        }
      }

      host_gen4->GetCPU().state.tasks_performed->Set(NOT_i);
      host_gen4->GetCPU().state.tasks_performed->Set(NAND_i);
      emp::Ptr<SGPHost> host_gen5 = (host_gen4->Reproduce()).DynamicCast<SGPHost>();
      for (unsigned int i = 0; i < CPU_BITSET_LENGTH; i++) {
        if(i == NOT_i){ // keeps NOT this gen, lost it previously
          REQUIRE(host_gen5->GetCPU().state.task_change_lose[i] == 1);
          REQUIRE(host_gen5->GetCPU().state.task_change_gain[i] == 1);
        } 
        else if(i == NAND_i){ // gained NAND this gen, lost it twice and gained it once previously
          REQUIRE(host_gen5->GetCPU().state.task_change_lose[i] == 2);
          REQUIRE(host_gen5->GetCPU().state.task_change_gain[i] == 2);
        }
        else if(i == EQU_i){ // lost EQU this gen, gained it and lost it once previously
          REQUIRE(host_gen5->GetCPU().state.task_change_lose[i] == 2);
          REQUIRE(host_gen5->GetCPU().state.task_change_gain[i] == 1);
        }
        else{
          REQUIRE(host_gen5->GetCPU().state.task_change_lose[i] == 1);
          REQUIRE(host_gen5->GetCPU().state.task_change_gain[i] == 0);
        }
      }

      host_gen2.Delete();
      host_gen3.Delete();
      host_gen4.Delete();
      host_gen5.Delete();
    }
    WHEN("The host parent has no symbiont") {
      emp::Ptr<SGPHost> host_baby = (host_parent->Reproduce()).DynamicCast<SGPHost>();
      THEN("The host child inherits the lineage's partner task flip count with no modifications"){
        for (unsigned int i = 0; i < CPU_BITSET_LENGTH; i++) {
          REQUIRE(host_baby->GetCPU().state.task_toward_partner[i] == 0);
          REQUIRE(host_baby->GetCPU().state.task_from_partner[i] == 0);
        }
      }
      host_baby.Delete();
    }
    WHEN("The host parent has a symbiont") {
      host_parent->AddSymbiont(emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(100)));
      emp::Ptr<SGPHost> host_baby = (host_parent->Reproduce()).DynamicCast<SGPHost>();
      
      THEN("The host child tracks how its tasks compare to its parent's partner's tasks") {

        REQUIRE(host_baby->GetCPU().state.task_toward_partner[0] == 0);
        REQUIRE(host_baby->GetCPU().state.task_from_partner[0] == 1);

        for (unsigned int i = 1; i < CPU_BITSET_LENGTH; i++) {
          REQUIRE(host_baby->GetCPU().state.task_toward_partner[i] == 0);
          REQUIRE(host_baby->GetCPU().state.task_from_partner[i] == 0);
        }
      }
      host_baby.Delete();
    }
  } 
}
