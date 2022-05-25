#include "../../lysis_mode/Phage.h"
#include "../../lysis_mode/Bacterium.h"

TEST_CASE("Phage constructor, GetIntVal", "[lysis]") {
    emp::Ptr<emp::Random> random = new emp::Random(27);
    LysisWorld w(*random);
    SymConfigBase config;
    LysisWorld * world = &w;

    WHEN("Int val is passed in as negative"){
        double int_val = -1;
        emp::Ptr<Phage> phage = emp::NewPtr<Phage>(random, world, &config, int_val);
        double expected_int_val = -1;
        THEN("Int val is set to be negative"){
            REQUIRE(phage->GetIntVal() == expected_int_val);
            REQUIRE(phage->GetAge() == 0);
            REQUIRE(phage->GetPoints() == 0);
            REQUIRE(phage->GetBurstTimer() == 0);
        }
        phage.Delete();
    }

    WHEN("Int val is passed in as zero"){
        double int_val = 0;
        emp::Ptr<Phage> phage = emp::NewPtr<Phage>(random, world, &config, int_val);
        double expected_int_val = 0;

        THEN("Int val is set to be zero"){
            REQUIRE(phage->GetIntVal() == expected_int_val);
            REQUIRE(phage->GetAge() == 0);
            REQUIRE(phage->GetPoints() == 0);
            REQUIRE(phage->GetBurstTimer() == 0);
        }
        phage.Delete();
    }

    WHEN("Lysis chance is random"){
        double int_val = 0;
        config.LYSIS_CHANCE(-1);
        emp::Ptr<Phage> phage = emp::NewPtr<Phage>(random, world, &config, int_val);

        THEN("Lysis chance is randomly between 0 and 1"){
            REQUIRE(phage->GetLysisChance() >= 0);
            REQUIRE(phage->GetLysisChance() <= 1);
            REQUIRE(phage->GetAge() == 0);
            REQUIRE(phage->GetPoints() == 0);
            REQUIRE(phage->GetBurstTimer() == 0);
        }
        phage.Delete();
    }

    WHEN("Lysis chance is not random"){
        double int_val = 0;
        config.LYSIS_CHANCE(.5);
        emp::Ptr<Phage> phage = emp::NewPtr<Phage>(random, world, &config, int_val);
        double expected_lysis_chance = 0.5;

        THEN("Lysis chance is set to what was passed in"){
            REQUIRE(phage->GetLysisChance() == expected_lysis_chance);
            REQUIRE(phage->GetAge() == 0);
            REQUIRE(phage->GetPoints() == 0);
            REQUIRE(phage->GetBurstTimer() == 0);
        }
        phage.Delete();
    }

    WHEN("Chance of induction is random"){
        double int_val = 0;
        config.CHANCE_OF_INDUCTION(-1);
        emp::Ptr<Phage> phage = emp::NewPtr<Phage>(random, world, &config, int_val);

        THEN("Chance of induction is randomly between 0 and 1"){
            REQUIRE(phage->GetInductionChance() >= 0);
            REQUIRE(phage->GetInductionChance() <= 1);
            REQUIRE(phage->GetAge() == 0);
            REQUIRE(phage->GetPoints() == 0);
            REQUIRE(phage->GetBurstTimer() == 0);
        }
        phage.Delete();
    }

    WHEN("Chance of induction is not random"){
        double int_val = 0;
        config.CHANCE_OF_INDUCTION(0.2);
        emp::Ptr<Phage> phage = emp::NewPtr<Phage>(random, world, &config, int_val);
        double expected_induction_chance = 0.2;

        THEN("Chance of induction is set to what was passed in"){
            REQUIRE(phage->GetInductionChance() == expected_induction_chance);
            REQUIRE(phage->GetAge() == 0);
            REQUIRE(phage->GetPoints() == 0);
            REQUIRE(phage->GetBurstTimer() == 0);
        }
        phage.Delete();
    }

    WHEN("Incorporation val is random"){
        double int_val = 0;
        config.PHAGE_INC_VAL(-1);
        emp::Ptr<Phage> phage = emp::NewPtr<Phage>(random, world, &config, int_val);

        THEN("Incorporation val is randomly between 0 and 1"){
            REQUIRE(phage->GetIncVal() >= 0);
            REQUIRE(phage->GetIncVal() <= 1);
            REQUIRE(phage->GetAge() == 0);
            REQUIRE(phage->GetPoints() == 0);
            REQUIRE(phage->GetBurstTimer() == 0);
        }
        phage.Delete();
    }

    WHEN("Incorporation val is not random"){
        double int_val = 0;
        config.PHAGE_INC_VAL(0.3);
        emp::Ptr<Phage> phage = emp::NewPtr<Phage>(random, world, &config, int_val);
        double expected_incorporation_value = 0.3;

        THEN("Incorporation val is set to what was passed in"){
            REQUIRE(phage->GetIncVal() == expected_incorporation_value);
            REQUIRE(phage->GetAge() == 0);
            REQUIRE(phage->GetPoints() == 0);
            REQUIRE(phage->GetBurstTimer() == 0);
        }
        phage.Delete();
    }
}

