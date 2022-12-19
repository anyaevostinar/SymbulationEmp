#include "../efficient_mode/EfficientWorld.h"
#include "symbulation.h"

#include "../Organism.cc"
#include "../default_mode/Symbiont.cc"
#include "../default_mode/WorldSetup.cc"
#include "../efficient_mode/EfficientWorldSetup.cc"

// This is the main function for the NATIVE version of this project.

int symbulation_main(int argc, char * argv[])
{
  SymConfigBase config;
  CheckConfigFile(config, argc, argv);

  config.Write(std::cout);
  emp::Random random(config.SEED());

  EfficientWorld world(random, &config);

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
