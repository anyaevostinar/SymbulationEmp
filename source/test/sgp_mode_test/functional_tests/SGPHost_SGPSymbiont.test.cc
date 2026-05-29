#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"


/**
 * This file is dedicated to interactions between SGPHosts and SGPSymbionts
 */

using world_t = sgpmode::SGPWorld;
using cpu_state_t = sgpmode::CPUState<world_t>;
using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
using program_t = typename world_t::sgp_prog_t;
using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;

TEST_CASE("Host Process allows symbionts to process", "[sgp][sgp-functional][test]") {
  GIVEN("An SGPWorld with a host infected with a symbiont"){
    emp::Random random(42);
    sgpmode::SymConfigSGP config;
    config.CYCLES_PER_UPDATE(8);
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    world_t world(random, &config);
    world.Setup();
    auto& builder = world.GetProgramBuilder();


    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, builder.CreateNotProgram(100));
    emp::Ptr<sgp_sym_t> sym = emp::NewPtr<sgp_sym_t>(&random, &world, &config, builder.CreateNotProgram(100));

    world.AddOrgAt(host, 0);
    host->AddSymbiont(sym);



    WHEN("Host Process is called") {
      host->Process(0);

      THEN("Symbiont should have processed and aged by 1 and executed the base cpu cycles") {
        REQUIRE(sym->GetHardware().GetCPUState().GetCPUCyclesSinceRepro() == 8);
        REQUIRE(sym->GetAge() == 1);
      }
    }
  }
}