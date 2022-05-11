#include "../../pgg_mode/PGGHost.h"
#include "../../pgg_mode/PGGSymbiont.h"
#include <set>

TEST_CASE("PGGHost constructor", "[pgg]"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    PGGWorld w(*random);
    PGGWorld * world = &w;

    double int_val = -2;
    REQUIRE_THROWS(emp::NewPtr<PGGHost>(random, world, &config, int_val) );

    int_val = -1;
    emp::Ptr<PGGHost> h1 = emp::NewPtr<PGGHost>(random, world, &config, int_val);
    CHECK(h1->GetIntVal() == int_val);
    CHECK(h1->GetAge() == 0);
    CHECK(h1->GetPoints() == 0);

    int_val = -1;
    emp::vector<emp::Ptr<Organism>> syms = {};
    emp::vector<emp::Ptr<Organism>> repro_syms = {};
    std::set<int> set = std::set<int>();
    double points = 10;
    emp::Ptr<PGGHost> h2 = emp::NewPtr<PGGHost>(random, world, &config, int_val, syms, repro_syms, set, points);
    CHECK(h2->GetIntVal() == int_val);
    CHECK(h2->GetAge() == 0);
    CHECK(h2->GetPoints() == points);

    int_val = 1;
    emp::Ptr<PGGHost> h3 = emp::NewPtr<PGGHost>(random, world, &config, int_val);
    CHECK(h3->GetIntVal() == int_val);
    CHECK(h3->GetAge() == 0);
    CHECK(h3->GetPoints() == 0);

    int_val = 2;
    REQUIRE_THROWS(emp::NewPtr<PGGHost>(random, world, &config, int_val) );

    h1.Delete();
    h2.Delete();
    h3.Delete();
}

TEST_CASE("PGGHost get pool", "[pgg]") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    PGGWorld w(*random);
    double pool = 1;

    emp::Ptr<PGGHost> h1 = emp::NewPtr<PGGHost>(random, &w, &config);
    double default_pool = 0.0;
    REQUIRE(h1->GetPool() == default_pool);

    emp::Ptr<PGGHost> h2 = emp::NewPtr<PGGHost>(random, &w, &config);
    h2->SetPool(pool);
    double expected_pool = 1;
    REQUIRE(h2->GetPool() == expected_pool);

    h1.Delete();
    h2.Delete();
}

TEST_CASE("PGGHost DistributeResources", "[pgg]") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    PGGWorld w(*random);

    WHEN("There are no symbionts and interaction value is between 0 and 1") {

        double int_val = 0.6;
        double resources = 80;
        double orig_points = 0; // call this default_points instead? (i'm not setting this val)
        config.SYNERGY(5);

        emp::Ptr<Host> h = emp::NewPtr<PGGHost>(random, &w, &config, int_val);
        h->DistribResources(resources);

        THEN("Points increase") {
            double expected_points = resources - (resources * int_val); // 48
            double points = h->GetPoints();
            REQUIRE(points == expected_points);
            REQUIRE(points > orig_points);
        }
        h.Delete();
    }


    WHEN("There are no symbionts and interaction value is 0") {

        double int_val = 0;
        double resources = 10;
        double orig_points = 0;
        config.SYNERGY(5);

        emp::Ptr<Host> h = emp::NewPtr<PGGHost>(random, &w, &config, int_val);
        h->DistribResources(resources);

        THEN("Resources are added to points") {
            double expected_points = orig_points + resources; // 0
            double points = h->GetPoints();
            REQUIRE(points == expected_points);
        }
        h.Delete();
    }

    WHEN("There are no symbionts and interaction value is between -1 and 0") {

        double int_val = -0.4;
        double resources = 30;
        double orig_points = 27;
        config.SYNERGY(5);

        emp::Ptr<Host> h = emp::NewPtr<PGGHost>(random, &w, &config, int_val);
        h->AddPoints(orig_points);
        h->DistribResources(resources);

        THEN("Points increase") {
            double host_defense =  -1.0 * int_val * resources; // the resources spent on defense
            double add_points  = resources - host_defense;
            double expected_points = orig_points + add_points;
            double points = h->GetPoints();
            REQUIRE(points == expected_points);
            REQUIRE(points > orig_points);
        }
        h.Delete();
    }
}

TEST_CASE("PGGHost makeNew", "[pgg]"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    PGGWorld w(*random);
    SymConfigBase config;

    double host_int_val = 0.2;
    emp::Ptr<Organism> h1 = emp::NewPtr<PGGHost>(random, &w, &config, host_int_val);
    emp::Ptr<Organism> h2 = h1->makeNew();
    THEN("The new host has properties of the original host and has 0 points and 0 age"){
      REQUIRE(h1->GetIntVal() == h2->GetIntVal());
      REQUIRE(h2->GetPoints() == 0);
      REQUIRE(h2->GetAge() == 0);
      //check that the offspring is the correct class
      REQUIRE(typeid(*h2).name() == typeid(*h1).name());
    }
    h1.Delete();
    h2.Delete();
}
