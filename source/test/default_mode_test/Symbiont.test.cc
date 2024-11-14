#include "../../default_mode/Host.h"
#include "../../default_mode/Symbiont.h"

TEST_CASE("Symbiont Constructor", "[default]") {

    emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(10);
    SymConfigBase config;
    emp::Ptr<SymWorld> world = emp::NewPtr<SymWorld>(*random, &config);
    double int_val; 

    WHEN("An interaction value of -2 is passed") {
      int_val = -2;
      emp::Ptr<Symbiont> symbiont = emp::NewPtr<Symbiont>(random, world, &config, int_val);
      THEN("The symbiont randomly determines its interaction value") {
        REQUIRE(symbiont->GetIntVal() >= -1);
        REQUIRE(symbiont->GetIntVal() <= 1);
      }
      symbiont.Delete();
    }

    WHEN("An interaction value < -1 other than -2 is passed") {
      int_val = -1.5;
      THEN("An exception is thrown") {
        REQUIRE_THROWS(emp::NewPtr<Symbiont>(random, world, &config, int_val));
      }
    }
    int_val = -1;
    emp::Ptr<Symbiont> sym1 = emp::NewPtr<Symbiont>(random, world, &config, int_val);
    CHECK(sym1->GetIntVal() == int_val);
    CHECK(sym1->GetAge() == 0);
    CHECK(sym1->GetPoints() == 0);

    int_val = -1;
    double points = 10;
    emp::Ptr<Symbiont> sym2 = emp::NewPtr<Symbiont>(random, world, &config, int_val, points);
    CHECK(sym2->GetIntVal() == int_val);
    CHECK(sym2->GetAge() == 0);
    CHECK(sym2->GetPoints() == points);

    int_val = 1;
    emp::Ptr<Symbiont> sym3 = emp::NewPtr<Symbiont>(random, world, &config, int_val);
    CHECK(sym3->GetIntVal() == int_val);
    CHECK(sym3->GetAge() == 0);
    CHECK(sym3->GetPoints() == 0);

    int_val = 2;
    REQUIRE_THROWS(emp::NewPtr<Symbiont>(random, world, &config, int_val) );

    sym1.Delete();
    sym2.Delete();
    sym3.Delete();
    world.Delete();
    random.Delete();
}

TEST_CASE("SetIntVal, GetIntVal", "[default]") {

    emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(10);
    SymConfigBase config;
    emp::Ptr<SymWorld> world = emp::NewPtr<SymWorld>(*random, &config);

    double int_val = -1;
    emp::Ptr<Symbiont> sym1 = emp::NewPtr<Symbiont>(random, world, &config, int_val);

    int_val = -2;
    REQUIRE_THROWS( sym1->SetIntVal(int_val) );

    int_val = -1;
    emp::Ptr<Symbiont> sym2 = emp::NewPtr<Symbiont>(random, world, &config, int_val);

    int_val = 1;
    sym2->SetIntVal(int_val);
    double orig_int_val = 1;

    REQUIRE(sym2->GetIntVal() == orig_int_val);

    int_val = -1;
    emp::Ptr<Symbiont> sym3 = emp::NewPtr<Symbiont>(random, world, &config, int_val);

    int_val = 2;
    REQUIRE_THROWS( sym3->SetIntVal(int_val) ) ;

    int_val = 0;
    emp::Ptr<Symbiont> sym4 = emp::NewPtr<Symbiont>(random, world, &config, int_val);

    int_val = -1;
    sym4->SetIntVal(int_val);
    orig_int_val = -1;

    REQUIRE( sym4->GetIntVal() == orig_int_val) ;

    sym1.Delete();
    sym2.Delete();
    sym3.Delete();
    sym4.Delete();
    world.Delete();
    random.Delete();
}

TEST_CASE("SetInfectionChance, GetInfectionChance", "[default]") {

    emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(10);
    SymConfigBase config;
    emp::Ptr<SymWorld> world = emp::NewPtr<SymWorld>(*random, &config);

    double int_val = -1;
    emp::Ptr<Symbiont> sym = emp::NewPtr<Symbiont>(random, world, &config, int_val);

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

    sym.Delete();
    world.Delete();
    random.Delete();
}

