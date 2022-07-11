#include "../../sgp_mode/CPUState.h"
#include "../../sgp_mode/Instructions.h"
#include "../../sgp_mode/Tasks.h"
#include "../../sgp_mode/CPU.h"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPSymbiont.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/ModularityAnalysis.h"

//have world call these methods

TEST_CASE("GetNumSites", "[sgp]"){
       
        WHEN("last position is in a site"){
        emp::vector<int> useful_genome = {1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,1};
        int site_count = GetNumSites(useful_genome);
        REQUIRE(site_count==3);
        }

        WHEN("when last position is a one-instruction size site"){
        emp::vector<int> useful_genome = {1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,0,1};
        int site_count = GetNumSites(useful_genome);
        REQUIRE(site_count==4);
        }

}

TEST_CASE("GetUsefulStarts", "[sgp]"){
        emp::vector<int> useful_genome_a = {1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,0,1};
        emp::vector<int> useful_genome_b = {0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,0,1};
        emp::vector<int> useful_genome_c = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,1};
        emp::vector<int> useful_genome_d = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
        emp::vector<int> useful_genome_e = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

        emp::vector<emp::vector<int>> useful_genomes = {useful_genome_a,useful_genome_b,useful_genome_c,useful_genome_d,useful_genome_e};

        emp::vector<int> calc_useful_starts = GetUsefulStarts(useful_genomes);
        emp::vector<int> true_useful_starts = {0,6,useful_genome_c.size()-5,useful_genome_c.size()-1, 0};

        REQUIRE(calc_useful_starts == true_useful_starts);

}

//TEST_CASE("GetPModularity", "[sgp]"){}


//TEST_CASE("GetUsefulEnds", "[sgp]"){}

//TEST_CASE("GetUsefulStarts", "[sgp]"){}

//TEST_CASE("GetSumSiteDist", "[sgp]"){}

//TEST_CASE("GetSummedValue", "[sgp]"){}

//TEST_CASE("CalcPModularity", "[sgp]"){}


