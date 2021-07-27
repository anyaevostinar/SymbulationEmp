#include "../SymWorld.h"
#include "../Phage.h"
#include "../Symbiont.h"
#include "../Host.h"
#include"../Pggsym.h"
#include"../Pgghost.h"
#include "../../../Empirical/include/emp/config/ArgManager.hpp"
#include <iostream>
#include "../WorldSetup.cc"
#include "../ConfigSetup.h"

using namespace std;

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
    cerr << "There was a problem in processing the options file." << endl;
    exit(1);
  }
  if (args.TestUnknown() == false) {
    cerr << "Leftover args no good." << endl;
    exit(1);
  }
  if (config.BURST_SIZE()%config.BURST_TIME() != 0 && config.BURST_SIZE() < 999999999) {
  	cerr << "BURST_SIZE must be an integer multiple of BURST_TIME." << endl;
  	exit(1);
  }

  config.Write(std::cout);
  emp::Random random(config.SEED());
      
  SymWorld world(random);
  

  int TIMING_REPEAT = config.DATA_INT();
  

  //Set up files
  //world.SetupPopulationFile().SetTimingRepeat(TIMING_REPEAT);

  if (config.LYSIS() == 1) {
    world.SetupLysisFile(config.FILE_PATH()+"Lysis"+config.FILE_NAME()+".data").SetTimingRepeat(TIMING_REPEAT);
    world.SetupLysisChanceFile(config.FILE_PATH()+"LysisChance_"+config.FILE_NAME()+".data").SetTimingRepeat(TIMING_REPEAT);
  }else if(config.PGG() == 1){
    world.SetupPGGSymIntValFile(config.FILE_PATH()+"PGGSymVals"+config.FILE_NAME()+".data").SetTimingRepeat(TIMING_REPEAT);
  }
  world.SetupSymIntValFile(config.FILE_PATH()+"SymVals"+config.FILE_NAME()+".data").SetTimingRepeat(TIMING_REPEAT);
  world.SetupHostIntValFile(config.FILE_PATH()+"HostVals"+config.FILE_NAME()+".data").SetTimingRepeat(TIMING_REPEAT);
  if (config.EFFICIENT_SYM() == 1) {
    world.SetupEfficiencyFile(config.FILE_PATH()+"Efficiency"+config.FILE_NAME()+".data").SetTimingRepeat(TIMING_REPEAT);
  }

  worldSetup(&world, &config);
  int numupdates = config.UPDATES();

  //Loop through updates
  for (int i = 0; i < numupdates; i++) {
    if((i%TIMING_REPEAT)==0) {
      cout <<"Update: "<< i << endl;
      cout.flush();
    }
    world.Update();
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
