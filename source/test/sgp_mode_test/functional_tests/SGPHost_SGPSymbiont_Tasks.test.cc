#include "emp/math/Random.hpp"

#include "../../../sgp_mode/hardware/SGPHardware.h"
#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorld.cc"
#include "../../../sgp_mode/SGPWorldSetup.cc"
#include "../../../sgp_mode/SGPW_InteractionMechanismSetup.cc"
#include "../../../sgp_mode/SGPW_TaskProfileSetup.cc"

#include "../../../catch/catch.hpp"

/**
 * This file is dedicated to checking the combination of only first task credit between hosts and symbionts
 */


using world_t = sgpmode::SGPWorld;
using cpu_state_t = sgpmode::CPUState<world_t>;
using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;

TEST_CASE("Only first task credit for hosts vs. symbionts","[sgp]"){
  GIVEN("A host and symbiont can theoretically do both NOT and NAND tasks"){
    emp::Random random(1);
    sgpmode::SymConfigSGP config;
    config.SEED(1);
    config.MUTATION_RATE(0.0);
    config.MUTATION_SIZE(0.002);
    config.TASK_PROFILE_MODE("self-all");
    config.VT_TASK_MATCH(1);
    config.INIT_POP_SIZE(1);
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.CYCLES_PER_UPDATE(52);

    world_t world(random, &config);
    world.Setup();
    auto& builder = world.GetProgramBuilder();

    //Creates a host that does both Not and Nand operations
    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, builder.CreateNotNandProgram(50));

    //Creates a symbiont that can not do any tasks
    emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config, builder.CreateNotNandProgram(50));

    //Adds host to world and sym to host.
    world.AddOrgAt(host, 0);
    host->AddSymbiont(symbiont);

    // get task ids
    const size_t not_task_id = world.GetTaskEnv().GetTaskSet().GetID("NOT");
    const size_t nand_task_id = world.GetTaskEnv().GetTaskSet().GetID("NAND");

    WHEN("Only first task credit for hosts is on"){
      config.HOST_ONLY_FIRST_TASK_CREDIT(1);
      WHEN("Only first task credit for symbionts is off"){
        config.SYM_ONLY_FIRST_TASK_CREDIT(0);
        world.Update();
        THEN("The host only gets credit for NOT"){
          REQUIRE(host->GetHardware().GetCPUState().GetTaskPerformed(not_task_id) == true);
          REQUIRE(host->GetHardware().GetCPUState().GetTaskPerformed(nand_task_id) == false);
          REQUIRE(host->GetHardware().GetCPUState().GetTasksPerformed().CountOnes() == 1);
          REQUIRE(host->GetPoints() == 5);
        }
        THEN("The symbiont gets credit for both NOT and NAND"){
          REQUIRE(symbiont->GetHardware().GetCPUState().GetTaskPerformed(not_task_id) == true);
          REQUIRE(symbiont->GetHardware().GetCPUState().GetTaskPerformed(nand_task_id) == true);
          REQUIRE(symbiont->GetHardware().GetCPUState().GetTasksPerformed().CountOnes() == 2);
          REQUIRE(symbiont->GetPoints() == 10);
        }
      }

      WHEN("Only first task credit for symbionts is on"){
        config.SYM_ONLY_FIRST_TASK_CREDIT(1);
        world.Update();
        THEN("The host only gets credit for NOT"){
          REQUIRE(host->GetHardware().GetCPUState().GetTaskPerformed(not_task_id) == true);
          REQUIRE(host->GetHardware().GetCPUState().GetTaskPerformed(nand_task_id) == false);
          REQUIRE(host->GetHardware().GetCPUState().GetTasksPerformed().CountOnes() == 1);
          REQUIRE(host->GetPoints() == 5);
        }
        THEN("The symbiont only gets credit for NOT"){
          REQUIRE(symbiont->GetHardware().GetCPUState().GetTaskPerformed(not_task_id) == true);
          REQUIRE(symbiont->GetHardware().GetCPUState().GetTaskPerformed(nand_task_id) == false);
          REQUIRE(symbiont->GetHardware().GetCPUState().GetTasksPerformed().CountOnes() == 1);
          REQUIRE(symbiont->GetPoints() == 5);
        }
      }

    }

    WHEN("Only first task credit for hosts is off"){
      config.HOST_ONLY_FIRST_TASK_CREDIT(0);
      WHEN("Only first task credit for symbionts is on"){
        config.SYM_ONLY_FIRST_TASK_CREDIT(1);
        world.Update();
        THEN("The host gets credit for both NOT and NAND"){
          REQUIRE(host->GetHardware().GetCPUState().GetTaskPerformed(not_task_id) == true);
          REQUIRE(host->GetHardware().GetCPUState().GetTaskPerformed(nand_task_id) == true);
          REQUIRE(host->GetHardware().GetCPUState().GetTasksPerformed().CountOnes() == 2);
          REQUIRE(host->GetPoints() == 10);
        }
        THEN("The symbiont only gets credit for NOT"){
          REQUIRE(symbiont->GetHardware().GetCPUState().GetTaskPerformed(not_task_id) == true);
          REQUIRE(symbiont->GetHardware().GetCPUState().GetTaskPerformed(nand_task_id) == false);
          REQUIRE(symbiont->GetHardware().GetCPUState().GetTasksPerformed().CountOnes() == 1);
          REQUIRE(symbiont->GetPoints() == 5);
        }
      }

      WHEN("Only first task credit for symbionts is off"){
        config.SYM_ONLY_FIRST_TASK_CREDIT(0);
        world.Update();
        THEN("The host gets credit for both NOT and NAND"){
          REQUIRE(host->GetHardware().GetCPUState().GetTaskPerformed(not_task_id) == true);
          REQUIRE(host->GetHardware().GetCPUState().GetTaskPerformed(nand_task_id) == true);
          REQUIRE(host->GetHardware().GetCPUState().GetTasksPerformed().CountOnes() == 2);
          REQUIRE(host->GetPoints() == 10);
        }
        THEN("The symbiont gets credit for both NOT and NAND"){
          REQUIRE(symbiont->GetHardware().GetCPUState().GetTaskPerformed(not_task_id) == true);
          REQUIRE(symbiont->GetHardware().GetCPUState().GetTaskPerformed(nand_task_id) == true);
          REQUIRE(symbiont->GetHardware().GetCPUState().GetTasksPerformed().CountOnes() == 2);
          REQUIRE(symbiont->GetPoints() == 10);
        }
      }
    }
  }
}