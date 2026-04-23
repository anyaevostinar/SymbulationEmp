#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"
//#include "../../../sgp_mode/SGPDataNodes.h"


/**
 * This file is dedicated to tests related to SGPHost reproduction, including checking that task tracking 
 * is working correctly.
 */

using world_t = sgpmode::SGPWorld;
using cpu_state_t = sgpmode::CPUState<world_t>;
using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;

TEST_CASE("Reproduction without points or mutations", "[sgp][sgp-functional]") {
  GIVEN("An SGPWorld and a host"){


    emp::Random random(65);
    sgpmode::SymConfigSGP config;
    config.GRID_X(10);
    config.GRID_Y(10);
    config.HOST_REPRO_RES(0);
    config.CYCLES_PER_UPDATE(1);
    config.SEED(62);
    config.POP_SIZE(0);
    config.START_MOI(0);
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");

    int world_size = config.GRID_X() * config.GRID_Y();
    world_t world(random, &config);
    world.Setup();
    world.Resize(100);
    auto& prog_builder = world.GetProgramBuilder();
    

    emp::Ptr<sgp_host_t> host_parent = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateReproProgram(10));

    world.AddOrgAt(host_parent, 0);
    REQUIRE(world.IsOccupied(0));
    REQUIRE(world.GetNumOrgs() ==1);

    WHEN("The World runs enough updates for host to reproduce"){
        for (int i = 0; i < 15; i++) {
          world.Update();
        }
      REQUIRE(world.GetNumOrgs() == 2);
      emp::Ptr<Organism> child_ptr;

      //Look for the offspring
      for(int i = 1; i < world_size; i++) {
        if(world.IsOccupied(i)) {
          child_ptr = &world.GetOrg(i);
          break;
      }
    }
      auto& host_baby = static_cast<sgp_host_t&>(*child_ptr);

      THEN("Host child increases its lineage reproduction count") {
        REQUIRE(host_baby.GetReproCount() == 1);
        //host_baby.Delete();
      }

    }
  }
}

TEST_CASE("Mutations occur during reproduction", "[sgp]") {
  GIVEN("A world with high mutation rate") {
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;

  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.GRID_X(2);
  config.GRID_Y(2);
  config.SGP_MUT_PER_BIT_RATE(100.0);
  config.HOST_REPRO_RES(0);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");


  world_t world(random, &config);
  world.Setup();
  world.Resize(2,2);

  auto& prog_builder = world.GetProgramBuilder();

  emp::Ptr<sgp_host_t> uninfected_host = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateReproProgram(10));

  world.AddOrgAt(uninfected_host, 0);
  REQUIRE(world.IsOccupied(0));

  WHEN("Reproduce is called") {
    auto org = uninfected_host->Reproduce();
    auto& host_baby = static_cast<sgp_host_t&>(*org);

    THEN("Host offspring is different than parent") {
      REQUIRE(host_baby != *uninfected_host);
      //host_baby.Delete();
    }
  }

  }


}