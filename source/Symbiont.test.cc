#include "Host.h"
#include "Symbiont.h"

TEST_CASE("Symbiont Constructor") {

    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    SymWorld w(*random);
    SymWorld * world = &w;

    double int_val = -2;
    REQUIRE_THROWS(new Symbiont(random, world, &config, int_val) );

    int_val = -1;
    Symbiont * s = new Symbiont(random, world, &config, int_val);
    CHECK(s->GetIntVal() == int_val);

    int_val = 1;
    Symbiont * s2 = new Symbiont(random, world, &config, int_val);
    CHECK(s2->GetIntVal() == int_val);

    int_val = 2;
    REQUIRE_THROWS(new Symbiont(random, world, &config, int_val) );
}

TEST_CASE("SetIntVal, GetIntVal") {

    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    SymWorld w(*random);
    SymWorld * world = &w;

    double int_val = -1;
    Symbiont * s = new Symbiont(random, world, &config, int_val);

    int_val = -2;
    REQUIRE_THROWS( s->SetIntVal(int_val) );

    int_val = -1;
    Symbiont * s2 = new Symbiont(random, world, &config, int_val);

    int_val = 1;
    s2->SetIntVal(int_val);
    double orig_int_val = 1;

    REQUIRE(s2->GetIntVal() == orig_int_val);

    int_val = -1;
    Symbiont * s3 = new Symbiont(random, world, &config, int_val);

    int_val = 2;
    REQUIRE_THROWS( s3->SetIntVal(int_val) ) ;

    int_val = 0;
    Symbiont * s4 = new Symbiont(random, world, &config, int_val);

    int_val = -1;
    s4->SetIntVal(int_val);
    orig_int_val = -1;

    REQUIRE( s4->GetIntVal() == orig_int_val) ;

}

TEST_CASE("SetInfectionChance, GetInfectionChance") {

    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    SymWorld w(*random);
    SymWorld * world = &w;

    double int_val = -1;
    Symbiont * sym = new Symbiont(random, world, &config, int_val);

    double infection_chance = -1;
    REQUIRE_THROWS( sym->SetInfectionChance(infection_chance));

    double orig_infection_chance = 0;
    sym->SetInfectionChance(orig_infection_chance);
    REQUIRE(sym->GetInfectionChance() == orig_infection_chance);

    double new_infection_chance = 1;
    sym->SetInfectionChance(new_infection_chance);
    REQUIRE(sym->GetInfectionChance() == new_infection_chance);

    infection_chance = 2;
    REQUIRE_THROWS(sym->SetInfectionChance(infection_chance));
}

TEST_CASE("SetPoints, GetPoints") {

    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    SymWorld w(*random);
    SymWorld * world = &w;

    double int_val = -1;
    Symbiont * s = new Symbiont(random, world, &config, int_val);

    int points = 1;
    s->SetPoints(points);
    double orig_points = 1;

    REQUIRE( s->GetPoints() == orig_points);

    int add_points = -1;
    s->AddPoints(add_points);
    orig_points = 0;

    REQUIRE( s->GetPoints() == orig_points);

    add_points = 150;
    s->AddPoints(add_points);
    orig_points = 150;

    REQUIRE(s->GetPoints() == orig_points);
}

TEST_CASE("Symbiont SetDead, GetDead"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    SymWorld w(*random);
    SymWorld * world = &w;

    double int_val = -1;
    Symbiont * s = new Symbiont(random, world, &config, int_val);

    //new symbionts are alive until specified otherwise
    bool expected_dead = false;
    REQUIRE(s->GetDead() == expected_dead);

    //verify that setting it to dead means that death is set to true
    expected_dead = true;
    s->SetDead();
    REQUIRE(s->GetDead() == expected_dead);
}

