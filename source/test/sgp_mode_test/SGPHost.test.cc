#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPWorldSetup.cc"
#include "../../sgp_mode/SGPDataNodes.h"

TEST_CASE("SGPHost MakeNew parental task tracking", "[sgp]") {
  emp::Random random(31);
  SymConfigSGP config;
  SGPWorld world(random, &config, LogicTasks);
  
  WHEN("Parental task tracking is on") {
    config.HOST_REPRO_RES(5000);
    config.MUTATION_RATE(0);
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
        emp::Ptr<SGPHost> host = (parent_host->MakeNew()).DynamicCast<SGPHost>();

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