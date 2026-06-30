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

#include "../../../catch/catch.hpp"

/**
 * This file is dedicated to functional tests for HealthMode
 */

/**
Overview of expected health mode behvaior:

conditions where no host cycles are stolen:
- uninfected host
- infected host, mismatching parasite
- infected host, matching parasite, PARASITE_CYCLE_STEAL_MULTIPLIER = 0 (host cycles are stolen but parasite doesn't benefit)
- infected host, matching parasite, PARASITE_CYCLE_LOSS_PROP = 0
- infected host, matching parasite, HEALTH_INTERACTION_CHANCE = 0

conditions where no cycles are donated:
- uninfected host
- infected host, mismatching mutualist
- infected host, matching mutualist, MUTUALIST_CYCLE_DONATE_MULTIPLIER = 0 (mutualist donates but host doesn't benefit)
- infected host, matching mutualist, MUTUALIST_CYCLE_GAIN_PROP = 0
- infected host, matching mutualist, HEALTH_INTERACTION_CHANCE = 0

TODO: test interaction value based health mode
*/

void ConfigureHealthTestConfig(sgpmode::SymConfigSGP& config) {
  // base world settings
  config.SEED(10);
  config.INIT_POP_SIZE(0);
  config.START_MOI(0);
  config.WORLD_WIDTH(2);
  config.WORLD_HEIGHT(2);
  config.SPATIAL_STRUCT_MODE("well-mixed");

  // general sgp settings
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.TASK_PROFILE_COMPATIBILITY_MODE("task-any-match");
  config.TASK_PROFILE_MODE("self-all");
  config.CYCLES_PER_UPDATE(4);
  config.TASK_IO_BANK_SIZE(10);

  // health settings
  config.ENABLE_HEALTH(1);
  config.HEALTH_INTERACTION_CHANCE(1);
  config.PARASITE_CYCLE_LOSS_PROP(0.5);
  config.PARASITE_CYCLE_STEAL_MULTIPLIER(1);
  config.PARASITE_BASE_CYCLE_PROP(0.5);
  config.MUTUALIST_CYCLE_DONATE_MULTIPLIER(1);
  config.MUTUALIST_CYCLE_GAIN_PROP(0.5);
}

TEST_CASE("Health hosts do not have modified cycles when uninfected", "[sgp][sgp-functional]") {
  using world_t = sgpmode::SGPWorld;

  GIVEN("A health host is uninfected") {
    sgpmode::SymConfigSGP config;
    ConfigureHealthTestConfig(config);
    size_t cycles_per_update = 4;

    emp::Random random(config.SEED());

    world_t world(random, &config);
    world.Setup();

    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, world.GetProgramBuilder().CreateNotProgram(100));

    world.AddOrgAt(host, 0);
    host->GetHardware().GetCPUState().MarkTaskPerformed(0);

    WHEN("Host Process is called") {
      int repeats = 25;
      int total_times_skipped_cycle = 0;
      for (int i = 0; i < repeats; i++) {
        size_t initial_stack_location = host->GetHardware().GetCPU().GetCore(0).GetProgramCounter();

        host->GetHardware().GetCPUState().SetLocation(0);
        host->GetHardware().GetCPUState().GainCPUCycles(cycles_per_update);
        host->Process(0);

        size_t new_stack_location = host->GetHardware().GetCPU().GetCore(0).GetProgramCounter();
        if (initial_stack_location == new_stack_location) {
          total_times_skipped_cycle++;
        }
      }

      int total_times_gained_cycles = 0;
      for (int i = 0; i < repeats; i++) {
        size_t initial_stack_location = host->GetHardware().GetCPU().GetCore(0).GetProgramCounter();

        host->GetHardware().GetCPUState().SetLocation(0);
        host->GetHardware().GetCPUState().GainCPUCycles(cycles_per_update);
        host->Process(0);

        size_t new_stack_location = host->GetHardware().GetCPU().GetCore(0).GetProgramCounter();

        if (new_stack_location - initial_stack_location == 8) {
          total_times_gained_cycles++;
        }
      }

      THEN("The host neither looses nor gains cycles") {
        REQUIRE((double)total_times_skipped_cycle/repeats == 0);
        REQUIRE((double)total_times_gained_cycles/repeats == 0);
        REQUIRE(host->GetHardware().GetCPUState().GetCPUCyclesSinceRepro() == 2 * cycles_per_update * repeats);
      }
    }
  }
}