TEST_CASE("WantsToInfect"){
    emp::Ptr<emp::Random> random = new emp::Random(17);
    SymConfigBase config;
    SymWorld w(*random);
    SymWorld * world = &w;
    double int_val = 0;

    WHEN("sym infection chance is 0"){
        config.SYM_INFECTION_CHANCE(0);
        Symbiont * sym1 = new Symbiont(random, world, &config, int_val);
        Symbiont * sym2 = new Symbiont(random, world, &config, int_val);

        THEN("syms never want to infect"){
            REQUIRE(sym1->WantsToInfect() == false);
            REQUIRE(sym2->WantsToInfect() == false);
        }
    }

    WHEN("sym infection chance is between 0 and 1"){
        config.SYM_INFECTION_CHANCE(0.6);
        Symbiont * sym1 = new Symbiont(random, world, &config, int_val);
        Symbiont * sym2 = new Symbiont(random, world, &config, int_val);

        THEN("syms sometimes want to infect, sometimes not"){
            REQUIRE(sym1->WantsToInfect() == false);
            REQUIRE(sym2->WantsToInfect() == true);
        }
    }

    WHEN("sym infection chance is 1"){
        config.SYM_INFECTION_CHANCE(1);
        Symbiont * sym1 = new Symbiont(random, world, &config, int_val);
        Symbiont * sym2 = new Symbiont(random, world, &config, int_val);

        THEN("syms always want to infect"){
            REQUIRE(sym1->WantsToInfect() == true);
            REQUIRE(sym2->WantsToInfect() == true);
        }
    }
}

TEST_CASE("InfectionFails"){
    emp::Ptr<emp::Random> random = new emp::Random(17);
    SymConfigBase config;
    SymWorld w(*random);
    SymWorld * world = &w;
    double int_val = 0;

    WHEN("sym infection failure rate is 0"){
        config.SYM_INFECTION_FAILURE_RATE(0);
        Symbiont * sym1 = new Symbiont(random, world, &config, int_val);
        Symbiont * sym2 = new Symbiont(random, world, &config, int_val);

        THEN("infection never fails"){
            REQUIRE(sym1->InfectionFails() == false);
            REQUIRE(sym2->InfectionFails() == false);
        }
    }

    WHEN("sym infection failure rate is between 0 and 1"){
        config.SYM_INFECTION_FAILURE_RATE(0.6);
        Symbiont * sym1 = new Symbiont(random, world, &config, int_val);
        Symbiont * sym2 = new Symbiont(random, world, &config, int_val);

        THEN("infection sometimes fails, sometimes doesn't"){
            REQUIRE(sym1->InfectionFails() == false);
            REQUIRE(sym2->InfectionFails() == true);
        }
    }

    WHEN("sym infection failure rate is 1"){
        config.SYM_INFECTION_FAILURE_RATE(1);
        Symbiont * sym1 = new Symbiont(random, world, &config, int_val);
        Symbiont * sym2 = new Symbiont(random, world, &config, int_val);

        THEN("infection always fails"){
            REQUIRE(sym1->InfectionFails() == true);
            REQUIRE(sym2->InfectionFails() == true);
        }
    }
}