TEST_CASE("Phage Reproduce", "[lysis]") {
    emp::Ptr<emp::Random> random = new emp::Random(3);
    LysisWorld w(*random);
    LysisWorld * world = &w;
    SymConfigBase config;
    config.MUTATE_LYSIS_CHANCE(1);
    config.LYSIS_CHANCE(.5);

    WHEN("Mutation rate is zero")  {
        double int_val = 0;
        double parent_orig_int_val = 0;
        double parent_orig_lysis_chance=.5;
        config.MUTATION_RATE(0);
        config.MUTATION_SIZE(0);
        emp::Ptr<Phage> phage = emp::NewPtr<Phage>(random, world, &config, int_val);
        emp::Ptr<Organism> phage_baby = phage->Reproduce();

        THEN("Offspring's interaction value and lysis chance equals parent's interaction value and lysis chance") {
            int phage_baby_int_val = 0;
            REQUIRE( phage_baby->GetIntVal() == phage_baby_int_val);
            REQUIRE( phage_baby->GetIntVal() == parent_orig_int_val);
            REQUIRE( phage->GetIntVal() == parent_orig_int_val);

            double phage_baby_lysis_chance = .5;
            REQUIRE( phage_baby->GetLysisChance() == phage_baby_lysis_chance);
            REQUIRE( phage_baby->GetLysisChance() == parent_orig_lysis_chance);
            REQUIRE( phage->GetLysisChance() == parent_orig_lysis_chance);
        }
        THEN("Offspring's points and burst timer are zero") {
            int phage_baby_points = 0;
            int phage_baby_burst_timer = 0;
            REQUIRE( phage_baby->GetPoints() == phage_baby_points);
            REQUIRE(phage_baby->GetBurstTimer() == phage_baby_burst_timer);
        }
        phage.Delete();
        phage_baby.Delete();
    }
    WHEN("Mutation rate is not zero") {
        double int_val = 0;
        double parent_orig_int_val = 0;
        double parent_orig_lysis_chance=.5;
        config.MUTATION_RATE(1);
        config.MUTATION_SIZE(0.002);
        emp::Ptr<Phage> phage = emp::NewPtr<Phage>(random, world, &config, int_val);
        emp::Ptr<Organism> phage_baby = phage->Reproduce();

        THEN("Offspring's interaction value and lysis chance does not equal parent's interaction value and lysis chance") {
            REQUIRE( phage_baby->GetIntVal() != parent_orig_int_val);
            REQUIRE( phage_baby->GetIntVal() <= parent_orig_int_val + 0.002*3);
            REQUIRE( phage_baby->GetIntVal() >= parent_orig_int_val - 0.002*3);

            REQUIRE( phage_baby->GetLysisChance() != parent_orig_lysis_chance);
            REQUIRE( phage_baby->GetLysisChance() <= parent_orig_lysis_chance + 0.002*3);
            REQUIRE( phage_baby->GetLysisChance() >= parent_orig_lysis_chance - 0.002*3);
        }

        THEN("Offspring's points and burst timer are zero") {
            int phage_baby_points = 0;
            REQUIRE( phage_baby->GetPoints() == phage_baby_points);
            int phage_baby_burst_timer = 0;
            REQUIRE( phage_baby->GetBurstTimer() == phage_baby_burst_timer);
        }
        phage.Delete();
        phage_baby.Delete();
    }
}