TEST_CASE("Health host cycles are stolen by matching parasites", "[sgp][sgp-functional]") {
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;
  GIVEN("A health host infected by a matching parasite") {
    sgpmode::SymConfigSGP config;
    ConfigureHealthTestConfig(config);
    config.HEALTH_TYPE("parasite");
    config.HEALTH_INTERACTION_CHANCE(0.5);
    config.PARASITE_CYCLE_LOSS_PROP(1);
    size_t cycles_per_update = 4;

    emp::Random random(config.SEED());

    world_t world(random, &config);
    world.Setup();

    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, world.GetProgramBuilder().CreateNotProgram(100));
    emp::Ptr<sgp_sym_t> parasite = emp::NewPtr<sgp_sym_t> (&random, &world, &config, world.GetProgramBuilder().CreateNotProgram(100));

    world.AddOrgAt(host, 0);
    host->AddSymbiont(parasite);

    host->GetHardware().GetCPUState().MarkTaskPerformed(0);
    parasite->GetHardware().GetCPUState().MarkTaskPerformed(0);

    WHEN("A matching parasite sometimes steals all of its host's cycles") {
      int repeats = 50;
      int total_times_skipped_cycle = 0;
      for (int i = 0; i < repeats; i++) {
        size_t initial_stack_location = host->GetHardware().GetCPU().GetCore(0).GetProgramCounter();

        host->GetHardware().GetCPUState().SetLocation(0);
        host->GetHardware().GetCPUState().GainCPUCycles(cycles_per_update);
        host->Process(0);

        size_t new_stack_location = host->GetHardware().GetCPU().GetCore(0).GetProgramCounter();
        if (initial_stack_location == new_stack_location) {
          total_times_skipped_cycle++;
        }
      }

      double expected_host_cycles = cycles_per_update * repeats * config.PARASITE_CYCLE_LOSS_PROP() * config.HEALTH_INTERACTION_CHANCE();
      double expected_stolen_cycles = (cycles_per_update * repeats) - expected_host_cycles;
      double expected_parasite_cycles = (config.PARASITE_BASE_CYCLE_PROP() * cycles_per_update * repeats) + expected_stolen_cycles;

      THEN("The host loses approximately 50% of its cycles") {
        REQUIRE((double)total_times_skipped_cycle/repeats > 0.45);
        REQUIRE((double)total_times_skipped_cycle/repeats < 0.55);
        REQUIRE(host->GetHardware().GetCPUState().GetCPUCyclesSinceRepro() > expected_host_cycles - expected_host_cycles*0.05);
        REQUIRE(host->GetHardware().GetCPUState().GetCPUCyclesSinceRepro() < expected_host_cycles + expected_host_cycles*0.05);
      }

      THEN("The matching parasite gains approximately 50% of the host's cycles") {
        REQUIRE(parasite->GetHardware().GetCPUState().GetCPUCyclesSinceRepro() > expected_parasite_cycles - expected_parasite_cycles*0.05);
        REQUIRE(parasite->GetHardware().GetCPUState().GetCPUCyclesSinceRepro() < expected_parasite_cycles + expected_parasite_cycles*0.05);
      }
    }
  }
}

