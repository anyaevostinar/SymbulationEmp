#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorld.cc"
#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"
#include "../../../sgp_mode/SGPWorldData.cc"
#include "../../../catch/catch.hpp"

/**
 * This file is dedicated to unit tests for SGPWorld
 */

using world_t = sgpmode::SGPWorld;
using cpu_state_t = sgpmode::CPUState<world_t>;
using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;
using program_t = typename world_t::sgp_prog_t;


TEST_CASE("Update only hosts test", "[sgp]") {


  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.GRID_X(2);
  config.GRID_Y(2);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");


  world_t world(random, &config);
  world.Resize(2,2);

  emp::Ptr<sgp_host_t> uninfected_host = emp::NewPtr<sgp_host_t>(&random, &world, &config);

  world.AddOrgAt(uninfected_host, 0);
  // TODO: this doesn't work if you add at position 1 instead, is that a problem?

  THEN("Organisms can be added to the world") {
    REQUIRE(world.GetNumOrgs() == 1);
    REQUIRE(world.GetOrgPtr(0)->GetAge() == 0);
  }



  for (int i = 0; i < 10; i++) {
    world.Update();
  }

  THEN("Organisms persist and are managed by the world") {
    REQUIRE(world.GetNumOrgs() == 1);
    REQUIRE(world.GetOrgPtr(0)->GetAge() == 10);
  }
}



TEST_CASE("Host Setup", "[sgp][sgp-unit]") {
  emp::Random random(1);
  sgpmode::SymConfigSGP config;
  config.SEED(2);
  config.MUTATION_RATE(0.0);
  config.MUTATION_SIZE(0.000);
  //config.TRACK_PARENT_TASKS(PARENTONLY);
  config.VT_TASK_MATCH(1);
  config.HOST_ONLY_FIRST_TASK_CREDIT(1);
  config.SYM_ONLY_FIRST_TASK_CREDIT(1);
  config.HOST_REPRO_RES(10000);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");

  //world.SetupHosts requires a pointer for the number of hosts in the world
  unsigned long setupCount = 1;
  //TODO?
}

TEST_CASE("Ousting is permitted", "[sgp]") {
  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.GRID_X(2);
  config.GRID_Y(2);
  config.OUSTING(1);
  config.SYM_LIMIT(1);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");

  world_t world(random, &config);
  world.Setup();
  world.Resize(2, 2);

  emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config);
  emp::Ptr<sgp_sym_t> old_symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config);
  emp::Ptr<sgp_sym_t> new_symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config);
  WHEN("Symbiont is added to host that has a symbiont") {
    host->AddSymbiont(old_symbiont);
    world.AddOrgAt(host, 0);

    REQUIRE(host->GetSymbionts().size() == 1);
    REQUIRE(world.GetGraveyard().size() == 0);
    host->AddSymbiont(new_symbiont);
    THEN("Less matching symbiont is ousted to graveyard") {
        REQUIRE(host->GetSymbionts().size() == 1);
        REQUIRE(world.GetGraveyard().size() == 1);
    }
  }

}

TEST_CASE("TaskMatchCheck", "[sgp]") {
  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.GRID_X(2);
  config.GRID_Y(2);
  config.SYM_LIMIT(2);
  config.POP_SIZE(1);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");

  world_t world(random, &config);
  world.Setup();
  world.Resize(2, 2);

  auto& builder = world.GetProgramBuilder();


  emp::Ptr<sgp_host_t> NOT_host = emp::NewPtr<sgp_host_t>(&random, &world, &config, builder.CreateNotProgram(100));
  emp::Ptr<sgp_sym_t> NOT_symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config, builder.CreateNotProgram(100));
  emp::Ptr<sgp_sym_t> NAND_symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config, builder.CreateNandProgram(100));

  NOT_host->AddSymbiont(NOT_symbiont);
  NOT_host->AddSymbiont(NAND_symbiont);
  world.AddOrgAt(NOT_host, 0);


  bool not_not_matched = false;
  bool not_nand_matched = false;
  for (int i = 0; i < 100; i++) {
    world.Update();

    not_not_matched = sgpmode::utils::AnyMatchingOnes(
      NOT_symbiont->GetHardware().GetCPUState().GetTasksPerformed(),
      NOT_host->GetHardware().GetCPUState().GetTasksPerformed()
    );
    not_nand_matched = sgpmode::utils::AnyMatchingOnes(
      NAND_symbiont->GetHardware().GetCPUState().GetTasksPerformed(),
      NOT_host->GetHardware().GetCPUState().GetTasksPerformed()
    );
  }

  WHEN("A host and symbiont can both do at least one same task") {
    THEN("TaskMatchCheck returns true") {
      REQUIRE(not_not_matched == true);
    }
  }
  WHEN("A host and symbiont have no tasks they can both do") {
    THEN("TaskMatchCheck returns false") {
      REQUIRE(not_nand_matched == false);
    }
  }
}