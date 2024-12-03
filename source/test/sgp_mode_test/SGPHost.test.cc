#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPWorldSetup.cc"
#include "../../sgp_mode/SGPDataNodes.h"

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
        THEN("Its parent's tasks are all marked as successful") {
          REQUIRE(parent_tasks->All());
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
  

  THEN("Host child increases its lineage reproduction count") {
    emp::Ptr<SGPHost> host_baby = (host_parent->Reproduce()).DynamicCast<SGPHost>();
    REQUIRE(host_parent->GetReproCount() == host_baby->GetReproCount() - 1);
    host_baby.Delete();
  }

  WHEN("Parental task tracking is on") {
    config.TRACK_PARENT_TASKS(1);
    emp::Ptr<SGPHost> host_baby = (host_parent->Reproduce()).DynamicCast<SGPHost>();
    THEN("Host child inherits its parent's completed task bitset") {
      REQUIRE(host_parent->GetCPU().state.parent_tasks_performed->All());
      REQUIRE(host_parent->GetCPU().state.tasks_performed->None());
      REQUIRE(host_baby->GetCPU().state.parent_tasks_performed->None());
    }
    host_baby.Delete();
  }

  host_parent.Delete();
}

