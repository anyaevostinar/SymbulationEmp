#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"
#include "../../../sgp_mode/ProgramBuilder.h"

#include "../../../catch/catch.hpp"

/**
 * This file is dedicated to unit tests for SGPHost
 */

using world_t = sgpmode::SGPWorld;
using cpu_state_t = sgpmode::CPUState<world_t>;
using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;


TEST_CASE("Mutate", "[refactor]") {

  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;

  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.GRID_X(2);
  config.GRID_Y(2);
  config.SGP_MUT_PER_BIT_RATE(1.0);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/functional_tests/hardware-test-env.json");


  world_t world(random, &config);
  world.Setup();
  world.Resize(2,2);

  auto& prog_builder = world.GetProgramBuilder();

  emp::Ptr<sgp_host_t> uninfected_host = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));
  emp::Ptr<sgp_host_t> second_host = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));

  REQUIRE(*uninfected_host == *second_host);


  THEN("Mutated organism is different") {
    for (int i =0; i<100; i++){
      uninfected_host->Mutate();
    }
    
    REQUIRE(*uninfected_host != *second_host);
  }

}

// TEST_CASE("SGPHost destructor cleans up shared pointers and in-progress reproduction", "[sgp][sgp-unit][refactor]") {
//     //TODO test for organism correctly invalidating its place in repro queue
// }


TEST_CASE("Host == operators", "[sgp][sgp-unit][refactor]") {


    GIVEN("A host"){
        emp::Random random(31);
        sgpmode::SymConfigSGP config;
        config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/functional_tests/hardware-test-env.json");


        world_t world(random, &config);
        auto& prog_builder = world.GetProgramBuilder();
        
        emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateNotProgram(100));
        WHEN("2 clones of the original host are created"){
            emp::Ptr<sgp_host_t> clone1 = emp::NewPtr<sgp_host_t>(*host);
            emp::Ptr<sgp_host_t> clone2 = emp::NewPtr<sgp_host_t>(*host);

            THEN("host is equal to first clone"){
                REQUIRE(*host == *clone1);
            }
            THEN("The first clone is equal to the second clone"){
                REQUIRE(*clone1 == *clone2);
            }
        }
        
        WHEN("A host that is different to the original is created"){
             emp::Ptr<sgp_host_t> different = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateNotProgram(99)); // For comparing
             THEN("The host is not equal to the different host"){
                REQUIRE_FALSE(*host == *different);
            }
        }
  }
}

TEST_CASE("Host > & < operators", "[sgp][sgp-unit][refactor]") {
    GIVEN("two different hosts"){
        emp::Random random(31);
        sgpmode::SymConfigSGP config;
        config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/functional_tests/hardware-test-env.json");
        world_t world(random, &config);
        auto& prog_builder = world.GetProgramBuilder();
        
        emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateNotProgram(100));
        emp::Ptr<sgp_host_t> different = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateNotProgram(99)); // For comparing
        WHEN("The two hosts are compared"){
            THEN("One host is less then the other host"){
                // Can't assert true/false without knowing bitcode ordering,
                // assert that bitcode ordering is well-defined
                bool lt = *host < *different || *different < *host;
                REQUIRE(lt);      
            }
        }
    }
}


TEST_CASE("MakeNew returns identical host", "[sgp][sgp-unit][refactor]"){
    GIVEN("A host"){
        emp::Random random(31);
        sgpmode::SymConfigSGP config;
        config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/functional_tests/hardware-test-env.json");
        world_t world(random, &config);
        auto& prog_builder = world.GetProgramBuilder();
        
        emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config);

        WHEN("A new host is created using the MakeNew function of the first host"){
            emp::Ptr<Organism> host_remade = host->MakeNew();
            THEN("MakeNew produces an identical host"){
                REQUIRE(*host_remade == *host);
            }
        }
    }
}

TEST_CASE("SetReproCount & GetReproCount","[sgp][sgp-unit][refactor]"){
    GIVEN("An SGPWorld and a host"){
        emp::Random random(31);
        sgpmode::SymConfigSGP config;
        config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/functional_tests/hardware-test-env.json");
        world_t world(random, &config);
        auto& prog_builder = world.GetProgramBuilder();
        emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateNotProgram(100));
        WHEN("The host is added to the world"){
            world.AddOrgAt(host, 0);
            
            THEN("Repro count of the host is 0"){
                REQUIRE(host->GetReproCount() == 0);
            }
            WHEN("The host's repro count is increased by 1"){
                host->SetReproCount(1);
                THEN("Repro count of the host is 1"){
                    REQUIRE(host->GetReproCount() == 1);
                }
            }
        }
    }
}
