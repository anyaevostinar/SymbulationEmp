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


TEST_CASE("Mutate", "[sgp]") {

  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;

  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.WORLD_WIDTH(2);
  config.WORLD_HEIGHT(2);
  config.SGP_MUT_PER_BIT_RATE(1.0);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");


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
  // clean up organisms since they aren't in the world
  uninfected_host.Delete();
  second_host.Delete();
}

TEST_CASE("No Mutate", "[sgp]") {

  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.WORLD_WIDTH(2);
  config.WORLD_HEIGHT(2);
  config.SGP_MUT_PER_BIT_RATE(0.0);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");


  world_t world(random, &config);
  world.Setup();
  world.Resize(2,2);

  auto& prog_builder = world.GetProgramBuilder();

  emp::Ptr<sgp_host_t> uninfected_host = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));
  emp::Ptr<sgp_host_t> second_host = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));

  REQUIRE(*uninfected_host == *second_host);


  THEN("Organism mutated with 0 mutation rate is not different") {
    for (int i =0; i<100; i++){
      uninfected_host->Mutate();
    }

    REQUIRE(*uninfected_host == *second_host);
  }
  // clean up organisms since they aren't in the world
  uninfected_host.Delete();
  second_host.Delete();
}

TEST_CASE("SGPHost destructor cleans up shared pointers and in-progress reproduction", "[sgp][sgp-unit]") {
    GIVEN("A host"){
        emp::Random random(31);
        sgpmode::SymConfigSGP config;
        config.WORLD_WIDTH(1);
        config.WORLD_HEIGHT(1);
        config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");

        world_t world(random, &config);
        world.Setup();
        auto& prog_builder = world.GetProgramBuilder();

        emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateNotProgram(100));

        emp::WorldPosition host_pos = emp::WorldPosition(1, 2);
        host->SetLocation(host_pos);
        const size_t queue_id = world.GetReproQueue().Enqueue(host->GetHardware().GetCPUState().GetOrgPtr(), host_pos);
        host->GetHardware().GetCPUState().MarkReproInProgress(queue_id);

        WHEN("The host is destroyed") {
            host.Delete();

            THEN("The host's position in the reproduction queue is marked as invalid") {
                REQUIRE(world.GetReproQueue().GetQueue()[queue_id].valid == false);
            }
        }
    }
}

TEST_CASE("Host == operators", "[sgp][sgp-unit]") {


    GIVEN("A host"){
        emp::Random random(31);
        sgpmode::SymConfigSGP config;
        config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");


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
            clone1.Delete();
            clone2.Delete();
        }

        WHEN("A host that is different to the original is created"){
             emp::Ptr<sgp_host_t> different = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateNotProgram(99)); // For comparing
             THEN("The host is not equal to the different host"){
                REQUIRE_FALSE(*host == *different);
            }
            different.Delete();
        }
        host.Delete();
  }
}

TEST_CASE("Host > & < operators", "[sgp][sgp-unit]") {
    GIVEN("two different hosts"){
        emp::Random random(31);
        sgpmode::SymConfigSGP config;
        config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
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
        host.Delete();
        different.Delete();
    }
}

TEST_CASE("MakeNew returns identical host", "[sgp][sgp-unit]"){
    GIVEN("A host"){
        emp::Random random(31);
        sgpmode::SymConfigSGP config;
        config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
        world_t world(random, &config);

        emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config);

        WHEN("A new host is created using the MakeNew function of the first host"){
            emp::Ptr<Organism> host_remade = host->MakeNew();
            THEN("MakeNew produces an identical host"){
                REQUIRE(*host_remade == *host);
            }
            host_remade.Delete();
        }
        host.Delete();

    }
}

TEST_CASE("SetReproCount & GetReproCount","[sgp][sgp-unit]"){
    GIVEN("An SGPWorld and a host"){
        emp::Random random(31);
        sgpmode::SymConfigSGP config;
        config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
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

TEST_CASE("ProcessOutputBuffer", "[sgp][sgp-unit]"){
    GIVEN("A host with valid values in its input and output buffers"){
        emp::Random random(31);
        sgpmode::SymConfigSGP config;
        config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/echo-task-env.json");
        world_t world(random, &config);
        world.Setup();
        auto& prog_builder = world.GetProgramBuilder();
        emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateNotProgram(100));
        world.AddOrgAt(host, 0);

        // save two of the inputs
        emp::vector<uint32_t> inputs;
        for (int i = 0; i < 2; i++) {
            inputs.push_back(host->GetHardware().GetCPUState().GetInputBuffer().read());
        }
        inputs.push_back(0); // add some incorrect output to make sure they aren't getting points for those
        inputs.push_back(1);
        host->GetHardware().GetCPUState().SetOutputs(inputs); // set output buffer to inputs with with some extra incorrect outputs
        WHEN("ProcessOutputBuffer is called"){
            host->ProcessOutputBuffer();
            THEN("The output buffer is cleared and host received 10 points for correctly echoing 2 inputs"){
                REQUIRE(host->GetHardware().GetCPUState().GetOutputBuffer().empty());
                REQUIRE(host->GetPoints() == 10);
            }
        }
    }
}