TEST_CASE("SetPoints, GetPoints", "[default]") {

    emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(10);
    SymConfigBase config;
    emp::Ptr<SymWorld> world = emp::NewPtr<SymWorld>(*random, &config);

    double int_val = -1;
    emp::Ptr<Symbiont> sym = emp::NewPtr<Symbiont>(random, world, &config, int_val);

    int points = 1;
    sym->SetPoints(points);
    double orig_points = 1;

    REQUIRE( sym->GetPoints() == orig_points);

    int add_points = -1;
    sym->AddPoints(add_points);
    orig_points = 0;

    REQUIRE( sym->GetPoints() == orig_points);

    add_points = 150;
    sym->AddPoints(add_points);
    orig_points = 150;

    REQUIRE(sym->GetPoints() == orig_points);

    sym.Delete();
    world.Delete();
    random.Delete();
}

TEST_CASE("Symbiont SetDead, GetDead", "[default]"){
    emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(10);
    SymConfigBase config;
    emp::Ptr<SymWorld> world = emp::NewPtr<SymWorld>(*random, &config);

    double int_val = -1;
    emp::Ptr<Symbiont> sym = emp::NewPtr<Symbiont>(random, world, &config, int_val);

    //new symbionts are alive until specified otherwise
    bool expected_dead = false;
    REQUIRE(sym->GetDead() == expected_dead);

    //verify that setting it to dead means that death is set to true
    expected_dead = true;
    sym->SetDead();
    REQUIRE(sym->GetDead() == expected_dead);

    sym.Delete();
    world.Delete();
    random.Delete();
}

TEST_CASE("WantsToInfect", "[default]"){
    emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(17);
    SymConfigBase config;
    emp::Ptr<SymWorld> world = emp::NewPtr<SymWorld>(*random, &config);
    double int_val = 0;

    WHEN("Symbiont infection chance is 0"){
        config.SYM_INFECTION_CHANCE(0);
        emp::Ptr<Symbiont> sym1 = emp::NewPtr<Symbiont>(random, world, &config, int_val);
        emp::Ptr<Symbiont> sym2 = emp::NewPtr<Symbiont>(random, world, &config, int_val);

        THEN("Symbionts never want to infect"){
            REQUIRE(sym1->WantsToInfect() == false);
            REQUIRE(sym2->WantsToInfect() == false);
        }

        sym1.Delete();
        sym2.Delete();
    }

    WHEN("Symbiont infection chance is 1"){
        config.SYM_INFECTION_CHANCE(1);
        emp::Ptr<Symbiont> sym1 = emp::NewPtr<Symbiont>(random, world, &config, int_val);
        emp::Ptr<Symbiont> sym2 = emp::NewPtr<Symbiont>(random, world, &config, int_val);

        THEN("Symbionts always want to infect"){
            REQUIRE(sym1->WantsToInfect() == true);
            REQUIRE(sym2->WantsToInfect() == true);
        }

        sym1.Delete();
        sym2.Delete();
    }
    world.Delete();
    random.Delete();
}

