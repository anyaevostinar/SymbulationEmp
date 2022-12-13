#include "../../pgg_mode/PGGHost.h"
#include "../../pgg_mode/PGGSymbiont.h"

TEST_CASE("PGGSymbiont Constructor", "[pgg]") {

    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    PGGWorld w(*random, &config);
    PGGWorld * world = &w;


    double donation = 1;

    double int_val = 0.5;
    emp::Ptr<PGGSymbiont> symbiont = emp::NewPtr<PGGSymbiont>(random, world, &config, int_val,donation);
    CHECK(symbiont->GetDonation() == donation);
    CHECK(symbiont->GetAge() == 0);
    CHECK(symbiont->GetPoints() == 0);

    donation = 2;
    emp::Ptr<PGGSymbiont> symbiont2 = emp::NewPtr<PGGSymbiont>(random, world, &config, int_val,donation);
    CHECK(symbiont2->GetDonation() == 2);
    CHECK(symbiont2->GetAge() == 0);
    CHECK(symbiont2->GetPoints() == 0);

    int_val = 2;
    REQUIRE_THROWS(emp::NewPtr<PGGSymbiont>(random, world, &config, int_val) );

    symbiont.Delete();
    symbiont2.Delete();
}

TEST_CASE("PGGmutate", "[pgg]") {

    emp::Ptr<emp::Random> random = new emp::Random(37);
    SymConfigBase config;
    PGGWorld w(*random, &config);
    PGGWorld * world = &w;

    WHEN("Mutation rate is not zero") {
        double int_val = 0;
        double donation = 0.01;
        config.MUTATION_SIZE(0.002);
        emp::Ptr<PGGSymbiont> symbiont = emp::NewPtr<PGGSymbiont>(random, world, &config, int_val,donation);

        symbiont->Mutate();

        THEN("Mutation occurs and donation value changes, but stays within bounds") {
            REQUIRE(symbiont->GetDonation() != donation);
            REQUIRE(symbiont->GetDonation() <= 1);
            REQUIRE(symbiont->GetDonation() >= 0);
        }
        symbiont.Delete();
    }
    WHEN("Mutation rate is zero") {
        double int_val = 1;
        double donation = 0.1;
        config.SYM_HORIZ_TRANS_RES(100.0);
        config.HORIZ_TRANS(true);
        config.MUTATION_RATE(0);
        config.MUTATION_SIZE(0);
        emp::Ptr<PGGSymbiont> symbiont = emp::NewPtr<PGGSymbiont>(random, world, &config, int_val, donation);

        symbiont->Mutate();


        THEN("Mutation does not occur and donation value does not change") {
            REQUIRE(symbiont->GetDonation() == donation);
        }
        symbiont.Delete();
    }
}

TEST_CASE("PGGSymbiont ProcessPool", "[pgg]"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    PGGWorld world(*random, &config);
    config.SYNERGY(5);
    config.PGG_SYNERGY(1.1);
    double host_int_val = 1;
    double sym_int_val = 0;
    double donation = 0.1;

    emp::Ptr<PGGSymbiont> symbiont = emp::NewPtr<PGGSymbiont>(random, &world, &config, sym_int_val,donation);
    emp::Ptr<PGGHost> host = emp::NewPtr<PGGHost>(random, &world, &config, host_int_val);
    host->AddSymbiont(symbiont);

    host->DistribResources(40);

    CHECK(symbiont->GetPoints() == 40.4);
    CHECK(host->GetPoints() == 0);

    host.Delete();
}

TEST_CASE("PGGProcess", "[pgg]") {

    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    PGGWorld w(*random, &config);
    PGGWorld * world = &w;

    //add new test for free living sym not moving when it shouldnt
    WHEN("Horizontal transmission is true and points is greater than sym_h_res") {
        double int_val = 1;
        double points = 0.0;
        config.SYM_HORIZ_TRANS_RES(140.0);
        config.HORIZ_TRANS(true);
        config.MUTATION_SIZE(0);
        emp::Ptr<PGGSymbiont> symbiont = emp::NewPtr<PGGSymbiont>(random, world, &config, int_val, 0,points);

        int add_points = 200;
        symbiont->AddPoints(add_points);

        int location = 10;
        symbiont->Process(location);


        THEN("Points changes and is set to 0") {
            int points_post_reproduction = 0;
            REQUIRE(symbiont->GetPoints() == points_post_reproduction);
        }
        symbiont.Delete();
    }


    WHEN("Horizontal transmission is true and points is less than sym_h_res") {
        double int_val = 1;
        double points = 0.0;
        config.SYM_HORIZ_TRANS_RES(200.0);
        config.HORIZ_TRANS(true);
        config.MUTATION_SIZE(0.0);
        emp::Ptr<PGGSymbiont> symbiont = emp::NewPtr<PGGSymbiont>(random, world, &config, int_val, 0,points);

        int add_points = 50;
        symbiont->AddPoints(add_points);

        int location = 10;
        symbiont->Process(location);


        THEN("Points does not change") {
            int points_post_reproduction = 50;
            REQUIRE(symbiont->GetPoints() == points_post_reproduction);
        }
        symbiont.Delete();
    }


    WHEN("Horizontal transmission is false and points and points is greater then sym_h_res") {
        double int_val = 1;
        double points = 100.0;
        config.SYM_HORIZ_TRANS_RES(80.0);
        config.HORIZ_TRANS(false);
        config.MUTATION_SIZE(0.0);
        emp::Ptr<PGGSymbiont> symbiont = emp::NewPtr<PGGSymbiont>(random, world, &config, int_val, 0,points);

        int location = 10;
        symbiont->Process(location);


        THEN("Points does not change") {
            int points_post_reproduction = 100;
            REQUIRE(symbiont->GetPoints() == points_post_reproduction);
        }
        symbiont.Delete();
    }

    WHEN("Horizontal transmission is false and points and points is less then sym_h_res") {
        double int_val = 1;
        double points = 40.0;
        config.SYM_HORIZ_TRANS_RES(80.0);
        config.HORIZ_TRANS(false);
        config.MUTATION_SIZE(0.0);
        emp::Ptr<PGGSymbiont> symbiont = emp::NewPtr<PGGSymbiont>(random, world, &config, int_val, 0,points);

        int location = 10;
        symbiont->Process(location);


        THEN("Points does not change") {
            int points_post_reproduction = 40;
            REQUIRE(symbiont->GetPoints() == points_post_reproduction);
        }
        symbiont.Delete();
    }

}

