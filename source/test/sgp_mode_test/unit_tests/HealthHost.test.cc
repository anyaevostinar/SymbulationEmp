#include "../../../sgp_mode/GenomeLibrary.h"
#include "../../../sgp_mode/CPU.h"
#include "../../../sgp_mode/HealthHost.h"
#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"
#include "../../../sgp_mode/SGPConfigSetup.h"
#include "../../../sgp_mode/SGPHost.cc"
#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPSymbiont.h"
#include "../../../default_mode/WorldSetup.cc"
#include "../../../default_mode/DataNodes.h"
#include "../../../sgp_mode/SGPDataNodes.h"

/**
 * This file is dedicated to unit tests for HealthHost
 */

TEST_CASE("Health host with symbiont loses/gains cycle 50% of time", "[sgp][sgp-unit]") {
  GIVEN("A health host"){
    emp::Random random(10);
    //TODO: The random number seed doesn't seem to be working, different values for the same seed

    SymConfigSGP config;
    config.SEED(10);
    config.INTERACTION_MECHANISM(HEALTH);
    config.LIMITED_RES_TOTAL(10);
    config.LIMITED_RES_INFLOW(500);
    config.VERTICAL_TRANSMISSION(0);
    config.HOST_REPRO_RES(100);
    config.SYM_HORIZ_TRANS_RES(10);
    config.SYNERGY(1);
    config.OUSTING(1);

    SGPWorld world(random, &config, LogicTasks);

    emp::Ptr<HealthHost> host = emp::NewPtr<HealthHost>(&random, &world, &config, CreateNotProgram(100));
    
    WHEN("the host is infected with a parasite"){
      config.SYMBIONT_TYPE(PARASITE);
      config.START_MOI(1);

      emp::Ptr<SGPSymbiont> parasite_symbiont = emp::NewPtr<SGPSymbiont> (&random, &world, &config);

      host->AddSymbiont(parasite_symbiont);
      world.AddOrgAt(host, 0);

      int total_times_skipped_cycle = 0;
      int repeats = 25;
      for (int i = 0; i < repeats; i++) {
        size_t initial_stack_location = host->GetCPU().GetCPUPointer().GetCore(0).GetProgramCounter();
        
        host->Process(0);
        size_t new_stack_location = host->GetCPU().GetCPUPointer().GetCore(0).GetProgramCounter();
        
        if(initial_stack_location == new_stack_location) {
          total_times_skipped_cycle++;
        }
      }
      THEN("the host loses around half of it's cycles to parasite"){
        REQUIRE((double)total_times_skipped_cycle/repeats <= 0.55);
        REQUIRE((double)total_times_skipped_cycle/repeats >= 0.45);
      }
    }

    WHEN("the host is not infected"){
      config.START_MOI(0);
      world.AddOrgAt(host, 0);

      int total_times_skipped_cycle = 0;
      int repeats = 25;
      for (int i = 0; i < repeats; i++) {
        size_t initial_stack_location = host->GetCPU().GetCPUPointer().GetCore(0).GetProgramCounter();
        
        host->Process(0);
        size_t new_stack_location = host->GetCPU().GetCPUPointer().GetCore(0).GetProgramCounter();
        
        if(initial_stack_location == new_stack_location) {
          total_times_skipped_cycle++;
        }
      }

      int total_times_gained_cycles = 0;
      for (int i = 0; i < repeats; i++) {
        size_t initial_stack_location = host->GetCPU().GetCPUPointer().GetCore(0).GetProgramCounter();
        
        host->Process(0);
        size_t new_stack_location = host->GetCPU().GetCPUPointer().GetCore(0).GetProgramCounter();
        
        if(new_stack_location - initial_stack_location == 8) {
          total_times_gained_cycles++;
        }
      }
      THEN("the host does not gain or lose cycles"){
        REQUIRE((double)total_times_skipped_cycle/repeats == 0);
        REQUIRE((double)total_times_gained_cycles/repeats == 0);
      }
    }

    WHEN("the host is infected with a mutualist"){
      config.START_MOI(1);
      config.SYMBIONT_TYPE(MUTUALIST);

      emp::Ptr<SGPSymbiont> mutualist_symbiont = emp::NewPtr<SGPSymbiont> (&random, &world, &config);

      host->AddSymbiont(mutualist_symbiont);
      world.AddOrgAt(host, 0);

      int total_times_gained_cycles = 0;
      int repeats = 25;
      for (int i = 0; i < repeats; i++) {
        size_t initial_stack_location = host->GetCPU().GetCPUPointer().GetCore(0).GetProgramCounter();
        
        host->Process(0);
        size_t new_stack_location = host->GetCPU().GetCPUPointer().GetCore(0).GetProgramCounter();
        
        if(new_stack_location - initial_stack_location == 8) {
          total_times_gained_cycles++;
        }
      }
      
      THEN("the host gains around half of the mutualists' cycles"){
        REQUIRE((double)total_times_gained_cycles/repeats <= 0.55);
        REQUIRE((double)total_times_gained_cycles/repeats >= 0.45);
      }
    }
  }
}

