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
    emp::Ptr<PGGHost> host1 = emp::NewPtr<PGGHost>(random, world, &config, int_val);
    CHECK(host1->GetIntVal() == int_val);
    CHECK(host1->GetAge() == 0);
    CHECK(host1->GetPoints() == 0);

    int_val = -1;
    emp::vector<emp::Ptr<Organism>> syms = {};
    emp::vector<emp::Ptr<Organism>> repro_syms = {};
    std::set<int> set = std::set<int>();
    double points = 10;
    emp::Ptr<PGGHost> host2 = emp::NewPtr<PGGHost>(random, world, &config, int_val, syms, repro_syms, set, points);
    CHECK(host2->GetIntVal() == int_val);
    CHECK(host2->GetAge() == 0);
    CHECK(host2->GetPoints() == points);

    int_val = 1;
    emp::Ptr<PGGHost> host3 = emp::NewPtr<PGGHost>(random, world, &config, int_val);
    CHECK(host3->GetIntVal() == int_val);
    CHECK(host3->GetAge() == 0);
    CHECK(host3->GetPoints() == 0);

    int_val = 2;
    REQUIRE_THROWS(emp::NewPtr<PGGHost>(random, world, &config, int_val) );

    host1.Delete();
    host2.Delete();
    host3.Delete();
}

TEST_CASE("PGGHost get pool", "[pgg]") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    PGGWorld world(*random);
    double pool = 1;

    emp::Ptr<PGGHost> host1 = emp::NewPtr<PGGHost>(random, &world, &config);
    double default_pool = 0.0;
    REQUIRE(host1->GetPool() == default_pool);

    emp::Ptr<PGGHost> host2 = emp::NewPtr<PGGHost>(random, &world, &config);
    host2->SetPool(pool);
    double expected_pool = 1;
    REQUIRE(host2->GetPool() == expected_pool);

    host1.Delete();
    host2.Delete();
}

TEST_CASE("PGGHost DistributeResources", "[pgg]") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    PGGWorld world(*random);

    WHEN("There are no symbionts and interaction value is between 0 and 1") {

        double int_val = 0.6;
        double resources = 80;
        double orig_points = 0; // call this default_points instead? (i'm not setting this val)
        config.SYNERGY(5);

        emp::Ptr<Host> host = emp::NewPtr<PGGHost>(random, &world, &config, int_val);
        host->DistribResources(resources);

        THEN("Points increase") {
            double expected_points = resources - (resources * int_val); // 48
            double points = host->GetPoints();
            REQUIRE(points == expected_points);
            REQUIRE(points > orig_points);
        }
        host.Delete();
    }


    WHEN("There are no symbionts and interaction value is 0") {

        double int_val = 0;
        double resources = 10;
        double orig_points = 0;
        config.SYNERGY(5);

        emp::Ptr<Host> host = emp::NewPtr<PGGHost>(random, &world, &config, int_val);
        host->DistribResources(resources);

        THEN("Resources are added to points") {
            double expected_points = orig_points + resources; // 0
            double points = host->GetPoints();
            REQUIRE(points == expected_points);
        }
        host.Delete();
    }

    WHEN("There are no symbionts and interaction value is between -1 and 0") {

        double int_val = -0.4;
        double resources = 30;
        double orig_points = 27;
        config.SYNERGY(5);

        emp::Ptr<Host> host = emp::NewPtr<PGGHost>(random, &world, &config, int_val);
        host->AddPoints(orig_points);
        host->DistribResources(resources);

        THEN("Points increase") {
            double host_defense =  -1.0 * int_val * resources; // the resources spent on defense
            double add_points  = resources - host_defense;
            double expected_points = orig_points + add_points;
            double points = host->GetPoints();
            REQUIRE(points == expected_points);
            REQUIRE(points > orig_points);
        }
        host.Delete();
    }
}

TEST_CASE("PGGHost MakeNew", "[pgg]"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    PGGWorld world(*random);
    SymConfigBase config;

    double host_int_val = 0.2;
    emp::Ptr<Organism> host1 = emp::NewPtr<PGGHost>(random, &world, &config, host_int_val);
    emp::Ptr<Organism> host2 = host1->MakeNew();
    THEN("The new host has properties of the original host and has 0 points and 0 age"){
      REQUIRE(host1->GetIntVal() == host2->GetIntVal());
      REQUIRE(host2->GetPoints() == 0);
      REQUIRE(host2->GetAge() == 0);
      //check that the offspring is the correct class
      REQUIRE(typeid(*host2).name() == typeid(*host1).name());
    }
    host1.Delete();
    host2.Delete();
}
