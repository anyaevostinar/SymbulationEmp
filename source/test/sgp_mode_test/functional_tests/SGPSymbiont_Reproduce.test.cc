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
 * This file is for testing the various aspects of symbiont reproduction,
 * including horizontal and vertical transmission (not just when Reproduce instruction is called).
 */
using world_t = sgpmode::SGPWorld;
using cpu_state_t = sgpmode::CPUState<world_t>;
using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
using program_t = typename world_t::sgp_prog_t;
using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;

TEST_CASE("SGPSymbiont Reproduce", "[sgp][sgp-functional]") {
  GIVEN("An SGPWorld with a symbiont only able to perform NOT"){
    emp::Random random(31);
    size_t not_id = 1;
    sgpmode::SymConfigSGP config;
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.TASK_PROFILE_MODE("parent-all");
    config.TASK_IO_BANK_SIZE(10);
    test_utils::SetWellMixed(config, 1, 1);
    world_t world(random, &config);
    world.Setup();
    auto& prog_builder = world.GetProgramBuilder();
    emp::Ptr<sgp_sym_t> sym_parent = emp::NewPtr<sgp_sym_t>(&random, &world, &config, prog_builder.CreateNotProgram(100));
    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateNotProgram(100));

    world.AddOrgAt(host, 0);
    host->AddSymbiont(sym_parent);

    WHEN("The Symbiont reproduces"){

      THEN("Symbiont child increases its lineage reproduction count") {
        emp::Ptr<sgp_sym_t> sym_baby = (sym_parent->Reproduce()).DynamicCast<sgp_sym_t>();
        REQUIRE(sym_parent->GetReproCount() == sym_baby->GetReproCount() - 1);
        sym_baby.Delete();
      }

      WHEN("Parental task tracking is set to parent-all and parent does NOT before reproduction") {

        for (int i = 0; i < 50; i++) {
          world.Update();
        }

        emp::Ptr<sgp_sym_t> sym_baby = (sym_parent->Reproduce()).DynamicCast<sgp_sym_t>();
        world.AssignNewEnvIO(sym_baby->GetHardware().GetCPUState());

        THEN("All tasks are tracked correctly for three generations") {

          REQUIRE(sym_parent->GetHardware().GetCPUState().GetParentTasksPerformed().None());

          REQUIRE(sym_parent->GetHardware().GetCPUState().GetTasksPerformed().Get(not_id));
          REQUIRE(sym_parent->GetHardware().GetCPUState().GetTasksPerformed().CountOnes() == 1);

          REQUIRE(sym_baby->GetHardware().GetCPUState().GetParentTasksPerformed().Get(not_id));
          REQUIRE(sym_baby->GetHardware().GetCPUState().GetParentTasksPerformed().CountOnes() == 1);
        }

        THEN("The symbiont child tracks any gains or loses in task completions") {
          // in this second generation, we expect the symbiont to gain the NOT task
          // since first-gen organisms are marked as having parents who have completed no tasks
          REQUIRE(sym_baby->GetHardware().GetCPUState().GetLineageTaskLossCount(not_id) == 0);
          REQUIRE(sym_baby->GetHardware().GetCPUState().GetLineageTaskGainCount(not_id) == 1);
          size_t num_tasks = sym_baby->GetHardware().GetCPUState().GetLineageTaskGain().size();
          for (size_t i=0; i<num_tasks; i++) {
            if (i != not_id) {
              REQUIRE(sym_baby->GetHardware().GetCPUState().GetLineageTaskGainCount(i) == 0);
              REQUIRE(sym_baby->GetHardware().GetCPUState().GetLineageTaskLossCount(i) == 0);
            }
          }
        }

        THEN("The symbiont child tracks how its tasks compare to its parent's partner's tasks") {
          REQUIRE(sym_baby->GetHardware().GetCPUState().GetLineageTaskConvergeToPartner(not_id) == 0);
          REQUIRE(sym_baby->GetHardware().GetCPUState().GetLineageTaskDivergeFromPartner(not_id) == 1);

          size_t num_tasks = sym_baby->GetHardware().GetCPUState().GetLineageTaskGain().size();
          for (size_t i=0; i<num_tasks; i++) {
            if (i != not_id) {
              REQUIRE(sym_baby->GetHardware().GetCPUState().GetLineageTaskGainCount(i) == 0);
              REQUIRE(sym_baby->GetHardware().GetCPUState().GetLineageTaskLossCount(i) == 0);
            }
          }
        }
        sym_baby.Delete();
      }
    }
  }
}

