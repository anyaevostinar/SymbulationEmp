#include "../../default_mode/Host.h"
#include "../../efficient_mode/EfficientSymbiont.h"

TEST_CASE("EfficientSymbiont mutate", "[efficient]") {

    emp::Ptr<emp::Random> random = new emp::Random(10);
    SymConfigBase config;
    EfficientWorld w(*random);
    EfficientWorld * world = &w;

    WHEN("Mutation rate is not zero") {
        double int_val = 0;
        double orig_efficiency = 0.5;
        double points = 0;
        config.MUTATION_SIZE(0.002);
        EfficientSymbiont * s = new EfficientSymbiont(random, world, &config, int_val, points, orig_efficiency);

        s->mutate();

        THEN("Mutation occurs and efficiency value changes, but within bounds") {
            REQUIRE(s->GetEfficiency() != orig_efficiency);
            REQUIRE(s->GetEfficiency() <= 1);
            REQUIRE(s->GetEfficiency() >= 0);
        }
    }


    WHEN("Mutation rate is zero") {
        double int_val = 1;
        double orig_efficiency = 0.5;
        double points = 0.0;
        config.SYM_HORIZ_TRANS_RES(100.0);
        config.HORIZ_TRANS(true);
        config.MUTATION_RATE(0);
        config.MUTATION_SIZE(0);
        EfficientSymbiont * s = new EfficientSymbiont(random, world, &config, int_val, points, orig_efficiency);

        s->mutate();


        THEN("Mutation does not occur and efficiency value does not change") {
            REQUIRE(s->GetEfficiency() == orig_efficiency);
        }

    }
}

TEST_CASE("EfficientSymbiont AddPoints", "[efficient]") {
    emp::Ptr<emp::Random> random = new emp::Random(3);
    SymConfigBase config;
    EfficientWorld w(*random);
    EfficientWorld * world = &w;
    double int_val = 0;
    double points = 0;
    double points_in = 10;

    WHEN("Efficiency is 1") {
        double efficiency = 1;
        EfficientSymbiont * s = new EfficientSymbiont(random, world, &config, int_val, points, efficiency);

        s->AddPoints(points_in);

        THEN("All points added") {
            REQUIRE( s->GetPoints() == points_in);
        }
    }

    WHEN("Efficiency is 0.5") {
        double efficiency = 0.5;
        EfficientSymbiont * s = new EfficientSymbiont(random, world, &config, int_val, points, efficiency);

        s->AddPoints(points_in);
        double actual_points = 5; //points_in * 0.5

        THEN("Half points get added") {
            REQUIRE( s->GetPoints() == actual_points);
        }
    }

    WHEN("Efficiency is 0") {
        double efficiency = 0;
        EfficientSymbiont * s = new EfficientSymbiont(random, world, &config, int_val, points, efficiency);

        s->AddPoints(points_in);
        double actual_points = 0; //points_in * 0

        THEN("No points get added") {
            REQUIRE( s->GetPoints() == actual_points);
        }
    }
}

TEST_CASE("EfficientSymbiont reproduce", "[efficient]") {

    emp::Ptr<emp::Random> random = new emp::Random(3);
    SymConfigBase config;
    EfficientWorld w(*random);
    EfficientWorld * world = &w;
    double int_val = 0;


    WHEN("Mutation rate is zero")  {
        // double efficiency = 0.5;
        double parent_orig_efficiency = 0.5;
        double points = 0.0;
        config.SYM_HORIZ_TRANS_RES(100.0);
        config.HORIZ_TRANS(true);
        config.MUTATION_SIZE(0);
        EfficientSymbiont * s = new EfficientSymbiont(random, world, &config, int_val, points, parent_orig_efficiency);

        emp::Ptr<Organism> sym_baby = s->reproduce();


        THEN("Offspring's efficiency equals parent's efficiency") {
            double sym_baby_efficiency = 0.5;
            REQUIRE( sym_baby->GetEfficiency() == sym_baby_efficiency);
            REQUIRE( sym_baby->GetEfficiency() == parent_orig_efficiency);
            REQUIRE( s->GetEfficiency() == parent_orig_efficiency);
        }

        THEN("Offspring's points are zero") {
            int sym_baby_points = 0;
            REQUIRE( sym_baby->GetPoints() == sym_baby_points);

        }

        sym_baby.Delete();
    }


    WHEN("Mutation rate is not zero") {
        double efficiency = 0.5;
        double parent_orig_efficiency = 0.5;
        double points = 0.0;
        config.SYM_HORIZ_TRANS_RES(100.0);
        config.HORIZ_TRANS(true);
        config.MUTATION_SIZE(0.01);
        EfficientSymbiont * s2 = new EfficientSymbiont(random, world, &config, int_val, points, efficiency);

        emp::Ptr<Organism> sym_baby = s2->reproduce();


        THEN("Offspring's efficiency value does not equal parent's efficiency value") {
            REQUIRE( sym_baby->GetEfficiency() != parent_orig_efficiency);
            REQUIRE(sym_baby->GetEfficiency() <= 1);
            REQUIRE(sym_baby->GetEfficiency() >= 0);
        }

        THEN("Offspring's points are zero") {
            int sym_baby_points = 0;
            REQUIRE( sym_baby->GetPoints() == sym_baby_points);

        }

        sym_baby.Delete();

    }

}

TEST_CASE("EfficientSymbiont HorizMutate", "[efficient]") {
    emp::Ptr<emp::Random> random = new emp::Random(10);
    SymConfigBase config;
    EfficientWorld w(*random);
    EfficientWorld * world = &w;
    double int_val = 0;
    double efficiency = 0.5;
    double points = 0;
    config.MUTATION_SIZE(0.002);

    WHEN("EfficiencyMutation rate is not zero but everything else is") {
        config.MUTATION_RATE(0);
        config.HORIZ_MUTATION_RATE(0);
        config.EFFICIENCY_MUT_RATE(1);
        EfficientSymbiont * s = new EfficientSymbiont(random, world, &config, int_val, points, efficiency);

        s->HorizMutate();

        THEN("Efficiency changes during horizontal mutation, int val stays the same") {
            REQUIRE(s->GetEfficiency() != efficiency);
            REQUIRE(s->GetIntVal() == int_val);
        }

    }
}

TEST_CASE("EfficientSymbiont's Process called from Host", "[efficient]") {
    emp::Ptr<emp::Random> random = new emp::Random(10);
    SymConfigBase config;
    config.SYM_HORIZ_TRANS_RES(10);
    config.EFFICIENT_SYM(1);
    EfficientWorld w(*random);
    EfficientWorld * world = &w;
    w.Resize(2);

    config.MUTATION_SIZE(0);
    config.MUTATION_RATE(0);

    WHEN("An EfficientSymbiont is added to a Host and about to reproduce horizontally and Host's Process is called") {
        double host_interaction_val = 1;
        double host_points = 0;
        Host * h = new Host(random, &w, &config, host_interaction_val, {}, {}, std::set<int>(), host_points);
        Host * h2 = new Host(random, &w, &config, host_interaction_val, {}, {}, std::set<int>(), host_points);
        double points = 11;
        double int_val = 0;
        double efficiency = 1;
        EfficientSymbiont * s = new EfficientSymbiont(random, world, &config, int_val, points, efficiency);
        h->AddSymbiont(s);
        w.AddOrgAt(h, 0);
        w.AddOrgAt(h2, 1);

        h->Process(0);

        THEN("EfficientSymbiont reproduces and offspring goes into neighboring Host") {
            REQUIRE(h2->GetSymbionts()[0]->GetEfficiency() == efficiency);
        }

    }
}
