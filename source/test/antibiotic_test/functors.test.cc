#include "../test_utils.h"

#include "../../default_mode/SymWorld.h"
#include "../../default_mode/WorldSetup.cc"
#include "../../default_mode/DataNodes.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPWorld.cc"
#include "../../sgp_mode/SGPWorldSetup.cc"
#include "../../sgp_mode/SGPWorldData.cc"
#include "../../sgp_mode/SGPW_InteractionMechanismSetup.cc"
#include "../../sgp_mode/SGPW_TaskProfileSetup.cc"
#include "../../sgp_mode/ProgramBuilder.h"
#include "../../antibiotic-mode/Human.h"
#include "../../antibiotic-mode/AntibioticConfigSetup.h"

#include "emp/math/Random.hpp"

#include "../../native/symbulation_antibiotic.cc"

#include "../../catch/catch.hpp"

using world_t = sgpmode::SGPWorld;
using cpu_state_t = sgpmode::CPUState<world_t>;
using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
using program_t = typename world_t::sgp_prog_t;
using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;

TEST_CASE("Test symbionts take points for tasks from host", "[antibiotic]") {
  antibioticmode::AntibioticConfig config;
  config.CYCLES_PER_UPDATE(1);
  config.STRESS_TYPE("neutral");
  config.HOST_REPRO_RES(1100);
  config.SEED(61);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.FILE_PATH("Antibiotic_test_output");
  config.START_MOI(1);
  config.TASK_IO_UNIQUE_OUTPUT(true);
  config.TASK_IO_BANK_SIZE(10);
  config.WORLD_HEIGHT(1);
    config.WORLD_WIDTH(1);
  test_utils::SetWellMixed(config, 1, 1);

  emp::Random random(config.SEED());
  world_t world(random, &config);

  //
  world.Setup();
  REQUIRE(world.GetNumOrgs() == 1);
  emp::Ptr<world_t> human_micro = emp::Ptr<world_t>(&world);
  setup_functors(config, &world);

    auto& builder = world.GetProgramBuilder();

    //Creates a host that does Nand operations
    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, builder.CreateReproProgram(100));
    host->SetPoints(10);

    //Creates a symbiont that can not do any tasks
    emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config, builder.CreateNandProgram(50));

    //Adds host to world and sym to host.
    world.AddOrgAt(host, 0);
    host->AddSymbiont(symbiont);
    hardware_t& hw = symbiont->GetHardware();
    REQUIRE(symbiont->GetPoints() == 0);
    REQUIRE(host->GetPoints() == 10);
    WHEN("Sufficient updates to run tasks and reproduce") {
        for(int i=0; i<52; i++) {
            world.Update();
        }
        THEN("Symbiont has done a task, but doesn't have any points; host lost points for symbiont's task and reproduction") {
            REQUIRE(symbiont->GetPoints() == 0);
            REQUIRE(symbiont->GetHardware().GetCPUState().GetTaskPerformed(world.GetTaskEnv().GetTaskSet().GetID("NAND")) == true);
            REQUIRE(host->GetPoints() == 8);
        }
    
  }
}

TEST_CASE("Test symbionts save hosts from stress event", "[antibiotic]") {
  antibioticmode::AntibioticConfig config;
  config.CYCLES_PER_UPDATE(1);
  config.STRESS_TYPE("neutral");
  config.ENABLE_STRESS(true);
  config.STRESS_FREQUENCY(53);
  config.HOST_REPRO_RES(1100);
  config.BASE_DEATH_CHANCE(1.0);
  config.MUTUALIST_DEATH_CHANCE(0.0);
  config.SEED(61);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.FILE_PATH("Antibiotic_test_output");
  config.START_MOI(1);
  config.TASK_IO_UNIQUE_OUTPUT(true);
  config.TASK_IO_BANK_SIZE(10);
  config.WORLD_HEIGHT(1);
    config.WORLD_WIDTH(1);
  test_utils::SetWellMixed(config, 1, 1);

  emp::Random random(config.SEED());
  world_t world(random, &config);

  world.Setup();
  REQUIRE(world.GetNumOrgs() == 1);
  emp::Ptr<world_t> human_micro = emp::Ptr<world_t>(&world);
  setup_functors(config, &world);

  auto& builder = world.GetProgramBuilder();

  //Creates a host that does repro operations
  emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, builder.CreateReproProgram(100));
  host->SetPoints(10);

  WHEN("Plasmid that can do NAND and stress event occurs") {
      //Creates a symbiont that can do Nand task
      emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config, builder.CreateNandProgram(50));

      //Adds host to world and sym to host.
      world.AddOrgAt(host, 0);
      host->AddSymbiont(symbiont);
      hardware_t& hw = symbiont->GetHardware();
      for(int i=0; i<55; i++) {
          world.Update();
      }

      REQUIRE(symbiont->GetHardware().GetCPUState().GetTaskPerformed(world.GetTaskEnv().GetTaskSet().GetID("NAND")) == true);
      REQUIRE(host->GetPoints() == 8);
      THEN("Host survives stress event because symbiont did a task") {
          REQUIRE(world.GetNumOrgs() == 1);
      }
  } 
  WHEN("Plasmid that cannot do NAND and stress event occurs") {
      //Creates a symbiont that can not do any tasks
      emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config, builder.CreateReproProgram(50));

      //Adds host to world and sym to host.
      world.AddOrgAt(host, 0);
      host->AddSymbiont(symbiont);
      hardware_t& hw = symbiont->GetHardware();
      for(int i=0; i<55; i++) {
          world.Update();
      }

      THEN("Host dies from stress event because symbiont did not do a task") {
          REQUIRE(world.GetNumOrgs() == 0);
      }
  }
    
}