TEST_CASE("SetBurstTimer, IncBurstTimer", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(5);
    LysisWorld w(*random);
    LysisWorld * world = &w;
    SymConfigBase config;
    double int_val = -1;
    emp::Ptr<Phage> phage = emp::NewPtr<Phage>(random, world, &config, int_val);

    int default_burst_time = 0;
    REQUIRE(phage->GetBurstTimer() == default_burst_time);

    phage->IncBurstTimer();
    REQUIRE(phage->GetBurstTimer() != default_burst_time);
    REQUIRE(phage->GetBurstTimer() <= default_burst_time + 1*3);
    REQUIRE(phage->GetBurstTimer() >= default_burst_time - 1*3);

    int burst_time = 15;
    phage->SetBurstTimer(burst_time);

    int expected_burst_time = 15;
    REQUIRE(phage->GetBurstTimer() == expected_burst_time);

    phage->IncBurstTimer();
    REQUIRE(phage->GetBurstTimer() <= expected_burst_time + 1*3);
    REQUIRE(phage->GetBurstTimer() >= expected_burst_time - 1*3);
    REQUIRE(phage->GetBurstTimer() != expected_burst_time);

    phage.Delete();
}

TEST_CASE("Phage SetLysisChance, GetLysisChance", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(5);
    LysisWorld w(*random);
    LysisWorld * world = &w;
    SymConfigBase config;
    double int_val = -1;
    emp::Ptr<Phage> phage = emp::NewPtr<Phage>(random, world, &config, int_val);

    double lysis_chance = 0.5;
    phage->SetLysisChance(lysis_chance);
    double expected_lysis_chance = 0.5;
    REQUIRE(phage->GetLysisChance() == expected_lysis_chance);

    phage.Delete();
}

TEST_CASE("Phage SetInductionChance, GetInductionChance", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(5);
    LysisWorld w(*random);
    LysisWorld * world = &w;
    SymConfigBase config;
    double int_val = -1;
    emp::Ptr<Phage> phage = emp::NewPtr<Phage>(random, world, &config, int_val);

    double induction_chance = 0.5;
    phage->SetInductionChance(induction_chance);
    double expected_induction_chance = 0.5;
    REQUIRE(phage->GetInductionChance() == expected_induction_chance);

    phage.Delete();
}

TEST_CASE("Phage SetIncVal, GetIncVal", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(5);
    LysisWorld w(*random);
    LysisWorld * world = &w;
    SymConfigBase config;
    double int_val = -1;
    emp::Ptr<Phage> phage = emp::NewPtr<Phage>(random, world, &config, int_val);

    double incorporation_val = 0.5;
    phage->SetIncVal(incorporation_val);
    double expected_incorporation_value = 0.5;
    REQUIRE(phage->GetIncVal() == expected_incorporation_value);

    phage.Delete();
}

TEST_CASE("Phage UponInjection", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(5);
    LysisWorld w(*random);
    LysisWorld * world = &w;
    SymConfigBase config;
    double int_val = -1;
    config.LYSIS_CHANCE(1);
    emp::Ptr<Phage> phage = emp::NewPtr<Phage>(random, world, &config, int_val);

    //initialization of phage sets lysogeny to false
    bool expected_lysogeny = false;
    REQUIRE(phage->GetLysogeny() == expected_lysogeny);

    //phage should choose lysis by default
    phage->UponInjection();
    expected_lysogeny = false;
    REQUIRE(phage->GetLysogeny() == expected_lysogeny);

    //if chance of lysis is 0, phage should choose lysogeny
    phage->SetLysisChance(0.0);
    phage->UponInjection();
    expected_lysogeny = true;
    REQUIRE(phage->GetLysogeny() == expected_lysogeny);

    phage.Delete();
}

