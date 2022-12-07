#include "../../efficient_mode/EfficientHost.h"
#include "../../efficient_mode/EfficientSymbiont.h"

TEST_CASE("EfficientSymbiont Constructor", "[efficient]"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    EfficientWorld w(*random, &config);
    EfficientWorld * world = &w;

    double int_val = -1.5;
    REQUIRE_THROWS(emp::NewPtr<EfficientSymbiont>(random, world, &config, int_val));

    int_val = -2;
    emp::Ptr< EfficientSymbiont> symbiont = emp::NewPtr<EfficientSymbiont>(random, world, &config, int_val);
    REQUIRE(symbiont->GetIntVal() >= -1);
    REQUIRE(symbiont->GetIntVal() <= 1);

    int_val = -1;
    emp::Ptr<EfficientSymbiont> symbiont1 = emp::NewPtr<EfficientSymbiont>(random, world, &config, int_val);
    CHECK(symbiont1->GetIntVal() == int_val);
    CHECK(symbiont1->GetAge() == 0);
    CHECK(symbiont1->GetPoints() == 0);

    int_val = -1;
    double points = 10;
    double efficiency = 0.5;

    emp::Ptr<EfficientSymbiont> symbiont2 = emp::NewPtr<EfficientSymbiont>(random, world, &config, int_val, points, efficiency);
    CHECK(symbiont2->GetIntVal() == int_val);
    CHECK(symbiont2->GetEfficiency() == efficiency);
    CHECK(symbiont2->GetAge() == 0);
    CHECK(symbiont2->GetPoints() == points);

    int_val = 1;
    emp::Ptr<EfficientSymbiont> symbiont3 = emp::NewPtr<EfficientSymbiont>(random, world, &config, int_val);
    CHECK(symbiont3->GetIntVal() == int_val);
    CHECK(symbiont3->GetAge() == 0);
    CHECK(symbiont3->GetPoints() == 0);

    int_val = 2;
    REQUIRE_THROWS(emp::NewPtr<EfficientSymbiont>(random, world, &config, int_val) );

    symbiont.Delete();
    symbiont1.Delete();
    symbiont2.Delete();
    symbiont3.Delete();
}

TEST_CASE("EfficientSymbiont Mutate", "[efficient]") {

    emp::Ptr<emp::Random> random = new emp::Random(10);
    SymConfigBase config;
    EfficientWorld w(*random, &config);
    EfficientWorld * world = &w;

    WHEN("Mutation rate is not zero") {
        double int_val = 0;
        double orig_efficiency = 0.5;
        double points = 0;
        config.MUTATION_SIZE(0.002);
        emp::Ptr<EfficientSymbiont> symbiont = emp::NewPtr<EfficientSymbiont>(random, world, &config, int_val, points, orig_efficiency);

        symbiont->Mutate("vertical");

        THEN("Mutation occurs and efficiency value changes, but within bounds") {
            REQUIRE(symbiont->GetEfficiency() != orig_efficiency);
            REQUIRE(symbiont->GetEfficiency() <= 1);
            REQUIRE(symbiont->GetEfficiency() >= 0);
        }
        symbiont.Delete();
    }


    WHEN("Mutation rate is zero") {
        double int_val = 1;
        double orig_efficiency = 0.5;
        double points = 0.0;
        config.SYM_HORIZ_TRANS_RES(100.0);
        config.HORIZ_TRANS(true);
        config.MUTATION_RATE(0);
        config.MUTATION_SIZE(0);
        emp::Ptr<EfficientSymbiont> symbiont = emp::NewPtr<EfficientSymbiont>(random, world, &config, int_val, points, orig_efficiency);

        symbiont->Mutate("vertical");


        THEN("Mutation does not occur and efficiency value does not change") {
            REQUIRE(symbiont->GetEfficiency() == orig_efficiency);
        }
        symbiont.Delete();
    }
}

