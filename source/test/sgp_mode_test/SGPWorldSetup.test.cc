#include "../../sgp_mode/SGPWorldSetup.cc"

TEST_CASE("SetupTaskProfileFun", "[sgp]") {
	emp::Random random(61);
	SymConfigSGP config;

	WHEN("TRACK_PARENT_TASKS is on") {
		config.TRACK_PARENT_TASKS(1);
		SGPWorld world(random, &config, LogicTasks);

		emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config);
		emp::Ptr<SGPSymbiont> symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config);

		host->GetCPU().state.tasks_performed->Set(0);
		host->GetCPU().state.parent_tasks_performed->Set(1);
		symbiont->GetCPU().state.tasks_performed->Set(7);
		symbiont->GetCPU().state.parent_tasks_performed->Set(8);

		THEN("fun_get_task_profile returns parental tasks completed"){
			REQUIRE(world.fun_get_task_profile(host).CountOnes() == 1);
			REQUIRE(world.fun_get_task_profile(host).Get(1) == 1);

			REQUIRE(world.fun_get_task_profile(symbiont).CountOnes() == 1);
			REQUIRE(world.fun_get_task_profile(symbiont).Get(8) == 1);
		}
	}
	WHEN("TRACK_PARENT_TASKS is off") {
		config.TRACK_PARENT_TASKS(0);
		SGPWorld world(random, &config, LogicTasks);

		emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config);
		emp::Ptr<SGPSymbiont> symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config);

		host->GetCPU().state.tasks_performed->Set(0);
		host->GetCPU().state.parent_tasks_performed->Set(1);
		symbiont->GetCPU().state.tasks_performed->Set(7);
		symbiont->GetCPU().state.parent_tasks_performed->Set(8);

		THEN("fun_get_task_profile returns self tasks completed") {
			REQUIRE(world.fun_get_task_profile(host).CountOnes() == 1);
			REQUIRE(world.fun_get_task_profile(host).Get(0) == 1);

			REQUIRE(world.fun_get_task_profile(symbiont).CountOnes() == 1);
			REQUIRE(world.fun_get_task_profile(symbiont).Get(7) == 1);
		}
	}
}