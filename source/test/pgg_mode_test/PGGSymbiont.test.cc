#include "../../pgg_mode/PGGHost.h"
#include "../../pgg_mode/PGGSymbiont.h"

TEST_CASE("PGGSymbiont Constructor", "[pgg]") {

    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    PGGWorld w(*random);
    PGGWorld * world = &w;


    double donation = 1;

    double int_val = 0.5;
    PGGSymbiont * s = new PGGSymbiont(random, world, &config, int_val,donation);
    CHECK(s->GetDonation() == donation);
    CHECK(s->GetAge() == 0);
    CHECK(s->GetPoints() == 0);

    donation = 2;
    PGGSymbiont * s2 = new PGGSymbiont(random, world, &config, int_val,donation);
    CHECK(s2->GetDonation() == 2);
    CHECK(s2->GetAge() == 0);
    CHECK(s2->GetPoints() == 0);

    int_val = 2;
    REQUIRE_THROWS(new PGGSymbiont(random, world, &config, int_val) );

}

TEST_CASE("PGGmutate", "[pgg]") {

    emp::Ptr<emp::Random> random = new emp::Random(37);
    SymConfigBase config;
    PGGWorld w(*random);
    PGGWorld * world = &w;

    WHEN("Mutation rate is not zero") {
        double int_val = 0;
        double donation = 0.01;
        config.MUTATION_SIZE(0.002);
        Organism * s = new PGGSymbiont(random, world, &config, int_val,donation);

        s->mutate();

        THEN("Mutation occurs and donation value changes, but stays within bounds") {
            REQUIRE(s->GetDonation() != donation);
            REQUIRE(s->GetDonation() <= 1);
            REQUIRE(s->GetDonation() >= 0);
        }
    }
    WHEN("Mutation rate is zero") {
        double int_val = 1;
        double donation = 0.1;
        config.SYM_HORIZ_TRANS_RES(100.0);
        config.HORIZ_TRANS(true);
        config.MUTATION_RATE(0);
        config.MUTATION_SIZE(0);
        Organism * s = new PGGSymbiont(random, world, &config, int_val, donation);

        s->mutate();


        THEN("Mutation does not occur and donation value does not change") {
            REQUIRE(s->GetDonation() == donation);
        }

    }
}

TEST_CASE("PGGSymbiont ProcessPool", "[pgg]"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    PGGWorld w(*random);
    config.SYNERGY(5);
    config.PGG_SYNERGY(1.1);
    double host_int_val = 1;
    double sym_int_val = 0;
    double donation = 0.1;

    PGGSymbiont * s = new PGGSymbiont(random, &w, &config, sym_int_val,donation);
    PGGHost * h = new PGGHost(random, &w, &config, host_int_val);
    h->AddSymbiont(s);

    //double piece = 40;
    // double host_donation = 20; //sym_piece * host_int_val;
    //double sym_portion = 0; //host_donation - (host_donation * sym_int_val);
    h->DistribResources(40);

    CHECK(s->GetPoints() == 40.4);
    CHECK(h->GetPoints() == 0);
}

TEST_CASE("PGGProcess", "[pgg]") {

    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    PGGWorld w(*random);
    PGGWorld * world = &w;

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

TEST_CASE("PGGSymbiont ProcessResources", "[pgg]"){
   emp::Ptr<emp::Random> random = new emp::Random(-1);
    PGGWorld w(*random);
    PGGWorld * world = &w;
    SymConfigBase config;
    config.SYNERGY(5);


    WHEN("sym_int_val < 0"){
        double sym_int_val = -0.6;


        WHEN("host_int_val > 0"){
            double host_int_val = 0.2;
            PGGHost * h = new PGGHost(random, &w, &config, host_int_val);
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
                PGGHost * h = new PGGHost(random, &w, &config, host_int_val);
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
                PGGHost * h = new PGGHost(random, &w, &config, host_int_val);
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
        PGGHost * h = new PGGHost(random, &w, &config, host_int_val);
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

TEST_CASE("PGGSymbiont makeNew", "[pgg]"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    PGGWorld w(*random);
    SymConfigBase config;

    double host_int_val = 0.2;
    Organism * s1 = new PGGSymbiont(random, &w, &config, host_int_val);
    Organism * s2 = s1->makeNew();
    THEN("The new symbiont has properties of the original symbiont and has 0 points and 0 age"){
      REQUIRE(s1->GetIntVal() == s2->GetIntVal());
      REQUIRE(s1->GetInfectionChance() == s2->GetInfectionChance());
      REQUIRE(s1->GetDonation() == s2->GetDonation());
      REQUIRE(s2->GetPoints() == 0);
      REQUIRE(s2->GetAge() == 0);
      //check that the offspring is the correct class
      REQUIRE(typeid(*s2).name() == typeid(*s1).name());
    }
}
