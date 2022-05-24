#include "../default_mode/SymWorld.h"
#include "../default_mode/Symbiont.h"
#include "../default_mode/Host.h"
#include "../default_mode/WorldSetup.cc"
#include "../../Empirical/include/emp/config/ArgManager.hpp"
#include <iostream>
#include "../ConfigSetup.h"
#include "../default_mode/DataNodes.h"

// This is the main function for the NATIVE version of this project.
int symbulation_main(int argc, char * argv[])
{
  SymConfigBase config;

  bool success = config.Read("SymSettings.cfg");
  if(!success) {
    std::cout << "You didn't have a SymSettings.cfg, so one is being written, please try again" << std::endl;
    config.Write("SymSettings.cfg");
  }

  auto args = emp::cl::ArgManager(argc, argv);
  if (args.ProcessConfigOptions(config, std::cout, "SymSettings.cfg") == false) {
    std::cerr << "There was a problem in processing the options file." << std::endl;
    exit(1);
  }
  if (args.TestUnknown() == false) {
    std::cerr << "Leftover args no good." << std::endl;
    exit(1);
  }

  config.Write(std::cout);
  emp::Random random(config.SEED());

  SymWorld world(random);


  int TIMING_REPEAT = config.DATA_INT();


  //Set up files
  //world.SetupPopulationFile().SetTimingRepeat(TIMING_REPEAT);

  std::string file_ending = "_SEED"+std::to_string(config.SEED())+".data";

  world.SetupHostIntValFile(config.FILE_PATH()+"HostVals"+config.FILE_NAME()+file_ending).SetTimingRepeat(TIMING_REPEAT);
  world.SetupSymIntValFile(config.FILE_PATH()+"SymVals"+config.FILE_NAME()+file_ending).SetTimingRepeat(TIMING_REPEAT);

  if(config.FREE_LIVING_SYMS() == 1){
    world.SetUpFreeLivingSymFile(config.FILE_PATH()+"FreeLivingSyms_"+config.FILE_NAME()+file_ending).SetTimingRepeat(TIMING_REPEAT);
  }

  worldSetup(&world, &config);

  //Loop through updates
  int numupdates = config.UPDATES();
  for (int i = 0; i < numupdates; i++) {
    if((i%TIMING_REPEAT)==0) {
      std::cout <<"Update: "<< i << std::endl;
      std::cout.flush();
    }
    world.Update();
  }

  config.MUTATION_RATE(0);
  config.MUTATION_SIZE(0);
  config.HOST_MUTATION_SIZE(0);
  config.HOST_MUTATION_RATE(0);
  config.MUTATE_LYSIS_CHANCE(0);
  config.MUTATE_INDUCTION_CHANCE(0);
  config.MUTATE_INC_VAL(0);

  int num_no_mut_updates = config.NO_MUT_UPDATES();
  for (int i = 0; i < num_no_mut_updates; i++) {
    if((i%TIMING_REPEAT)==0) {
      std::cout <<"No mutation update: "<< i << std::endl;
      std::cout.flush();
    }
    world.Update();
  }

  //retrieve the dominant taxons for each organism and write them to a file
  if(config.PHYLOGENY() == 1){
    world.WritePhylogenyFile(config.FILE_PATH()+"Phylogeny_"+config.FILE_NAME()+file_ending);
  }
  return 0;
}

/*
This definition guard prevents main from being defined twice during testing.
In testing, Catch will define a main function which will initiate tests
(including testing the symbulation_main function above).
*/
#ifndef CATCH_CONFIG_MAIN
int main(int argc, char * argv[]) {
  return symbulation_main(argc, argv);
}
#endif
