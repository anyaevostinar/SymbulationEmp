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
        clone1.Delete();
        clone2.Delete();
    }
    
    WHEN("A symbiont that is different to the original is created"){
      emp::Ptr<sgp_sym_t> different = emp::NewPtr<sgp_sym_t>(&random, &world, &config, prog_builder.CreateNotProgram(99)); // For comparing
      THEN("The original symbiont is not equal to the different host"){
        REQUIRE_FALSE(*sym_parent == *different);
      }
      different.Delete();
    }
    sym_parent.Delete();
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
    sym_parent.Delete();
    different.Delete();
  }
}

TEST_CASE("Symbiont Process", "[sgp][sgp-unit]") {
  GIVEN("A symbiont that gets 3 cycles per update") {
    emp::Random random(34);
    sgpmode::SymConfigSGP config;
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.CYCLES_PER_UPDATE(3);
    world_t world(random, &config);
    world.Setup();
    auto& prog_builder = world.GetProgramBuilder();

    emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateNotProgram(100));
    emp::Ptr<sgp_sym_t> sym = emp::NewPtr<sgp_sym_t>(&random, &world, &config, prog_builder.CreateNotProgram(100));


    world.AddOrgAt(host, 0);
    host->AddSymbiont(sym);
    sym->GetHardware().GetCPUState().SetCPUCyclesToExec(config.CYCLES_PER_UPDATE()); // this is normally done in host process, but since we're skipping that, doing it manually

    WHEN("Symbiont process called") {
      sym->Process(sym->GetLocation());
      THEN("Symbiont should have executed 3 cycles and aged by 1") {
        REQUIRE(sym->GetHardware().GetCPUState().GetCPUCyclesSinceRepro() == 3);
        REQUIRE(sym->GetAge() == 1);
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