TEST_CASE("EfficientSymbiont AddPoints", "[efficient]") {
    emp::Ptr<emp::Random> random = new emp::Random(3);
    SymConfigBase config;
    EfficientWorld w(*random, &config);
    EfficientWorld * world = &w;
    double int_val = 0;
    double points = 0;
    double points_in = 10;

    WHEN("Efficiency is 1") {
        double efficiency = 1;
        emp::Ptr<EfficientSymbiont> symbiont = emp::NewPtr<EfficientSymbiont>(random, world, &config, int_val, points, efficiency);

        symbiont->AddPoints(points_in);

        THEN("All points added") {
            REQUIRE( symbiont->GetPoints() == points_in);
        }
        symbiont.Delete();
    }

    WHEN("Efficiency is 0.5") {
        double efficiency = 0.5;
        emp::Ptr<EfficientSymbiont> symbiont = emp::NewPtr<EfficientSymbiont>(random, world, &config, int_val, points, efficiency);

        symbiont->AddPoints(points_in);
        double actual_points = 5; //points_in * 0.5

        THEN("Half points get added") {
            REQUIRE( symbiont->GetPoints() == actual_points);
        }
        symbiont.Delete();
    }

    WHEN("Efficiency is 0") {
        double efficiency = 0;
        emp::Ptr<EfficientSymbiont> symbiont = emp::NewPtr<EfficientSymbiont>(random, world, &config, int_val, points, efficiency);

        symbiont->AddPoints(points_in);
        double actual_points = 0; //points_in * 0

        THEN("No points get added") {
            REQUIRE( symbiont->GetPoints() == actual_points);
        }
        symbiont.Delete();
    }
}

TEST_CASE("INT_VAL_MUT_RATE", "[efficient]") {
    emp::Ptr<emp::Random> random = new emp::Random(11);
    SymConfigBase config;
    EfficientWorld w(*random, &config);
    EfficientWorld * world = &w;

    WHEN("Mutation rate is not zero, but interaction value mut rate is 0 and vertical transmission occurs") {
        double int_val = 0;
        double orig_efficiency = 0.5;
        double points = 0;
        config.MUTATION_SIZE(0.002);
        config.INT_VAL_MUT_RATE(0);
        emp::Ptr<EfficientSymbiont> symbiont = emp::NewPtr<EfficientSymbiont>(random, world, &config, int_val, points, orig_efficiency);

        symbiont->Mutate("vertical");

        THEN("Efficiency mutates but interaction value does not") {

            REQUIRE(symbiont->GetEfficiency() != orig_efficiency);
            REQUIRE(symbiont->GetEfficiency() <= 1);
            REQUIRE(symbiont->GetEfficiency() >= 0);
            REQUIRE(symbiont->GetIntVal() == int_val);
        }
        symbiont.Delete();
    }
    WHEN("Mutation rate is not zero, but interaction value mut rate is 0 and horizontal transmission occurs") {
        double int_val = 0;
        double orig_efficiency = 0.5;
        double points = 0;
        config.MUTATION_SIZE(0.002);
        config.INT_VAL_MUT_RATE(0);
        emp::Ptr<EfficientSymbiont> symbiont = emp::NewPtr<EfficientSymbiont>(random, world, &config, int_val, points, orig_efficiency);

        symbiont->Mutate("horizontal");

        THEN("Efficiency mutates but interaction value does not") {

            REQUIRE(symbiont->GetEfficiency() != orig_efficiency);
            REQUIRE(symbiont->GetEfficiency() <= 1);
            REQUIRE(symbiont->GetEfficiency() >= 0);
            REQUIRE(symbiont->GetIntVal() == int_val);
        }
        symbiont.Delete();
    }

    WHEN("Mutation rate is zero, but interaction value mut rate is not zero and vertical transmission occurs") {
        double int_val = 0;
        double orig_efficiency = 0.5;
        double points = 0;
        config.MUTATION_SIZE(0.002);
        config.MUTATION_RATE(0);
        config.INT_VAL_MUT_RATE(1);
        emp::Ptr<EfficientSymbiont> symbiont = emp::NewPtr<EfficientSymbiont>(random, world, &config, int_val, points, orig_efficiency);

        symbiont->Mutate("vertical");

        THEN("Efficiency does not mutate but interaction value does") {

            REQUIRE(symbiont->GetEfficiency() == orig_efficiency);
            REQUIRE(symbiont->GetIntVal() <= 1);
            REQUIRE(symbiont->GetIntVal() >= -1);
            REQUIRE(symbiont->GetIntVal() != int_val);
        }
        symbiont.Delete();
    }
    WHEN("Mutation rate is zero, but interaction value mut rate is not zero and horizontal transmission occurs") {
        double int_val = 0;
        double orig_efficiency = 0.5;
        double points = 0;
        config.MUTATION_SIZE(0.002);
        config.MUTATION_RATE(0);
        config.INT_VAL_MUT_RATE(1);
        emp::Ptr<EfficientSymbiont> symbiont = emp::NewPtr<EfficientSymbiont>(random, world, &config, int_val, points, orig_efficiency);

        symbiont->Mutate("horizontal");

        THEN("Efficiency does not mutate but interaction value does") {

            REQUIRE(symbiont->GetEfficiency() == orig_efficiency);
            REQUIRE(symbiont->GetIntVal() <= 1);
            REQUIRE(symbiont->GetIntVal() >= -1);
            REQUIRE(symbiont->GetIntVal() != int_val);
        }
        symbiont.Delete();

    }


}

