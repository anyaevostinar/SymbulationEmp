
#include "../ConfigSetup.h"
#include "../default_mode/DataNodes.h"
#include "../default_mode/Host.h"
#include "../default_mode/Symbiont.h"

#include "../sgp_mode/hardware/SGPHardwareSpec.h"
#include "../sgp_mode/SGPConfigSetup.h"
#include "../sgp_mode/SGPWorld.h"
#include "../antibiotic-mode/Human.h"
#include "../antibiotic-mode/AntibioticConfigSetup.h"
#include "symbulation.h"

// Empirical doesn't support more than one translation unit, so any CC files are
// included last. It still fixes include issues, but doesn't improve build time.
#include "../default_mode/WorldSetup.cc"
#include "../sgp_mode/SGPWorldSetup.cc"
#include "../../Empirical/include/emp/config/ArgManager.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "../default_mode/WorldSetup.cc"
#include "../sgp_mode/SGPWorld.cc"
#include "../sgp_mode/SGPWorldSetup.cc"
#include "../sgp_mode/SGPWorldData.cc"
#include "../sgp_mode/SGPW_InteractionMechanismSetup.cc"
#include "../sgp_mode/SGPW_TaskProfileSetup.cc"

// This is the main function for the NATIVE version of this project.

int symbulation_main(int argc, char *argv[]) {

  antibioticmode::AntibioticConfig config;
  CheckConfigFile(config, argc, argv);
  config.LIMITED_RES_TOTAL(10);
  config.LIMITED_RES_INFLOW(10); //TODO: go change these defaults in the custom config?
  config.WORLD_WIDTH(10);
  config.WORLD_HEIGHT(10);


  emp::Random random(config.SEED());

  antibioticmode::Human human(random, &config);

  using sgp_host_t = sgpmode::SGPWorld::sgp_host_t;
  using sgp_sym_t = sgpmode::SGPWorld::sgp_sym_t;
  emp::Ptr<sgpmode::SGPWorld> human_micro = human.GetMicrobiome();

      fun_sym_do_birth = [this](
      emp::Ptr<sgp_sym_t> sym_baby_ptr,
      const emp::WorldPosition& parent_pos
    ) -> emp::WorldPosition {
      return FreeLivingSymDoBirth(sym_baby_ptr, parent_pos);
    }
  human_micro->SetCheckSymIndependentReproReqsFunctor([human_micro](
    emp::Ptr<sgp_host_t> host, 
    emp::Ptr<sgp_sym_t> symbiont) {
    //Hosts pay the cost instead of symbionts
    return host->GetPoints() >= 1; // TODO: change to using config that is captured
  });

  human_micro->SetPaySymIndependentReproCostFunctor([](sgp_sym_t& symbiont) {
    //Hosts pay the cost instead of symbionts
    symbiont.GetHost()->AddPoints(-1);
  });

  human_micro->SetApplySymOutputPointsFunctor([](sgp_sym_t& symbiont, double total_points) {
    symbiont.GetHost()->AddPoints(-total_points); //Hosts pays the cost of the task
  });





  return 0;
}

/*
This definition guard prevents main from being defined twice during testing.
In testing, Catch will define a main function which will initiate tests
(including testing the symbulation_main function above).
*/
#ifndef CATCH_CONFIG_MAIN
int main(int argc, char *argv[]) { return symbulation_main(argc, argv); }
#endif
