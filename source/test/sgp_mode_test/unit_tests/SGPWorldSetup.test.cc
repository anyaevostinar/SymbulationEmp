#include "../../../sgp_mode/SGPWorldSetup.cc"

/**
 * This file is dedicated to unit tests for SGPWorldSetup
 */

using world_t = sgpmode::SGPWorld;
using cpu_state_t = sgpmode::CPUState<world_t>;
using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;
using program_t = typename world_t::sgp_prog_t;


TEST_CASE("Setup correctly sets host task profile functions", "[sgp][sgp-unit]") {
	emp::Random random(61);
	sgpmode::SymConfigSGP config;
	config.GRID_X(2);
	config.GRID_Y(2);
	config.POP_SIZE(0);
	config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");

	WHEN("TASK_PROFILE_MODE tracks all parent tasks") {
		config.TASK_PROFILE_MODE("parent-all");
		world_t world(random, &config);
		world.Setup();

		emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config);

		host->GetHardware().GetCPUState().MarkTaskPerformed(0);
		host->GetHardware().GetCPUState().SetParentTaskPerformed(1);

		THEN("GetHostTaskProfile returns parental tasks completed"){
			REQUIRE(world.GetHostTaskProfile(*host).CountOnes() == 1);
			REQUIRE(world.GetHostTaskProfile(*host).Get(1) == 1);
		}
		host.Delete();
	}

	WHEN("TASK_PROFILE_MODE tracks all self tasks") {
		config.TASK_PROFILE_MODE("self-all");
		world_t world(random, &config);
		world.Setup();
		
		emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config);

		host->GetHardware().GetCPUState().MarkTaskPerformed(0);
		host->GetHardware().GetCPUState().SetParentTaskPerformed(1);

		THEN("GetHostTaskProfile returns self tasks completed") {
			REQUIRE(world.GetHostTaskProfile(*host).CountOnes() == 1);
			REQUIRE(world.GetHostTaskProfile(*host).Get(0) == 1);
		}
		host.Delete();
	
	}
}

TEST_CASE("Setup correctly sets symbiont task profile functions", "[sgp][sgp-unit]") {
	emp::Random random(61);
	sgpmode::SymConfigSGP config;
	config.GRID_X(2);
	config.GRID_Y(2);
	config.POP_SIZE(0);
	config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");

	WHEN("TASK_PROFILE_MODE tracks all parent tasks") {
		config.TASK_PROFILE_MODE("parent-all");
		world_t world(random, &config);
		world.Setup();

		emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config);

		symbiont->GetHardware().GetCPUState().MarkTaskPerformed(7);
		symbiont->GetHardware().GetCPUState().SetParentTaskPerformed(8);

		THEN("GetSymbiontTaskProfile returns parental tasks completed"){
			REQUIRE(world.GetSymbiontTaskProfile(*symbiont).CountOnes() == 1);
			REQUIRE(world.GetSymbiontTaskProfile(*symbiont).Get(8) == 1);
		}
		symbiont.Delete();
	}

	WHEN("TASK_PROFILE_MODE tracks all self tasks") {
		config.TASK_PROFILE_MODE("self-all");
		world_t world(random, &config);
		world.Setup();
		
		emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config);

		symbiont->GetHardware().GetCPUState().MarkTaskPerformed(7);
		symbiont->GetHardware().GetCPUState().SetParentTaskPerformed(8);

		THEN("GetSymbiontTaskProfile returns self tasks completed") {\
			REQUIRE(world.GetSymbiontTaskProfile(*symbiont).CountOnes() == 1);
			REQUIRE(world.GetSymbiontTaskProfile(*symbiont).Get(7) == 1);
		}
		symbiont.Delete();
	}
}
	