TEST_CASE("EfficientSymbiont Reproduce", "[efficient]") {

    emp::Ptr<emp::Random> random = new emp::Random(3);
    SymConfigBase config;
    EfficientWorld w(*random, &config);
    EfficientWorld * world = &w;
    double int_val = 0;


    WHEN("Mutation rate is zero")  {
        // double efficiency = 0.5;
        double parent_orig_efficiency = 0.5;
        double points = 0.0;
        config.SYM_HORIZ_TRANS_RES(100.0);
        config.HORIZ_TRANS(true);
        config.MUTATION_SIZE(0);
        emp::Ptr<EfficientSymbiont> symbiont = emp::NewPtr<EfficientSymbiont>(random, world, &config, int_val, points, parent_orig_efficiency);
        symbiont->SetAge(10);

        emp::Ptr<EfficientSymbiont> sym_baby = symbiont->Reproduce("vertical").DynamicCast<EfficientSymbiont>();


        THEN("Offspring's efficiency equals parent's efficiency") {
            double sym_baby_efficiency = 0.5;
            REQUIRE( sym_baby->GetEfficiency() == sym_baby_efficiency);
            REQUIRE( sym_baby->GetEfficiency() == parent_orig_efficiency);
            REQUIRE( symbiont->GetEfficiency() == parent_orig_efficiency);
        }

        THEN("Offspring's points are zero") {
            int sym_baby_points = 0;
            REQUIRE( sym_baby->GetPoints() == sym_baby_points);

        }

        THEN("Offspring's age is 0") {
            REQUIRE(sym_baby->GetAge() == 0);
        }
        symbiont.Delete();
        sym_baby.Delete();
    }


    WHEN("Mutation rate is not zero") {
        double efficiency = 0.5;
        double parent_orig_efficiency = 0.5;
        double points = 0.0;
        config.SYM_HORIZ_TRANS_RES(100.0);
        config.HORIZ_TRANS(true);
        config.MUTATION_SIZE(0.01);
        emp::Ptr<EfficientSymbiont> symbiont2 = emp::NewPtr<EfficientSymbiont>(random, world, &config, int_val, points, efficiency);

        emp::Ptr<EfficientSymbiont> sym_baby = symbiont2->Reproduce("vertical").DynamicCast<EfficientSymbiont>();


        THEN("Offspring's efficiency value does not equal parent's efficiency value") {
            REQUIRE(sym_baby->GetEfficiency() != parent_orig_efficiency);
            REQUIRE(sym_baby->GetEfficiency() <= 1);
            REQUIRE(sym_baby->GetEfficiency() >= 0);
        }

        THEN("Offspring's points are zero") {
            int sym_baby_points = 0;
            REQUIRE( sym_baby->GetPoints() == sym_baby_points);

        }

        THEN("Offspring's age is 0") {
            REQUIRE(sym_baby->GetAge() == 0);
        }
        symbiont2.Delete();
        sym_baby.Delete();

    }

}