TEST_CASE("Phage Mutate", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(5);
    LysisWorld w(*random);
    LysisWorld * world = &w;
    SymConfigBase config;
    config.LYSIS_CHANCE(0.5);
    config.CHANCE_OF_INDUCTION(0.5);
    config.PHAGE_INC_VAL(0.5);

    WHEN("Mutation size is not zero and chance of lysis/induction/incorporation mutations are enabled") {
        double int_val = 0;
        config.MUTATION_SIZE(0.002);
        config.MUTATE_LYSIS_CHANCE(1);
        config.MUTATE_INDUCTION_CHANCE(1);
        config.MUTATE_INC_VAL(1);

        emp::Ptr<Organism> phage = emp::NewPtr<Phage>(random, world, &config, int_val);
        phage->Mutate();
        THEN("Mutation occurs and chance of lysis changes") {
            REQUIRE(phage->GetLysisChance() != 0.5);
            REQUIRE(phage->GetLysisChance() >= 0.5 - 0.002*3);
            REQUIRE(phage->GetLysisChance() <= 0.5 + 0.002*3);
            REQUIRE(phage->GetInductionChance() != 0.5);
            REQUIRE(phage->GetInductionChance() >= 0.5 - 0.002*3);
            REQUIRE(phage->GetInductionChance() <= 0.5 + 0.002*3);
            REQUIRE(phage->GetIncVal() != 0.5);
            REQUIRE(phage->GetIncVal() >= 0.5 - 0.002*3);
            REQUIRE(phage->GetIncVal() <= 0.5 + 0.002*3);
        }
        phage.Delete();
    }

    WHEN("Mutation rate is not zero and chance of lysis/induction mutations are not enabled"){
        double int_val = 0;
        config.MUTATION_SIZE(0.002);
        config.MUTATE_LYSIS_CHANCE(0);
        config.MUTATE_INDUCTION_CHANCE(0);
        config.MUTATE_INC_VAL(0);
        emp::Ptr<Organism> phage = emp::NewPtr<Phage>(random, world, &config, int_val);
        phage->Mutate();
        double lysis_chance_post_mutation = 0.5;
        double induction_chance_post_mutation = 0.5;
        double incorporation_val_post_mutation = 0.5;
        THEN("Mutation does not occur and chance of lysis/chance of induction does not change") {
            REQUIRE(phage->GetLysisChance() == Approx(lysis_chance_post_mutation));
            REQUIRE(phage->GetInductionChance() == Approx(induction_chance_post_mutation));
            REQUIRE(phage->GetIncVal() == Approx(incorporation_val_post_mutation));
        }
        phage.Delete();
    }

    WHEN("Mutation rate is zero and chance of lysis/chance of induction mutations are enabled"){
        double int_val = 0;
        config.MUTATION_RATE(0);
        config.MUTATION_SIZE(0);
        config.MUTATE_LYSIS_CHANCE(1);
        config.MUTATE_INDUCTION_CHANCE(1);
        config.MUTATE_INC_VAL(1);
        emp::Ptr<Organism> phage = emp::NewPtr<Phage>(random, world, &config, int_val);
        phage->Mutate();
        double lysis_chance_post_mutation = 0.5;
        double induction_chance_post_mutation = 0.5;
        double incorporation_val_post_mutation = 0.5;
        THEN("Mutation does not occur and chance of lysis/chance of induction does not change") {
            REQUIRE(phage->GetLysisChance() == Approx(lysis_chance_post_mutation));
            REQUIRE(phage->GetInductionChance() == Approx(induction_chance_post_mutation));
            REQUIRE(phage->GetIncVal() == Approx(incorporation_val_post_mutation));
        }
        phage.Delete();
    }

    WHEN("Mutation rate is zero and chance of lysis mutations are not enabled and chance of induction mutations are not enabled"){
        double int_val = 0;
        config.MUTATION_RATE(0);
        config.MUTATION_SIZE(0);
        config.MUTATE_LYSIS_CHANCE(0);
        config.MUTATE_INDUCTION_CHANCE(0);
        config.MUTATE_INC_VAL(0);
        emp::Ptr<Organism> phage = emp::NewPtr<Phage>(random, world, &config, int_val);
        phage->Mutate();
        double lysis_chance_post_mutation = 0.5;
        double induction_chance_post_mutation = 0.5;
        double incorporation_val_post_mutation = 0.5;
        THEN("Mutation does not occur and chance of lysis/chance of induction does not change") {
            REQUIRE(phage->GetLysisChance() == Approx(lysis_chance_post_mutation));
            REQUIRE(phage->GetInductionChance() == Approx(induction_chance_post_mutation));
            REQUIRE(phage->GetIncVal() == Approx(incorporation_val_post_mutation));
        }
        phage.Delete();
    }
}

