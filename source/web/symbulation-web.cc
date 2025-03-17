#define UIT_VENDORIZE_EMP
#define UIT_SUPPRESS_MACRO_INSEEP_WARNINGS

#include <iostream>
#include "../default_mode/SymWorld.h"
#include "../../Empirical/include/emp/config/ArgManager.hpp"
#include "../SymAnimate.h"
//#include "../SymConfig.h"

using namespace std;

namespace UI = emp::web;
SymAnimate anim;

int symbulation_main(int argc, char * argv[]){
    return 0;
}

#ifndef CATCH_CONFIG_MAIN
int main(int argc, char * argv[]) {
  return symbulation_main(argc, argv);
}
#endif
