#define UIT_VENDORIZE_EMP
#define UIT_SUPPRESS_MACRO_INSEEP_WARNINGS

#include <iostream>
#include "../sgp_mode/SGPWorld.h"
#include "../ConfigSetup.h"
#include "../default_mode/DataNodes.h"
#include "../default_mode/Host.h"
#include "../default_mode/Symbiont.h"

#include "../sgp_mode/hardware/SGPHardwareSpec.h"
#include "../sgp_mode/SGPConfigSetup.h"
#include "../sgp_mode/SGPWorld.h"

// Empirical doesn't support more than one translation unit, so any CC files are
// included last. It still fixes include issues, but doesn't improve build time.
#include "../default_mode/WorldSetup.cc"
#include "../sgp_mode/SGPWorldSetup.cc"
#include "../../Empirical/include/emp/config/ArgManager.hpp"

#include <fstream>
#include <memory>
#include <string>

#include "../default_mode/WorldSetup.cc"
#include "../sgp_mode/SGPWorld.cc"
#include "../sgp_mode/SGPWorldSetup.cc"
#include "../sgp_mode/SGPWorldData.cc"
#include "../sgp_mode/SGPW_InteractionMechanismSetup.cc"
#include "../sgp_mode/SGPW_TaskProfileSetup.cc"

#include "../../Empirical/include/emp/config/ArgManager.hpp"
#include "../sgp_mode/web/SGPSymAnimate.h"
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