TEST_CASE("Phage process", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(9);
    LysisWorld world(*random);
  //  LysisWorld * world = &w;
    SymConfigBase config;

    config.LYSIS(1); //phage process only happens when lysis is enabled
    config.GRID_X(2);
    config.GRID_Y(1);
    config.SYM_LIMIT(2);
    int location = 0;

    WHEN("The phage chooses lysogeny"){
        config.LYSIS_CHANCE(0.0); //0% chance of lysis, 100% chance of lysogeny

        WHEN("The phage chooses to induce"){
            config.CHANCE_OF_INDUCTION(1);

            double int_val = 0;
            emp::Ptr<Phage> phage;
            phage.New(random, &world, &config, int_val);

            emp::Ptr<Bacterium> bacterium;
            bacterium.New(random, &world, &config, int_val);

            //verify that the phage chooses lysogeny first
            bool expected_lysogeny = true;
            bacterium->AddSymbiont(phage);
            REQUIRE(phage->GetLysogeny() == expected_lysogeny);

            expected_lysogeny = false;
            phage->Process(location);

            THEN("The phage turns lytic"){
                REQUIRE(phage->GetLysogeny() == expected_lysogeny);
            }
            bacterium.Delete();
        }

        WHEN("The phage does not induce"){
            config.CHANCE_OF_INDUCTION(0);

            WHEN("The prophage loss rate is 1"){
                config.PROPHAGE_LOSS_RATE(1);

                double int_val = 0;
                emp::Ptr<Phage> phage;
                phage.New(random, &world, &config, int_val);

                emp::Ptr<Bacterium> bacterium;
                bacterium.New(random, &world, &config, int_val);

                bacterium->AddSymbiont(phage);

                bool expected_dead = true;

                phage->Process(location);

                THEN("The phage dies"){
                    REQUIRE(phage->GetDead() == expected_dead);
                }
                bacterium.Delete();
            }

            WHEN("The prophage loss rate is 0"){
                config.PROPHAGE_LOSS_RATE(0);
                double int_val = 0;
                double expected_int_val = 0;
                emp::Ptr<Phage> phage = emp::NewPtr<Phage>(random, &world, &config, int_val);
                emp::Ptr<Bacterium> bacterium = emp::NewPtr<Bacterium>(random, &world, &config, int_val);
                bacterium->AddSymbiont(phage);

                double points = 0;
                double expected_points = 0;
                phage->SetPoints(points);

                double burst_timer = 0;
                double expected_burst_timer = 0;
                phage->SetBurstTimer(burst_timer);

                bool expected_dead = false;
                long unsigned int expected_repro_syms_size = size(bacterium->GetReproSymbionts());

                phage->Process(location);

                THEN("The phage does nothing; it is temperate and still alive"){
                    REQUIRE(phage->GetIntVal() == expected_int_val);
                    REQUIRE(phage->GetPoints() == expected_points);
                    REQUIRE(phage->GetBurstTimer() == expected_burst_timer);
                    REQUIRE(size(bacterium->GetReproSymbionts()) == expected_repro_syms_size);
                    REQUIRE(phage->GetDead() == expected_dead);
                }
                bacterium.Delete();
            }
        }
    }

    WHEN("The phage chooses lysis"){
        config.LYSIS_CHANCE(1);
        double burst_timer = 10;
        double sym_repro_points = 5.0;
        config.SYM_LYSIS_RES(sym_repro_points);
        config.BURST_TIME(burst_timer);

        WHEN("It is time to burst"){
            double int_val = 0;
            emp::Ptr<Phage> phage = emp::NewPtr<Phage>(random, &world, &config, int_val);

            //create two hosts and add both to world as neighbors
            emp::Ptr<Bacterium> orig_bacterium = emp::NewPtr<Bacterium>(random, &world, &config, int_val);
            emp::Ptr<Bacterium> new_bacterium = emp::NewPtr<Bacterium>(random, &world, &config, int_val);
            orig_bacterium->AddSymbiont(phage);
            world.AddOrgAt(orig_bacterium, 0);
            world.AddOrgAt(new_bacterium, 1);

            //add phage offspring to the original host's repro syms
            emp::Ptr<Organism> p_baby1 = phage->Reproduce();
            emp::Ptr<Organism> p_baby2 = phage->Reproduce();
            orig_bacterium->AddReproSym(p_baby1);
            orig_bacterium->AddReproSym(p_baby2);

            //call the process such that the phage bursts and we can check injection
            phage->SetBurstTimer(burst_timer);
            phage->Process(location);

            THEN("The phage offspring are injected into new hosts and the current host dies"){
                REQUIRE(size(new_bacterium->GetSymbionts()) > 0);
                REQUIRE(size(orig_bacterium->GetReproSymbionts()) == 0);
                REQUIRE(orig_bacterium->GetDead() == true);
            }
        }

        WHEN("It is not time to burst"){
            double int_val = 0;
            emp::Ptr<Phage> phage = emp::NewPtr<Phage>(random, &world, &config, int_val);
            emp::Ptr<Bacterium> bacterium = emp::NewPtr<Bacterium>(random, &world, &config, int_val);
            bacterium->AddSymbiont(phage);

            phage->SetBurstTimer(0.0);

            WHEN("The phage doesn't have enough resources to reproduce"){
                double repro_syms_size_pre_process = size(bacterium->GetReproSymbionts());
                double orig_points = 3.0;
                double expected_points = 3.0;
                phage->SetPoints(orig_points);
                phage->Process(location);

                THEN("The burst timer is incremented but no offspring are created"){
                    REQUIRE(phage->GetBurstTimer() <= 0 + 1*3);
                    REQUIRE(phage->GetBurstTimer() >= 0 - 1*3);
                    REQUIRE(phage->GetBurstTimer() != 0);
                    REQUIRE(size(bacterium->GetReproSymbionts()) == repro_syms_size_pre_process);
                    REQUIRE(phage->GetPoints() == expected_points);
                }
            }

            WHEN("The phage has enough resources to reproduce"){
                double expected_repro_syms_size_post_process = size(bacterium->GetReproSymbionts()) + 1; //one offspring created
                double orig_points = sym_repro_points;//symbiont given enough resources to produce one offspring
                double expected_points = 0.0;
                phage->SetPoints(orig_points);
                phage->Process(location);

                THEN("The burst timer is incremented and offspring are created"){
                    REQUIRE(phage->GetBurstTimer() <= 0 + 1*3);
                    REQUIRE(phage->GetBurstTimer() >= 0 - 1*3);
                    REQUIRE(phage->GetBurstTimer() != 0);
                    REQUIRE(size(bacterium->GetReproSymbionts()) == expected_repro_syms_size_post_process);
                    REQUIRE(phage->GetPoints() == expected_points);
                }
            }
            bacterium.Delete();
        }

    }

}

