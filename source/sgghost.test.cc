#include "Sgghost.h"
#include "Symbiont.h"
#include <set>

TEST_CASE("Host SetIntVal, GetIntVal") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    SymWorld w(*random);
    double int_val = 1;

    sggHost * h1 = new sggHost(random, &w, &config);
    double default_int_val = 0.0;
    REQUIRE(h1->GetIntVal() == default_int_val);

    sggHost * h2 = new sggHost(random, &w, &config, int_val);
    
    double expected_int_val = 1;
    REQUIRE(h2->GetIntVal() == expected_int_val);

    int_val = -0.7;
    h2->SetIntVal(int_val);
    expected_int_val = -0.7;
    REQUIRE(h2->GetIntVal() == expected_int_val);

    // int_val = -1.3;
    // REQUIRE_THROWS(new Host(random, &w, &config, int_val));

    // int_val = 1.8;
    // REQUIRE_THROWS(new Host(random, &w, &config, int_val));
    
}