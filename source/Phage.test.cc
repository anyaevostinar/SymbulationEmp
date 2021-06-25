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

TEST_CASE("Phage reproduce 2") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymWorld w(*random);
    SymWorld * world = &w;
    SymConfigBase config;
    config.MUTATE_LYSIS_CHANCE(1);
    config.LYSIS_CHANCE(.5);
    
    WHEN("Mutation rate is zero")  {
        double int_val = 0;
        double parent_orig_int_val = 0;
        double parent_orig_lysis_chance=.5;
        double points = 0.0;
        config.MUTATION_RATE(0);
        config.MUTATION_SIZE(0);
        emp::Ptr<Phage> p = new Phage(random, world, &config, int_val);
        emp::Ptr<Organism> phage_baby = p->reproduce();


        THEN("Offspring's interaction value and lysis chance equals parent's interaction value and lysis chance") {
            int phage_baby_int_val = 0;
            REQUIRE( phage_baby->GetIntVal() == phage_baby_int_val);
            REQUIRE( phage_baby->GetIntVal() == parent_orig_int_val);
            REQUIRE( p->GetIntVal() == parent_orig_int_val);
            double phage_baby_lysis_chance = .5;
            REQUIRE( phage_baby->GetLysisChance() == phage_baby_lysis_chance);
            REQUIRE( phage_baby->GetLysisChance() == parent_orig_lysis_chance);
            REQUIRE( p->GetLysisChance() == parent_orig_lysis_chance);
        }

        THEN("Offspring's points and burst timer are zero") {
            int phage_baby_points = 0;
            int phage_baby_burst_timer = 0;
            REQUIRE( phage_baby->GetPoints() == phage_baby_points);
            REQUIRE(phage_baby->GetBurstTimer() == phage_baby_burst_timer);


        }
    }


    WHEN("Mutation rate is not zero") {
        double int_val = 0;
        double parent_orig_int_val = 0;
        double parent_orig_lysis_chance=.5;
        double points = 0.0;
        config.MUTATION_RATE(1);
        config.MUTATION_SIZE(0.002);
        emp::Ptr<Phage> p = new Phage(random, world, &config, int_val);
        emp::Ptr<Organism> phage_baby = p->reproduce();


        THEN("Offspring's interaction value and lysis chance does not equal parent's interaction value and lysis chance") {
            double phage_baby_int_val = -0.0020898421;
            REQUIRE( phage_baby->GetIntVal() != parent_orig_int_val);
            REQUIRE( phage_baby->GetIntVal() == Approx(phage_baby_int_val));
            
            double phage_baby_lysis_chance = 2.0;
            std::cout << phage_baby->GetLysisChance() << std::endl;
            REQUIRE( phage_baby->GetLysisChance() != parent_orig_lysis_chance);
            REQUIRE( phage_baby->GetLysisChance() == Approx(phage_baby_lysis_chance));
        }

        THEN("Offspring's points and burst timer are zero") {
            int phage_baby_points = 0;
            REQUIRE( phage_baby->GetPoints() == phage_baby_points);
            int phage_baby_burst_timer = 0;
            REQUIRE( phage_baby->GetBurstTimer() == phage_baby_burst_timer);

        }

    }
 
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
    config.LYSIS_CHANCE(1);
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

    WHEN("Mutation rate is not zero and chance of lysis mutations are enabled") {
        double int_val = 0;
        config.MUTATION_SIZE(0.002);
        config.MUTATE_LYSIS_CHANCE(1);
        emp::Ptr<Phage> p = new Phage(random, world, &config, int_val);
        p->mutate();
        double lysis_chance_post_mutation = 0.503078154;
        THEN("Mutation occurs and chance of lysis changes") {
            REQUIRE(p->GetLysisChance() == Approx(lysis_chance_post_mutation));
        }
    }

    WHEN("Mutation rate is not zero and chance of lysis mutations are not enabled"){
        double int_val = 0;
        config.MUTATION_SIZE(0.002);
        config.MUTATE_LYSIS_CHANCE(0);
        emp::Ptr<Phage> p = new Phage(random, world, &config, int_val);
        p->mutate();
        double lysis_chance_post_mutation = 0.5;
        THEN("Mutation does not occur and chance of lysis does not change") {
            REQUIRE(p->GetLysisChance() == Approx(lysis_chance_post_mutation));
        }
    }

    WHEN("Mutation rate is zero and chance of lysis mutations are enabled"){
        double int_val = 0;
        config.MUTATION_RATE(0);
        config.MUTATION_SIZE(0);
        config.MUTATE_LYSIS_CHANCE(1);
        emp::Ptr<Phage> p = new Phage(random, world, &config, int_val);
        p->mutate();
        double lysis_chance_post_mutation = 0.5;
        THEN("Mutation does not occur and chance of lysis does not change") {
            REQUIRE(p->GetLysisChance() == Approx(lysis_chance_post_mutation));
        }
    }

    WHEN("Mutation rate is zero and chance of lysis mutations are not enabled"){
        double int_val = 0;
        config.MUTATION_RATE(0);
        config.MUTATION_SIZE(0);
        config.MUTATE_LYSIS_CHANCE(0);
        emp::Ptr<Phage> p = new Phage(random, world, &config, int_val);
        p->mutate();
        double lysis_chance_post_mutation = 0.5;
        THEN("Mutation does not occur and chance of lysis does not change") {
            REQUIRE(p->GetLysisChance() == Approx(lysis_chance_post_mutation));
        }
    }
}