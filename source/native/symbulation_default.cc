#include "../default_mode/SymWorld.h"
#include "../default_mode/WorldSetup.cc"
#include "../default_mode/DataNodes.h"
#include "symbulation.h"

// This is the main function for the NATIVE version of this project.
int symbulation_main(int argc, char * argv[])
{
  SymConfigBase config;
  CheckConfigFile(config, argc, argv);

  config.Write(std::cout);
  emp::Random random(config.SEED());

  SymWorld world(random, &config);

  worldSetup(&world, &config);
  world.CreateDateFiles();
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
