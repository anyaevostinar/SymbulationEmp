#include "../../sgp_mode/CPUState.h"
#include "../../sgp_mode/Instructions.h"
#include "../../sgp_mode/Tasks.h"
#include "../../sgp_mode/CPU.h"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPSymbiont.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/ModularityAnalysis.h"



TEST_CASE("GetNumSites", "[sgp]"){
        int length = 20;
        
        //host constructor is very wrong but before I fix that I just want the make to be able to see the test file

        emp::Ptr<SGPHost> sample = world.GetFullPop().back().DynamicCast<SGPHost>();
        sample->GetCPU().PrintCode();


        emp::vector<emp::Ptr<int>> useful_genome = {1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,1};
        int site_count = sample.GetNumSites(useful_genome,length);
        REQUIRE(site_count==3);
}

//TEST_CASE("GetPModularity", "[sgp]"){}


//TEST_CASE("GetUsefulEnds", "[sgp]"){}

//TEST_CASE("GetUsefulStarts", "[sgp]"){}

//TEST_CASE("GetSumSiteDist", "[sgp]"){}

//TEST_CASE("GetSummedValue", "[sgp]"){}

//TEST_CASE("CalcPModularity", "[sgp]"){}
