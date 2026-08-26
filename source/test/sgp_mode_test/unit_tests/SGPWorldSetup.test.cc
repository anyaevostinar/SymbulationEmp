#include "../../test_utils.h"
#include "../../../default_mode/SymWorld.h"
#include "../../../default_mode/WorldSetup.cc"
#include "../../../default_mode/DataNodes.h"
#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorld.cc"
#include "../../../sgp_mode/SGPWorldSetup.cc"
#include "../../../sgp_mode/SGPWorldData.cc"
#include "../../../sgp_mode/SGPW_InteractionMechanismSetup.cc"
#include "../../../sgp_mode/SGPW_TaskProfileSetup.cc"
#include "../../../sgp_mode/ProgramBuilder.h"

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

// todo: SetupOrgMode test

TEST_CASE("Setup with an empty population", "[sgp]") {
  sgpmode::SymConfigSGP config;
  config.SYM_LIMIT(1);
	test_utils::SetWellMixed(config, 4, 0);
	config.TASK_IO_BANK_SIZE(10);
  config.SEED(234);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  emp::Random random(config.SEED());
  world_t world(random, &config);

  WHEN("POP_SIZE is 0 and setup is called") {
		config.INIT_POP_SIZE(0);
    world.Setup();

    THEN("The world is sized correctly") {
      REQUIRE(world.GetSize() == 4);
    }
    THEN("The world is empty") {
      REQUIRE(world.GetNumOrgs() == 0);
    }
  }
}

TEST_CASE("SetupHosts adds correct number of hosts", "[sgp][sgp-unit]") {
  sgpmode::SymConfigSGP config;
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
	test_utils::SetWellMixed(config, 4, 0);
	config.TASK_IO_BANK_SIZE(10);
  config.SEED(44);
  emp::Random random(config.SEED());
  world_t world(random, &config);

  size_t host_count = 4;

  WHEN("SetupHosts is called (as part of Setup)") {
		config.INIT_POP_SIZE(host_count);
		world.Setup();
    THEN("The correct number of hosts are added") {
      REQUIRE(world.GetSize() == host_count);
      REQUIRE(world.GetNumOrgs() == host_count);
    }
  }
}

TEST_CASE("SetupHosts adds infected hosts correctly", "[sgp][sgp-unit]") {
  sgpmode::SymConfigSGP config;
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
	test_utils::SetWellMixed(config, 4);
	config.TASK_IO_BANK_SIZE(10);
  config.SEED(44);
  config.INIT_POP_SIZE(0);
  emp::Random random(config.SEED());
  world_t world(random, &config);

  size_t host_count = 4;

  WHEN("SetupHosts is called (as part of Setup) and START_MOI is 1") {
		config.START_MOI(1);
		config.INIT_POP_SIZE(host_count);
		world.Setup();

    THEN("The correct number of infected hosts are added") {
      for (size_t i = 0; i < host_count; i++) {
          REQUIRE(world.GetOrgPtr(i)->HasSym());
      }
    }
  }
}


TEST_CASE("Setup correctly sets host task profile functions", "[sgp][sgp-unit]") {
	emp::Random random(61);
	sgpmode::SymConfigSGP config;
	test_utils::SetWellMixed(config, 4, 0);
	config.TASK_IO_BANK_SIZE(10);
	config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");

	WHEN("TASK_PROFILE_MODE tracks all parent tasks") {
		config.TASK_PROFILE_MODE("parent-all");
		world_t world(random, &config);
		world.Setup();

		emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config);

		host->GetHardware().GetCPUState().MarkTaskPerformed(0);
		host->GetHardware().GetCPUState().SetParentTaskPerformed(1);

		THEN("GetHostTaskProfile returns parental tasks completed") {
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
	test_utils::SetWellMixed(config, 4, 0);
	config.TASK_IO_BANK_SIZE(10);
	config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");

	WHEN("TASK_PROFILE_MODE tracks all parent tasks") {
		config.TASK_PROFILE_MODE("parent-all");
		world_t world(random, &config);
		world.Setup();

		emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config);

		symbiont->GetHardware().GetCPUState().MarkTaskPerformed(7);
		symbiont->GetHardware().GetCPUState().SetParentTaskPerformed(8);

		THEN("GetSymbiontTaskProfile returns parental tasks completed") {
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