TEST_CASE("Health host cycles are not stolen by mismatching parasites", "[sgp][sgp-functional]") {
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
  using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;
  GIVEN("A health host infected by a mismatching parasite") {
    sgpmode::SymConfigSGP config;
    ConfigureHealthTestConfig(config);
    config.HEALTH_TYPE("parasite");
    size_t cycles_per_update = 4;

    emp::Random random(config.SEED());

    world_t world(random, &config);
    world.Setup();

    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, world.GetProgramBuilder().CreateNotProgram(100));
    emp::Ptr<sgp_sym_t> parasite = emp::NewPtr<sgp_sym_t> (&random, &world, &config, world.GetProgramBuilder().CreateNotProgram(100));

    world.AddOrgAt(host, 0);
    host->AddSymbiont(parasite);

    host->GetHardware().GetCPUState().MarkTaskPerformed(0);
    parasite->GetHardware().GetCPUState().MarkTaskPerformed(1);

    WHEN("A matching parasite always steals 50% its host's cycles") {
      int repeats = 25;
      for (int i = 0; i < repeats; i++) {
        host->GetHardware().GetCPUState().SetLocation(0);
        host->GetHardware().GetCPUState().GainCPUCycles(cycles_per_update);
        host->Process(0);
      }

      double expected_host_cycles = cycles_per_update * repeats;
      double expected_parasite_cycles = config.PARASITE_BASE_CYCLE_PROP() * cycles_per_update * repeats;

      THEN("The host loses no cycles") {
        REQUIRE(host->GetHardware().GetCPUState().GetCPUCyclesSinceRepro() == expected_host_cycles);
      }

      THEN("The mismatching parasite gains no cycles") {
        REQUIRE(parasite->GetHardware().GetCPUState().GetCPUCyclesSinceRepro() == expected_parasite_cycles);
      }
    }
  }
}

TEST_CASE("Health host cycles are stolen by matching parasites when PARASITE_CYCLE_STEAL_MULTIPLIER is 0", "[sgp][sgp-functional]") {
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
  using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;
  GIVEN("A health host infected by a matching parasite") {
    sgpmode::SymConfigSGP config;
    ConfigureHealthTestConfig(config);
    config.HEALTH_TYPE("parasite");
    config.PARASITE_CYCLE_STEAL_MULTIPLIER(0);
    size_t cycles_per_update = 4;

    emp::Random random(config.SEED());

    world_t world(random, &config);
    world.Setup();

    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, world.GetProgramBuilder().CreateNotProgram(100));
    emp::Ptr<sgp_sym_t> parasite = emp::NewPtr<sgp_sym_t> (&random, &world, &config, world.GetProgramBuilder().CreateNotProgram(100));

    world.AddOrgAt(host, 0);
    host->AddSymbiont(parasite);

    host->GetHardware().GetCPUState().MarkTaskPerformed(0);
    parasite->GetHardware().GetCPUState().MarkTaskPerformed(0);

    WHEN("A matching parasite always steals 0 times 50% its host's cycles") {
      int repeats = 25;
      for (int i = 0; i < repeats; i++) {
        host->GetHardware().GetCPUState().SetLocation(0);
        host->GetHardware().GetCPUState().GainCPUCycles(cycles_per_update);
        host->Process(0);
      }

      double expected_host_cycles = cycles_per_update * repeats * config.PARASITE_CYCLE_LOSS_PROP() * config.HEALTH_INTERACTION_CHANCE();
      double expected_parasite_cycles = config.PARASITE_BASE_CYCLE_PROP() * cycles_per_update * repeats;

      THEN("The host loses cycles") {
        REQUIRE(host->GetHardware().GetCPUState().GetCPUCyclesSinceRepro() == expected_host_cycles);
      }

      THEN("The mismatching parasite gains no cycles (the stolen cycles from the host are negated by the 0 multiplier)") {
        REQUIRE(parasite->GetHardware().GetCPUState().GetCPUCyclesSinceRepro() == expected_parasite_cycles);
      }
    }
  }
}

TEST_CASE("Health host cycles are not stolen by matching parasites when PARASITE_CYCLE_LOSS_PROP is 0", "[sgp][sgp-functional]") {
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
  using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;
  GIVEN("A health host infected by a matching parasite") {
    sgpmode::SymConfigSGP config;
    ConfigureHealthTestConfig(config);
    config.HEALTH_TYPE("parasite");
    config.PARASITE_CYCLE_LOSS_PROP(0);
    size_t cycles_per_update = 4;

    emp::Random random(config.SEED());

    world_t world(random, &config);
    world.Setup();

    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, world.GetProgramBuilder().CreateNotProgram(100));
    emp::Ptr<sgp_sym_t> parasite = emp::NewPtr<sgp_sym_t> (&random, &world, &config, world.GetProgramBuilder().CreateNotProgram(100));

    world.AddOrgAt(host, 0);
    host->AddSymbiont(parasite);

    host->GetHardware().GetCPUState().MarkTaskPerformed(0);
    parasite->GetHardware().GetCPUState().MarkTaskPerformed(0);

    WHEN("A matching parasite always steals 0% its host's cycles") {
      int repeats = 25;
      for (int i = 0; i < repeats; i++) {
        host->GetHardware().GetCPUState().SetLocation(0);
        host->GetHardware().GetCPUState().GainCPUCycles(cycles_per_update);
        host->Process(0);
      }

      double expected_host_cycles = cycles_per_update * repeats;
      double expected_parasite_cycles = config.PARASITE_BASE_CYCLE_PROP() * cycles_per_update * repeats;

      THEN("The host loses no cycles") {
        REQUIRE(host->GetHardware().GetCPUState().GetCPUCyclesSinceRepro() == expected_host_cycles);
      }

      THEN("The mismatching parasite gains no cycles") {
        REQUIRE(parasite->GetHardware().GetCPUState().GetCPUCyclesSinceRepro() == expected_parasite_cycles);
      }
    }
  }
}

