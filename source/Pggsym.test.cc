#include "Pgghost.h"
#include "Pggsym.h"

TEST_CASE("PGGSymbiont Constructor") {

    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    SymWorld w(*random);
    SymWorld * world = &w;

    
    double donation = 1;

    double int_val = 0.5;
    PGGSymbiont * s = new PGGSymbiont(random, world, &config, int_val,donation);
    CHECK(s->GetDonation() == donation);

    donation = 2;
    PGGSymbiont * s2 = new PGGSymbiont(random, world, &config, int_val,donation);
    CHECK(s2->GetDonation() == 2);

    int_val = 2;
    REQUIRE_THROWS(new PGGSymbiont(random, world, &config, int_val) );

}
TEST_CASE("Pggmutate") {

    emp::Ptr<emp::Random> random = new emp::Random(37);
    SymConfigBase config;
    SymWorld w(*random);
    SymWorld * world = &w;

    WHEN("Mutation rate is not zero") {
        double int_val = 0;
        double donation =0.01;
        config.MUTATION_SIZE(0.002);
        PGGSymbiont * s = new PGGSymbiont(random, world, &config, int_val,donation);

        s->mutate();
       
        double int_val_post_mutation = 0.0092037339; 
        THEN("Mutation occurs and interaction value changes") {
            REQUIRE(s->GetDonation() == Approx(int_val_post_mutation));
        }
    }
    WHEN("Mutation rate is zero") {
        double int_val = 1;
        int orig_int_val = 1;
        double points = 0.0;
        config.SYM_HORIZ_TRANS_RES(100.0);
        config.HORIZ_TRANS(true);
        config.MUTATION_RATE(0);
        config.MUTATION_SIZE(0);
        PGGSymbiont * s = new PGGSymbiont(random, world, &config, int_val, points);

        s->mutate();


        THEN("Mutation does not occur and interaction value does not change") {
            REQUIRE(s->GetIntVal() == orig_int_val);
        }

    }
}   

TEST_CASE("PGGSymbiont ProcessPool"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    SymWorld w(*random);
    config.SYNERGY(5);
    double host_int_val = 1;
    double sym_int_val = 0;
    double donation = 0.1;

    PGGSymbiont * s = new PGGSymbiont(random, &w, &config, sym_int_val,donation);
    PggHost * h = new PggHost(random, &w, &config, host_int_val);
    h->AddSymbiont(s);

    double piece = 40;
        // double host_donation = 20; //sym_piece * host_int_val;  
    double sym_portion = 0; //host_donation - (host_donation * sym_int_val);
    h->DistribResources(40);
    
    CHECK(s->GetPoints() == 40);
    CHECK(h->GetPoints() == 0);
}

TEST_CASE("PGGProcess") {

    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    SymWorld w(*random);
    SymWorld * world = &w;

    //add new test for free living sym not moving when it shouldnt
    WHEN("Horizontal transmission is true and points is greater than sym_h_res") {
        double int_val = 1;
        // double parent_orig_int_val = 1;
        double points = 0.0;
        config.SYM_HORIZ_TRANS_RES(140.0);
        config.HORIZ_TRANS(true);
        config.MUTATION_SIZE(0);
        PGGSymbiont * s = new PGGSymbiont(random, world, &config, int_val, 0,points);

        int add_points = 200;
        s->AddPoints(add_points);

        int location = 10;
        s->Process(location);


        THEN("Points changes and is set to 0") {
            int points_post_reproduction = 0;
            REQUIRE(s->GetPoints() == points_post_reproduction);
        }
    }


    WHEN("Horizontal transmission is true and points is less than sym_h_res") {
        double int_val = 1;
        // double parent_orig_int_val = 1;
        double points = 0.0;
        config.SYM_HORIZ_TRANS_RES(200.0);
        config.HORIZ_TRANS(true);
        config.MUTATION_SIZE(0.0);
        PGGSymbiont * s = new PGGSymbiont(random, world, &config, int_val, 0,points);

        int add_points = 50;
        s->AddPoints(add_points);

        int location = 10;
        s->Process(location);


        THEN("Points does not change") {
            int points_post_reproduction = 50;
            REQUIRE(s->GetPoints() == points_post_reproduction);
        }
    }


    WHEN("Horizontal transmission is false and points and points is greater then sym_h_res") {
        double int_val = 1;
        // double parent_orig_int_val = 1;
        double points = 100.0;
        config.SYM_HORIZ_TRANS_RES(80.0);
        config.HORIZ_TRANS(false);
        config.MUTATION_SIZE(0.0);
        PGGSymbiont * s = new PGGSymbiont(random, world, &config, int_val, 0,points);

        int location = 10;
        s->Process(location);


        THEN("Points does not change") {
            int points_post_reproduction = 100;
            REQUIRE(s->GetPoints() == points_post_reproduction);
        }
    }

    WHEN("Horizontal transmission is false and points and points is less then sym_h_res") {
        double int_val = 1;
        // double parent_orig_int_val = 1;
        double points = 40.0;
        config.SYM_HORIZ_TRANS_RES(80.0);
        config.HORIZ_TRANS(false);
        config.MUTATION_SIZE(0.0);
        PGGSymbiont * s = new PGGSymbiont(random, world, &config, int_val, 0,points);

        int location = 10;
        s->Process(location);


        THEN("Points does not change") {
            int points_post_reproduction = 40;
            REQUIRE(s->GetPoints() == points_post_reproduction);
        }
    }

}

