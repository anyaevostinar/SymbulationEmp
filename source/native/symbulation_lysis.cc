#include "../lysis_mode/LysisWorld.h"
#include "../default_mode/WorldSetup.cc"
#include "../lysis_mode/LysisWorldSetup.cc"
#include "symbulation.h"

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

  //retrieve the dominant taxons for each organism and write them to a file
  if(config.PHYLOGENY() == 1){
    std::string file_ending = "_SEED"+std::to_string(config.SEED())+".data";
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