TEST_CASE("SGPSymbiont Vertical Transmission", "[sgp][sgp-functional]"){
  //Note: Catch automatically reruns everything from the top of the test case for each GIVEN and WHEN, so the world/orgs get reset
  emp::Random random(51);
  sgpmode::SymConfigSGP config;
  config.HOST_REPRO_RES(0);
  config.SYM_VERT_TRANS_RES(0);
  config.HORIZ_TRANS(0);
  config.DATA_INT(26);
  config.VERTICAL_TRANSMISSION(1);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.TASK_IO_BANK_SIZE(10);
  test_utils::SetWellMixed(config, 100, 0);
  GIVEN("A host infected with a symbiont in the world when self-all task mode and task match required for vt"){
    config.TASK_PROFILE_MODE("self-all");
    config.VT_TASK_MATCH(true);
    world_t world(random, &config);
    world.Setup();
    auto& prog_builder = world.GetProgramBuilder();

    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));
    emp::Ptr<sgp_sym_t> sym = emp::NewPtr<sgp_sym_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));

    world.AddOrgAt(host, 0);
    host->AddSymbiont(sym);
    REQUIRE(world.GetNumOrgs() == 1);
    WHEN("Host and Symbiont share a matching task and host reproduces"){
      sym->GetHardware().GetCPUState().MarkTaskPerformed(0);
      host->GetHardware().GetCPUState().MarkTaskPerformed(0);
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont vertically transmits"){
        REQUIRE(world.GetVerticalTransmissionAttemptCount().GetCount() == 1);
        REQUIRE(world.GetVerticalTransmissionSuccessCount().GetCount() == 1);
      }
    }
    WHEN("Host and Symbiont share a matching parent task, but not self task, and host reproduces"){
      sym->GetHardware().GetCPUState().SetParentTaskPerformed(0);
      host->GetHardware().GetCPUState().SetParentTaskPerformed(0);
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont tries but fails to vertically transmit"){
        REQUIRE(world.GetVerticalTransmissionAttemptCount().GetCount() == 1);
        REQUIRE(world.GetVerticalTransmissionSuccessCount().GetCount() == 0);
      }
    }

    WHEN("Host and Symbiont do not share a matching task and host reproduces"){
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont tries, but does not vertically transmit"){
        REQUIRE(world.GetVerticalTransmissionAttemptCount().GetCount() == 1);
        REQUIRE(world.GetVerticalTransmissionSuccessCount().GetCount() == 0);
      }
    }
  }

  GIVEN("A host infected with a symbiont in the world when parent-all task mode and task match required for vt"){
    config.TASK_PROFILE_MODE("parent-all");
    config.VT_TASK_MATCH(true);
    world_t world(random, &config);
    world.Setup();
    auto& prog_builder = world.GetProgramBuilder();

    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));
    emp::Ptr<sgp_sym_t> sym = emp::NewPtr<sgp_sym_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));

    world.AddOrgAt(host, 0);
    host->AddSymbiont(sym);
    REQUIRE(world.GetNumOrgs() == 1);
    WHEN("Host and Symbiont share a matching task and host reproduces"){
      sym->GetHardware().GetCPUState().MarkTaskPerformed(0);
      host->GetHardware().GetCPUState().MarkTaskPerformed(0);
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont tries, but fails to, vertically transmit"){
        REQUIRE(world.GetVerticalTransmissionAttemptCount().GetCount() == 1);
        REQUIRE(world.GetVerticalTransmissionSuccessCount().GetCount() == 0);
      }
    }
    WHEN("Host and Symbiont share a matching parent task, but not self task, and host reproduces"){
      sym->GetHardware().GetCPUState().SetParentTaskPerformed(0);
      host->GetHardware().GetCPUState().SetParentTaskPerformed(0);
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont vertically transmits"){
        REQUIRE(world.GetVerticalTransmissionAttemptCount().GetCount() == 1);
        REQUIRE(world.GetVerticalTransmissionSuccessCount().GetCount() == 1);
      }
    }

    WHEN("Host and Symbiont do not share a matching task and host reproduces"){
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont tries, but does not vertically transmit"){
        REQUIRE(world.GetVerticalTransmissionAttemptCount().GetCount() == 1);
        REQUIRE(world.GetVerticalTransmissionSuccessCount().GetCount() == 0);
      }
    }
  }

  GIVEN("A host infected with a symbiont in the world when VT task match is off"){
    config.TASK_PROFILE_MODE("self-all");
    config.VT_TASK_MATCH(false);
    world_t world(random, &config);
    world.Setup();
    auto& prog_builder = world.GetProgramBuilder();

    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));
    emp::Ptr<sgp_sym_t> sym = emp::NewPtr<sgp_sym_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));

    world.AddOrgAt(host, 0);
    host->AddSymbiont(sym);
    REQUIRE(world.GetNumOrgs() == 1);
    WHEN("Host and Symbiont share a matching task and host reproduces"){
      sym->GetHardware().GetCPUState().MarkTaskPerformed(0);
      host->GetHardware().GetCPUState().MarkTaskPerformed(0);
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont vertically transmits"){
        REQUIRE(world.GetVerticalTransmissionAttemptCount().GetCount() == 1);
        REQUIRE(world.GetVerticalTransmissionSuccessCount().GetCount() == 1);
      }
    }

    WHEN("Host and Symbiont do not share a matching task and host reproduces"){
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont vertically transmits"){
        REQUIRE(world.GetVerticalTransmissionAttemptCount().GetCount() == 1);
        REQUIRE(world.GetVerticalTransmissionSuccessCount().GetCount() == 1);
      }
    }
  }

}