TEST_CASE("Health host cycles are not stolen by matching parasites when HEALTH_INTERACTION_CHANCE is 0", "[sgp][sgp-functional]") {
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
  using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;
  GIVEN("A health host infected by a matching parasite") {
    sgpmode::SymConfigSGP config;
    ConfigureHealthTestConfig(config);
    config.HEALTH_TYPE("parasite");
    config.HEALTH_INTERACTION_CHANCE(0);
    size_t cycles_per_update = 4;

    emp::Random random(config.SEED());

    world_t world(random, &config);
    world.Setup();

    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, world.GetProgramBuilder().CreateNotProgram(100));
    emp::Ptr<sgp_sym_t> parasite = emp::NewPtr<sgp_sym_t> (&random, &world, &config, world.GetProgramBuilder().CreateNotProgram(100));

    world.AddOrgAt(host, 0);
    host->AddSymbiont(parasite);

    host->GetHardware().GetCPUState().MarkTaskPerformed(0);
    parasite->GetHardware().GetCPUState().MarkTaskPerformed(0);

    WHEN("A matching parasite never steals 50% its host's cycles") {
      int repeats = 25;
      for (int i = 0; i < repeats; i++) {
        host->GetHardware().GetCPUState().SetLocation(0);
        host->GetHardware().GetCPUState().GainCPUCycles(cycles_per_update);
        host->Process(0);
      }

      double expected_host_cycles = cycles_per_update * repeats;
      double expected_parasite_cycles = config.PARASITE_BASE_CYCLE_PROP() * cycles_per_update * repeats;

      THEN("The host loses no cycles") {
        REQUIRE(host->GetHardware().GetCPUState().GetCPUCyclesSinceRepro() == expected_host_cycles);
      }

      THEN("The mismatching parasite gains no cycles") {
        REQUIRE(parasite->GetHardware().GetCPUState().GetCPUCyclesSinceRepro() == expected_parasite_cycles);
      }
    }
  }
}