TEST_CASE("EfficientSymbiont HorizMutate", "[efficient]") {
    emp::Ptr<emp::Random> random = new emp::Random(10);
    SymConfigBase config;
    EfficientWorld w(*random, &config);
    EfficientWorld * world = &w;
    double int_val = 0;
    double efficiency = 0.5;
    double points = 0;
    config.MUTATION_SIZE(0.002);

    WHEN("EfficiencyMutation rate is not zero but everything else is") {
        config.MUTATION_RATE(0);
        config.HORIZ_MUTATION_RATE(0);
        config.EFFICIENCY_MUT_RATE(1);
        emp::Ptr<EfficientSymbiont> symbiont = emp::NewPtr<EfficientSymbiont>(random, world, &config, int_val, points, efficiency);

        symbiont->Mutate("horizontal");

        THEN("Efficiency changes during horizontal mutation, int val stays the same") {
            REQUIRE(symbiont->GetEfficiency() != efficiency);
            REQUIRE(symbiont->GetIntVal() == int_val);
        }
        symbiont.Delete();
    }
}

TEST_CASE("EfficientSymbiont Mutate with horizontal transmission", "[efficient]") {
    emp::Ptr<emp::Random> random = new emp::Random(10);
    SymConfigBase config;
    EfficientWorld w(*random, &config);
    EfficientWorld * world = &w;
    double int_val = 0;
    double efficiency = 0.5;
    double points = 0;
    config.MUTATION_SIZE(0.002);

    WHEN("EfficiencyMutation rate is not zero but everything else is") {
        config.MUTATION_RATE(0);
        config.HORIZ_MUTATION_RATE(0);
        config.EFFICIENCY_MUT_RATE(1);
        emp::Ptr<EfficientSymbiont> symbiont = emp::NewPtr<EfficientSymbiont>(random, world, &config, int_val, points, efficiency);

        symbiont->Mutate("horizontal");

        THEN("Efficiency changes during horizontal mutation, int val stays the same") {
            REQUIRE(symbiont->GetEfficiency() != efficiency);
            REQUIRE(symbiont->GetIntVal() == int_val);
        }
        symbiont.Delete();
    }
}

TEST_CASE("EfficientSymbiont Mutate with vertical transmission", "[efficient]") {
    emp::Ptr<emp::Random> random = new emp::Random(10);
    SymConfigBase config;
    EfficientWorld w(*random, &config);
    EfficientWorld * world = &w;
    double int_val = 0;
    double efficiency = 0.5;
    double points = 0;
    config.MUTATION_SIZE(0.002);

    WHEN("EfficiencyMutation rate is -1,  HMR is 0, regular mutation rate is not 0 and transmission is vertical") {
        config.MUTATION_RATE(1);
        config.HORIZ_MUTATION_RATE(0);
        config.EFFICIENCY_MUT_RATE(-1);
        emp::Ptr<EfficientSymbiont> symbiont = emp::NewPtr<EfficientSymbiont>(random, world, &config, int_val, points, efficiency);

        symbiont->Mutate("vertical");

        THEN("Efficiency and int val should change because pulls from regular mutation rate") {
            REQUIRE(symbiont->GetEfficiency() != efficiency);
            REQUIRE(symbiont->GetIntVal() != int_val);
        }
        symbiont.Delete();
    }
}

