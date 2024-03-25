#include "../../efficient_mode/EfficientHost.h"
#include <typeinfo>

TEST_CASE("EfficientHost Constructor", "[efficient]"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    EfficientWorld w(*random, &config);
    EfficientWorld * world = &w;

    double int_val = -1.5;
    REQUIRE_THROWS( emp::NewPtr<EfficientHost>(random, world, &config, int_val) );

    int_val = -2;
    emp::Ptr< EfficientHost> host = emp::NewPtr<EfficientHost>(random, world, &config, int_val);
    REQUIRE(host->GetIntVal() >= -1);
    REQUIRE(host->GetIntVal() <= 1);

    int_val = -1;
    emp::Ptr<EfficientHost> host1 = emp::NewPtr<EfficientHost>(random, world, &config, int_val);
    CHECK(host1->GetIntVal() == int_val);
    CHECK(host1->GetAge() == 0);
    CHECK(host1->GetPoints() == 0);

    int_val = -1;
    emp::vector<emp::Ptr<Organism>> syms = {};
    emp::vector<emp::Ptr<Organism>> repro_syms = {};
    double points = 10;
    double efficiency = 0.5;
    emp::Ptr<EfficientHost> host2 = emp::NewPtr<EfficientHost>(random, world, &config, int_val, syms, repro_syms, points, efficiency);
    CHECK(host2->GetIntVal() == int_val);
    CHECK(host2->GetEfficiency() == efficiency);
    CHECK(host2->GetAge() == 0);
    CHECK(host2->GetPoints() == points);

    int_val = 1;
    emp::Ptr<EfficientHost> host3 = emp::NewPtr<EfficientHost>(random, world, &config, int_val);
    CHECK(host3->GetIntVal() == int_val);
    CHECK(host3->GetAge() == 0);
    CHECK(host3->GetPoints() == 0);

    int_val = 2;
    REQUIRE_THROWS(emp::NewPtr<EfficientHost>(random, world, &config, int_val) );

    host.Delete();
    host1.Delete();
    host2.Delete();
    host3.Delete();
}

TEST_CASE("EfficientHost MakeNew", "[efficient]"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    EfficientWorld world(*random, &config);

    double parent_int_val = 0.2;
    double parent_efficiency = 0.5;
    emp::Ptr<Organism> host1 = emp::NewPtr<EfficientHost>(random, &world, &config, parent_int_val);
    host1->SetEfficiency(parent_efficiency);
    emp::Ptr<Organism> host2 = host1->MakeNew();

    THEN("The new host has properties of the original host, and has 0 points and 0 age"){
        REQUIRE(host2->GetIntVal() == host1->GetIntVal());
        REQUIRE(host2->GetEfficiency() == host1->GetEfficiency());
        REQUIRE(host2->GetPoints() == 0);
        REQUIRE(host2->GetAge() == 0);
        //check that the offspring is the correct class
        REQUIRE(host2->GetName() == "EfficientHost");
    }
    host1.Delete();
    host2.Delete();
}

TEST_CASE("EfficientHost SetEfficiency and GetEfficiency", "[efficient]"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    EfficientWorld w(*random, &config);
    EfficientWorld * world = &w;
    double int_val = -1;
    emp::Ptr<EfficientHost> host = emp::NewPtr<EfficientHost>(random, world, &config, int_val);

    double efficiency = 0.5;
    host->SetEfficiency(efficiency);
    double expected_efficieny = 0.5;
    REQUIRE(host->GetEfficiency() == expected_efficieny);

    host.Delete();
}
