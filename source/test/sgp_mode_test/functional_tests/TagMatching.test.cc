#include "emp/math/Random.hpp"

#include "../../../sgp_mode/hardware/SGPHardware.h"
#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorld.cc"
#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"
#include "../../../sgp_mode/SGPWorldData.cc"
#include "../../../sgp_mode/ProgramBuilder.h"

#include "../../../catch/catch.hpp"

/**
 * This file is dedicated to tag-based interaction for SGP organisms
 */

using world_t = sgpmode::SGPWorld;
using cpu_state_t = sgpmode::CPUState<world_t>;
using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;
using program_t = typename world_t::sgp_prog_t;

TEST_CASE("SGP Tag Matching for organism interaction compatibility", "[sgp]"){
  // set up configs
  sgpmode::SymConfigSGP config;
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.FILE_PATH("TempChangingEnv_test_output");
  config.SEED(89);
  config.INIT_POP_SIZE(0);
  config.WORLD_WIDTH(2);
  config.WORLD_HEIGHT(2);
  config.CYCLES_PER_UPDATE(4);
  config.TAG_MATCHING(1);
  config.INTERACTION_PROFILE_COMPATIBILITY_MODE("tag-probabilistic-match");

  // initialize world
  emp::Random random(config.SEED());
  world_t world(random, &config);
  world.Setup();

  
  GIVEN("Tag-based interaction compatability is on and a host and symbiont have matching tags"){
    
    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, world.GetProgramBuilder().CreateNotProgram(50));
    emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config, world.GetProgramBuilder().CreateNotProgram(50));
    
    world.AddOrgAt(host, 0);
    host->AddSymbiont(symbiont);

    emp::BitSet<32> all_zeroes = emp::BitSet<32>(32, false);
    host->SetTag(all_zeroes);
    symbiont->SetTag(all_zeroes);

    WHEN("A host and symbiont try to interact"){
      bool can_interact = world.GetInteractionCompatibility(*host, *symbiont);

      THEN("They are allowed to interact"){
        REQUIRE(can_interact == true);
      }
    }
  }

  GIVEN("Tag-based interaction compatability is on and a host and symbiont have mismatching tags"){
    
    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, world.GetProgramBuilder().CreateNotProgram(50));
    emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config, world.GetProgramBuilder().CreateNotProgram(50));
    
    world.AddOrgAt(host, 0);
    host->AddSymbiont(symbiont);

    emp::BitSet<32> all_zeroes = emp::BitSet<32>(32, false);
    emp::BitSet<32> all_ones = emp::BitSet<32>(32, true);
    host->SetTag(all_zeroes);
    symbiont->SetTag(all_ones);

    WHEN("A host and symbiont try to interact"){
      bool can_interact = world.GetInteractionCompatibility(*host, *symbiont);

      THEN("They are not allowed to interact"){
        REQUIRE(can_interact == false);
      }
    }
  }
}