TEST_CASE("EfficientSymbiont's Process called from Host when mutation rate and size are zero", "[efficient]") {
    emp::Ptr<emp::Random> random = new emp::Random(25);
    random->GetUInt(0, 1); //issue with random number generator led to location 0 being picked for most random seeds on the first random number
    SymConfigBase config;
    config.SYM_HORIZ_TRANS_RES(10);
    config.EFFICIENT_SYM(1);
    EfficientWorld world(*random, &config);
    world.Resize(4);
    config.MUTATION_SIZE(0);
    config.MUTATION_RATE(0);
    double points = 11;
    double int_val = 0;
    double efficiency = 0.9;
    double host_interaction_val = 1;

    WHEN("The horizontal transmission mutation rate and size are also zero and an EfficientSymbiont is added to a Host and about to reproduce horizontally and Host's Process is called") {
        emp::Ptr<EfficientHost> host = emp::NewPtr<EfficientHost>(random, &world, &config, host_interaction_val);

        emp::Ptr<EfficientHost> host2 = emp::NewPtr<EfficientHost>(random, &world, &config, host_interaction_val);
        emp::Ptr<EfficientHost> host3 = emp::NewPtr<EfficientHost>(random, &world, &config, host_interaction_val);
        emp::Ptr<EfficientHost> host4 = emp::NewPtr<EfficientHost>(random, &world, &config, host_interaction_val);
        emp::Ptr<EfficientSymbiont> symbiont = emp::NewPtr<EfficientSymbiont>(random, &world, &config, int_val, points, efficiency);
        host->AddSymbiont(symbiont);
        world.AddOrgAt(host, 0);
        world.AddOrgAt(host2, 1);
        world.AddOrgAt(host3, 2);
        world.AddOrgAt(host4, 3);

        host->Process(0);

        THEN("EfficientSymbiont reproduces and offspring goes into neighboring Host and offspring has identical efficiency value") {
            emp::Ptr<EfficientHost> new_infected = nullptr;
            if(host2->HasSym()) {
                new_infected = host2;
                REQUIRE(host3->HasSym() == false);
                REQUIRE(host4->HasSym() == false);
            } else if (host3->HasSym()) {
                new_infected = host3;
                REQUIRE(host2->HasSym() == false);
                REQUIRE(host4->HasSym() == false);
            } else if(host4->HasSym()) {
                new_infected = host4;
                REQUIRE(host3->HasSym() == false);
                REQUIRE(host2->HasSym() == false);
            }
            REQUIRE(new_infected != nullptr);
            REQUIRE(new_infected->HasSym());
            REQUIRE(new_infected->GetSymbionts()[0].DynamicCast<EfficientSymbiont>()->GetEfficiency() == efficiency);
        }
    }

    WHEN("The horizontal mutation rate and size are not zero and an EfficientSymbiont is added to a Host and about to reproduce horizontally and Host's Process is called") {
        config.HORIZ_MUTATION_SIZE(0.002);
        config.HORIZ_MUTATION_RATE(1.0);
        emp::Ptr<EfficientHost> host = emp::NewPtr<EfficientHost>(random, &world, &config, host_interaction_val);
        emp::Ptr<EfficientHost> host2 = emp::NewPtr<EfficientHost>(random, &world, &config, host_interaction_val);
        emp::Ptr<EfficientHost> host3 = emp::NewPtr<EfficientHost>(random, &world, &config, host_interaction_val);
        emp::Ptr<EfficientHost> host4 = emp::NewPtr<EfficientHost>(random, &world, &config, host_interaction_val);

        emp::Ptr<EfficientSymbiont> symbiont = emp::NewPtr<EfficientSymbiont>(random, &world, &config, int_val, points, efficiency);
        host->AddSymbiont(symbiont);
        world.AddOrgAt(host, 0);
        world.AddOrgAt(host2, 1);
        world.AddOrgAt(host3, 2);
        world.AddOrgAt(host4, 3);

        host->Process(0);

        THEN("EfficientSymbiont reproduces and offspring with a different efficiency value goes into neighboring Host") {
            emp::Ptr<EfficientHost> new_infected = nullptr;
            if(host2->HasSym()) {
                new_infected = host2;
                REQUIRE(host3->HasSym() == false);
                REQUIRE(host4->HasSym() == false);
            } else if (host3->HasSym()) {
                new_infected = host3;
                REQUIRE(host2->HasSym() == false);
                REQUIRE(host4->HasSym() == false);
            } else if(host4->HasSym()) {
                new_infected = host4;
                REQUIRE(host3->HasSym() == false);
                REQUIRE(host2->HasSym() == false);
            }
            REQUIRE(new_infected != nullptr);
            REQUIRE(new_infected->HasSym());
            REQUIRE(new_infected->GetSymbionts()[0].DynamicCast<EfficientSymbiont>()->GetEfficiency() != efficiency);
            REQUIRE(new_infected->GetSymbionts()[0].DynamicCast<EfficientSymbiont>()->GetIntVal() != int_val);
        }

    }
}

