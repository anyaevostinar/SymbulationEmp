#include "emp/math/Random.hpp"

#include "../../sgp_mode/hardware/SGPHardware.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPWorld.cc"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPWorldSetup.cc"
#include "../../sgp_mode/SGPWorldData.cc"
#include "../../sgp_mode/ProgramBuilder.h"

#include "../../catch/catch.hpp"

TEST_CASE("SGPHost Reproduce function results in correct parental task tracking", "[sgp]") {
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
  // using program_t = typename world_t::sgp_prog_t;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;

  sgpmode::SymConfigSGP config;
  config.CYCLES_PER_UPDATE(4);
  config.RANDOM_ANCESTOR(false);
  config.HOST_REPRO_RES(1);
  config.SEED(61);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.FILE_PATH("SGPHost_test_output");
  config.POP_SIZE(1);
  config.START_MOI(0);
  config.TASK_IO_UNIQUE_OUTPUT(true);
  // Zero out mutation rates
  config.MUTATION_RATE(0);
  config.MUTATION_SIZE(0);
  config.SGP_MUT_PER_BIT_RATE(0);

  // Initialize world with one host
  emp::Random random(config.SEED());
  world_t world(random, &config);
  world.Setup();
  REQUIRE(world.IsOccupied(0));

  // Grab host to use for test
  auto& org = world.GetOrg(0);
  auto& sgp_host = static_cast<sgp_host_t&>(org);
  hardware_t& hw = sgp_host.GetHardware();
  // Assert that added host is what we expect
  REQUIRE(hw.GetCPUState().GetNumTasks() == 9);

  REQUIRE(world.GetTaskEnv().GetTaskSet().HasTask("NOT"));
  const size_t not_task_id = world.GetTaskEnv().GetTaskSet().GetID("NOT");

  WHEN("A host can only perform NOT") {
    WHEN("It is one of the first generation (does not have parents)") {

      auto& host_parent_tasks = hw.GetCPUState().GetParentTasksPerformed();
      auto& host_tasks = hw.GetCPUState().GetTasksPerformed();

      THEN("Its own tasks are initially all marked as not completed") {
        REQUIRE(host_tasks.None());
      }

      THEN("Its parent's tasks are marked with initial not task") {
        REQUIRE(host_parent_tasks.Get(not_task_id));
      }

      // Run world for enough updates to evaluate full genome
      for (size_t i = 0; i < 25; ++i) {
        world.Update();
      }

      THEN("After running for 25 updates, host tasks should show NOT completed") {
        REQUIRE(host_tasks.Get(not_task_id));
        REQUIRE(host_tasks.CountOnes() == 1);
        REQUIRE(hw.GetCPUState().GetTaskPerformanceCount(not_task_id) == 1);
      }

      THEN("Offspring should have correct parent tasks marked as completed") {
        emp::Ptr<sgp_host_t> offspring = static_cast<sgp_host_t*>(sgp_host.Reproduce().Raw());
        auto& offspring_hw = offspring->GetHardware();
        auto& offspring_parent_tasks = offspring_hw.GetCPUState().GetParentTasksPerformed();
        auto& offspring_tasks = offspring_hw.GetCPUState().GetTasksPerformed();
        REQUIRE(offspring_tasks.None());
        REQUIRE(offspring_parent_tasks.CountOnes() == 1);
        REQUIRE(offspring_parent_tasks.Get(not_task_id));
        offspring.Delete();
      }
    }
  }
}

