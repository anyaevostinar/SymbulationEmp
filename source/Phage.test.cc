#include "Phage.h"
#include "Symbiont.h"

TEST_CASE("Phage constructor, GetIntVal") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymWorld w(*random);
    SymConfigBase config;
    SymWorld * world = &w;
    
    double int_val = -1;
    Phage * p = new Phage(random, world, &config, int_val);
    double expected_int_val = -1;
    REQUIRE(p->GetIntVal() == expected_int_val);

    int_val = 0;
    Phage * p2 = new Phage(random, world, &config, int_val);
    expected_int_val = -1;
    REQUIRE(p2->GetIntVal() == expected_int_val);
    delete p;
    delete p2;
}

TEST_CASE("Phage reproduce") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymWorld w(*random);
    SymWorld * world = &w;
    SymConfigBase config;
    double int_val = -1;

    emp::Ptr<Phage> p = new Phage(random, world, &config, int_val);
    emp::Ptr<Organism> phage_baby = p->reproduce();
    double expected_points = 0;
    double expected_burst_time = 0;
    REQUIRE(phage_baby->GetBurstTimer() == expected_burst_time);
    REQUIRE(phage_baby->GetPoints() == expected_points);

    phage_baby.Delete();
}

TEST_CASE("SetBurstTimer, IncBurstTimer")
{
    emp::Ptr<emp::Random> random = new emp::Random(5);
    SymWorld w(*random);
    SymWorld * world = &w;
    SymConfigBase config;
    double int_val = -1;
    emp::Ptr<Phage> p = new Phage(random, world, &config, int_val);
    
    // int default_burst_time = 0;
    REQUIRE(p->GetBurstTimer() == 0);

    p->IncBurstTimer();
    double incremented_burst_time = 1.3804783599;
    REQUIRE(p->GetBurstTimer() == Approx(incremented_burst_time));

    int burst_time = 15;
    p->SetBurstTimer(burst_time);
    
    int expected_burst_time = 15;
    REQUIRE(p->GetBurstTimer() == expected_burst_time);

    p->IncBurstTimer();
    incremented_burst_time = 17.2075375655;
    REQUIRE(p->GetBurstTimer() == Approx(incremented_burst_time));

}