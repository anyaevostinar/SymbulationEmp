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
    REQUIRE(p->GetIntVal() == expected_int_val);
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

}

TEST_CASE("SetBurstTimer, IncBurstTimer")
{
    emp::Ptr<emp::Random> random = new emp::Random(5);
    SymWorld w(*random);
    SymWorld * world = &w;
    SymConfigBase config;
    double int_val = -1;
    emp::Ptr<Phage> p = new Phage(random, world, &config, int_val);
    
    int default_burst_time = 0;
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

TEST_CASE("Phage SetLysisChance, GetLysisChance"){
    emp::Ptr<emp::Random> random = new emp::Random(5);
    SymWorld w(*random);
    SymWorld * world = &w;
    SymConfigBase config;
    double int_val = -1;
    emp::Ptr<Phage> p = new Phage(random, world, &config, int_val);

    double lysis_chance = 0.5;
    p->SetLysisChance(lysis_chance);
    double expected_lysis_chance = 0.5;
    REQUIRE(p->GetLysisChance() == expected_lysis_chance);
}

TEST_CASE("Phage uponInjection"){
    emp::Ptr<emp::Random> random = new emp::Random(5);
    SymWorld w(*random);
    SymWorld * world = &w;
    SymConfigBase config;
    double int_val = -1;
    emp::Ptr<Phage> p = new Phage(random, world, &config, int_val);

    //initialization of phage sets lysogeny to false
    bool expected_lysogeny = false;
    REQUIRE(p->GetLysogeny() == expected_lysogeny);

    //phage should choose lysis by default
    p->uponInjection();
    expected_lysogeny = false;
    REQUIRE(p->GetLysogeny() == expected_lysogeny);

    //if chance of lysis is 0, phage should choose lysogeny
    p->SetLysisChance(0.0);
    p->uponInjection();
    expected_lysogeny = true;
    REQUIRE(p->GetLysogeny() == expected_lysogeny);
}
TEST_CASE("phage_mutate"){
    emp::Ptr<emp::Random> random = new emp::Random(5);
    SymWorld w(*random);
    SymWorld * world = &w;
    SymConfigBase config;
    config.LYSIS_CHANCE(.5);
    WHEN("Mutation rate is not zero") {
        double int_val = 0;
        config.MUTATION_SIZE(0.002);
        emp::Ptr<Phage> p = new Phage(random, world, &config, int_val);
        p->mutate();
        double lysis_chance_post_mutation = 2.5;
        THEN("Mutation occurs and interaction value changes") {
            REQUIRE(p->GetLysisChance() == Approx(lysis_chance_post_mutation));
        }
    }
}