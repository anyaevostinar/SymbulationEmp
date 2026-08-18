
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
#include <filesystem>

#include "../default_mode/WorldSetup.cc"
#include "../sgp_mode/SGPWorld.cc"
#include "../sgp_mode/SGPWorldSetup.cc"
#include "../sgp_mode/SGPWorldData.cc"
#include "../sgp_mode/SGPW_InteractionMechanismSetup.cc"
#include "../sgp_mode/SGPW_TaskProfileSetup.cc"

// This is the main function for the NATIVE version of this project.
using sgp_host_t = sgpmode::SGPWorld::sgp_host_t;
using sgp_sym_t = sgpmode::SGPWorld::sgp_sym_t;

void setup_functors(antibioticmode::AntibioticConfig& config, emp::Ptr<sgpmode::SGPWorld> human_micro) {
  human_micro->SetCheckSymIndependentReproReqsFunctor([human_micro](
    Organism& symbiont) {
    //Hosts pay the cost instead of symbionts
    return symbiont.GetHost()->GetPoints() >= 1; // TODO: change to using config that is captured
  });

  human_micro->SetPaySymIndependentReproCostFunctor([](Organism& symbiont) {
    //Hosts pay the cost instead of symbionts
    symbiont.GetHost()->AddPoints(-0.5);
  });

  human_micro->SetApplySymOutputPointsFunctor([](sgp_sym_t& symbiont, double total_points) {
    symbiont.GetHost()->AddPoints(-0.1); //Hosts pays the cost of the task
  });

  human_micro->AddBeforeHostCPUExecSig([human_micro](sgp_host_t& host) {
      if (!human_micro->IsStressExtinctionUpdate()) {
        return;
      }
      // If host has a mutualist symbiont with a matching task profile, death_chance = mutualist death chance
      // Otherwise, base death chance.
      auto& endosymbionts = host.GetSymbionts();
      bool resistant = false;
      for (size_t sym_i = 0; sym_i < endosymbionts.size(); ++sym_i) {
        // Check if symbiont matches task profile
        emp::Ptr<sgp_sym_t> endosym_ptr = static_cast<sgp_sym_t*>(endosymbionts[sym_i].Raw());
        resistant = endosym_ptr->GetHardware().GetCPUState().GetTaskPerformed(human_micro->GetTaskEnv().GetTaskSet().GetID("NAND")); //NAND only for now
        if (resistant) {
          break;
        }
      }
      const double death_chance = (resistant) ?
      human_micro->GetConfig().MUTUALIST_DEATH_CHANCE() :
      human_micro->GetConfig().BASE_DEATH_CHANCE();
      // Kill host with chosen probability
      if (human_micro->GetRandom().P(death_chance)) {
        host.SetDead();
      } 
    });
}

int symbulation_main(int argc, char *argv[]) {

  antibioticmode::AntibioticConfig config;
  CheckConfigFile(config, argc, argv);


  emp::Random random(config.SEED());

  antibioticmode::Human human(random, &config);

  emp::Ptr<sgpmode::SGPWorld> human_micro = human.GetMicrobiome();

  setup_functors(config, human_micro);

  if (config.TRACK_R0() == 1) {
    emp_assert(config.PHYLOGENY() == 1, "Cannot track R0 without tracking phylogeny.");
    emp_assert(config.PHYLOGENY_TAXON_TYPE() == "horizontal-clade", "Need horizontal clade tracking to track R0.");
    emp_assert(config.STORE_EXTINCT() == 1, "R0 will be biased if we don't store extinct");
    std::filesystem::path output_dir = config.FILE_PATH();
    std::filesystem::path antibiotic_resistance_fpath = output_dir / ("AntibioticResistance"+config.FILE_NAME()+".csv");    
    human_micro->SetupAntibioticResistanceFile(antibiotic_resistance_fpath.string()).SetTimingRepeat(config.DATA_INT());  
  }

  human_micro->Run(true);

  // std::string file_ending = "_SEED" + std::to_string(config.SEED()) + ".data";
  // if(config.PHYLOGENY() == 1){
  //   human_micro->WritePhylogenyFile(config.FILE_PATH()+"Phylogeny_"+config.FILE_NAME()+file_ending);
  // }
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