TEST_CASE("PGGSymbiont ProcessResources", "[pgg]"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    PGGWorld w(*random, &config);
    PGGWorld * world = &w;
    config.SYNERGY(5);


    WHEN("sym_int_val < 0"){
        double sym_int_val = -0.6;


        WHEN("host_int_val > 0"){
            double host_int_val = 0.2;
            emp::Ptr<PGGHost> host = emp::NewPtr<PGGHost>(random, world, &config, host_int_val);
            emp::Ptr<PGGSymbiont> symbiont = emp::NewPtr<PGGSymbiont>(random, world, &config, sym_int_val);
            host->AddSymbiont(symbiont);

            double expected_sym_points = 68; // hostDonation + stolen
            double expected_return = 0; // hostportion * synergy

            host->SetResInProcess(80);

            THEN("sym receives a donation and stolen resources, host receives betrayal"){
                REQUIRE(symbiont->ProcessResources(20) == expected_return);
                REQUIRE(symbiont->GetPoints() == expected_sym_points);

            }
            host.Delete();
        }

        WHEN("host_int_val < 0 and resources are placed into defense"){

            WHEN("host successfully defends from symsteal"){
                double host_int_val = -0.8;
                emp::Ptr<PGGHost> host = emp::NewPtr<PGGHost>(random, world, &config, host_int_val);
                emp::Ptr<PGGSymbiont> symbiont = emp::NewPtr<PGGSymbiont>(random, world, &config, sym_int_val);
                host->AddSymbiont(symbiont);

                double expected_sym_points = 0; // hostDonation + stolen
                double expected_return = 0; // hostportion * synergy

                host->SetResInProcess(20);
                THEN("symbiont is unsuccessful at stealing"){
                    REQUIRE(symbiont->ProcessResources(0) == expected_return);
                    REQUIRE(symbiont->GetPoints() == expected_sym_points);
                }
                host.Delete();
            }

            WHEN("host fails at defense"){
                double host_int_val = -0.5;
                emp::Ptr<PGGHost> host = emp::NewPtr<PGGHost>(random, world, &config, host_int_val);
                emp::Ptr<PGGSymbiont> symbiont = emp::NewPtr<PGGSymbiont>(random, world, &config, sym_int_val);
                host->AddSymbiont(symbiont);

                double expected_sym_points = 5; // hostDonation + stolen
                double expected_return = 0; // hostportion * synergy

                host->SetResInProcess(50);

                THEN("Sym steals successfully"){
                    REQUIRE(symbiont->ProcessResources(0) == expected_return);
                    REQUIRE(symbiont->GetPoints() == Approx(expected_sym_points));
                }
                host.Delete();
            }

        }

    }

    WHEN("sym_int_val > 0") {
        double sym_int_val = 0.2;
        double host_int_val = 0.5;
        emp::Ptr<PGGHost> host = emp::NewPtr<PGGHost>(random, world, &config, host_int_val);
        emp::Ptr<PGGSymbiont> symbiont = emp::NewPtr<PGGSymbiont>(random, world, &config, sym_int_val);
        host->AddSymbiont(symbiont);

        double expected_sym_points = 40; // hostDonation - hostPortion
        double expected_return = 50; // hostPortion * synergy

        host->SetResInProcess(50);


        THEN("Sym attempts to give benefit back"){
            REQUIRE(symbiont->ProcessResources(50) == expected_return);
            REQUIRE(symbiont->GetPoints() == expected_sym_points);
        }
        host.Delete();
    }
}

TEST_CASE("PGGSymbiont MakeNew", "[pgg]"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    PGGWorld world(*random, &config);

    double host_int_val = 0.2;
    emp::Ptr<PGGSymbiont> symbiont1 = emp::NewPtr<PGGSymbiont>(random, &world, &config, host_int_val);
    emp::Ptr<PGGSymbiont> symbiont2 = symbiont1->MakeNew().DynamicCast<PGGSymbiont>();
    THEN("The new symbiont has properties of the original symbiont and has 0 points and 0 age"){
      REQUIRE(symbiont1->GetIntVal() == symbiont2->GetIntVal());
      REQUIRE(symbiont1->GetInfectionChance() == symbiont2->GetInfectionChance());
      REQUIRE(symbiont1->GetDonation() == symbiont2->GetDonation());
      REQUIRE(symbiont2->GetPoints() == 0);
      REQUIRE(symbiont2->GetAge() == 0);
      //check that the offspring is the correct class
      REQUIRE(symbiont2->GetName() == "PGGSymbiont");
    }
    symbiont1.Delete();
    symbiont2.Delete();
}
