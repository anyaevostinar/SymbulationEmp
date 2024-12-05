#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPWorldSetup.cc"
#include "../../sgp_mode/SGPDataNodes.h"

TEST_CASE("SGPSymbiont Reproduce", "[sgp]") {
	emp::Random random(31);
	SymConfigSGP config;
	SGPWorld world(random, &config, LogicTasks);
	emp::Ptr<SGPSymbiont> sym_parent = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(100));
	

	THEN("Symbiont child increases its lineage reproduction count"){
		emp::Ptr<SGPSymbiont> sym_baby = (sym_parent->Reproduce()).DynamicCast<SGPSymbiont>();
		REQUIRE(sym_parent->GetReproCount() == sym_baby->GetReproCount() - 1);
		sym_baby.Delete();
	}

	WHEN("Parental task tracking is on") {
		config.TRACK_PARENT_TASKS(1);
		emp::Ptr<SGPSymbiont> sym_baby = (sym_parent->Reproduce()).DynamicCast<SGPSymbiont>();
		THEN("Symbiont child inherits its parent's completed task bitset") {
			REQUIRE(sym_parent->GetCPU().state.parent_tasks_performed->All());
			REQUIRE(sym_parent->GetCPU().state.tasks_performed->None());
			REQUIRE(sym_baby->GetCPU().state.parent_tasks_performed->None());
		}
		sym_baby.Delete();
	}
	
	sym_parent.Delete();
	
}

TEST_CASE("SGPSymbiont track divergence from parent & from parent's partner") {
	REQUIRE(1 == 0);
}