#include "Sgghost.h"
#include "Symbiont.h"
#include <set>

TEST_CASE("sggHost get pool") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    SymWorld w(*random);
    double pool = 1;

    sggHost * h1 = new sggHost(random, &w, &config);
    double default_pool = 0.0;
    REQUIRE(h1->GetPool() == default_pool);

    sggHost * h2 = new sggHost(random, &w, &config);
    h2->SetPool(pool);
    double expected_pool = 1;
    REQUIRE(h2->GetPool() == expected_pool);

}