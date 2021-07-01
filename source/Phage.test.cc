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
    emp::Ptr<emp::Random> random = new emp::Random(1);
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
            double phage_baby_int_val = 0.0005266628;
            REQUIRE( phage_baby->GetIntVal() != parent_orig_int_val);
            REQUIRE( phage_baby->GetIntVal() == Approx(phage_baby_int_val));

            double phage_baby_lysis_chance = 0.4961644916;
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

TEST_CASE("SetBurstTimer, IncBurstTimer"){
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

TEST_CASE("Phage process"){
    emp::Ptr<emp::Random> random = new emp::Random(9);
    SymWorld w(*random);
    SymWorld * world = &w;
    SymConfigBase config;

    config.LYSIS(1); //phage process only happens when lysis is enabled
    config.GRID_X(2); 
    config.GRID_Y(1);
    config.SYM_LIMIT(2);
    int location = 0;

    WHEN("The phage chooses lysogeny"){
        config.LYSIS_CHANCE(0.0); //0% chance of lysis, 100% chance of lysogeny

        double int_val = 0;
        double expected_int_val = 0;
        emp::Ptr<Phage> p = new Phage(random, world, &config, int_val);
        Host * h = new Host(random, &w, &config, int_val);
        h->AddSymbiont(p);

        double points = 0;
        double expected_points = 0;
        p->SetPoints(points);

        double burst_timer = 0;
        double expected_burst_timer = 0;
        p->SetBurstTimer(burst_timer);

        int orig_repro_syms_size = size(h->GetReproSymbionts());
        int expected_repro_syms_size = size(h->GetReproSymbionts());

        p->process(location);

        THEN("There is no change to the status of any objects - the phage is temperate"){
            REQUIRE(p->GetIntVal() == expected_int_val);
            REQUIRE(p->GetPoints() == expected_points);
            REQUIRE(p->GetBurstTimer() == expected_burst_timer);
            REQUIRE(size(h->GetReproSymbionts()) == expected_repro_syms_size);
        }
    }

    WHEN("The phage chooses lysis"){
        config.LYSIS_CHANCE(1.0); //100% chance of lysis, 0% chance of lysogeny
        config.SYM_LYSIS_RES(5);
        config.BURST_TIME(10); 

        WHEN("It is not time to burst"){
            double int_val = 0;
            double expected_int_val = 0;
            emp::Ptr<Phage> p = new Phage(random, world, &config, int_val);
            Host * h = new Host(random, &w, &config, int_val);
            h->AddSymbiont(p);

            p->SetBurstTimer(0.0);

            WHEN("The phage doesn't have enough resources to reproduce"){
                double repro_syms_size_pre_process = size(h->GetReproSymbionts());
                double orig_points = 3.0;
                double expected_points = 3.0;
                p->SetPoints(orig_points);
                p->process(location);

                THEN("The burst timer is incremented but no offspring are created"){
                    double expected_burst_timer = 1.5306015114;
                    REQUIRE(p->GetBurstTimer() == Approx(expected_burst_timer));
                    REQUIRE(size(h->GetReproSymbionts()) == repro_syms_size_pre_process);
                    REQUIRE(p->GetPoints() == expected_points);
                }
            }

            WHEN("The phage has enough resources to reproduce"){
                double expected_repro_syms_size_post_process = size(h->GetReproSymbionts()) + 1; //one offspring created
                double orig_points = 5.0;
                double expected_points = 0.0;
                p->SetPoints(orig_points);
                p->process(location);

                THEN("The burst timer is incremented and offspring are created"){
                    double expected_burst_timer = 1.5306015114;
                    REQUIRE(p->GetBurstTimer() == Approx(expected_burst_timer));
                    REQUIRE(size(h->GetReproSymbionts()) == expected_repro_syms_size_post_process);
                    REQUIRE(p->GetPoints() == expected_points);
                }
            }
        }

        WHEN("It is time to burst"){
            double int_val = 0;
            double expected_int_val = 0;
            emp::Ptr<Phage> p = new Phage(random, world, &config, int_val);
            
            //create two hosts and add both to world as neighbors
            Host * orig_h = new Host(random, &w, &config, int_val);
            Host * new_h = new Host(random, &w, &config, int_val);
            orig_h->AddSymbiont(p);
            world->AddOrgAt(orig_h, 0);
            world->AddOrgAt(new_h, 1);

            //add phage offspring to the original host's repro syms
            emp::Ptr<Organism> p_baby1 = p->reproduce();
            emp::Ptr<Organism> p_baby2 = p->reproduce();
            orig_h->AddReproSym(p_baby1);
            orig_h->AddReproSym(p_baby2);
 
            //call the process such that the phage bursts and we can check injection
            int expected_newh_syms = size(new_h->GetSymbionts()) + 2;
            p->SetBurstTimer(10.0);
            p->process(location);

            THEN("The phage offspring are injected into new hosts and the current host dies"){
                REQUIRE(size(new_h->GetSymbionts()) == expected_newh_syms);
                REQUIRE(size(orig_h->GetReproSymbionts()) == 0);
                REQUIRE(orig_h->GetDead() == true);
            }
        }
    }
}

TEST_CASE("Phage ProcessResources"){
    emp::Ptr<emp::Random> random = new emp::Random(9);
    SymWorld w(*random);
    SymWorld * world = &w;
    SymConfigBase config;

    WHEN("Phage is Lysogenic"){
        config.LYSIS(1);
        config.LYSIS_CHANCE(0);

        double int_val=0;
        emp::Ptr<Phage> p = new Phage(random, world, &config, int_val);
        p->uponInjection();

        double sym_piece = 40;
        double expected_return = 40;

        THEN("Phage doesn't take or give resources to the host"){
            REQUIRE(p->ProcessResources(sym_piece)==expected_return);
        }
    }
}