TEST_CASE("PGGSymbiont ProcessResources"){
   emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymWorld w(*random);
    SymWorld * world = &w;
    SymConfigBase config;
    config.SYNERGY(5); 


    WHEN("sym_int_val < 0"){
        double sym_int_val = -0.6;


        WHEN("host_int_val > 0"){
            double host_int_val = 0.2;
            PggHost * h = new PggHost(random, &w, &config, host_int_val);
            PGGSymbiont * s = new PGGSymbiont(random, world, &config, sym_int_val);
            h->AddSymbiont(s);

            // double resources = 100;
            // double hostDonation = 20;
            // double stolen = 48;
            double expected_sym_points = 68; // hostDonation + stolen
            double expected_return = 0; // hostportion * synergy

            h->SetResInProcess(80);

            THEN("sym receives a donation and stolen resources, host receives betrayal"){
                REQUIRE(s->ProcessResources(20) == expected_return);
                REQUIRE(s->GetPoints() == expected_sym_points);

            }
        }

        WHEN("host_int_val < 0 and resources are placed into defense"){

            WHEN("host successfully defends from symsteal"){
                double host_int_val = -0.8;
                PggHost * h = new PggHost(random, &w, &config, host_int_val);
                PGGSymbiont * s = new PGGSymbiont(random, world, &config, sym_int_val);
                h->AddSymbiont(s);

                // double resources = 100;
                // double hostDonation = 0;
                // double stolen = 0;
                // double hostDefense = 80;
                double expected_sym_points = 0; // hostDonation + stolen
                double expected_return = 0; // hostportion * synergy

                h->SetResInProcess(20);
                THEN("symbiont is unsuccessful at stealing"){
                    REQUIRE(s->ProcessResources(0) == expected_return);
                    REQUIRE(s->GetPoints() == expected_sym_points);
                }
            }

            WHEN("host fails at defense"){
                double host_int_val = -0.5;
                PggHost * h = new PggHost(random, &w, &config, host_int_val);
                PGGSymbiont * s = new PGGSymbiont(random, world, &config, sym_int_val);
                h->AddSymbiont(s);

                // double resources = 100;
                // double hostDonation = 0;
                // double stolen = 5;
                // double hostDefense = 50;
                double expected_sym_points = 5; // hostDonation + stolen
                double expected_return = 0; // hostportion * synergy

                h->SetResInProcess(50);

                THEN("Sym steals successfully"){
                    REQUIRE(s->ProcessResources(0) == expected_return);
                    REQUIRE(s->GetPoints() == Approx(expected_sym_points));             
                }
            }

        }

    }

    WHEN("sym_int_val > 0") {
        double sym_int_val = 0.2;
        double host_int_val = 0.5;
        PggHost * h = new PggHost(random, &w, &config, host_int_val);
        PGGSymbiont * s = new PGGSymbiont(random, world, &config, sym_int_val);
        h->AddSymbiont(s);

        // double resources = 100;
        // double hostDonation = 50;
        // double hostPortion = 10; hostDonation * sym_int_val        
        double expected_sym_points = 40; // hostDonation - hostPortion
        double expected_return = 50; // hostPortion * synergy

        h->SetResInProcess(50);
        

        THEN("Sym attempts to give benefit back"){
            REQUIRE(s->ProcessResources(50) == expected_return);
            REQUIRE(s->GetPoints() == expected_sym_points);
        }
    }


}