TEST_CASE("InfectionFails", "[default]"){
    emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(17);
    SymConfigBase config;
    emp::Ptr<SymWorld> world = emp::NewPtr<SymWorld>(*random, &config);
    double int_val = 0;

    WHEN("Symbiont infection failure rate is 0"){
        config.SYM_INFECTION_FAILURE_RATE(0);
        emp::Ptr<Symbiont> sym1 = emp::NewPtr<Symbiont>(random, world, &config, int_val);
        emp::Ptr<Symbiont> sym2 = emp::NewPtr<Symbiont>(random, world, &config, int_val);

        THEN("infection never fails"){
            REQUIRE(sym1->InfectionFails() == false);
            REQUIRE(sym2->InfectionFails() == false);
        }

        sym1.Delete();
        sym2.Delete();
    }

    WHEN("Symbiont infection failure rate is between 0 and 1"){
        config.SYM_INFECTION_FAILURE_RATE(0.5);
        emp::Ptr<Organism> sym1 = emp::NewPtr<Symbiont>(random, world, &config, int_val);
        emp::Ptr<Organism> sym2 = emp::NewPtr<Symbiont>(random, world, &config, int_val);
        emp::Ptr<Organism> sym3 = emp::NewPtr<Symbiont>(random, world, &config, int_val);
        emp::Ptr<Organism> sym4 = emp::NewPtr<Symbiont>(random, world, &config, int_val);
        size_t failed_infection_count = 0;
        size_t total_possible = 4;

        if(sym1->InfectionFails() == true) failed_infection_count = failed_infection_count + 1;
        if(sym2->InfectionFails() == true) failed_infection_count = failed_infection_count + 1;
        if(sym3->InfectionFails() == true) failed_infection_count = failed_infection_count + 1;
        if(sym4->InfectionFails() == true) failed_infection_count = failed_infection_count + 1;

        THEN("infection sometimes fails, sometimes doesn't"){
            REQUIRE(failed_infection_count < total_possible);
            REQUIRE(failed_infection_count > 0);
        }

        sym1.Delete();
        sym2.Delete();
        sym3.Delete();
        sym4.Delete();
    }

    WHEN("Symbiont infection failure rate is 1"){
        config.SYM_INFECTION_FAILURE_RATE(1);
        emp::Ptr<Symbiont> sym1 = emp::NewPtr<Symbiont>(random, world, &config, int_val);
        emp::Ptr<Symbiont> sym2 = emp::NewPtr<Symbiont>(random, world, &config, int_val);

        THEN("infection always fails"){
            REQUIRE(sym1->InfectionFails() == true);
            REQUIRE(sym2->InfectionFails() == true);
        }

        sym1.Delete();
        sym2.Delete();
    }
    world.Delete();
    random.Delete();
}

