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

//SGP mode
#include "../test/sgp_mode_test/unit_tests/GenomeLibrary.test.cc"
#include "../test/sgp_mode_test/unit_tests/HealthHost.test.cc"
#include "../test/sgp_mode_test/unit_tests/SGPSymbiont.test.cc"
#include "../test/sgp_mode_test/unit_tests/SGPWorld.test.cc"
#include "../test/sgp_mode_test/unit_tests/SGPWorldSetup.test.cc"
#include "../test/sgp_mode_test/unit_tests/StressHost.test.cc"
#include "../test/sgp_mode_test/unit_tests/Tasks.test.cc"


#include "../test/sgp_mode_test/functional_tests/GenomeLibrary_SGPWorld.test.cc"
#include "../test/sgp_mode_test/functional_tests/HealthHost_Instructions.test.cc"
#include "../test/sgp_mode_test/functional_tests/SGPDataNodes_SGPWorld.test.cc"
#include "../test/sgp_mode_test/functional_tests/SGPHost_Tasks.test.cc"
#include "../test/sgp_mode_test/functional_tests/SGPHost_Reproduce.test.cc"
#include "../test/sgp_mode_test/functional_tests/SGPSymbiont_Tasks.test.cc"
#include "../test/sgp_mode_test/functional_tests/SGPSymbiont_Reproduce.test.cc"
#include "../test/sgp_mode_test/functional_tests/SGPWorld_StressHost.test.cc"
#include "../test/sgp_mode_test/functional_tests/SGPWorld_SGPConfig.test.cc"
#include "../test/sgp_mode_test/functional_tests/StressHost_SGPDataNodes.test.cc"
#include "../test/sgp_mode_test/functional_tests/Tasks_ProcessOutput.test.cc"

//Integration tests
#include "../test/integration_test/spatial_structure/vt.test.cc"
#include "../test/integration_test/lysogeny/plr.test.cc"
#include "../test/integration_test/endosymbiosis/res_distribute.test.cc"
#include "../test/integration_test/dirty_transmission/hz_mut_rate.test.cc"
#include "../test/integration_test/multi_infection/pgg.test.cc"
#include "../test/integration_test/mutualists_constrain/SymsEffectEvo.test.cc"