TEST_CASE("mutate") {

    emp::Ptr<emp::Random> random = new emp::Random(37);
    SymConfigBase config;
    SymWorld w(*random);
    SymWorld * world = &w;

    WHEN("Mutation rate is not zero") {
        double int_val = 0;
        config.MUTATION_SIZE(0.002);

        WHEN("free living symbionts are allowed"){
            config.FREE_LIVING_SYMS(1);
            Symbiont * s = new Symbiont(random, world, &config, int_val);
            s->mutate();

            double int_val_post_mutation = 0.0010984306;
            double infection_chance_post_mutation = 0.9991229745;
            THEN("Mutation occurs and both interaction value and infection chance change"){
                REQUIRE(s->GetIntVal() == Approx(int_val_post_mutation));
                REQUIRE(s->GetInfectionChance() == Approx(infection_chance_post_mutation));
            }
        }

        WHEN("free living symbionts are not allowed"){
            Symbiont * s = new Symbiont(random, world, &config, int_val);
            s->mutate();

            double int_val_post_mutation = 0.0010984306;
            THEN("Mutation occurs and only interaction value changes") {
                REQUIRE(s->GetIntVal() == Approx(int_val_post_mutation));
            }
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
        Symbiont * s = new Symbiont(random, world, &config, int_val, points);

        s->mutate();


        THEN("Mutation does not occur and interaction value does not change") {
            REQUIRE(s->GetIntVal() == orig_int_val);
        }
    }
}

TEST_CASE("reproduce") {

    emp::Ptr<emp::Random> random = new emp::Random(3);
    SymConfigBase config;
    SymWorld w(*random);
    SymWorld * world = &w;


    WHEN("Mutation rate is zero")  {
        double int_val = 0;
        double parent_orig_int_val = 0;
        double points = 0.0;
        config.SYM_HORIZ_TRANS_RES(100.0);
        config.HORIZ_TRANS(true);
        config.MUTATION_SIZE(0);
        Symbiont * s = new Symbiont(random, world, &config, int_val, points);

        emp::Ptr<Organism> sym_baby = s->reproduce();


        THEN("Offspring's interaction value equals parent's interaction value") {
            int sym_baby_int_val = 0;
            REQUIRE( sym_baby->GetIntVal() == sym_baby_int_val);
            REQUIRE( sym_baby->GetIntVal() == parent_orig_int_val);
            REQUIRE( s->GetIntVal() == parent_orig_int_val);
        }

        THEN("Offspring's points are zero") {
            int sym_baby_points = 0;
            REQUIRE( sym_baby->GetPoints() == sym_baby_points);

        }

        sym_baby.Delete();
    }


    WHEN("Mutation rate is not zero") {
        double int_val = 0;
        double parent_orig_int_val = 0;
        double points = 0.0;
        config.SYM_HORIZ_TRANS_RES(100.0);
        config.HORIZ_TRANS(true);
        config.MUTATION_SIZE(0.01);
        Symbiont * s2 = new Symbiont(random, world, &config, int_val, points);

        emp::Ptr<Organism> sym_baby = s2->reproduce();


        THEN("Offspring's interaction value does not equal parent's interaction value") {
            double sym_baby_int_val = 0.0057803391;
            //double parent_int_val = 0.0139135536;
            REQUIRE( sym_baby->GetIntVal() != parent_orig_int_val);
            REQUIRE( sym_baby->GetIntVal() == Approx(sym_baby_int_val));
            //REQUIRE( s2->GetIntVal() == Approx(parent_int_val));
            //The above is for when parents mutate when they reproduce only
        }

        THEN("Offspring's points are zero") {
            int sym_baby_points = 0;
            REQUIRE( sym_baby->GetPoints() == sym_baby_points);

        }

        sym_baby.Delete();
    }

}

TEST_CASE("Process") {

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
        Symbiont * s = new Symbiont(random, world, &config, int_val, points);

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
        Symbiont * s = new Symbiont(random, world, &config, int_val, points);

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
        Symbiont * s = new Symbiont(random, world, &config, int_val, points);

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
        Symbiont * s = new Symbiont(random, world, &config, int_val, points);

        int location = 10;
        s->Process(location);


        THEN("Points does not change") {
            int points_post_reproduction = 40;
            REQUIRE(s->GetPoints() == points_post_reproduction);
        }
    }

}

TEST_CASE("Symbiont ProcessResources"){
   emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymWorld w(*random);
    SymWorld * world = &w;
    SymConfigBase config;
    config.SYNERGY(5);


    WHEN("sym_int_val < 0"){
        double sym_int_val = -0.6;

        WHEN("host_int_val > 0"){
            double host_int_val = 0.2;
            Host * h = new Host(random, &w, &config, host_int_val);
            Symbiont * s = new Symbiont(random, world, &config, sym_int_val);
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
                Host * h = new Host(random, &w, &config, host_int_val);
                Symbiont * s = new Symbiont(random, world, &config, sym_int_val);
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
                Host * h = new Host(random, &w, &config, host_int_val);
                Symbiont * s = new Symbiont(random, world, &config, sym_int_val);
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
        Host * h = new Host(random, &w, &config, host_int_val);
        Symbiont * s = new Symbiont(random, world, &config, sym_int_val);
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