TEST_CASE("mutate", "[default]") {

    emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(37);
    SymConfigBase config;
    emp::Ptr<SymWorld> world = emp::NewPtr<SymWorld>(*random, &config);

    WHEN("Mutation rate is not zero") {
        double int_val = 0;
        double orig_infection_chance = 1;
        config.MUTATION_SIZE(0.002);

        WHEN("free living symbionts are allowed"){
            config.FREE_LIVING_SYMS(1);
            config.SYM_INFECTION_CHANCE(orig_infection_chance);
            emp::Ptr<Symbiont> sym = emp::NewPtr<Symbiont>(random, world, &config, int_val);
            sym->Mutate();

            THEN("Mutation occurs and both interaction value and infection chance change"){
                REQUIRE(sym->GetIntVal() != int_val);
                REQUIRE(sym->GetIntVal() <= 1);
                REQUIRE(sym->GetIntVal() >= -1);
                REQUIRE(sym->GetInfectionChance() != orig_infection_chance);
                REQUIRE(sym->GetInfectionChance() >= 0);
                REQUIRE(sym->GetInfectionChance() <= 1);
            }

            sym.Delete();
        }

        WHEN("free living symbionts are not allowed"){
            emp::Ptr<Symbiont> sym = emp::NewPtr<Symbiont>(random, world, &config, int_val);
            sym->Mutate();

            THEN("Mutation occurs and only interaction value changes") {
                REQUIRE(sym->GetIntVal() != int_val);
                REQUIRE(sym->GetIntVal() <= 1);
                REQUIRE(sym->GetIntVal() >= -1);
            }

            sym.Delete();
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
        emp::Ptr<Symbiont> sym = emp::NewPtr<Symbiont>(random, world, &config, int_val, points);

        sym->Mutate();


        THEN("Mutation does not occur and interaction value does not change") {
            REQUIRE(sym->GetIntVal() == orig_int_val);
        }

        sym.Delete();
    }

    //TAG MUTATION SIZE
    WHEN("Tag matching is on") {
      THEN("Tags mutate according to tag mutation rate") {
        emp::HammingMetric<32> metric = emp::HammingMetric<32>();
        config.TAG_MATCHING(1);
        config.TAG_MUTATION_SIZE(0.1);
        emp::Ptr<Symbiont> symbiont = emp::NewPtr<Symbiont>(random, world, &config, 0);
        emp::BitSet<32> bit_set = emp::BitSet<32>();
        symbiont->SetTag(bit_set);

        REQUIRE(metric.calculate(symbiont->GetTag(), bit_set) == 0);
        symbiont->Mutate();
        REQUIRE(metric.calculate(symbiont->GetTag(), bit_set) > 0);
        REQUIRE(metric.calculate(symbiont->GetTag(), bit_set) <= 1);

        symbiont.Delete();
        config.TAG_MATCHING(0); // don't try to delete a non-existent
        // SymWorld metric
      }
    }
    world.Delete();
    random.Delete();
}

TEST_CASE("reproduce", "[default]") {

    emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(3);
    SymConfigBase config;
    emp::Ptr<SymWorld> world = emp::NewPtr<SymWorld>(*random, &config);


    WHEN("Mutation rate is zero")  {
        double int_val = 0;
        double inf_chance = 0.5;
        double parent_orig_inf_chance = 0.5;
        double parent_orig_int_val = 0;
        double points = 0.0;
        config.SYM_HORIZ_TRANS_RES(100.0);
        config.HORIZ_TRANS(true);
        config.MUTATION_SIZE(0);
        config.FREE_LIVING_SYMS(1);
        config.SYM_INFECTION_CHANCE(inf_chance);
        emp::Ptr<Symbiont> sym = emp::NewPtr<Symbiont>(random, world, &config, int_val, points);
        sym->SetAge(10);

        emp::Ptr<Organism> sym_baby = sym->Reproduce();


        THEN("Offspring's interaction value equals parent's interaction value") {
            double sym_baby_int_val = 0;
            REQUIRE( sym_baby->GetIntVal() == sym_baby_int_val);
            REQUIRE( sym_baby->GetIntVal() == parent_orig_int_val);
            REQUIRE( sym->GetIntVal() == parent_orig_int_val);
        }

        THEN("Offspring's infection chance equals parent's infection chance") {
            double sym_baby_inf_chance = 0.5;
            REQUIRE( sym_baby->GetInfectionChance() == sym_baby_inf_chance);
            REQUIRE( sym_baby->GetInfectionChance() == parent_orig_inf_chance);
            REQUIRE( sym->GetInfectionChance() == parent_orig_inf_chance);
        }

        THEN("Offspring's points are zero") {
            int sym_baby_points = 0;
            REQUIRE( sym_baby->GetPoints() == sym_baby_points);

        }

        THEN("Offspring's age is 0") {
            REQUIRE(sym_baby->GetAge() == 0);
        }

        sym.Delete();
        sym_baby.Delete();
    }


    WHEN("Mutation rate is not zero") {
        double int_val = 0;
        double inf_chance = 0.5;
        double parent_orig_inf_chance = 0.5;
        double parent_orig_int_val = 0;
        double points = 0.0;
        config.SYM_HORIZ_TRANS_RES(100.0);
        config.HORIZ_TRANS(true);
        config.MUTATION_SIZE(0.01);
        config.FREE_LIVING_SYMS(1);
        config.SYM_INFECTION_CHANCE(inf_chance);
        emp::Ptr<Symbiont> sym2 = emp::NewPtr<Symbiont>(random, world, &config, int_val, points);

        emp::Ptr<Organism> sym_baby = sym2->Reproduce();


        THEN("Offspring's interaction value does not equal parent's interaction value") {
            REQUIRE(sym_baby->GetIntVal() != parent_orig_int_val);
            REQUIRE(sym_baby->GetIntVal() <= 1);
            REQUIRE(sym_baby->GetIntVal() >= -1);
            REQUIRE(sym2->GetIntVal() == parent_orig_int_val);
        }

        THEN("Offspring's infection chance does not equal parent's infection chance") {
            REQUIRE( sym_baby->GetInfectionChance() != parent_orig_inf_chance);
            REQUIRE( sym_baby->GetInfectionChance() <= 1);
            REQUIRE( sym_baby->GetInfectionChance() >= -1);
            REQUIRE( sym2->GetInfectionChance() == parent_orig_inf_chance);
        }

        THEN("Offspring's points are zero") {
            int sym_baby_points = 0;
            REQUIRE( sym_baby->GetPoints() == sym_baby_points);

        }

        THEN("Offspring's age is 0") {
            REQUIRE(sym_baby->GetAge() == 0);
        }

        sym2.Delete();
        sym_baby.Delete();
    }
    world.Delete();
    random.Delete();
}

TEST_CASE("Process", "[default]") {

    emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(10);
    SymConfigBase config;
    emp::Ptr<SymWorld> world = emp::NewPtr<SymWorld>(*random, &config);

    //add new test for free living sym not moving when it shouldn't
    WHEN("Horizontal transmission is true and points is greater than sym_h_res") {
        double int_val = 1;
        // double parent_orig_int_val = 1;
        double points = 0.0;
        config.SYM_HORIZ_TRANS_RES(140.0);
        config.HORIZ_TRANS(true);
        config.MUTATION_SIZE(0);
        emp::Ptr<Symbiont> sym = emp::NewPtr<Symbiont>(random, world, &config, int_val, points);

        int add_points = 200;
        sym->AddPoints(add_points);

        int location = 10;
        sym->Process(location);


        THEN("Points changes and is set to 0") {
            int points_post_reproduction = 0;
            REQUIRE(sym->GetPoints() == points_post_reproduction);
        }

        sym.Delete();
    }


    WHEN("Horizontal transmission is true and points is less than sym_h_res") {
        double int_val = 1;
        // double parent_orig_int_val = 1;
        double points = 0.0;
        config.SYM_HORIZ_TRANS_RES(200.0);
        config.HORIZ_TRANS(true);
        config.MUTATION_SIZE(0.0);
        emp::Ptr<Symbiont> sym = emp::NewPtr<Symbiont>(random, world, &config, int_val, points);

        int add_points = 50;
        sym->AddPoints(add_points);

        int location = 10;
        sym->Process(location);


        THEN("Points does not change") {
            int points_post_reproduction = 50;
            REQUIRE(sym->GetPoints() == points_post_reproduction);
        }

        sym.Delete();
    }


    WHEN("Horizontal transmission is false and points and points is greater then sym_h_res") {
        double int_val = 1;
        // double parent_orig_int_val = 1;
        double points = 100.0;
        config.SYM_HORIZ_TRANS_RES(80.0);
        config.HORIZ_TRANS(false);
        config.MUTATION_SIZE(0.0);
        emp::Ptr<Symbiont> sym = emp::NewPtr<Symbiont>(random, world, &config, int_val, points);

        int location = 10;
        sym->Process(location);


        THEN("Points does not change") {
            int points_post_reproduction = 100;
            REQUIRE(sym->GetPoints() == points_post_reproduction);
        }

        sym.Delete();
    }

    WHEN("Horizontal transmission is false and points and points is less then sym_h_res") {
        double int_val = 1;
        // double parent_orig_int_val = 1;
        double points = 40.0;
        config.SYM_HORIZ_TRANS_RES(80.0);
        config.HORIZ_TRANS(false);
        config.MUTATION_SIZE(0.0);
        emp::Ptr<Symbiont> sym = emp::NewPtr<Symbiont>(random, world, &config, int_val, points);

        int location = 10;
        sym->Process(location);


        THEN("Points does not change") {
            int points_post_reproduction = 40;
            REQUIRE(sym->GetPoints() == points_post_reproduction);
        }

        sym.Delete();
    }

    WHEN("The symbiont is free living and horizontal transmission is true") {
      config.FREE_LIVING_SYMS(1);
      config.SYM_HORIZ_TRANS_RES(140.0);
      config.HORIZ_TRANS(true);
      double int_val = 0;
      int points = 0;

      WHEN("Free living symbionts have a different reproductive resource threshold than hosted symbionts") {
        int free_sym_repro_res = 70;
        config.FREE_SYM_REPRO_RES(free_sym_repro_res);
        emp::Ptr<Symbiont> sym = emp::NewPtr<Symbiont>(random, world, &config, int_val, points);
        emp::WorldPosition location = emp::WorldPosition(0, 10);
        world->AddOrgAt(sym, location);

        WHEN("The free living symbiont does not have enough resources to reproduce") {
          int orig_points = free_sym_repro_res - 10;
          size_t orig_num_orgs = world->GetNumOrgs();
          sym->AddPoints(orig_points);
          sym->Process(location);

          THEN("The free living symbiont does not reproduce") {
            REQUIRE(world->GetNumOrgs() == orig_num_orgs);
            REQUIRE(sym->GetPoints() == orig_points);
          }

        }
        WHEN("The free living symbiont has enough resources to reproduce") {
          int orig_points = free_sym_repro_res + 10;
          size_t orig_num_orgs = world->GetNumOrgs();
          sym->AddPoints(orig_points);
          sym->Process(location);

          THEN("The free living symbiont reproduces and sets its points to 0") {
            REQUIRE(world->GetNumOrgs() == (orig_num_orgs + 1));
            REQUIRE(sym->GetPoints() == 0);
          }
        }
      }
      WHEN("Free living symbionts have the same reproductive resource threshold as hosted symbionts do for horizontal transmission") {
        int sym_h_res = 120;
        config.SYM_HORIZ_TRANS_RES(sym_h_res);
        config.FREE_SYM_REPRO_RES(-1);
        emp::Ptr<Symbiont> sym = emp::NewPtr<Symbiont>(random, world, &config, int_val, points);
        emp::WorldPosition location = emp::WorldPosition(0, 10);
        world->AddOrgAt(sym, location);

        WHEN("The free living symbiont does not have enough resources to reproduce") {
          int orig_points = sym_h_res - 10;
          size_t orig_num_orgs = world->GetNumOrgs();
          sym->AddPoints(orig_points);
          sym->Process(location);

          THEN("The free living symbiont does not reproduce") {
            REQUIRE(world->GetNumOrgs() == orig_num_orgs);
            REQUIRE(sym->GetPoints() == orig_points);
          }

        }
        WHEN("The free living symbiont has enough resources to reproduce") {
          int orig_points = sym_h_res + 10;
          size_t orig_num_orgs = world->GetNumOrgs();
          sym->AddPoints(orig_points);
          sym->Process(location);

          THEN("The free living symbiont reproduces and sets its points to 0") {
            REQUIRE(world->GetNumOrgs() == (orig_num_orgs + 1));
            REQUIRE(sym->GetPoints() == 0);
          }
        }
      }
    }
    world.Delete();
    random.Delete();
}

TEST_CASE("Symbiont ProcessResources", "[default]"){
    emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(10);
    SymConfigBase config;
    emp::Ptr<SymWorld> world = emp::NewPtr<SymWorld>(*random, &config);
    config.SYNERGY(5);


    WHEN("sym_int_val < 0"){
        double sym_int_val = -0.6;

        WHEN("host_int_val > 0"){
            double host_int_val = 0.2;
            emp::Ptr<Host> host = emp::NewPtr<Host>(random, world, &config, host_int_val);
            emp::Ptr<Symbiont> sym = emp::NewPtr<Symbiont>(random, world, &config, sym_int_val);
            host->AddSymbiont(sym);

            double expected_sym_points = 68; // hostDonation + stolen
            double expected_return = 0; // hostportion * synergy

            host->SetResInProcess(80);

            THEN("sym receives a donation and stolen resources, host receives betrayal"){
                REQUIRE(sym->ProcessResources(20) == expected_return);
                REQUIRE(sym->GetPoints() == expected_sym_points);

            }

            host.Delete();
        }

        WHEN("host_int_val < 0 and resources are placed into defense"){

            WHEN("host successfully defends from symsteal"){
                double host_int_val = -0.8;
                emp::Ptr<Host> host = emp::NewPtr<Host>(random, world, &config, host_int_val);
                emp::Ptr<Symbiont> sym = emp::NewPtr<Symbiont>(random, world, &config, sym_int_val);
                host->AddSymbiont(sym);

                double expected_sym_points = 0; // hostDonation + stolen
                double expected_return = 0; // hostportion * synergy

                host->SetResInProcess(20);
                THEN("symbiont is unsuccessful at stealing"){
                    REQUIRE(sym->ProcessResources(0) == expected_return);
                    REQUIRE(sym->GetPoints() == expected_sym_points);
                }

                host.Delete();
            }

            WHEN("host fails at defense"){
                double host_int_val = -0.5;
                emp::Ptr<Host> host = emp::NewPtr<Host>(random, world, &config, host_int_val);
                emp::Ptr<Symbiont> sym = emp::NewPtr<Symbiont>(random, world, &config, sym_int_val);
                host->AddSymbiont(sym);

                double expected_sym_points = 5; // hostDonation + stolen
                double expected_return = 0; // hostportion * synergy

                host->SetResInProcess(50);

                THEN("Sym steals successfully"){
                    REQUIRE(sym->ProcessResources(0) == expected_return);
                    REQUIRE(sym->GetPoints() == Approx(expected_sym_points));
                }

                host.Delete();
            }

        }

    }

    WHEN("sym_int_val > 0") {
        double sym_int_val = 0.2;
        double host_int_val = 0.5;
        emp::Ptr<Host> host = emp::NewPtr<Host>(random, world, &config, host_int_val);
        emp::Ptr<Symbiont> sym = emp::NewPtr<Symbiont>(random, world, &config, sym_int_val);
        host->AddSymbiont(sym);

        double expected_sym_points = 40; // hostDonation - hostPortion
        double expected_return = 50; // hostPortion * synergy

        host->SetResInProcess(50);


        THEN("Sym attempts to give benefit back"){
            REQUIRE(sym->ProcessResources(50) == expected_return);
            REQUIRE(sym->GetPoints() == expected_sym_points);
        }

        host.Delete();
    }
    world.Delete();
    random.Delete();
}

TEST_CASE("Symbiont GrowOlder", "[default]"){
    emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(10);
    SymConfigBase config;
    SymWorld world(*random, &config);
    world.Resize(2,2);
    config.SYM_AGE_MAX(2);

    WHEN ("A free-living symbiont reaches its maximum age"){
      config.FREE_LIVING_SYMS(1);
      emp::Ptr<Symbiont> sym = emp::NewPtr<Symbiont>(random, &world, &config, 1);
      world.AddOrgAt(sym, emp::WorldPosition(0,1));
      THEN("The symbiont dies and gets removed from the world"){
        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(sym->GetDead() == false);
        REQUIRE(sym->GetAge() == 0);
        world.Update(); //sym goes from age 1->2
        REQUIRE(sym->GetAge() == 1);
        world.Update();
        REQUIRE(sym->GetAge() == 2);
        world.Update(); //sym goes from age 2->3, gets set to dead
        world.Update(); //sym is deleted (before it can process)
        REQUIRE(world.GetNumOrgs() == 0);
      }
    }
    WHEN ("A hosted symbiont reaches its maximum age"){
      emp::Ptr<Symbiont> sym = emp::NewPtr<Symbiont>(random, &world, &config, 1);
      emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, 1);
      world.AddOrgAt(host, 1);
      host->AddSymbiont(sym);
      THEN("It dies and gets removed from its host"){
        REQUIRE(host->HasSym() == true);
        REQUIRE(sym->GetAge() == 0);
        host->Process(1);
        REQUIRE(sym->GetAge() == 1);
        host->Process(1);
        REQUIRE(host->HasSym() == true);
        REQUIRE(sym->GetAge() == 2);
        host->Process(1); //should now be dead and removed
        REQUIRE(host->HasSym() == false);
      }
    }
    random.Delete();
}

TEST_CASE("Symbiont MakeNew", "[default]"){
    emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(10);
    SymConfigBase config;
    SymWorld world(*random, &config);

    double sym_int_val = 0.2;
    emp::Ptr<Organism> sym1 = emp::NewPtr<Symbiont>(random, &world, &config, sym_int_val);
    emp::Ptr<Organism> sym2 = sym1->MakeNew();

    THEN("The new symbiont has the same genome as its parent, but age and points 0"){
        REQUIRE(sym2->GetIntVal() == sym1->GetIntVal());
        REQUIRE(sym2->GetInfectionChance() == sym1->GetInfectionChance());
        REQUIRE(sym2->GetAge() == 0);
        REQUIRE(sym2->GetPoints() == 0);
        //check that the offspring is the correct class
        REQUIRE(sym2->GetName() == "Symbiont");
    }

    sym1.Delete();
    sym2.Delete();
    random.Delete();
}