TEST_CASE("SGPSymbiont Vertical Transmission off", "[sgp][sgp-functional]"){
  //Note: Catch automatically reruns everything from the top of the test case for each WHEN, so the world/orgs get reset
  emp::Random random(51);
  sgpmode::SymConfigSGP config;
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.DATA_INT(26);
  config.HOST_REPRO_RES(0);
  config.SYM_VERT_TRANS_RES(0);
  config.HORIZ_TRANS(0);
  config.VERTICAL_TRANSMISSION(0);
  config.TASK_IO_BANK_SIZE(10);
  test_utils::SetWellMixed(config, 100, 0);
  GIVEN("A host infected with a symbiont in the world"){
    world_t world(random, &config);
    world.Setup();
    auto& prog_builder = world.GetProgramBuilder();

    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));
    emp::Ptr<sgp_sym_t> sym = emp::NewPtr<sgp_sym_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));

    world.AddOrgAt(host, 0);
    host->AddSymbiont(sym);
    REQUIRE(world.GetNumOrgs() == 1);
    WHEN("Host and Symbiont share a matching task and matching parent task and host reproduces"){
      sym->GetHardware().GetCPUState().MarkTaskPerformed(0);
      host->GetHardware().GetCPUState().MarkTaskPerformed(0);
      sym->GetHardware().GetCPUState().SetParentTaskPerformed(0);
      host->GetHardware().GetCPUState().SetParentTaskPerformed(0);
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont does not vertically transmit"){
        REQUIRE(world.GetVerticalTransmissionAttemptCount().GetCount() == 0);
        REQUIRE(world.GetVerticalTransmissionSuccessCount().GetCount() == 0);
      }
    }
  }
}


