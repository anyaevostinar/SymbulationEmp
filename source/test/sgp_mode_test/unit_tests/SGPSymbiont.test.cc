#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"
#include "../../../sgp_mode/SGPDataNodes.h"

/**
 * This file is dedicated to unit tests for SGPSymbiont
 */

// TEST_CASE("SGPSymbiont DoTaskInteraction in nutrient mode", "[sgp][sgp-unit]") {
//   GIVEN("An SGPWorld with a host infected with a symbiont where Stress is the interaction mechanism"){
//     emp::Random random(42);
//     sgp_mode::SymConfigSGP config;
//     config.INTERACTION_MECHANISM(NUTRIENT);
//     config.NUTRIENT_DONATE_STEAL_PROP(0.5);
//     SGPWorld world(random, &config, LogicTasks);

//     ProgramBuilder builder;
//     builder.AddNot(); //First task
//     emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, builder.Build(100));
//     emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, builder.Build(100));

//     world.AddOrgAt(host, 0);
//     host->AddSymbiont(sym);

//     host->GetCPU().state.tasks_performed->Set(0);

//     double initial_host_points = 10.0;
//     double sym_score = 8.0;
//     double expected_transfer = config.NUTRIENT_DONATE_STEAL_PROP() * sym_score;

//     WHEN("Parasite steals from host") {
//       config.SYMBIONT_TYPE(PARASITE);
//       host->SetPoints(initial_host_points);
//       double result = sym->DoTaskInteraction(sym_score, 0);

//       THEN("Symbiont receives expected amount and host loses the amount") {
//         REQUIRE(result == expected_transfer + sym_score);
//         REQUIRE(host->GetPoints() == initial_host_points - expected_transfer);
//       }
//     }

//     WHEN("Mutualist donates to host") {
//       config.SYMBIONT_TYPE(MUTUALIST);
//       host->SetPoints(initial_host_points);
//       double result = sym->DoTaskInteraction(sym_score, 0);

//       double expected_score_remain = sym_score - expected_transfer;

//       THEN("Symbiont keep the expected score and host receives the donate amount") {
//         REQUIRE(result == expected_score_remain);
//         REQUIRE(host->GetPoints() == initial_host_points + expected_transfer);
//       }
//     }
//   }
// }

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

TEST_CASE("Symbiont == operator", "[sgp][sgp-unit]"){
  GIVEN("A symbiont"){
    emp::Random random(31);
    sgp_mode::SymConfigSGP config;
    SGPWorld world(random, &config, LogicTasks);
    
    emp::Ptr<SGPSymbiont> sym_parent = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(100));
    WHEN("2 symbionts that are clones of the original symbiont"){
        emp::Ptr<SGPSymbiont> clone1 = emp::NewPtr<SGPSymbiont>(*sym_parent);
        emp::Ptr<SGPSymbiont> clone2 = emp::NewPtr<SGPSymbiont>(*sym_parent);
        
        THEN("symbiont is equal to first clone"){
          REQUIRE(*sym_parent == *clone1);
        }
        THEN("The first clone is equal to the second clone"){
          REQUIRE(*clone1 == *clone2);
        }
    }
    
    WHEN("A host that is different to the original is created"){
      emp::Ptr<SGPSymbiont> different = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(99)); // For comparing
      THEN("The original symbiont is not equal to the different host"){
        REQUIRE_FALSE(*sym_parent == *different);
      }
    }
  }
}

TEST_CASE("Symbiont > & < operator","[sgp][sgp-unit]"){
  GIVEN("Two different symbionts"){
    emp::Random random(31);
    SymConfigSGP config;
    SGPWorld world(random, &config, LogicTasks);

    emp::Ptr<SGPSymbiont> sym_parent = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(100));
    emp::Ptr<SGPSymbiont> different = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(99)); // For comparing
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