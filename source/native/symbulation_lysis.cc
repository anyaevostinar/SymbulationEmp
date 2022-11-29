#include "../lysis_mode/LysisWorld.h"
#include "symbulation.h"

#include "../Organism.cc"
#include "../default_mode/Symbiont.cc"
#include "../default_mode/WorldSetup.cc"
#include "../default_mode/WorldSetup.cc"
#include "../lysis_mode/LysisWorldSetup.cc"
#include "../lysis_mode/LysisDataNodes.cc"

/**
 * Input: The SymConfig object and the command line arguments.
 *
 * Output: None
 *
 * Purpose: To validate the passed config settings and throw appropriate error messages,
 * including unique lysis mode checks.
 */
void LysisCheckConfigFile(SymConfigBase& config, int argc, char * argv[]){
  CheckConfigFile(config, argc, argv);
  if (config.BURST_SIZE()%config.BURST_TIME() != 0 && config.BURST_SIZE() < 999999999) {
  	std::cerr << "BURST_SIZE must be an integer multiple of BURST_TIME." << std::endl;
  	exit(1);
  }
}

// This is the main function for the NATIVE version of this project.
int symbulation_main(int argc, char * argv[])
{
  SymConfigBase config;
  LysisCheckConfigFile(config, argc, argv);

  config.Write(std::cout);
  emp::Random random(config.SEED());

  LysisWorld world(random, &config);

  world.Setup();
  world.CreateDataFiles();
  
  world.RunExperiment();

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