TEST_CASE("SGPSymbiont Horizontal Transmission", "[sgp][sgp-functional]"){
  emp::Random random(42);
  sgpmode::SymConfigSGP config;
  config.TASK_IO_BANK_SIZE(10);
  test_utils::SetWellMixed(config, 4, 0);
  config.HOST_REPRO_RES(100);
  config.SYM_HORIZ_TRANS_RES(0);
  config.VERTICAL_TRANSMISSION(0);
  config.TASK_PROFILE_COMPATIBILITY_MODE("task-any-match");
  config.HORIZONTAL_TRANSMISSION_COMPATIBILITY_MODE("task-profile-compatible");
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.OUSTING(true); // This way, if the offspring happens to try to infect parent's host, the tests still pass because successful horizontal transmission.

  GIVEN("Two hosts one of which is infected with a symbiont, horiz task match on and self-all task profile mode"){
    config.HORIZ_TRANS(1);

    config.TASK_PROFILE_MODE("self-all");

    world_t world(random, &config);
    world.Setup();
    auto& prog_builder = world.GetProgramBuilder();

    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));
    emp::Ptr<sgp_host_t> host_uninfected = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));
    emp::Ptr<sgp_sym_t> sym = emp::NewPtr<sgp_sym_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));

    world.AddOrgAt(host, 0);
    world.AddOrgAt(host_uninfected,1);
    host->AddSymbiont(sym);

    REQUIRE(world.GetNumOrgs() == 2);

    WHEN("Only Uninfected Host and Symbiont share a matching task and symbiont reproduces"){
      sym->GetHardware().GetCPUState().MarkTaskPerformed(0);
      host_uninfected->GetHardware().GetCPUState().MarkTaskPerformed(0);
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont horizontally transmits"){
        REQUIRE(sym->GetHardware().GetCPUState().GetTaskPerformed(0) == true);
        REQUIRE(host_uninfected->GetHardware().GetCPUState().GetTaskPerformed(0) == true);
        REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetCount() == 1);
        REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetCount() == 1);
      }

    }
    WHEN("Only Uninfected Host parent and Symbiont parent share a matching task and symbiont reproduces"){
      sym->GetHardware().GetCPUState().SetParentTaskPerformed(0);
      host_uninfected->GetHardware().GetCPUState().SetParentTaskPerformed(0);
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont does not horizontally transmit"){
        REQUIRE(sym->GetHardware().GetCPUState().GetParentTaskPerformed(0) == true);
        REQUIRE(host_uninfected->GetHardware().GetCPUState().GetParentTaskPerformed(0) == true);
        REQUIRE(sym->GetHardware().GetCPUState().GetTaskPerformed(0) == false);
        REQUIRE(host_uninfected->GetHardware().GetCPUState().GetTaskPerformed(0) == false);
        REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetCount() == 1);
        REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetCount() == 0);
      }
    }

    WHEN("Host and Symbiont do not share a matching task and symbiont reproduces"){
      sym->GetHardware().GetCPUState().MarkTaskPerformed(0);
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont does not horizontally transmit"){
        REQUIRE(sym->GetHardware().GetCPUState().GetParentTaskPerformed(0) == false);
        REQUIRE(host_uninfected->GetHardware().GetCPUState().GetParentTaskPerformed(0) == false);
        REQUIRE(sym->GetHardware().GetCPUState().GetTaskPerformed(0) == true);
        REQUIRE(host_uninfected->GetHardware().GetCPUState().GetTaskPerformed(0) == false);
        REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetCount() == 1);
        REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetCount() == 0);
      }
    }
  }

  GIVEN("Two hosts one of which is infected with a symbiont, horiz task match on and parent-all task profile mode"){
    config.HORIZ_TRANS(1);
    config.TASK_PROFILE_MODE("parent-all");

    world_t world(random, &config);
    world.Setup();
    auto& prog_builder = world.GetProgramBuilder();

    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));
    emp::Ptr<sgp_host_t> host_uninfected = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));
    emp::Ptr<sgp_sym_t> sym = emp::NewPtr<sgp_sym_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));

    world.AddOrgAt(host, 0);
    world.AddOrgAt(host_uninfected,1);
    host->AddSymbiont(sym);

    REQUIRE(world.GetNumOrgs() == 2);

    WHEN("Only Uninfected Host and Symbiont share a matching task and symbiont reproduces"){
      sym->GetHardware().GetCPUState().MarkTaskPerformed(0);
      host_uninfected->GetHardware().GetCPUState().MarkTaskPerformed(0);
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont tries, but fails to, horizontally transmit"){
        REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetCount() == 1);
        REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetCount() == 0);
      }

    }
    WHEN("Only Uninfected Host parent and Symbiont parent share a matching task and symbiont reproduces"){
      sym->GetHardware().GetCPUState().SetParentTaskPerformed(0);
      host_uninfected->GetHardware().GetCPUState().SetParentTaskPerformed(0);
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont horizontally transmits"){
        REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetCount() == 1);
        REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetCount() == 1);
      }
    }

    WHEN("Host and Symbiont do not share a matching task and symbiont reproduces"){
      sym->GetHardware().GetCPUState().MarkTaskPerformed(0);
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont does not horizontally transmit"){
        REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetCount() == 1);
        REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetCount() == 0);
      }
    }
  }

  GIVEN("Two hosts one of which is infected with a symbiont, horiz task match off"){
    config.HORIZ_TRANS(1);
    config.HORIZONTAL_TRANSMISSION_COMPATIBILITY_MODE("always");

    world_t world(random, &config);
    world.Setup();
    auto& prog_builder = world.GetProgramBuilder();

    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));
    emp::Ptr<sgp_host_t> host_uninfected = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));
    emp::Ptr<sgp_sym_t> sym = emp::NewPtr<sgp_sym_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));

    world.AddOrgAt(host, 0);
    world.AddOrgAt(host_uninfected,1);
    host->AddSymbiont(sym);

    REQUIRE(world.GetNumOrgs() == 2);

    WHEN("Only Uninfected Host and Symbiont share a matching task and symbiont reproduces"){
      sym->GetHardware().GetCPUState().MarkTaskPerformed(0);
      host_uninfected->GetHardware().GetCPUState().MarkTaskPerformed(0);
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont horizontally transmits"){
        REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetCount() == 1);
        REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetCount() == 1);
      }

    }
    WHEN("Only Uninfected Host parent and Symbiont parent share a matching task and symbiont reproduces"){
      sym->GetHardware().GetCPUState().SetParentTaskPerformed(0);
      host_uninfected->GetHardware().GetCPUState().SetParentTaskPerformed(0);
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont horizontally transmits"){
        REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetCount() == 1);
        REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetCount() == 1);
      }
    }

    WHEN("Host and Symbiont do not share a matching task and symbiont reproduces"){
      sym->GetHardware().GetCPUState().MarkTaskPerformed(0);
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont horizontally transmits"){
        REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetCount() == 1);
        REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetCount() == 1);
      }
    }
  }

  GIVEN("Two hosts one of which is infected with a symbiont, horizontal transmission is off"){
    config.HORIZ_TRANS(0);

    world_t world(random, &config);
    world.Setup();
    auto& prog_builder = world.GetProgramBuilder();

    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));
    emp::Ptr<sgp_host_t> host_uninfected = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));
    emp::Ptr<sgp_sym_t> sym = emp::NewPtr<sgp_sym_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));

    world.AddOrgAt(host, 0);
    world.AddOrgAt(host_uninfected,1);
    host->AddSymbiont(sym);

    REQUIRE(world.GetNumOrgs() == 2);

    WHEN("Only Uninfected Host and Symbiont share a matching task and symbiont 'reproduces'"){
      sym->GetHardware().GetCPUState().MarkTaskPerformed(0);
      host_uninfected->GetHardware().GetCPUState().MarkTaskPerformed(0);
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont fails to attempt to horizontally transmit"){
        REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetCount() == 0);
        REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetCount() == 0);
      }

    }
    WHEN("Only Uninfected Host parent and Symbiont parent share a matching task and symbiont reproduces"){
      sym->GetHardware().GetCPUState().SetParentTaskPerformed(0);
      host_uninfected->GetHardware().GetCPUState().SetParentTaskPerformed(0);
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont fails to attempt to horizontally transmit"){
        REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetCount() == 0);
        REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetCount() == 0);
      }
    }

    WHEN("Host and Symbiont do not share a matching task and symbiont reproduces"){
      sym->GetHardware().GetCPUState().MarkTaskPerformed(0);
      for(int i = 0; i < 26; i++){
        world.Update();
      }
      THEN("Symbiont fails to attempt to horizontally transmit"){
        REQUIRE(world.GetHorizontalTransmissionAttemptCount().GetCount() == 0);
        REQUIRE(world.GetHorizontalTransmissionSuccessCount().GetCount() == 0);
      }
    }
  }
}