TEST_CASE("Matching mutualists donate cycles to health hosts", "[sgp][sgp-functional]") {
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
  using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;
  GIVEN("A health host infected by a matching mutualist") {
    sgpmode::SymConfigSGP config;
    ConfigureHealthTestConfig(config);
    config.HEALTH_TYPE("mutualist");
    config.HEALTH_INTERACTION_CHANCE(0.5);
    config.MUTUALIST_CYCLE_GAIN_PROP(1);
    size_t cycles_per_update = 4;
    emp::Random random(config.SEED());

    world_t world(random, &config);
    world.Setup();

    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, world.GetProgramBuilder().CreateNotProgram(100));
    emp::Ptr<sgp_sym_t> mutualist = emp::NewPtr<sgp_sym_t> (&random, &world, &config, world.GetProgramBuilder().CreateNotProgram(100));

    world.AddOrgAt(host, 0);
    host->AddSymbiont(mutualist);

    host->GetHardware().GetCPUState().MarkTaskPerformed(0);
    mutualist->GetHardware().GetCPUState().MarkTaskPerformed(0);

    WHEN("A matching mutualist sometimes donates all of its cycles") {
      int repeats = 50;
      int total_times_gained_cycles = 0;
      for (int i = 0; i < repeats; i++) {
        size_t initial_stack_location = host->GetHardware().GetCPU().GetCore(0).GetProgramCounter();

        host->GetHardware().GetCPUState().SetLocation(0);
        host->GetHardware().GetCPUState().GainCPUCycles(cycles_per_update);
        host->Process(0);

        size_t new_stack_location = host->GetHardware().GetCPU().GetCore(0).GetProgramCounter();

        if (new_stack_location - initial_stack_location == 8) {
          total_times_gained_cycles++;
        }
      }

      double expected_donated_cycles = cycles_per_update * repeats * config.MUTUALIST_CYCLE_GAIN_PROP() * config.HEALTH_INTERACTION_CHANCE();
      double expected_host_cycles = (cycles_per_update * repeats) + expected_donated_cycles;
      double expected_mutualist_cycles = (cycles_per_update * repeats) - expected_donated_cycles;

      THEN("The host gains approximately 50% of the mutualist's cycles") {
        REQUIRE((double)total_times_gained_cycles/repeats > 0.45);
        REQUIRE((double)total_times_gained_cycles/repeats < 0.55);
        REQUIRE(host->GetHardware().GetCPUState().GetCPUCyclesSinceRepro() > expected_host_cycles - expected_host_cycles*0.05);
        REQUIRE(host->GetHardware().GetCPUState().GetCPUCyclesSinceRepro() < expected_host_cycles + expected_host_cycles*0.05);
      }

      THEN("The matching mutualist loses approximately 50% of its cycles") {
        REQUIRE(mutualist->GetHardware().GetCPUState().GetCPUCyclesSinceRepro() > expected_mutualist_cycles - expected_mutualist_cycles*0.05);
        REQUIRE(mutualist->GetHardware().GetCPUState().GetCPUCyclesSinceRepro() < expected_mutualist_cycles + expected_mutualist_cycles*0.05);
      }
    }
  }
}

TEST_CASE("Mismatching mutualists do not donate cycles to health hosts", "[sgp][sgp-functional]") {
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
  using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;
  GIVEN("A health host infected by a matching mutualist") {
    sgpmode::SymConfigSGP config;
    ConfigureHealthTestConfig(config);
    config.HEALTH_TYPE("mutualist");
    size_t cycles_per_update = 4;

    emp::Random random(config.SEED());

    world_t world(random, &config);
    world.Setup();

    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, world.GetProgramBuilder().CreateNotProgram(100));
    emp::Ptr<sgp_sym_t> mutualist = emp::NewPtr<sgp_sym_t> (&random, &world, &config, world.GetProgramBuilder().CreateNotProgram(100));

    world.AddOrgAt(host, 0);
    host->AddSymbiont(mutualist);

    host->GetHardware().GetCPUState().MarkTaskPerformed(0);
    mutualist->GetHardware().GetCPUState().MarkTaskPerformed(1);

    WHEN("A matching mutualist always donates 50% of its cycles") {
      int repeats = 25;
      for (int i = 0; i < repeats; i++) {
        host->GetHardware().GetCPUState().SetLocation(0);
        host->GetHardware().GetCPUState().GainCPUCycles(cycles_per_update);
        host->Process(0);
      }

      double expected_host_cycles = cycles_per_update * repeats;
      double expected_mutualist_cycles = cycles_per_update * repeats;

      THEN("The host does not gain cycles") {
        REQUIRE(host->GetHardware().GetCPUState().GetCPUCyclesSinceRepro() == expected_host_cycles);
      }

      THEN("The mismatching mutualist does not lose cycles") {
        REQUIRE(mutualist->GetHardware().GetCPUState().GetCPUCyclesSinceRepro() == expected_mutualist_cycles);
      }
    }
  }
}