TEST_CASE("EfficientSymbiont MakeNew", "[efficient]"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    EfficientWorld world(*random, &config);

    double sym_int_val = 0.2;
    emp::Ptr<EfficientSymbiont> symbiont1 = emp::NewPtr<EfficientSymbiont>(random, &world, &config, sym_int_val);
    emp::Ptr<EfficientSymbiont> symbiont2 = symbiont1->MakeNew();

    THEN("The new efficient symbiont has the same genome as its parent, but age and points 0"){
        REQUIRE(symbiont2->GetIntVal() == symbiont1->GetIntVal());
        REQUIRE(symbiont2->GetInfectionChance() == symbiont1->GetInfectionChance());
        REQUIRE(symbiont2->GetEfficiency() == symbiont1->GetEfficiency());
        REQUIRE(symbiont2->GetAge() == 0);
        REQUIRE(symbiont2->GetPoints() == 0);
        //check that the offspring is the correct class
        REQUIRE(symbiont2->GetName() == "EfficientSymbiont");
    }
    symbiont1.Delete();
    symbiont2.Delete();
}

TEST_CASE("EfficientSymbiont SetEfficiency and GetEfficiency", "[efficient]"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    EfficientWorld w(*random, &config);
    EfficientWorld * world = &w;
    double int_val = -1;
    emp::Ptr<EfficientSymbiont> symbiont = emp::NewPtr<EfficientSymbiont>(random, world, &config, int_val);

    double efficiency = 0.5;
    symbiont->SetEfficiency(efficiency);
    double expected_efficieny = 0.5;
    REQUIRE(symbiont->GetEfficiency() == expected_efficieny);

    symbiont.Delete();
}

TEST_CASE("EfficientSymbiont VerticalTransmission", "[efficient]"){
  double int_val = 0;
  emp::Ptr<emp::Random> random = new emp::Random(-1);
  SymConfigBase config;
  EfficientWorld world(*random, &config);
  int points_to_transmit = 100;

  config.SYM_VERT_TRANS_RES(points_to_transmit);


  emp::Ptr<Symbiont> symbiont = emp::NewPtr<EfficientSymbiont>(random, &world, &config, int_val);
  emp::Ptr<Organism> host = emp::NewPtr<EfficientHost>(random, &world, &config, int_val);
  REQUIRE(host->HasSym() == false);

  WHEN("the world permits vertical transmission"){
    config.VERTICAL_TRANSMISSION(1);

    WHEN("the symbiont has enough points to transmit"){
      symbiont->SetPoints(points_to_transmit);

      THEN("the symbiont offspring is vertically transmitted to the host"){
        symbiont->VerticalTransmission(host);
        REQUIRE(host->HasSym() == true);
      }
    }

    WHEN("the symbiont does not have enough points to transmit"){
      symbiont->SetPoints(points_to_transmit - 1);

      THEN("vertical transmission does not occur"){
        symbiont->VerticalTransmission(host);
        REQUIRE(host->HasSym() == false);
      }
    }
  }

  WHEN("the world does not permit vertical transmission"){
    config.VERTICAL_TRANSMISSION(0);
    symbiont->SetPoints(points_to_transmit);

    THEN("vertical transmission does not occur"){
      symbiont->VerticalTransmission(host);
      REQUIRE(host->HasSym() == false);
    }
  }
  host.Delete();
  symbiont.Delete();
}
