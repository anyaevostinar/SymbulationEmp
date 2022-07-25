#include "../../Empirical/include/emp/config/ArgManager.hpp"
#include "../../Empirical/include/emp/config/config.hpp"
#include <iostream>
#include "../ConfigSetup.h"

/**
 * Input: The SymConfig object and the command line arguments.
 *
 * Output: None
 *
 * Purpose: To validate the passed config settings and throw appropriate error messages.
 */
void CheckConfigFile(SymConfigBase& config, int argc, char * argv[]){
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
}
