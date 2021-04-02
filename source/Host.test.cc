#include "Host.h"
#include "Symbiont.h"
#include <set>

TEST_CASE("Host SetIntVal, GetIntVal") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);

    double default_int_val = 0.0;
    Host * h1 = new Host(random);
    REQUIRE(h1->GetIntVal() == default_int_val);

    double int_val = -1.33;
    REQUIRE_THROWS(h1->SetIntVal(int_val));

    int_val = -1.97;
    REQUIRE_THROWS(h1->SetIntVal(int_val));

    int_val = 1;
    Host * h2 = new Host(random, int_val);
    double expected_int_val = 1;
    REQUIRE(h2->GetIntVal() == expected_int_val);

    int_val = -0.7;
    h2->SetIntVal(int_val);
    expected_int_val = -0.7;
    REQUIRE(h2->GetIntVal() == expected_int_val);

    int_val = -1.3;
    REQUIRE_THROWS(new Host(random, int_val));

    int_val = 1.8;
    REQUIRE_THROWS(new Host(random, int_val));
    
}

TEST_CASE("SetPoints, AddPoints, GetPoints") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    double int_val = 1;

    Host * h = new Host(random, int_val);

    double points = 50;
    h->SetPoints(points);
    double expected_points = 50;
    REQUIRE(h->GetPoints() == expected_points);

    points = 76;
    h->AddPoints(points);
    expected_points = 126;
    REQUIRE(h->GetPoints() == expected_points);

}

TEST_CASE("SetResTypes, GetResTypes") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    double int_val = 1;
    emp::vector<emp::Ptr<Organism>> syms = {};
    emp::vector<emp::Ptr<Organism>> repro_syms = {};
    std::set<int> res_types {1,3,5,9,2};

    Host * h = new Host(random, int_val, syms, repro_syms, res_types);
    
    std::set<int> expected_res_types = h->GetResTypes();
    for (int number : res_types)
    {
        // Tests if each integer from res_types is in expected_res_types
        REQUIRE(expected_res_types.find(number) != expected_res_types.end());
    }

    res_types = {0,1};
    h->SetResTypes(res_types);
    expected_res_types = h->GetResTypes();
    for (int number : res_types)
    {
        // Tests if each integer from res_types is in expected_res_types
        REQUIRE(expected_res_types.find(number) != expected_res_types.end());
    }

}

TEST_CASE("HasSym") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    double int_val = 1;

    WHEN("Host has no symbionts") {
        Host * h = new Host(random, int_val);

        THEN("HasSym is false") {
            bool expected = false;
            REQUIRE(h->HasSym() == expected);
        }
    }
}

TEST_CASE("Host Mutate") {
    emp::Ptr<emp::Random> random = new emp::Random(3);
    double int_val = 1;

    Host * h = new Host(random, int_val);
    h->mutate();
    double expected_int_val = 0.9994602838;
    REQUIRE(h->GetIntVal() == Approx(expected_int_val));

    int_val = -0.31;
    h->SetIntVal(int_val);
    h->mutate();
    expected_int_val = -0.3116640493;
    REQUIRE(h->GetIntVal() == Approx(expected_int_val));

}

TEST_CASE("DistributeResources") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);

    WHEN("There are no symbionts and interaction value is between 0 and 1") {

        double int_val = 0.6;
        double resources = 80;
        double synergy = 5;
        double orig_points = 0; 
        
        Host * h = new Host(random, int_val);
        h->DistribResources(resources, synergy);
        
        THEN("Points increase") {
            double expected_points = resources - (resources * int_val); // 48
            double points = h->GetPoints();
            REQUIRE(points == expected_points);
            REQUIRE(points > orig_points);
        }
    }


    WHEN("There are no symbionts and interaction value is 0") {

        double int_val = 0;
        double resources = 10;
        double synergy = 5;
        double orig_points = 0;

        Host * h = new Host(random, int_val);
        h->DistribResources(resources, synergy);
        
        THEN("Resources are added to points") {
            double expected_points = orig_points + resources; // 0
            double points = h->GetPoints();
            REQUIRE(points == expected_points);
        }
    }

    WHEN("There are no symbionts and interaction value is between -1 and 0") {

        double int_val = -0.4;
        double resources = 30;
        double synergy = 5;
        double orig_points = 27;

        Host * h = new Host(random, int_val);
        h->AddPoints(orig_points);
        h->DistribResources(resources, synergy);
        
        THEN("Points increase") {
            double host_defense =  -1.0 * int_val * resources; // the resources spent on defense
            double add_points  = resources - host_defense;
            double expected_points = orig_points + add_points;
            double points = h->GetPoints();
            REQUIRE(points == expected_points);
            REQUIRE(points > orig_points);
        }
    }
}