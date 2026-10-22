#define UIT_VENDORIZE_EMP
#define UIT_SUPPRESS_MACRO_INSEEP_WARNINGS

#include <iostream>
#include "../source/sgp_mode/SGPWorld.h"
#include "../source/ConfigSetup.h"
#include "../source/default_mode/DataNodes.h"
#include "../source/default_mode/Host.h"
#include "../source/default_mode/Symbiont.h"

#include "../source/sgp_mode/hardware/SGPHardwareSpec.h"
#include "../source/sgp_mode/SGPConfigSetup.h"
#include "../source/sgp_mode/SGPWorld.h"

// Empirical doesn't support more than one translation unit, so any CC files are
// included last. It still fixes include issues, but doesn't improve build time.
#include "../source/default_mode/WorldSetup.cc"
#include "../source/sgp_mode/SGPWorldSetup.cc"
#include "../Empirical/include/emp/config/ArgManager.hpp"

#include <fstream>
#include <memory>
#include <string>

#include "../source/default_mode/WorldSetup.cc"
#include "../source/sgp_mode/SGPWorld.cc"
#include "../source/sgp_mode/SGPWorldSetup.cc"
#include "../source/sgp_mode/SGPWorldData.cc"
#include "../source/sgp_mode/SGPW_InteractionMechanismSetup.cc"
#include "../source/sgp_mode/SGPW_TaskProfileSetup.cc"

#include "../Empirical/include/emp/config/ArgManager.hpp"
#include "../web/SGPSymAnimate.h"
//#include "../SymConfig.h"

using namespace std;

namespace UI = emp::web;
SGPSymAnimate anim;

int symbulation_main(int argc, char * argv[]){
    return 0;
}

#ifndef CATCH_CONFIG_MAIN
int main(int argc, char * argv[]) {
  return symbulation_main(argc, argv);
}
#endif
