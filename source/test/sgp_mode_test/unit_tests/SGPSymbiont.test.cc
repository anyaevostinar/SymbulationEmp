#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"
//#include "../../../sgp_mode/SGPDataNodes.h"
#include "../../../sgp_mode/hardware/SGPHardware.h"
#include "../../../sgp_mode/SGPWorld.cc"
#include "../../../sgp_mode/ProgramBuilder.h"
using world_t = sgpmode::SGPWorld;
using cpu_state_t = sgpmode::CPUState<world_t>;
using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;

/**
 * This file is dedicated to unit tests for SGPSymbiont
 */

 TEST_CASE("Symbiont == operator", "[sgp][sgp-unit]"){
  GIVEN("A symbiont"){
    emp::Random random(31);
    sgpmode::SymConfigSGP config;
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    world_t world(random, &config);
    auto& prog_builder = world.GetProgramBuilder();
    
    emp::Ptr<sgp_sym_t> sym_parent = emp::NewPtr<sgp_sym_t>(&random, &world, &config, prog_builder.CreateNotProgram(100));
    WHEN("2 symbionts that are clones of the original symbiont"){
        emp::Ptr<sgp_sym_t> clone1 = emp::NewPtr<sgp_sym_t>(*sym_parent);
        emp::Ptr<sgp_sym_t> clone2 = emp::NewPtr<sgp_sym_t>(*sym_parent);
        
        THEN("symbiont is equal to first clone"){
          REQUIRE(*sym_parent == *clone1);
        }
        THEN("The first clone is equal to the second clone"){
          REQUIRE(*clone1 == *clone2);
        }
    }
    
    WHEN("A host that is different to the original is created"){
      emp::Ptr<sgp_sym_t> different = emp::NewPtr<sgp_sym_t>(&random, &world, &config, prog_builder.CreateNotProgram(99)); // For comparing
      THEN("The original symbiont is not equal to the different host"){
        REQUIRE_FALSE(*sym_parent == *different);
      }
    }
  }
}

TEST_CASE("Symbiont > & < operator","[sgp][sgp-unit]"){
  GIVEN("Two different symbionts"){
    emp::Random random(31);
    sgpmode::SymConfigSGP config;
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    world_t world(random, &config);
    auto& prog_builder = world.GetProgramBuilder();

    emp::Ptr<sgp_sym_t> sym_parent = emp::NewPtr<sgp_sym_t>(&random, &world, &config, prog_builder.CreateNotProgram(100));
    emp::Ptr<sgp_sym_t> different = emp::NewPtr<sgp_sym_t>(&random, &world, &config, prog_builder.CreateNotProgram(99)); // For comparing
    WHEN("The two symbionts are compared"){
      THEN("One symbiont is less then the other symbiont"){
        // Can't assert true/false without knowing bitcode ordering,
        // assert that bitcode ordering is well-defined
        bool lt = *sym_parent < *different || *different < *sym_parent;
        REQUIRE(lt);
      }
    }
  }
}