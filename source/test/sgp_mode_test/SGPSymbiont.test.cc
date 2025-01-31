#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPWorldSetup.cc"
#include "../../sgp_mode/SGPDataNodes.h"

TEST_CASE("SGPSymbiont Reproduce", "[sgp]") {
	emp::Random random(31);
	sgpmode::SymConfigSGP config;
	sgpmode::SGPWorld world(random, &config, sgpmode::LogicTasks);
  world.SetupScheduler();
	emp::Ptr<sgpmode::SGPSymbiont> sym_parent = emp::NewPtr<sgpmode::SGPSymbiont>(
    &random,
    &world,
    &config,
    sgpmode::CreateNotProgram(100)
  );


	THEN("Symbiont child increases its lineage reproduction count"){
		emp::Ptr<sgpmode::SGPSymbiont> sym_baby = (sym_parent->Reproduce()).DynamicCast<sgpmode::SGPSymbiont>();
		REQUIRE(sym_parent->GetReproCount() == sym_baby->GetReproCount() - 1);
		sym_baby.Delete();
    sym_parent.Delete();
	}

  WHEN("Parental task tracking is on") {

    emp::Ptr<sgpmode::SGPHost> host = emp::NewPtr<sgpmode::SGPHost>(
      &random,
      &world,
      &config,
      sgpmode::CreateNotProgram(100)
    );
    config.TRACK_PARENT_TASKS(1);
    world.AddOrgAt(host, 0);
    host->AddSymbiont(sym_parent);

    for (int i = 0; i < 25; i++) {
      world.Update();
    }

    emp::Ptr<sgpmode::SGPSymbiont> sym_baby = (sym_parent->Reproduce()).DynamicCast<sgpmode::SGPSymbiont>();

    THEN("Symbiont child inherits its parent's completed task bitset") {
      REQUIRE(sym_parent->GetCPU().state.parent_tasks_performed->All());

      REQUIRE(sym_parent->GetCPU().state.tasks_performed->Get(0));
      REQUIRE(sym_parent->GetCPU().state.tasks_performed->CountOnes() == 1);

      REQUIRE(sym_baby->GetCPU().state.parent_tasks_performed->Get(0));
      REQUIRE(sym_baby->GetCPU().state.parent_tasks_performed->CountOnes() == 1);
    }

    THEN("The symbiont child tracks any gains or loses in task completions") {
      // in this second generation, we expect the symbiont to lose every task except for NOT
      // since first-gen organisms are uniquely marked as having parents who complete every task
      REQUIRE(sym_baby->GetCPU().state.task_change_lose[0] == 0);
      REQUIRE(sym_baby->GetCPU().state.task_change_gain[0] == 0);
      for (unsigned int i = 1; i < sgpmode::CPU_BITSET_LENGTH; i++) {
        REQUIRE(sym_baby->GetCPU().state.task_change_lose[i] == 1);
        REQUIRE(sym_baby->GetCPU().state.task_change_gain[i] == 0);
      }
    }

    THEN("The symbiont child tracks how its tasks compare to its parent's partner's tasks") {
      REQUIRE(sym_baby->GetCPU().state.task_toward_partner[0] == 0);
      REQUIRE(sym_baby->GetCPU().state.task_from_partner[0] == 0);

      for (unsigned int i = 1; i < sgpmode::CPU_BITSET_LENGTH; i++) {
        REQUIRE(sym_baby->GetCPU().state.task_toward_partner[i] == 0);
        REQUIRE(sym_baby->GetCPU().state.task_from_partner[i] == 1);
      }
    }
    sym_baby.Delete();
  }
}