// TODO : resolve overlap with SGPSymbiont Reproduce. Maybe delete?
TEST_CASE("SGPSymbiont Reproduce tracks lineage task information", "[sgp][sgp-functional]") {
  sgpmode::SymConfigSGP config;
  config.CYCLES_PER_UPDATE(4);
  config.HOST_REPRO_RES(1);
  config.SEED(61);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.FILE_PATH("SGPSymbiont_test_output");
  config.START_MOI(1); // Initialize host with a symbiont
  config.TASK_IO_UNIQUE_OUTPUT(true);
  config.VERTICAL_TRANSMISSION(0); // No vertical transmission for this test
  config.TASK_IO_BANK_SIZE(10);
  test_utils::SetWellMixed(config, 1, 1);
  // Zero out mutation rates
  config.MUTATION_RATE(0);
  config.MUTATION_SIZE(0);
  config.SGP_MUT_PER_BIT_RATE(0);

  // Initialize world with one host
  emp::Random random(config.SEED());
  world_t world(random, &config);
  world.Setup();

  // Assert that the world environment is as expected
  REQUIRE(world.GetTaskEnv().GetTaskSet().HasTask("NOT"));
  REQUIRE(world.GetTaskEnv().GetTaskSet().HasTask("NAND"));
  REQUIRE(world.GetTaskEnv().GetTaskSet().HasTask("OR_NOT"));
  REQUIRE(world.GetTaskEnv().GetTaskSet().HasTask("AND"));
  REQUIRE(world.GetTaskEnv().GetTaskSet().HasTask("OR"));
  REQUIRE(world.GetTaskEnv().GetTaskSet().HasTask("AND_NOT"));
  REQUIRE(world.GetTaskEnv().GetTaskSet().HasTask("NOR"));
  REQUIRE(world.GetTaskEnv().GetTaskSet().HasTask("XOR"));
  REQUIRE(world.GetTaskEnv().GetTaskSet().HasTask("EQU"));
  const size_t num_tasks = world.GetTaskEnv().GetTaskSet().GetSize();
  const size_t not_task_id = world.GetTaskEnv().GetTaskSet().GetID("NOT");
  const size_t nand_task_id = world.GetTaskEnv().GetTaskSet().GetID("NAND");
  const size_t equ_task_id = world.GetTaskEnv().GetTaskSet().GetID("EQU");
  const size_t xor_task_id = world.GetTaskEnv().GetTaskSet().GetID("XOR");

  // Get host with the symbiont we'll use for testing
  REQUIRE(world.IsOccupied(0));
  auto& host_org = world.GetOrg(0);
  sgp_host_t& sgp_host = static_cast<sgp_host_t&>(host_org);
  REQUIRE(sgp_host.IsHost());
  REQUIRE(sgp_host.HasSym());
  // Get host's symbiont
  auto& sym_org = *(sgp_host.GetSymbionts()[0]);
  sgp_sym_t& sgp_sym = static_cast<sgp_sym_t&>(sym_org);
  hardware_t& sym_hw = sgp_sym.GetHardware();
  REQUIRE(sym_hw.GetCPUState().HasHost());
  // For purpose of tests, reset all task performance / parent task performance to 0
  for (size_t task_i = 0; task_i < num_tasks; ++task_i) {
    sym_hw.GetCPUState().SetParentTaskPerformed(task_i, false);
    sym_hw.GetCPUState().ResetTaskPerformance(task_i);
    sgp_host.GetHardware().GetCPUState().SetParentTaskPerformed(task_i, false);
    sgp_host.GetHardware().GetCPUState().ResetTaskPerformance(task_i);
  }

  THEN("Symbiont offspring increases its lineage reproduction count") {
    // Mark repro in progress to check that reproduce function resets repro in progress
    // flag on parent.

    size_t q_id = world.GetReproQueue().Enqueue(&sgp_sym, sgp_sym.GetLocation());
    sym_hw.GetCPUState().MarkReproInProgress(q_id);
    REQUIRE(sym_hw.GetCPUState().ReproInProgress());

    emp::Ptr<sgp_sym_t> sym_offspring_ptr = static_cast<sgp_sym_t*>(sgp_sym.Reproduce().Raw());
    REQUIRE(sym_offspring_ptr->GetReproCount() == (sgp_sym.GetReproCount() + 1));
    REQUIRE(!sym_hw.GetCPUState().ReproAttempt());
    REQUIRE(!sym_hw.GetCPUState().ReproInProgress());
    REQUIRE(sym_hw.GetCPUState().NotReproducing());
    sym_offspring_ptr.Delete();
  }

  SECTION("Symbiont offspring inherits parent's completed task profile") {
    // Mark some of parent's tasks completed
    sym_hw.GetCPUState().MarkTaskPerformed(0);
    sym_hw.GetCPUState().MarkTaskPerformed(0);
    sym_hw.GetCPUState().MarkTaskPerformed(2);
    sym_hw.GetCPUState().MarkTaskPerformed(4);
    sym_hw.GetCPUState().MarkTaskPerformed(6);
    sym_hw.GetCPUState().MarkTaskPerformed(8);
    // Assert that MarkTaskPerformed worked as expected
    REQUIRE(sym_hw.GetCPUState().GetTaskPerformanceCount(0) == 2);
    REQUIRE(sym_hw.GetCPUState().GetTaskPerformanceCount(1) == 0);
    REQUIRE(sym_hw.GetCPUState().GetTaskPerformanceCount(2) == 1);
    REQUIRE(sym_hw.GetCPUState().GetTaskPerformed(0));
    REQUIRE(!sym_hw.GetCPUState().GetTaskPerformed(1));
    REQUIRE(sym_hw.GetCPUState().GetTaskPerformed(2));
    // Reproduce symbiont
    emp::Ptr<sgp_sym_t> sym_offspring_ptr = static_cast<sgp_sym_t*>(
      sgp_sym.Reproduce().Raw()
    );
    auto& sym_offspring_hw = sym_offspring_ptr->GetHardware();
    REQUIRE(!sym_offspring_hw.GetCPUState().GetParentTaskPerformed(1));
    REQUIRE(!sym_offspring_hw.GetCPUState().GetParentTaskPerformed(3));
    REQUIRE(!sym_offspring_hw.GetCPUState().GetParentTaskPerformed(5));
    REQUIRE(!sym_offspring_hw.GetCPUState().GetParentTaskPerformed(7));
    REQUIRE(sym_offspring_hw.GetCPUState().GetParentTaskPerformed(0));
    REQUIRE(sym_offspring_hw.GetCPUState().GetParentTaskPerformed(2));
    REQUIRE(sym_offspring_hw.GetCPUState().GetParentTaskPerformed(4));
    REQUIRE(sym_offspring_hw.GetCPUState().GetParentTaskPerformed(6));
    REQUIRE(sym_offspring_hw.GetCPUState().GetParentTaskPerformed(8));
    sym_offspring_ptr.Delete();
  }

  SECTION("Symbiont offspring tracks any gains or losses in tasks") {
    // Gen 1 (sgp_sym): parent tasks all 0s
    REQUIRE(!sym_hw.GetCPUState().GetParentTaskPerformed(not_task_id));
    REQUIRE(!sym_hw.GetCPUState().GetParentTaskPerformed(nand_task_id));
    REQUIRE(!sym_hw.GetCPUState().GetParentTaskPerformed(equ_task_id));

    REQUIRE(sym_hw.GetCPUState().GetLineageTaskGainCount(not_task_id) == 0);
    REQUIRE(sym_hw.GetCPUState().GetLineageTaskGainCount(nand_task_id) == 0);
    REQUIRE(sym_hw.GetCPUState().GetLineageTaskGainCount(equ_task_id) == 0);
    REQUIRE(sym_hw.GetCPUState().GetLineageTaskLossCount(not_task_id) == 0);
    REQUIRE(sym_hw.GetCPUState().GetLineageTaskLossCount(nand_task_id) == 0);
    REQUIRE(sym_hw.GetCPUState().GetLineageTaskLossCount(equ_task_id) == 0);
    // Gen 1 performs:
    // - not (gain), nand (gain), equ (gain)
    sym_hw.GetCPUState().MarkTaskPerformed(not_task_id);
    sym_hw.GetCPUState().MarkTaskPerformed(nand_task_id);
    sym_hw.GetCPUState().MarkTaskPerformed(equ_task_id);
    // Gen 2 (sym_gen2)
    emp::Ptr<sgp_sym_t> sym_gen2 = static_cast<sgp_sym_t*>(sgp_sym.Reproduce().Raw());
    REQUIRE(sym_gen2->GetHardware().GetCPUState().GetLineageTaskGainCount(not_task_id) == 1);
    REQUIRE(sym_gen2->GetHardware().GetCPUState().GetLineageTaskGainCount(nand_task_id) == 1);
    REQUIRE(sym_gen2->GetHardware().GetCPUState().GetLineageTaskGainCount(equ_task_id) == 1);
    REQUIRE(sym_gen2->GetHardware().GetCPUState().GetLineageTaskLossCount(not_task_id) == 0);
    REQUIRE(sym_gen2->GetHardware().GetCPUState().GetLineageTaskLossCount(nand_task_id) == 0);
    REQUIRE(sym_gen2->GetHardware().GetCPUState().GetLineageTaskLossCount(equ_task_id) == 0);
    // Gen 2 performs:
    // - nand (--)
    // Loses: not, equ
    sym_gen2->GetHardware().GetCPUState().MarkTaskPerformed(nand_task_id);
    REQUIRE(!sym_gen2->GetHardware().GetCPUState().GetTaskPerformed(not_task_id));
    REQUIRE(sym_gen2->GetHardware().GetCPUState().GetTaskPerformed(nand_task_id));
    REQUIRE(!sym_gen2->GetHardware().GetCPUState().GetTaskPerformed(equ_task_id));
    // Gen 3 (sym_gen3): No gains (from prev gen), loses equ
    emp::Ptr<sgp_sym_t> sym_gen3 = static_cast<sgp_sym_t*>(sym_gen2->Reproduce().Raw());
    REQUIRE(!sym_gen3->GetHardware().GetCPUState().GetParentTaskPerformed(not_task_id));
    REQUIRE(sym_gen3->GetHardware().GetCPUState().GetParentTaskPerformed(nand_task_id));
    REQUIRE(!sym_gen3->GetHardware().GetCPUState().GetParentTaskPerformed(equ_task_id));
    REQUIRE(sym_gen3->GetHardware().GetCPUState().GetLineageTaskGainCount(not_task_id) == 1);
    REQUIRE(sym_gen3->GetHardware().GetCPUState().GetLineageTaskGainCount(nand_task_id) == 1);
    REQUIRE(sym_gen3->GetHardware().GetCPUState().GetLineageTaskGainCount(equ_task_id) == 1);
    REQUIRE(sym_gen3->GetHardware().GetCPUState().GetLineageTaskLossCount(not_task_id) == 1);
    REQUIRE(sym_gen3->GetHardware().GetCPUState().GetLineageTaskLossCount(nand_task_id) == 0);
    REQUIRE(sym_gen3->GetHardware().GetCPUState().GetLineageTaskLossCount(equ_task_id) == 1);
    // Gen 3 performs:
    // - nand (--), equ (gain)
    // - loses: none
    sym_gen3->GetHardware().GetCPUState().MarkTaskPerformed(nand_task_id);
    sym_gen3->GetHardware().GetCPUState().MarkTaskPerformed(equ_task_id);
    REQUIRE(!sym_gen3->GetHardware().GetCPUState().GetTaskPerformed(not_task_id));
    REQUIRE(sym_gen3->GetHardware().GetCPUState().GetTaskPerformed(nand_task_id));
    REQUIRE(sym_gen3->GetHardware().GetCPUState().GetTaskPerformed(equ_task_id));
    REQUIRE(!sym_gen3->GetHardware().GetCPUState().GetParentTaskPerformed(not_task_id));
    REQUIRE(sym_gen3->GetHardware().GetCPUState().GetParentTaskPerformed(nand_task_id));
    REQUIRE(!sym_gen3->GetHardware().GetCPUState().GetParentTaskPerformed(equ_task_id));
    // Gen 4 (host_gen4): Gain equ
    emp::Ptr<sgp_sym_t> sym_gen4 = static_cast<sgp_sym_t*>(sym_gen3->Reproduce().Raw());
    REQUIRE(sym_gen4->GetHardware().GetCPUState().GetLineageTaskGainCount(not_task_id) == 1);
    REQUIRE(sym_gen4->GetHardware().GetCPUState().GetLineageTaskGainCount(nand_task_id) == 1);
    REQUIRE(sym_gen4->GetHardware().GetCPUState().GetLineageTaskGainCount(equ_task_id) == 2);
    REQUIRE(sym_gen4->GetHardware().GetCPUState().GetLineageTaskLossCount(not_task_id) == 1);
    REQUIRE(sym_gen4->GetHardware().GetCPUState().GetLineageTaskLossCount(nand_task_id) == 0);
    REQUIRE(sym_gen4->GetHardware().GetCPUState().GetLineageTaskLossCount(equ_task_id) == 1);

    WHEN("The symbiont has no host") {
      // No-host symbionts should have same converge/diverge as parents
      REQUIRE(!sym_gen2->GetHardware().GetCPUState().HasHost());
      REQUIRE(!sym_gen3->GetHardware().GetCPUState().HasHost());
      REQUIRE(!sym_gen4->GetHardware().GetCPUState().HasHost());
      THEN("The symbiont offspring inherits the lineage's partner task flip count with no modifications") {
        for (size_t i = 0; i < num_tasks; ++i) {
          // gen2 and gen3 don't have host's, so they should match each other
          REQUIRE(sym_gen3->GetHardware().GetCPUState().GetLineageTaskConvergeToPartner(i) == sym_gen2->GetHardware().GetCPUState().GetLineageTaskConvergeToPartner(i));
          REQUIRE(sym_gen3->GetHardware().GetCPUState().GetLineageTaskDivergeFromPartner(i) == sym_gen2->GetHardware().GetCPUState().GetLineageTaskDivergeFromPartner(i));
        }
      }
      sym_gen2.Delete();
    }

    WHEN("The symbiont has a host") {
      // sym has a host, so can diverge / converge
      // On gen1->gen2 reproduction, sym diverges from do-nothing host partner on not, nand, equ tasks
      REQUIRE(sym_gen2->GetHardware().GetCPUState().GetLineageTaskConvergeToPartner(not_task_id) == 0);
      REQUIRE(sym_gen2->GetHardware().GetCPUState().GetLineageTaskConvergeToPartner(nand_task_id) == 0);
      REQUIRE(sym_gen2->GetHardware().GetCPUState().GetLineageTaskConvergeToPartner(equ_task_id) == 0);
      REQUIRE(sym_gen2->GetHardware().GetCPUState().GetLineageTaskConvergeToPartner(xor_task_id) == 0);

      REQUIRE(sym_gen2->GetHardware().GetCPUState().GetLineageTaskDivergeFromPartner(not_task_id) == 1);
      REQUIRE(sym_gen2->GetHardware().GetCPUState().GetLineageTaskDivergeFromPartner(nand_task_id) == 1);
      REQUIRE(sym_gen2->GetHardware().GetCPUState().GetLineageTaskDivergeFromPartner(equ_task_id) == 1);
      REQUIRE(sym_gen2->GetHardware().GetCPUState().GetLineageTaskDivergeFromPartner(xor_task_id) == 0);

      // Need to make sure that converge tracking works because above examples don't test
      // Reuse sym_gen2 for convenience
      // Reproduce host
      emp::Ptr<sgp_host_t> host_gen2 = static_cast<sgp_host_t*>(
        sgp_host.Reproduce().Raw()
      );
      // Add sym_gen2 to host_gen2
      REQUIRE(!host_gen2->HasSym());
      host_gen2->AddSymbiont(sym_gen2);
      // Mark host gen2 as performing xor

      // Set sym gen2 parent tasks explicitly
      for (size_t task_i = 0; task_i < num_tasks; ++task_i) {
        sym_gen2->GetHardware().GetCPUState().SetParentTaskPerformed(task_i, false);
        sym_gen2->GetHardware().GetCPUState().ResetTaskPerformance(task_i);
        host_gen2->GetHardware().GetCPUState().SetParentTaskPerformed(task_i, false);
        host_gen2->GetHardware().GetCPUState().ResetTaskPerformance(task_i);
      }
      host_gen2->GetHardware().GetCPUState().SetParentTaskPerformed(xor_task_id, true);
      host_gen2->GetHardware().GetCPUState().MarkTaskPerformed(xor_task_id);
      sym_gen2->GetHardware().GetCPUState().SetParentTaskPerformed(nand_task_id, true); // Same
      sym_gen2->GetHardware().GetCPUState().SetParentTaskPerformed(not_task_id, true);  // Loss
      sym_gen2->GetHardware().GetCPUState().SetParentTaskPerformed(equ_task_id, true);  // Loss
      // Mark sym gen2 (partner) as performing xor
      sym_gen2->GetHardware().GetCPUState().MarkTaskPerformed(nand_task_id); // Same
      sym_gen2->GetHardware().GetCPUState().MarkTaskPerformed(xor_task_id);  // Gain
      REQUIRE(sym_gen2->GetHardware().GetCPUState().HasHost());
      emp::Ptr<sgp_sym_t> sym_gen2_offspring = static_cast<sgp_sym_t*>(sym_gen2->Reproduce().Raw());
      // Sym gen1:
      // - nand, not, equ
      // Host gen2 / host parent gen2:
      // - xor
      // Sym gen2:
      // - nand (no change), xor (C), losses:[not (C), equ (C)]
      REQUIRE(sym_gen2_offspring->GetHardware().GetCPUState().GetLineageTaskConvergeToPartner(not_task_id) == 1);
      REQUIRE(sym_gen2_offspring->GetHardware().GetCPUState().GetLineageTaskConvergeToPartner(nand_task_id) == 0);
      REQUIRE(sym_gen2_offspring->GetHardware().GetCPUState().GetLineageTaskConvergeToPartner(equ_task_id) == 1);
      REQUIRE(sym_gen2_offspring->GetHardware().GetCPUState().GetLineageTaskConvergeToPartner(xor_task_id) == 1);

      REQUIRE(sym_gen2_offspring->GetHardware().GetCPUState().GetLineageTaskDivergeFromPartner(not_task_id) == 1);
      REQUIRE(sym_gen2_offspring->GetHardware().GetCPUState().GetLineageTaskDivergeFromPartner(nand_task_id) == 1);
      REQUIRE(sym_gen2_offspring->GetHardware().GetCPUState().GetLineageTaskDivergeFromPartner(equ_task_id) == 1);
      REQUIRE(sym_gen2_offspring->GetHardware().GetCPUState().GetLineageTaskDivergeFromPartner(xor_task_id) == 0);
      host_gen2.Delete(); // This also deletes sym_gen2
      sym_gen2_offspring.Delete();
    }

    // sym_gen2.Delete(); (deleted when host_gen2 is deleted)
    sym_gen3.Delete();
    sym_gen4.Delete();
  }
}