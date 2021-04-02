#include "Phage.h"
#include "Symbiont.h"

TEST_CASE("Phage constructor, GetIntVal") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymWorld w(*random);
    SymWorld * world = &w;
    
    double int_val = -1;
    Phage * p = new Phage(random, world, int_val);
    double expected_int_val = -1;
    REQUIRE(p->GetIntVal() == expected_int_val);

    int_val = 0;
    Phage * p2 = new Phage(random, world, int_val);
    expected_int_val = -1;
    REQUIRE(p->GetIntVal() == expected_int_val);
}

TEST_CASE("Phage reproduce") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymWorld w(*random);
    SymWorld * world = &w;
    double int_val = -1;

    emp::Ptr<Phage> p = new Phage(random, world, int_val);
    emp::Ptr<Organism> phage_baby = p->reproduce();
    double expected_points = 0;
    double expected_burst_time = 0;
    REQUIRE(phage_baby->GetBurstTimer() == expected_burst_time);
    REQUIRE(phage_baby->GetPoints() == expected_points);

}