TEST_CASE("Phage ProcessResources", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(9);
    LysisWorld w(*random);
    LysisWorld * world = &w;
    SymConfigBase config;

    GIVEN("Phage is Lysogenic"){

        WHEN("Benefits to the host are not enabled"){
            config.LYSIS(1);
            config.LYSIS_CHANCE(0);
            config.BENEFIT_TO_HOST(0);

            double int_val=0;
            emp::Ptr<Phage> phage;
            phage.New(random, world, &config, int_val);
            emp::Ptr<Bacterium> bacterium;
            bacterium.New(random, world, &config, int_val);
            bacterium->AddSymbiont(phage);
            phage->UponInjection();

            double sym_piece = 40;
            double expected_return = 0;

            THEN("Phage doesn't take or give resources to the host"){
                REQUIRE(phage->ProcessResources(sym_piece)==expected_return);
            }

            bacterium.Delete();
        }

        WHEN("Benefits to the host are enabled"){
            config.LYSIS(1);
            config.LYSIS_CHANCE(0);
            config.BENEFIT_TO_HOST(1);
            config.HOST_INC_VAL(0);
            config.SYNERGY(2);

            double orig_host_resources = 10;
            double sym_piece = 0;
            double int_val=0;
            emp::Ptr<Bacterium> bacterium;
            bacterium.New(random, world, &config, int_val);

            WHEN("The incorporation vals are similar"){
                config.PHAGE_INC_VAL(0);

                emp::Ptr<Phage> phage;
                phage.New(random, world, &config, int_val);

                bacterium->AddSymbiont(phage);
                bacterium->SetResInProcess(orig_host_resources);

                double expected_resources = 20;

                THEN("The host resources increase"){
                    REQUIRE(phage->ProcessResources(sym_piece)==expected_resources);
                }
            }

            WHEN("The incorporation vals are neutral"){
                config.PHAGE_INC_VAL(0.5);

                emp::Ptr<Phage> phage;
                phage.New(random, world, &config, int_val);

                bacterium->AddSymbiont(phage);
                bacterium->SetResInProcess(orig_host_resources);

                double expected_resources = 10;

                THEN("The host resources stay the same"){
                    REQUIRE(phage->ProcessResources(sym_piece)==expected_resources);
                }
            }

            WHEN("The incorporation vals are far apart"){
                config.PHAGE_INC_VAL(1);

                emp::Ptr<Phage> phage;
                phage.New(random, world, &config, int_val);

                bacterium->AddSymbiont(phage);
                bacterium->SetResInProcess(orig_host_resources);

                double expected_resources = 0;

                THEN("The host resources are diminished"){
                    REQUIRE(phage->ProcessResources(sym_piece)==expected_resources);
                }
            }

            bacterium.Delete();
        }
    }

    THEN("Phage, not Symbiont, ProcessResources is used"){
      double int_val = 0.2;

      config.LYSIS(1);
      config.LYSIS_CHANCE(0);
      config.BENEFIT_TO_HOST(0);
      config.SYNERGY(5);

      emp::Ptr<Organism> phage = emp::NewPtr<Phage>(random, world, &config, int_val);
      emp::Ptr<Host> bacterium = emp::NewPtr<Bacterium>(random, world, &config, int_val);
      phage->UponInjection();

      emp::Ptr<Organism> symbiont = emp::NewPtr<Symbiont>(random, world, &config, int_val);
      emp::Ptr<Host> host = emp::NewPtr<Host>(random, world, &config, int_val);
      REQUIRE(host->GetPoints() == bacterium->GetPoints());

      double sym_piece = 20;
      host->DistribResToSym(symbiont, sym_piece);
      bacterium->DistribResToSym(phage, sym_piece);

      double expected_host_points = 20;
      double expected_bacterium_points = 16;
      double host_points = host->GetPoints();
      double bacterium_points = bacterium->GetPoints();

      REQUIRE(host_points != bacterium_points);
      REQUIRE(host_points == expected_host_points);
      REQUIRE(bacterium_points == expected_bacterium_points);

      phage.Delete();
      bacterium.Delete();
      symbiont.Delete();
      host.Delete();
    }
}

TEST_CASE("Phage MakeNew", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    LysisWorld world(*random);
    SymConfigBase config;

    double phage_int_val = 0.2;
    emp::Ptr<Organism> phage = emp::NewPtr<Phage>(random, &world, &config, phage_int_val);
    emp::Ptr<Organism> new_phage = phage->MakeNew();

    THEN("The new phage has the same genome as its parent, but age and points 0"){
        REQUIRE(new_phage->GetIntVal() == phage->GetIntVal());
        REQUIRE(new_phage->GetIncVal() == phage->GetIncVal());
        REQUIRE(new_phage->GetLysisChance() == phage->GetLysisChance());
        REQUIRE(new_phage->GetInductionChance() == phage->GetInductionChance());
        REQUIRE(new_phage->GetInfectionChance() == phage->GetInfectionChance());
        REQUIRE(new_phage->GetAge() == 0);
        REQUIRE(new_phage->GetPoints() == 0);
        REQUIRE(new_phage->GetBurstTimer() == 0);

        //check that the offspring is the correct class
        REQUIRE(typeid(*new_phage).name() == typeid(*phage).name());
    }

    phage.Delete();
    new_phage.Delete();
}
