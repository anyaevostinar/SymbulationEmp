#include "Host.h"
#include "EfficientSymbiont.h"

TEST_CASE("EfficientSymbiont mutate") {

    emp::Ptr<emp::Random> random = new emp::Random(10);
    SymConfigBase config;
    SymWorld w(*random);
    SymWorld * world = &w;

    WHEN("Mutation rate is not zero") {
        double int_val = 0;
        double efficiency = 0.5;
        double points = 0;
        config.MUTATION_SIZE(0.002);
        EfficientSymbiont * s = new EfficientSymbiont(random, world, &config, int_val, points, efficiency);
        
        s->mutate();

        double efficiency_post_mutation = 0.5016575043;
        THEN("Mutation occurs and efficiency value changes") {
            REQUIRE(s->GetEfficiency() == Approx(efficiency_post_mutation));
        }
    }


    WHEN("Mutation rate is zero") {
        double int_val = 1;
        int orig_efficiency = 0.5;
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

TEST_CASE("EfficientSymbiont AddPoints") {
    emp::Ptr<emp::Random> random = new emp::Random(3);
    SymConfigBase config;
    SymWorld w(*random);
    SymWorld * world = &w;
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

TEST_CASE("EfficientSymbiont reproduce") {

    emp::Ptr<emp::Random> random = new emp::Random(3);
    SymConfigBase config;
    SymWorld w(*random);
    SymWorld * world = &w;
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
            double sym_baby_efficiency = 0.5139135536;
            REQUIRE( sym_baby->GetEfficiency() != parent_orig_efficiency);
            REQUIRE( sym_baby->GetEfficiency() == Approx(sym_baby_efficiency));
        }

        THEN("Offspring's points are zero") {
            int sym_baby_points = 0;
            REQUIRE( sym_baby->GetPoints() == sym_baby_points);

        }

        sym_baby.Delete();

    }
 
}

TEST_CASE("EfficientSymbiont HorizMutate") {
    emp::Ptr<emp::Random> random = new emp::Random(10);
    SymConfigBase config;
    SymWorld w(*random);
    SymWorld * world = &w;
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