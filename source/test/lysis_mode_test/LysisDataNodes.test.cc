#include "../../lysis_mode/LysisWorld.h"
#include "../../lysis_mode/Phage.h"
#include "../../lysis_mode/Bacterium.h"

TEST_CASE("GetLysisChanceDataNode", "[lysis]"){
    //make three phage with lysis chances 0.4, 0.5, and 1.0
    //show that 0.5 and 1 are not being put in the right places
    //also show difference between setupbins with 10 vs 11 bins
    //whats up with the bin mins not equaling each other
    emp::Random random(17);
    SymConfigBase config;

    double int_val = 0;
    double chance1 = 0.4;
    double chance2 = 0.5;
    double chance3 = 1.0;

    WHEN("There are 11 histogram bins"){
        LysisWorld w(random);

        REQUIRE(w.GetNumOrgs() == 0);

        Phage * p1 = new Phage(&random, &w, &config, int_val);
        p1->SetLysisChance(chance1);
        w.AddOrgAt(p1, emp::WorldPosition(0, 0));

        Phage * p2 = new Phage(&random, &w, &config, int_val);
        p2->SetLysisChance(chance2);
        w.AddOrgAt(p2, emp::WorldPosition(0, 1));

        Phage * p3 = new Phage(&random, &w, &config, int_val);
        p3->SetLysisChance(chance3);
        w.AddOrgAt(p3, emp::WorldPosition(0, 2));

        REQUIRE(w.GetNumOrgs() == 3);

        emp::DataMonitor<double,emp::data::Histogram>& node = w.GetLysisChanceDataNode();
        node.SetupBins(0.0, 1.1, 11);

        CHECK(node.GetHistCounts() == emp::vector<size_t>({0,0,0,0,0,0,0,0,0,0,0}));

        w.Update();

        //-------print bin counts------
        const emp::vector<size_t> & counts = node.GetHistCounts();
        for (long unsigned int i = 0; i < counts.size(); i++){
            std::cout << "bin " << i << ": " << counts[i] << std::endl;
        }

        //------check other bin properties------
        CHECK(node.GetHistWidth(4) == 0.1);
        //the below is failing - is it a floating point comparison error?
        //CHECK(node.GetBinMins() == emp::vector<double>({0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0}));

        THEN("Lysis chance 0.5 is placed incorrectly"){
            //The 4th bin should be 0.4 <= lysis chance < 0.5
            //A lysis chance of 0.5 should be placed in the 5th bin
            //Instead, phage with lysis chances of 0.4 and 0.5 are both put in the 4th bin
            REQUIRE(node.GetHistCount(4) == 2);

            //With 11 bins, a lysis chance of 1.0 is placed where we would like it to go (bin 9)
            //However, this is still an issue because we'd expect that the 9th bin would
            //be 0.9 <= lysis chance < 1.0 and the 10th bin would be 1.0 <= lysis chance < 1.1
            REQUIRE(node.GetHistCount(9) == 1);
        }
    }

    WHEN("There are 10 histogram bins"){
        LysisWorld w(random);
        w.Resize(3);

        REQUIRE(w.GetNumOrgs() == 0);

        Phage * p1 = new Phage(&random, &w, &config, int_val);
        p1->SetLysisChance(chance1);
        w.AddOrgAt(p1, emp::WorldPosition(0, 0));

        Phage * p2 = new Phage(&random, &w, &config, int_val);
        p2->SetLysisChance(chance2);
        w.AddOrgAt(p2, emp::WorldPosition(0, 1));

        Phage * p3 = new Phage(&random, &w, &config, int_val);
        p3->SetLysisChance(chance3);
        w.AddOrgAt(p3, emp::WorldPosition(0, 2));

        REQUIRE(w.GetNumOrgs() == 3);

        emp::DataMonitor<double,emp::data::Histogram>& node = w.GetLysisChanceDataNode();
        node.SetupBins(0.0, 1.0, 10);

        CHECK(node.GetHistCounts() == emp::vector<size_t>({0,0,0,0,0,0,0,0,0,0}));

        w.Update();

        //-------print bin counts------
        const emp::vector<size_t> & counts = node.GetHistCounts();
        for (long unsigned int i = 0; i < counts.size(); i++){
            std::cout << "bin " << i << ": " << counts[i] << std::endl;
        }

        //------check other bin properties------
        CHECK(node.GetHistWidth(4) == 0.1);
        //the below is failing - is it a floating point comparison error?
        //CHECK(node.GetBinMins() == emp::vector<double>({0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9}));

        THEN("Lysis chance 0.5 is placed incorrectly"){
            //The 4th bin should be 0.4 <= lysis chance < 0.5
            //A lysis chance of 0.5 should be placed in the 5th bin
            //Instead, phage with lysis chances of 0.4 and 0.5 are both put in the 4th bin
            REQUIRE(node.GetHistCount(4) == 2);

            //Even though the max of the histogram bins is 1.0, 
            //a lysis chance of 1.0 is excluded from the counts
            REQUIRE(node.GetHistMax() == 1.0);
            REQUIRE(node.GetHistCount(9) == 0);
        }
    }
}