TEST_CASE("SGPHost Reproduce", "[sgp]") {
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
  // using program_t = typename world_t::sgp_prog_t;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;

  sgpmode::SymConfigSGP config;
  config.CYCLES_PER_UPDATE(0);
  config.RANDOM_ANCESTOR(false);
  config.HOST_REPRO_RES(1);
  config.SEED(61);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.FILE_PATH("SGPHost_test_output");
  config.POP_SIZE(1);
  config.START_MOI(0);
  config.TASK_IO_UNIQUE_OUTPUT(true);

  // Initialize world with one host
  emp::Random random(config.SEED());
  world_t world(random, &config);
  world.Setup();
  auto& prog_builder = world.GetProgramBuilder();
  REQUIRE(world.IsOccupied(0));

  // Grab host to use for test
  auto& org = world.GetOrg(0);
  auto& sgp_host = static_cast<sgp_host_t&>(org);
  hardware_t& hw = sgp_host.GetHardware();
  REQUIRE(hw.GetCPUState().GetNumTasks() == 9);

  // Configure host's program to include a NOT task
  hw.SetProgram(
    prog_builder.CreateNotProgram(50)
  );

  SECTION("Host offspring increases lineage reproduction count and resets parent repro state") {
    emp::Ptr<sgp_host_t> offspring_ptr = static_cast<sgp_host_t*>(sgp_host.Reproduce().Raw());
    REQUIRE(offspring_ptr->GetReproCount() == (sgp_host.GetReproCount() + 1));
    REQUIRE(!sgp_host.GetHardware().GetCPUState().ReproAttempt());
    REQUIRE(!sgp_host.GetHardware().GetCPUState().ReproInProgress());
    offspring_ptr.Delete();
  }

  // (3) Check that offspring has been configured appropriately
  SECTION("Host offspring inherits parent task profile correctly") {
    // Mark some of parent's tasks completed
    hw.GetCPUState().MarkTaskPerformed(0);
    hw.GetCPUState().MarkTaskPerformed(0);
    hw.GetCPUState().MarkTaskPerformed(2);
    hw.GetCPUState().MarkTaskPerformed(4);
    hw.GetCPUState().MarkTaskPerformed(6);
    hw.GetCPUState().MarkTaskPerformed(8);
    // Assert that MarkTaskPerformed worked as expected
    REQUIRE(hw.GetCPUState().GetTaskPerformanceCount(0) == 2);
    REQUIRE(hw.GetCPUState().GetTaskPerformanceCount(1) == 0);
    REQUIRE(hw.GetCPUState().GetTaskPerformanceCount(2) == 1);
    REQUIRE(hw.GetCPUState().GetTaskPerformed(0));
    REQUIRE(!hw.GetCPUState().GetTaskPerformed(1));
    REQUIRE(hw.GetCPUState().GetTaskPerformed(2));
    // Reproduce
    emp::Ptr<sgp_host_t> offspring_ptr = static_cast<sgp_host_t*>(sgp_host.Reproduce().Raw());
    auto& offspring_hw = offspring_ptr->GetHardware();
    REQUIRE(!offspring_hw.GetCPUState().GetParentTaskPerformed(1));
    REQUIRE(!offspring_hw.GetCPUState().GetParentTaskPerformed(3));
    REQUIRE(!offspring_hw.GetCPUState().GetParentTaskPerformed(5));
    REQUIRE(!offspring_hw.GetCPUState().GetParentTaskPerformed(7));
    REQUIRE(offspring_hw.GetCPUState().GetParentTaskPerformed(0));
    REQUIRE(offspring_hw.GetCPUState().GetParentTaskPerformed(2));
    REQUIRE(offspring_hw.GetCPUState().GetParentTaskPerformed(4));
    REQUIRE(offspring_hw.GetCPUState().GetParentTaskPerformed(6));
    REQUIRE(offspring_hw.GetCPUState().GetParentTaskPerformed(8));
    offspring_ptr.Delete();
  }

  SECTION("Host offspring tracks any gains or loses in task completions") {
    REQUIRE(world.GetTaskEnv().GetTaskSet().HasTask("NOT"));
    REQUIRE(world.GetTaskEnv().GetTaskSet().HasTask("NAND"));
    REQUIRE(world.GetTaskEnv().GetTaskSet().HasTask("OR_NOT"));
    REQUIRE(world.GetTaskEnv().GetTaskSet().HasTask("AND"));
    REQUIRE(world.GetTaskEnv().GetTaskSet().HasTask("OR"));
    REQUIRE(world.GetTaskEnv().GetTaskSet().HasTask("AND_NOT"));
    REQUIRE(world.GetTaskEnv().GetTaskSet().HasTask("NOR"));
    REQUIRE(world.GetTaskEnv().GetTaskSet().HasTask("XOR"));
    REQUIRE(world.GetTaskEnv().GetTaskSet().HasTask("EQU"));
    const size_t not_task_id = world.GetTaskEnv().GetTaskSet().GetID("NOT");
    const size_t nand_task_id = world.GetTaskEnv().GetTaskSet().GetID("NAND");
    const size_t equ_task_id = world.GetTaskEnv().GetTaskSet().GetID("EQU");

    // Gen 1 (sgp_host): parent tasks all 0s
    REQUIRE(!sgp_host.GetHardware().GetCPUState().GetParentTaskPerformed(not_task_id));
    REQUIRE(!sgp_host.GetHardware().GetCPUState().GetParentTaskPerformed(nand_task_id));
    REQUIRE(!sgp_host.GetHardware().GetCPUState().GetParentTaskPerformed(equ_task_id));

    REQUIRE(sgp_host.GetHardware().GetCPUState().GetLineageTaskGainCount(not_task_id) == 0);
    REQUIRE(sgp_host.GetHardware().GetCPUState().GetLineageTaskGainCount(nand_task_id) == 0);
    REQUIRE(sgp_host.GetHardware().GetCPUState().GetLineageTaskGainCount(equ_task_id) == 0);
    REQUIRE(sgp_host.GetHardware().GetCPUState().GetLineageTaskLossCount(not_task_id) == 0);
    REQUIRE(sgp_host.GetHardware().GetCPUState().GetLineageTaskLossCount(nand_task_id) == 0);
    REQUIRE(sgp_host.GetHardware().GetCPUState().GetLineageTaskLossCount(equ_task_id) == 0);
    // Gen 1 performs:
    // - not (gain), nand (gain), equ (gain)
    sgp_host.GetHardware().GetCPUState().MarkTaskPerformed(not_task_id);
    sgp_host.GetHardware().GetCPUState().MarkTaskPerformed(nand_task_id);
    sgp_host.GetHardware().GetCPUState().MarkTaskPerformed(equ_task_id);
    // Gen 2 (host_gen2)
    emp::Ptr<sgp_host_t> host_gen2 = static_cast<sgp_host_t*>(sgp_host.Reproduce().Raw());
    REQUIRE(host_gen2->GetHardware().GetCPUState().GetLineageTaskGainCount(not_task_id) == 1);
    REQUIRE(host_gen2->GetHardware().GetCPUState().GetLineageTaskGainCount(nand_task_id) == 1);
    REQUIRE(host_gen2->GetHardware().GetCPUState().GetLineageTaskGainCount(equ_task_id) == 1);
    REQUIRE(host_gen2->GetHardware().GetCPUState().GetLineageTaskLossCount(not_task_id) == 0);
    REQUIRE(host_gen2->GetHardware().GetCPUState().GetLineageTaskLossCount(nand_task_id) == 0);
    REQUIRE(host_gen2->GetHardware().GetCPUState().GetLineageTaskLossCount(equ_task_id) == 0);
    // Gen 2 performs:
    // - nand (--)
    // Loses: not, equ
    host_gen2->GetHardware().GetCPUState().MarkTaskPerformed(nand_task_id);
    REQUIRE(!host_gen2->GetHardware().GetCPUState().GetTaskPerformed(not_task_id));
    REQUIRE(host_gen2->GetHardware().GetCPUState().GetTaskPerformed(nand_task_id));
    REQUIRE(!host_gen2->GetHardware().GetCPUState().GetTaskPerformed(equ_task_id));
    // Gen 3 (host_gen3): No gains (from prev gen), loses equ
    emp::Ptr<sgp_host_t> host_gen3 = static_cast<sgp_host_t*>(host_gen2->Reproduce().Raw());
    REQUIRE(!host_gen3->GetHardware().GetCPUState().GetParentTaskPerformed(not_task_id));
    REQUIRE(host_gen3->GetHardware().GetCPUState().GetParentTaskPerformed(nand_task_id));
    REQUIRE(!host_gen3->GetHardware().GetCPUState().GetParentTaskPerformed(equ_task_id));
    REQUIRE(host_gen3->GetHardware().GetCPUState().GetLineageTaskGainCount(not_task_id) == 1);
    REQUIRE(host_gen3->GetHardware().GetCPUState().GetLineageTaskGainCount(nand_task_id) == 1);
    REQUIRE(host_gen3->GetHardware().GetCPUState().GetLineageTaskGainCount(equ_task_id) == 1);
    REQUIRE(host_gen3->GetHardware().GetCPUState().GetLineageTaskLossCount(not_task_id) == 1);
    REQUIRE(host_gen3->GetHardware().GetCPUState().GetLineageTaskLossCount(nand_task_id) == 0);
    REQUIRE(host_gen3->GetHardware().GetCPUState().GetLineageTaskLossCount(equ_task_id) == 1);
    // Gen 3 performs:
    // - nand (--), equ (gain)
    // - loses: none
    host_gen3->GetHardware().GetCPUState().MarkTaskPerformed(nand_task_id);
    host_gen3->GetHardware().GetCPUState().MarkTaskPerformed(equ_task_id);
    REQUIRE(!host_gen3->GetHardware().GetCPUState().GetTaskPerformed(not_task_id));
    REQUIRE(host_gen3->GetHardware().GetCPUState().GetTaskPerformed(nand_task_id));
    REQUIRE(host_gen3->GetHardware().GetCPUState().GetTaskPerformed(equ_task_id));
    REQUIRE(!host_gen3->GetHardware().GetCPUState().GetParentTaskPerformed(not_task_id));
    REQUIRE(host_gen3->GetHardware().GetCPUState().GetParentTaskPerformed(nand_task_id));
    REQUIRE(!host_gen3->GetHardware().GetCPUState().GetParentTaskPerformed(equ_task_id));
    // Gen 4 (host_gen4): Gain equ
    emp::Ptr<sgp_host_t> host_gen4 = static_cast<sgp_host_t*>(host_gen3->Reproduce().Raw());
    REQUIRE(host_gen4->GetHardware().GetCPUState().GetLineageTaskGainCount(not_task_id) == 1);
    REQUIRE(host_gen4->GetHardware().GetCPUState().GetLineageTaskGainCount(nand_task_id) == 1);
    REQUIRE(host_gen4->GetHardware().GetCPUState().GetLineageTaskGainCount(equ_task_id) == 2);
    REQUIRE(host_gen4->GetHardware().GetCPUState().GetLineageTaskLossCount(not_task_id) == 1);
    REQUIRE(host_gen4->GetHardware().GetCPUState().GetLineageTaskLossCount(nand_task_id) == 0);
    REQUIRE(host_gen4->GetHardware().GetCPUState().GetLineageTaskLossCount(equ_task_id) == 1);

    host_gen2.Delete();
    host_gen3.Delete();
    host_gen4.Delete();
  }

  WHEN("The host parent has no symbiont") {
    // TODO
    THEN("The host offspring inherits the lineage's partner task flip count with no modifications") {

    }
  }

  WHEN("The host parent has a symbiont") {
    // TODO
    THEN("The host offspring tracks how its tasks compare to its parent's partner's tasks") {
      // TODO
    }
  }

  // BOOKMARK

  // TODO


}