TEST_CASE("Matching mutualists do not donate cycles to health hosts when MUTUALIST_CYCLE_DONATE_MULTIPLIER is 0", "[sgp][sgp-functional]") {
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
  using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;
  GIVEN("A health host infected by a matching mutualist") {
    sgpmode::SymConfigSGP config;
    ConfigureHealthTestConfig(config);
    config.HEALTH_TYPE("mutualist");
    config.MUTUALIST_CYCLE_DONATE_MULTIPLIER(0);
    size_t cycles_per_update = 4;

    emp::Random random(config.SEED());

    world_t world(random, &config);
    world.Setup();

    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, world.GetProgramBuilder().CreateNotProgram(100));
    emp::Ptr<sgp_sym_t> mutualist = emp::NewPtr<sgp_sym_t> (&random, &world, &config, world.GetProgramBuilder().CreateNotProgram(100));

    world.AddOrgAt(host, 0);
    host->AddSymbiont(mutualist);

    host->GetHardware().GetCPUState().MarkTaskPerformed(0);
    mutualist->GetHardware().GetCPUState().MarkTaskPerformed(0);

    WHEN("A matching mutualist always donates 0 times 50% of its cycles") {
      int repeats = 25;
      for (int i = 0; i < repeats; i++) {
        host->GetHardware().GetCPUState().SetLocation(0);
        host->GetHardware().GetCPUState().GainCPUCycles(cycles_per_update);
        host->Process(0);
      }

      double expected_attempted_donated_cycles = cycles_per_update * repeats * config.MUTUALIST_CYCLE_GAIN_PROP() * config.HEALTH_INTERACTION_CHANCE();
      double expected_host_cycles = cycles_per_update * repeats;
      double expected_mutualist_cycles = (cycles_per_update * repeats) - expected_attempted_donated_cycles;

      THEN("The host does not gain cycles") {
        REQUIRE(host->GetHardware().GetCPUState().GetCPUCyclesSinceRepro() == expected_host_cycles);
      }

      THEN("The matching mutualist loses cycles (it tries to donate, but its donations are negated by the 0 multiplier)") {
        REQUIRE(mutualist->GetHardware().GetCPUState().GetCPUCyclesSinceRepro() == expected_mutualist_cycles);
      }
    }
  }
}

TEST_CASE("Matching mutualists do not donate cycles to health hosts when MUTUALIST_CYCLE_GAIN_PROP is 0", "[sgp][sgp-functional]") {
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
  using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;
  GIVEN("A health host infected by a matching mutualist") {
    sgpmode::SymConfigSGP config;
    ConfigureHealthTestConfig(config);
    config.HEALTH_TYPE("mutualist");
    config.MUTUALIST_CYCLE_GAIN_PROP(0);
    size_t cycles_per_update = 4;

    emp::Random random(config.SEED());

    world_t world(random, &config);
    world.Setup();

    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, world.GetProgramBuilder().CreateNotProgram(100));
    emp::Ptr<sgp_sym_t> mutualist = emp::NewPtr<sgp_sym_t> (&random, &world, &config, world.GetProgramBuilder().CreateNotProgram(100));

    world.AddOrgAt(host, 0);
    host->AddSymbiont(mutualist);

    host->GetHardware().GetCPUState().MarkTaskPerformed(0);
    mutualist->GetHardware().GetCPUState().MarkTaskPerformed(0);

    WHEN("A matching mutualist always donates 0% of its cycles") {
      int repeats = 25;
      for (int i = 0; i < repeats; i++) {
        host->GetHardware().GetCPUState().SetLocation(0);
        host->GetHardware().GetCPUState().GainCPUCycles(cycles_per_update);
        host->Process(0);
      }

      double expected_host_cycles = cycles_per_update * repeats;
      double expected_mutualist_cycles = cycles_per_update * repeats;

      THEN("The host does not gain cycles") {
        REQUIRE(host->GetHardware().GetCPUState().GetCPUCyclesSinceRepro() == expected_host_cycles);
      }

      THEN("The matching mutualist does not lose cycles") {
        REQUIRE(mutualist->GetHardware().GetCPUState().GetCPUCyclesSinceRepro() == expected_mutualist_cycles);
      }
    }
  }
}

