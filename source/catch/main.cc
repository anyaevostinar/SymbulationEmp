// Let Catch provide main():
#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "sanity_check.test.cc"
#include "../test/SymWorld.test.cc"
#include "../test/DataNodes.test.cc"

#include "../test/default_mode_test/Host.test.cc"
#include "../test/default_mode_test/Symbiont.test.cc"
#include "../test/default_mode_test/HostSymbiontInteraction.test.cc"
#include "../test/default_mode_test/HostSymbiontUnitTest.test.cc"

#include "../test/efficient_mode_test/EfficientSymbiont.test.cc"

#include "../test/lysis_mode_test/Bacterium.test.cc"
#include "../test/lysis_mode_test/Phage.test.cc"
#include "../test/lysis_mode_test/BacteriumPhageUnitTest.test.cc"

#include "../test/pgg_mode_test/Pggsym.test.cc"
#include "../test/pgg_mode_test/Pgghost.test.cc"
#include "../test/pgg_mode_test/PggHostPggSymUnitTest.test.cc"

//#include "../Pggendtoend.test.cc"
//#include "../test/end_to_end.test.cc"
