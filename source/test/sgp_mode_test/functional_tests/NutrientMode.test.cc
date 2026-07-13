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

//Note, this can't be easily combined with the previous test, because the functor is created at setup, so you can't change the nutrient type on the fly, is that okay for our long term plans?
TEST_CASE("SGPSymbiont Normal Nutrient without multiplier", "[sgp][sgp-functional]") {
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
  using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;

  GIVEN("An SGPWorld with a host infected with a symbiont where Stress is the interaction mechanism") {
    emp::Random random(42);
    sgpmode::SymConfigSGP config;
    config.ENABLE_NUTRIENT(true);
    config.NUTRIENT_DONATE_PROP(0.5);
    config.NUTRIENT_STEAL_PROP(0.5);
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.TASK_IO_BANK_SIZE(10);
    config.NUTRIENT_INTERACTION_MULTIPLIER(1.0); // for testing, set multiplier to 1 so we can directly compare expected transfer to actual transfer
    test_utils::SetWellMixed(config, 1, 0);
    world_t world(random, &config);
    double initial_points = 10.0;
    double sym_score = 8.0;

    GIVEN("All Symbionts are Parasites") {
      config.NUTRIENT_TYPE("parasite");
      world.Setup();
      auto& builder = world.GetProgramBuilder();

      emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, builder.CreateNotProgram(100));
      emp::Ptr<sgp_sym_t> sym = emp::NewPtr<sgp_sym_t>(&random, &world, &config, builder.CreateNotProgram(100));

      world.AddOrgAt(host, 0);
      host->AddSymbiont(sym);

      // Set them both to having done the NOT task so they will interact
      const size_t not_task_id = world.GetTaskEnv().GetTaskSet().GetID("NOT");
      host->GetHardware().GetCPUState().MarkTaskPerformed(not_task_id);
      sym->GetHardware().GetCPUState().MarkTaskPerformed(not_task_id);

      double expected_transfer = config.NUTRIENT_STEAL_PROP() * sym_score;

      WHEN("Parasite Steals from Host") {

        sym->SetPoints(initial_points);
        host->SetPoints(initial_points);
        double host_result = world.CalcHostNutrientInteraction(*host, *sym, 8.0, not_task_id,1);
        double sym_result = world.CalcSymNutrientInteraction(*host, *sym, 8.0, not_task_id,1);

        THEN("Symbiont is set to gain the expected amount") {
          REQUIRE(sym_result == expected_transfer);
        }
        THEN("Host is set to lose the expected amount") {
          REQUIRE(host_result == -expected_transfer);
        }

        world.ApplyHostPoints(*host, 8.0, not_task_id);


        THEN("Symbiont gains the expected amount") {
          REQUIRE(sym->GetPoints() == initial_points+expected_transfer);
        }
        THEN("Host loses the expected amount") {
          REQUIRE(host->GetPoints() == initial_points + 8 + -expected_transfer);
        }
      }

    }


    GIVEN("All Symbionts are Mutualists") {
      config.NUTRIENT_TYPE("mutualist");
      world.Setup();
      auto& builder = world.GetProgramBuilder();

      emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, builder.CreateNotProgram(100));
      emp::Ptr<sgp_sym_t> sym = emp::NewPtr<sgp_sym_t>(&random, &world, &config, builder.CreateNotProgram(100));

      world.AddOrgAt(host, 0);
      host->AddSymbiont(sym);

      // Set them both to having done the NOT task so they will interact
      const size_t not_task_id = world.GetTaskEnv().GetTaskSet().GetID("NOT");
      host->GetHardware().GetCPUState().MarkTaskPerformed(not_task_id);
      sym->GetHardware().GetCPUState().MarkTaskPerformed(not_task_id);


      double expected_transfer = config.NUTRIENT_DONATE_PROP() * sym_score;
      WHEN("A Mutualist symbiont donates to a host") {
        sym->SetPoints(initial_points);
        host->SetPoints(initial_points);
        double host_result = world.CalcHostNutrientInteraction(*host, *sym, 8.0, not_task_id,1);
        double sym_result = world.CalcSymNutrientInteraction(*host, *sym, 8.0, not_task_id,1);

        THEN("Symbiont is set to lose the expected amount") {
          REQUIRE(sym_result == -expected_transfer);
        }
        THEN("Host is set to gain the expected amount") {
          REQUIRE(host_result == expected_transfer);
        }

        world.ApplyHostPoints(*host, 8.0, not_task_id);


        THEN("Symbiont loses the expected amount") {
          REQUIRE(sym->GetPoints() == initial_points-expected_transfer);
        }
        THEN("Host gains the expected amount") {
          REQUIRE(host->GetPoints() == initial_points + 8 + expected_transfer);
        }
      }
    }
  }
}