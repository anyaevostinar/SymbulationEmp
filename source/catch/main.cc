// Let Catch provide main():
#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "sanity_check.test.cc"

#include "../test/default_mode_test/SymWorld.test.cc"
#include "../test/default_mode_test/DataNodes.test.cc"

#include "../test/default_mode_test/Host.test.cc"
#include "../test/default_mode_test/Symbiont.test.cc"
#include "../test/default_mode_test/HostSymbiontInteraction.test.cc"
#include "../test/default_mode_test/HostSymbiontUnitTest.test.cc"

#include "../test/efficient_mode_test/EfficientSymbiont.test.cc"
#include "../test/efficient_mode_test/EfficientHost.test.cc"
#include "../test/efficient_mode_test/EfficientDataNodes.test.cc"

#include "../test/lysis_mode_test/Bacterium.test.cc"
#include "../test/lysis_mode_test/Phage.test.cc"
#include "../test/lysis_mode_test/BacteriumPhageUnitTest.test.cc"
#include "../test/lysis_mode_test/LysisDataNodes.test.cc"
#include "../test/lysis_mode_test/LysisWorld.test.cc"

#include "../test/pgg_mode_test/PGGSymbiont.test.cc"
#include "../test/pgg_mode_test/PGGHost.test.cc"
#include "../test/pgg_mode_test/PGGHostPGGSymbiontUnitTest.test.cc"
#include "../test/pgg_mode_test/PGGDataNodes.test.cc"
#include "../test/pgg_mode_test/PGGWorld.test.cc"

#include "../test/sgp_mode_test/SGPModularity.test.cc"
#include "../test/sgp_mode_test/CPU.test.cc"
#include "../test/sgp_mode_test/AnalysisTools.test.cc"
#include "../test/sgp_mode_test/GenomeLibrary.test.cc"
#include "../test/sgp_mode_test/SymbiontImpact.test.cc"
#include "../test/sgp_mode_test/SquareFrequencyMap.test.cc"
#include "../test/sgp_mode_test/SGPDiversity.test.cc"
#include "../test/sgp_mode_test/SGPWorld.test.cc"
#include "../test/sgp_mode_test/SGPHost.test.cc"

#include "../test/integration_test/spatial_structure/vt.test.cc"
#include "../test/integration_test/lysogeny/plr.test.cc"
#include "../test/integration_test/endosymbiosis/res_distribute.test.cc"
#include "../test/integration_test/dirty_transmission/hz_mut_rate.test.cc"

