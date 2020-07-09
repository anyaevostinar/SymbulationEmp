#include <iostream>
#include "../SymWorld.h"
#include "../../Empirical/source/config/ArgManager.h"
#include "../SymAnimate.h"
#include "../SymConfig.h"

using namespace std;

namespace UI = emp::web;
SymConfigBase config;
SymAnimate anim;

int symbulation_main(int argc, char * argv[]){
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
    return 0;
}

#ifndef CATCH_CONFIG_MAIN
int main(int argc, char * argv[]) {
  return symbulation_main(argc, argv);
}
#endif
