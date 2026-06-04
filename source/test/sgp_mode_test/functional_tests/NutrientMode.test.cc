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


TEST_CASE("SGPSymbiont Normal Nutrient Parasite without multiplier", "[sgp][sgp-functional]") {
  // TODO: move to functional folder
  GIVEN("An SGPWorld with a host infected with a symbiont where Nutrient is the interaction mechanism"){
    emp::Random random(42);
    sgpmode::SymConfigSGP config;
    config.ENABLE_NUTRIENT(true);
    config.NUTRIENT_DONATE_PROP(0.5);
    config.NUTRIENT_STEAL_PROP(0.5);
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.NUTRIENT_TYPE("parasite");
    config.NUTRIENT_INTERACTION_MULTIPLIER(1.0); 
    config.PARASITE_BASE_TASK_VALUE_PROP(1.0); 
    world_t world(random, &config);
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

    double initial_points = 10.0;
    double sym_score = 8.0;
    double expected_transfer = config.NUTRIENT_STEAL_PROP() * sym_score; 
    


    WHEN("Parasite steals from host") {
      
      sym->SetPoints(initial_points);
      host->SetPoints(initial_points);
      double host_result = world.CalcHostNutrientInteraction(*host, *sym, 8.0, not_task_id,1);
      double sym_result = world.CalcSymNutrientInteraction(*host, *sym, 8.0, not_task_id,1);
      
      

      THEN("Symbiont keep the expected score and host receives the donate amount") {
        REQUIRE(host_result == -expected_transfer);
        REQUIRE(sym_result == expected_transfer);
      }
    }
  }
}

//Note, this can't be easily combined with the previous test, because the functor is created at setup, so you can't change the nutrient type on the fly, is that okay for our long term plans?
TEST_CASE("SGPSymbiont Normal Nutrient mutualist without multiplier", "[sgp][sgp-functional]") {
  // TODO: move to functional folder
  GIVEN("An SGPWorld with a host infected with a symbiont where Stress is the interaction mechanism"){
    emp::Random random(42);
    sgpmode::SymConfigSGP config;
    config.ENABLE_NUTRIENT(true);
    config.NUTRIENT_DONATE_PROP(0.5);
    config.NUTRIENT_STEAL_PROP(0.5);
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.NUTRIENT_TYPE("mutualist");
    config.NUTRIENT_INTERACTION_MULTIPLIER(1.0); // for testing, set multiplier to 1 so we can directly compare expected transfer to actual transfer
    world_t world(random, &config);
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

    double initial_points = 10.0;
    double sym_score = 8.0;
    double expected_transfer = config.NUTRIENT_DONATE_PROP() * sym_score;
    WHEN("Mutualist donates to host") {
      config.NUTRIENT_TYPE("mutualist");
      sym->SetPoints(initial_points);
      host->SetPoints(initial_points);
      double host_result = world.CalcHostNutrientInteraction(*host, *sym, 8.0, not_task_id,1);
      double sym_result = world.CalcSymNutrientInteraction(*host, *sym, 8.0, not_task_id,1);
      
      

      THEN("Symbiont keep the expected score and host receives the donate amount") {
        REQUIRE(host_result == expected_transfer);
        REQUIRE(sym_result == -expected_transfer);
      }
    }
  }
}