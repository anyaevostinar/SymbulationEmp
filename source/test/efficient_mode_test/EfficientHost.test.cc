#include "../../efficient_mode/EfficientHost.h"
#include <typeinfo>

TEST_CASE("EfficientHost Constructor", "[efficient]"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    EfficientWorld w(*random);
    EfficientWorld * world = &w;

    double int_val = -2;
    REQUIRE_THROWS( new EfficientHost(random, world, &config, int_val) );

    int_val = -1;
    EfficientHost * h1 = new EfficientHost(random, world, &config, int_val);
    CHECK(h1->GetIntVal() == int_val);
    CHECK(h1->GetAge() == 0);
    CHECK(h1->GetPoints() == 0);

    int_val = -1;
    emp::vector<emp::Ptr<Organism>> syms = {};
    emp::vector<emp::Ptr<Organism>> repro_syms = {};
    std::set<int> set = std::set<int>();
    double points = 10;
    double efficiency = 0.5;
    EfficientHost * h2 = new EfficientHost(random, world, &config, int_val, syms, repro_syms, set, points, efficiency);
    CHECK(h2->GetIntVal() == int_val);
    CHECK(h2->GetEfficiency() == efficiency);
    CHECK(h2->GetAge() == 0);
    CHECK(h2->GetPoints() == points);

    int_val = 1;
    EfficientHost * h3 = new EfficientHost(random, world, &config, int_val);
    CHECK(h3->GetIntVal() == int_val);
    CHECK(h3->GetAge() == 0);
    CHECK(h3->GetPoints() == 0);

    int_val = 2;
    REQUIRE_THROWS(new EfficientHost(random, world, &config, int_val) );
}

TEST_CASE("EfficientHost MakeNew", "[efficient]"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    EfficientWorld w(*random);
    SymConfigBase config;

    double parent_int_val = 0.2;
    double parent_efficiency = 0.5;
    Organism * h1 = new EfficientHost(random, &w, &config, parent_int_val);
    h1->SetEfficiency(parent_efficiency);
    Organism * h2 = h1->MakeNew();

    THEN("The new host has properties of the original host, and has 0 points and 0 age"){
        REQUIRE(h2->GetIntVal() == h1->GetIntVal());
        REQUIRE(h2->GetEfficiency() == h1->GetEfficiency());
        REQUIRE(h2->GetPoints() == 0);
        REQUIRE(h2->GetAge() == 0);
        //check that the offspring is the correct class
        REQUIRE(typeid(*h2).name() == typeid(*h1).name());
    }
}

TEST_CASE("EfficientHost SetEfficiency and GetEfficiency", "[efficient]"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    EfficientWorld w(*random);
    EfficientWorld * world = &w;
    SymConfigBase config;
    double int_val = -1;
    emp::Ptr<EfficientHost> h = new EfficientHost(random, world, &config, int_val);

    double efficiency = 0.5;
    h->SetEfficiency(efficiency);
    double expected_efficieny = 0.5;
    REQUIRE(h->GetEfficiency() == expected_efficieny);

    delete h;
}
