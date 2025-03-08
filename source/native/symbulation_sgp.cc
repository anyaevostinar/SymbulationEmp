
#include "../ConfigSetup.h"
#include "../default_mode/DataNodes.h"
#include "../default_mode/Host.h"
#include "../default_mode/Symbiont.h"

#include "../sgp_mode/hardware/SGPHardwareSpec.h"
#include "../sgp_mode/SGPConfigSetup.h"
#include "../sgp_mode/SGPWorld.h"

#include "symbulation.h"

#include "../../Empirical/include/emp/config/ArgManager.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "../default_mode/WorldSetup.cc"
#include "../sgp_mode/SGPWorld.cc"
#include "../sgp_mode/SGPWorldSetup.cc"
#include "../sgp_mode/SGPWorldData.cc"

// This is the main function for the NATIVE version of this project.

int symbulation_main(int argc, char *argv[]) {
  sgpmode::SymConfigSGP config;
  CheckConfigFile(config, argc, argv);

  emp::Random random(config.SEED());

  sgpmode::SGPWorld world(random, &config);
  world.Setup();
  world.Run();

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
