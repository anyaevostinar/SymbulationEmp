#include "../../sgp_mode/Organism.h"
#include "../../sgp_mode/SGPCpu.h"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPSymbiont.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/ModularityAnalysis.h"

//test for sites
//test for dist
//test for Sum
//test for calPM

TEST_CASE("GetNumSites", "[sgp]"){
        emp::Ptr<SGPHost> host1 = emp::NewPtr<PGGHost>(random, world, &config, int_val);
        emp::vector<emp::Ptr<int>> usefulGenome = {1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,1};
        int siteCount = usefulGenome.GetNumSites(usefulGenome,);
        REQUIRE(siteCount==3);
}

//TEST_CASE("GetPModularity", "[sgp]"){}


//TEST_CASE("GetUsefulEnds", "[sgp]"){}

//TEST_CASE("GetUsefulStarts", "[sgp]"){}

//TEST_CASE("GetSumSiteDist", "[sgp]"){}

//TEST_CASE("GetSummedValue", "[sgp]"){}

//TEST_CASE("CalcPModularity", "[sgp]"){}
