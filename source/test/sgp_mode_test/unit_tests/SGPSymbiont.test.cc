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

/**
 * This file is dedicated to unit tests for SGPSymbiont
 */

 TEST_CASE("Symbiont == operator", "[sgp][sgp-unit]"){
  GIVEN("A symbiont"){
    emp::Random random(31);
    sgpmode::SymConfigSGP config;
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    world_t world(random, &config);
    auto& prog_builder = world.GetProgramBuilder();
    
    emp::Ptr<sgp_sym_t> sym_parent = emp::NewPtr<sgp_sym_t>(&random, &world, &config, prog_builder.CreateNotProgram(100));
    WHEN("2 symbionts that are clones of the original symbiont"){
        emp::Ptr<sgp_sym_t> clone1 = emp::NewPtr<sgp_sym_t>(*sym_parent);
        emp::Ptr<sgp_sym_t> clone2 = emp::NewPtr<sgp_sym_t>(*sym_parent);
        
        THEN("symbiont is equal to first clone"){
          REQUIRE(*sym_parent == *clone1);
        }
        THEN("The first clone is equal to the second clone"){
          REQUIRE(*clone1 == *clone2);
        }
    }
    
    WHEN("A host that is different to the original is created"){
      emp::Ptr<sgp_sym_t> different = emp::NewPtr<sgp_sym_t>(&random, &world, &config, prog_builder.CreateNotProgram(99)); // For comparing
      THEN("The original symbiont is not equal to the different host"){
        REQUIRE_FALSE(*sym_parent == *different);
      }
    }
  }
}

TEST_CASE("Symbiont > & < operator","[sgp][sgp-unit]"){
  GIVEN("Two different symbionts"){
    emp::Random random(31);
    sgpmode::SymConfigSGP config;
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    world_t world(random, &config);
    auto& prog_builder = world.GetProgramBuilder();

    emp::Ptr<sgp_sym_t> sym_parent = emp::NewPtr<sgp_sym_t>(&random, &world, &config, prog_builder.CreateNotProgram(100));
    emp::Ptr<sgp_sym_t> different = emp::NewPtr<sgp_sym_t>(&random, &world, &config, prog_builder.CreateNotProgram(99)); // For comparing
    WHEN("The two symbionts are compared"){
      THEN("One symbiont is less then the other symbiont"){
        // Can't assert true/false without knowing bitcode ordering,
        // assert that bitcode ordering is well-defined
        bool lt = *sym_parent < *different || *different < *sym_parent;
        REQUIRE(lt);
      }
    }
  }
}

TEST_CASE("SGPSymbiont Normal Nutrient Parasite without multiplier", "[sgp][sgp-functional]") {
  // TODO: move to functional folder
  GIVEN("An SGPWorld with a host infected with a symbiont where Stress is the interaction mechanism"){
    emp::Random random(42);
    sgpmode::SymConfigSGP config;
    config.ENABLE_NUTRIENT(true);
    config.NUTRIENT_DONATE_PROP(0.5);
    config.NUTRIENT_STEAL_PROP(0.5);
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.NUTRIENT_TYPE("parasite");
    config.NUTRIENT_INTERACTION_MULTIPLIER(7.0); 
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

    std::cout << "not task id" << not_task_id << std::endl;
    double initial_host_points = 10.0;
    double sym_score = 8.0;
    double expected_transfer = config.NUTRIENT_STEAL_PROP() * sym_score; 



    WHEN("Parasite steals from host") {
      host->SetPoints(initial_host_points);
      double result = world.ApplyNutrientInteraction(*sym, 8.0, not_task_id);
      // = sym->DoTaskInteraction(sym_score, 0);

      THEN("Symbiont receives expected amount and host loses the amount") {
        REQUIRE(result == (sym_score* config.NUTRIENT_INTERACTION_MULTIPLIER()));
        REQUIRE(host->GetPoints() == initial_host_points - expected_transfer);
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

    double initial_host_points = 10.0;
    double sym_score = 8.0;
    double expected_transfer = config.NUTRIENT_DONATE_PROP() * sym_score;
    WHEN("Mutualist donates to host") {
      config.NUTRIENT_TYPE("mutualist");
      host->SetPoints(initial_host_points);
      double result = world.ApplyNutrientInteraction(*sym, 8.0, not_task_id);


      double expected_score_remain = sym_score - expected_transfer;

      THEN("Symbiont keep the expected score and host receives the donate amount") {
        REQUIRE(result == expected_score_remain);
        REQUIRE(host->GetPoints() == initial_host_points + expected_transfer);
      }
    }
  }
}

// TEST_CASE("SGPSymbiont destructor cleans up shared pointers and in-progress reproduction", "[sgp][sgp-unit]") {
//   GIVEN("A symbiont"){
//     emp::Random random(31);
//     SymConfigSGP config;
//     SGPWorld world(random, &config, LogicTasks);
//     emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(100));
//     sym->SetLocation(emp::WorldPosition(1, 2));
//     sym->GetCPU().state.in_progress_repro = 3;
//     world.to_reproduce.resize(5); 

//     WHEN("The symbiont is destroyed") {
//       sym.Delete(); 
      
//       THEN("Reproduction queue is invalidated after the symbiont is destroyed") {
//         REQUIRE(world.to_reproduce[3] == nullptr);
//       }
//     }
//   }
// }