TEST_CASE("Matching mutualists do not donate cycles to health hosts when HEALTH_INTERACTION_CHANCE is 0", "[sgp][sgp-functional]") {
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
  using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;
  GIVEN("A health host infected by a matching mutualist") {
    sgpmode::SymConfigSGP config;
    ConfigureHealthTestConfig(config);
    config.HEALTH_TYPE("mutualist");
    config.HEALTH_INTERACTION_CHANCE(0);
    size_t cycles_per_update = 4;

    emp::Random random(config.SEED());

    world_t world(random, &config);
    world.Setup();

    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, world.GetProgramBuilder().CreateNotProgram(100));
    emp::Ptr<sgp_sym_t> mutualist = emp::NewPtr<sgp_sym_t> (&random, &world, &config, world.GetProgramBuilder().CreateNotProgram(100));

    world.AddOrgAt(host, 0);
    host->AddSymbiont(mutualist);

    host->GetHardware().GetCPUState().MarkTaskPerformed(0);
    mutualist->GetHardware().GetCPUState().MarkTaskPerformed(0);

    WHEN("A matching mutualist never donates 50% of its cycles") {
      int repeats = 25;
      for (int i = 0; i < repeats; i++) {
        host->GetHardware().GetCPUState().SetLocation(0);
        host->GetHardware().GetCPUState().GainCPUCycles(cycles_per_update);
        host->Process(0);
      }

      double expected_host_cycles = cycles_per_update * repeats;
      double expected_mutualist_cycles = cycles_per_update * repeats;

      THEN("The host does not gain cycles") {
        REQUIRE(host->GetHardware().GetCPUState().GetCPUCyclesSinceRepro() == expected_host_cycles);
      }

      THEN("The matching mutualist does not lose cycles") {
        REQUIRE(mutualist->GetHardware().GetCPUState().GetCPUCyclesSinceRepro() == expected_mutualist_cycles);
      }
    }
  }
}

TEST_CASE("Health hosts evolve", "[sgp][sgp-functional][health-mode-evolution]") {
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  sgpmode::SymConfigSGP config;
  config.SEED(32);
  config.START_MOI(0);
  config.SPATIAL_STRUCT_MODE("well-mixed");
  config.WORLD_WIDTH(10);
  config.WORLD_HEIGHT(100);
  config.HOST_REPRO_RES(20);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.TASK_PROFILE_COMPATIBILITY_MODE("task-any-match");
  config.TASK_PROFILE_MODE("self-all");
  config.CYCLES_PER_UPDATE(4);
  config.ENABLE_HEALTH(1);
  size_t world_size = config.WORLD_WIDTH() * config.WORLD_HEIGHT();

  emp::Random random(config.SEED());
  world_t world(random, &config);

  size_t no_mut_NOT_rate_per_100 = 3400; // 3400 NOTs per 100 updates from empirical runs.
  size_t run_updates = 3000; // The mutation population initially dips and then recovers around 2k updates.

  WHEN("Mutation size is 0") {
    config.SGP_MUT_PER_BIT_RATE(0);
    world.Setup();
    REQUIRE(world.GetNumOrgs() > 0);
    size_t not_task_id = world.GetTaskEnv().GetTaskSet().GetID("NOT");
    size_t total_NOTs = 0;
    for (size_t i = 0; i < run_updates; i++) {
      REQUIRE(world.GetNumOrgs() > 0);
      world.Update();
      if (i >= (run_updates - 100)) {
        // Only considering last 100 updates to make it easier to compare reasonable numbers
        total_NOTs += world.GetHostTaskSuccesses().at(not_task_id);
      }
    }
    THEN("Health hosts do not accrue mutations late in an experiment") {
      REQUIRE(world.GetNumOrgs() == world_size);
      REQUIRE(total_NOTs > no_mut_NOT_rate_per_100 - no_mut_NOT_rate_per_100*0.25);
      REQUIRE(total_NOTs < no_mut_NOT_rate_per_100 + no_mut_NOT_rate_per_100*0.25);
    }
  }

  WHEN("Mutation size is greater than 0") {
    config.SGP_MUT_PER_BIT_RATE(0.001);
    world.Setup();
    size_t not_task_id = world.GetTaskEnv().GetTaskSet().GetID("NOT");
    size_t total_NOTs = 0;

    for (size_t i = 0; i < run_updates; i++) {
      world.Update();
      if (i >= (run_updates-100)) {
        //Only considering last 100 updates to make it easier to compare reasonable numbers
        total_NOTs += world.GetHostTaskSuccesses().at(not_task_id);
      }
    }
    THEN("Health hosts accrue more mutations late in an experiment") {
      REQUIRE(world.GetNumOrgs() == world_size);
      REQUIRE(total_NOTs > no_mut_NOT_rate_per_100 * 3);
    }
  }
}