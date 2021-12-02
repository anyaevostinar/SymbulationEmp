#include "../../lysis_mode/Phage.h"

TEST_CASE("Phage constructor, GetIntVal", "[lysis]") {
    emp::Ptr<emp::Random> random = new emp::Random(27);
    LysisWorld w(*random);
    SymConfigBase config;
    LysisWorld * world = &w;

    WHEN("Int val is passed in as negative"){
        double int_val = -1;
        Phage * p = new Phage(random, world, &config, int_val);
        double expected_int_val = -1;
        THEN("Int val is set to be negative"){
            REQUIRE(p->GetIntVal() == expected_int_val);
        }
        delete p;
    }

    WHEN("Int val is passed in as zero"){
        double int_val = 0;
        Phage * p2 = new Phage(random, world, &config, int_val);
        double expected_int_val = 0;

        THEN("Int val is set to be zero"){
            REQUIRE(p2->GetIntVal() == expected_int_val);
        }
        delete p2;
    }

    WHEN("Lysis chance is random"){
        double int_val = 0;
        config.LYSIS_CHANCE(-1);
        Phage * p3 = new Phage(random, world, &config, int_val);

        THEN("Lysis chance is randomly between 0 and 1"){
            REQUIRE(p3->GetLysisChance() >= 0);
            REQUIRE(p3->GetLysisChance() <= 1);
        }
        delete p3;
    }

    WHEN("Lysis chance is not random"){
        double int_val = 0;
        config.LYSIS_CHANCE(.5);
        Phage * p4 = new Phage(random, world, &config, int_val);
        double expected_lysis_chance = 0.5;

        THEN("Lysis chance is set to what was passed in"){
            REQUIRE(p4->GetLysisChance() == expected_lysis_chance);
        }
        delete p4;
    }

    WHEN("Chance of induction is random"){
        double int_val = 0;
        config.CHANCE_OF_INDUCTION(-1);
        Phage * p5 = new Phage(random, world, &config, int_val);

        THEN("Chance of induction is randomly between 0 and 1"){
            REQUIRE(p5->GetInductionChance() >= 0);
            REQUIRE(p5->GetInductionChance() <= 1);
        }
        delete p5;
    }

    WHEN("Chance of induction is not random"){
        double int_val = 0;
        config.CHANCE_OF_INDUCTION(0.2);
        Phage * p6 = new Phage(random, world, &config, int_val);
        double expected_induction_chance = 0.2;

        THEN("Chance of induction is set to what was passed in"){
            REQUIRE(p6->GetInductionChance() == expected_induction_chance);
        }
        delete p6;
    }

    WHEN("Incorporation val is random"){
        double int_val = 0;
        config.PHAGE_INC_VAL(-1);
        Phage * p7 = new Phage(random, world, &config, int_val);

        THEN("Incorporation val is randomly between 0 and 1"){
            REQUIRE(p7->GetIncVal() >= 0);
            REQUIRE(p7->GetIncVal() <= 1);
        }
        delete p7;
    }

    WHEN("Incorporation val is not random"){
        double int_val = 0;
        config.PHAGE_INC_VAL(0.3);
        Phage * p8 = new Phage(random, world, &config, int_val);
        double expected_incorporation_value = 0.3;

        THEN("Incorporation val is set to what was passed in"){
            REQUIRE(p8->GetIncVal() == expected_incorporation_value);
        }
        delete p8;
    }
}

TEST_CASE("Phage reproduce", "[lysis]") {
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
        delete p;
        delete phage_baby;
    }
    WHEN("Mutation rate is not zero") {
        double int_val = 0;
        double parent_orig_int_val = 0;
        double parent_orig_lysis_chance=.5;
        config.MUTATION_RATE(1);
        config.MUTATION_SIZE(0.002);
        emp::Ptr<Phage> p = new Phage(random, world, &config, int_val);
        emp::Ptr<Organism> phage_baby = p->reproduce();

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
        delete p;
        delete phage_baby;
    }
}

TEST_CASE("SetBurstTimer, IncBurstTimer", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(5);
    LysisWorld w(*random);
    LysisWorld * world = &w;
    SymConfigBase config;
    double int_val = -1;
    emp::Ptr<Phage> p = new Phage(random, world, &config, int_val);

    int default_burst_time = 0;
    REQUIRE(p->GetBurstTimer() == default_burst_time);

    p->IncBurstTimer();
    REQUIRE(p->GetBurstTimer() != default_burst_time);
    REQUIRE(p->GetBurstTimer() <= default_burst_time + 1*3);
    REQUIRE(p->GetBurstTimer() >= default_burst_time - 1*3);

    int burst_time = 15;
    p->SetBurstTimer(burst_time);

    int expected_burst_time = 15;
    REQUIRE(p->GetBurstTimer() == expected_burst_time);

    p->IncBurstTimer();
    REQUIRE(p->GetBurstTimer() <= expected_burst_time + 1*3);
    REQUIRE(p->GetBurstTimer() >= expected_burst_time - 1*3);
    REQUIRE(p->GetBurstTimer() != expected_burst_time);

}

TEST_CASE("Phage SetLysisChance, GetLysisChance", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(5);
    LysisWorld w(*random);
    LysisWorld * world = &w;
    SymConfigBase config;
    double int_val = -1;
    emp::Ptr<Phage> p = new Phage(random, world, &config, int_val);

    double lysis_chance = 0.5;
    p->SetLysisChance(lysis_chance);
    double expected_lysis_chance = 0.5;
    REQUIRE(p->GetLysisChance() == expected_lysis_chance);

    delete p;
}

TEST_CASE("Phage SetInductionChance, GetInductionChance", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(5);
    LysisWorld w(*random);
    LysisWorld * world = &w;
    SymConfigBase config;
    double int_val = -1;
    emp::Ptr<Phage> p = new Phage(random, world, &config, int_val);

    double induction_chance = 0.5;
    p->SetInductionChance(induction_chance);
    double expected_induction_chance = 0.5;
    REQUIRE(p->GetInductionChance() == expected_induction_chance);

    delete p;
}

TEST_CASE("Phage SetIncVal, GetIncVal", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(5);
    LysisWorld w(*random);
    LysisWorld * world = &w;
    SymConfigBase config;
    double int_val = -1;
    emp::Ptr<Phage> p = new Phage(random, world, &config, int_val);

    double incorporation_val = 0.5;
    p->SetIncVal(incorporation_val);
    double expected_incorporation_value = 0.5;
    REQUIRE(p->GetIncVal() == expected_incorporation_value);

    delete p;
}

TEST_CASE("Phage uponInjection", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(5);
    LysisWorld w(*random);
    LysisWorld * world = &w;
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

    delete p;
}

TEST_CASE("phage_mutate", "[lysis]"){
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

        emp::Ptr<Organism> p = new Phage(random, world, &config, int_val);
        p->mutate();
        THEN("Mutation occurs and chance of lysis changes") {
            REQUIRE(p->GetLysisChance() != 0.5);
            REQUIRE(p->GetLysisChance() >= 0.5 - 0.002*3);
            REQUIRE(p->GetLysisChance() <= 0.5 + 0.002*3);
            REQUIRE(p->GetInductionChance() != 0.5);
            REQUIRE(p->GetInductionChance() >= 0.5 - 0.002*3);
            REQUIRE(p->GetInductionChance() <= 0.5 + 0.002*3);
            REQUIRE(p->GetIncVal() != 0.5);
            REQUIRE(p->GetIncVal() >= 0.5 - 0.002*3);
            REQUIRE(p->GetIncVal() <= 0.5 + 0.002*3);
        }
        delete p;
    }

    WHEN("Mutation size is not zero, but horizontal mutation size is, and chance of lysis/induction/incorporation mutations are enabled") {
        double int_val = 0;
        config.MUTATION_SIZE(0.002);
        config.HORIZ_MUTATION_SIZE(0);
        config.MUTATE_LYSIS_CHANCE(1);
        config.MUTATE_INDUCTION_CHANCE(1);
        config.MUTATE_INC_VAL(1);

        emp::Ptr<Organism> p = new Phage(random, world, &config, int_val);
        p->mutate("horizontal");
        THEN("Mutation does not occur in horizontal mode") {
            REQUIRE(p->GetLysisChance() == 0.5);
            REQUIRE(p->GetInductionChance() == 0.5);
            REQUIRE(p->GetIncVal() == 0.5);
        }
        delete p;
    }

    WHEN("Mutation rate is not zero and chance of lysis/induction mutations are not enabled"){
        double int_val = 0;
        config.MUTATION_SIZE(0.002);
        config.MUTATE_LYSIS_CHANCE(0);
        config.MUTATE_INDUCTION_CHANCE(0);
        config.MUTATE_INC_VAL(0);
        emp::Ptr<Organism> p = new Phage(random, world, &config, int_val);
        p->mutate();
        double lysis_chance_post_mutation = 0.5;
        double induction_chance_post_mutation = 0.5;
        double incorporation_val_post_mutation = 0.5;
        THEN("Mutation does not occur and chance of lysis/chance of induction does not change") {
            REQUIRE(p->GetLysisChance() == Approx(lysis_chance_post_mutation));
            REQUIRE(p->GetInductionChance() == Approx(induction_chance_post_mutation));
            REQUIRE(p->GetIncVal() == Approx(incorporation_val_post_mutation));
        }
        delete p;
    }

    WHEN("Mutation rate is zero and chance of lysis/chance of induction mutations are enabled"){
        double int_val = 0;
        config.MUTATION_RATE(0);
        config.MUTATION_SIZE(0);
        config.MUTATE_LYSIS_CHANCE(1);
        config.MUTATE_INDUCTION_CHANCE(1);
        config.MUTATE_INC_VAL(1);
        emp::Ptr<Organism> p = new Phage(random, world, &config, int_val);
        p->mutate();
        double lysis_chance_post_mutation = 0.5;
        double induction_chance_post_mutation = 0.5;
        double incorporation_val_post_mutation = 0.5;
        THEN("Mutation does not occur and chance of lysis/chance of induction does not change") {
            REQUIRE(p->GetLysisChance() == Approx(lysis_chance_post_mutation));
            REQUIRE(p->GetInductionChance() == Approx(induction_chance_post_mutation));
            REQUIRE(p->GetIncVal() == Approx(incorporation_val_post_mutation));
        }
        delete p;
    }

    WHEN("Mutation rate is zero and chance of lysis mutations are not enabled and chance of induction mutations are not enabled"){
        double int_val = 0;
        config.MUTATION_RATE(0);
        config.MUTATION_SIZE(0);
        config.MUTATE_LYSIS_CHANCE(0);
        config.MUTATE_INDUCTION_CHANCE(0);
        config.MUTATE_INC_VAL(0);
        emp::Ptr<Organism> p = new Phage(random, world, &config, int_val);
        p->mutate();
        double lysis_chance_post_mutation = 0.5;
        double induction_chance_post_mutation = 0.5;
        double incorporation_val_post_mutation = 0.5;
        THEN("Mutation does not occur and chance of lysis/chance of induction does not change") {
            REQUIRE(p->GetLysisChance() == Approx(lysis_chance_post_mutation));
            REQUIRE(p->GetInductionChance() == Approx(induction_chance_post_mutation));
            REQUIRE(p->GetIncVal() == Approx(incorporation_val_post_mutation));
        }
        delete p;
    }
}

TEST_CASE("Phage process", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(9);
    LysisWorld w(*random);
    LysisWorld * world = &w;
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
            emp::Ptr<Phage> p;
            p.New(random, world, &config, int_val);

            emp::Ptr<Bacterium> h;
            h.New(random, &w, &config, int_val);

            //verify that the phage chooses lysogeny first
            bool expected_lysogeny = true;
            h->AddSymbiont(p);
            REQUIRE(p->GetLysogeny() == expected_lysogeny);

            expected_lysogeny = false;
            p->Process(location);

            THEN("The phage turns lytic"){
                REQUIRE(p->GetLysogeny() == expected_lysogeny);
            }
        }

        WHEN("The phage does not induce"){
            config.CHANCE_OF_INDUCTION(0);

            WHEN("The prophage loss rate is 1"){
                config.PROPHAGE_LOSS_RATE(1);

                double int_val = 0;
                emp::Ptr<Phage> p;
                p.New(random, world, &config, int_val);

                emp::Ptr<Bacterium> h;
                h.New(random, &w, &config, int_val);

                h->AddSymbiont(p);

                bool expected_dead = true;

                p->Process(location);

                THEN("The phage dies"){
                    REQUIRE(p->GetDead() == expected_dead);
                }
                h.Delete();
            }

            WHEN("The prophage loss rate is 0"){
                config.PROPHAGE_LOSS_RATE(0);
                double int_val = 0;
                double expected_int_val = 0;
                emp::Ptr<Phage> p = new Phage(random, world, &config, int_val);
                Bacterium * h = new Bacterium(random, &w, &config, int_val);
                h->AddSymbiont(p);

                double points = 0;
                double expected_points = 0;
                p->SetPoints(points);

                double burst_timer = 0;
                double expected_burst_timer = 0;
                p->SetBurstTimer(burst_timer);

                bool expected_dead = false;
                long unsigned int expected_repro_syms_size = size(h->GetReproSymbionts());

                p->Process(location);

                THEN("The phage does nothing; it is temperate and still alive"){
                    REQUIRE(p->GetIntVal() == expected_int_val);
                    REQUIRE(p->GetPoints() == expected_points);
                    REQUIRE(p->GetBurstTimer() == expected_burst_timer);
                    REQUIRE(size(h->GetReproSymbionts()) == expected_repro_syms_size);
                    REQUIRE(p->GetDead() == expected_dead);
                }
                delete h;
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
            emp::Ptr<Phage> p = new Phage(random, world, &config, int_val);

            //create two hosts and add both to world as neighbors
            Bacterium * orig_h = new Bacterium(random, &w, &config, int_val);
            Bacterium * new_h = new Bacterium(random, &w, &config, int_val);
            orig_h->AddSymbiont(p);
            world->AddOrgAt(orig_h, 0);
            world->AddOrgAt(new_h, 1);

            //add phage offspring to the original host's repro syms
            emp::Ptr<Organism> p_baby1 = p->reproduce();
            emp::Ptr<Organism> p_baby2 = p->reproduce();
            orig_h->AddReproSym(p_baby1);
            orig_h->AddReproSym(p_baby2);

            //call the process such that the phage bursts and we can check injection
            p->SetBurstTimer(burst_timer);
            p->Process(location);

            THEN("The phage offspring are injected into new hosts and the current host dies"){
                REQUIRE(size(new_h->GetSymbionts()) > 0);
                REQUIRE(size(orig_h->GetReproSymbionts()) == 0);
                REQUIRE(orig_h->GetDead() == true);
            }
        }

        WHEN("It is not time to burst"){
            double int_val = 0;
            emp::Ptr<Phage> p = new Phage(random, world, &config, int_val);
            Bacterium * h = new Bacterium(random, &w, &config, int_val);
            h->AddSymbiont(p);

            p->SetBurstTimer(0.0);

            WHEN("The phage doesn't have enough resources to reproduce"){
                double repro_syms_size_pre_process = size(h->GetReproSymbionts());
                double orig_points = 3.0;
                double expected_points = 3.0;
                p->SetPoints(orig_points);
                p->Process(location);

                THEN("The burst timer is incremented but no offspring are created"){
                    REQUIRE(p->GetBurstTimer() <= 0 + 1*3);
                    REQUIRE(p->GetBurstTimer() >= 0 - 1*3);
                    REQUIRE(p->GetBurstTimer() != 0);
                    REQUIRE(size(h->GetReproSymbionts()) == repro_syms_size_pre_process);
                    REQUIRE(p->GetPoints() == expected_points);
                }
            }

            WHEN("The phage has enough resources to reproduce"){
                double expected_repro_syms_size_post_process = size(h->GetReproSymbionts()) + 1; //one offspring created
                double orig_points = sym_repro_points;//symbiont given enough resources to produce one offspring
                double expected_points = 0.0;
                p->SetPoints(orig_points);
                p->Process(location);

                THEN("The burst timer is incremented and offspring are created"){
                    REQUIRE(p->GetBurstTimer() <= 0 + 1*3);
                    REQUIRE(p->GetBurstTimer() >= 0 - 1*3);
                    REQUIRE(p->GetBurstTimer() != 0);
                    REQUIRE(size(h->GetReproSymbionts()) == expected_repro_syms_size_post_process);
                    REQUIRE(p->GetPoints() == expected_points);
                }
            }
            delete h;
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
            emp::Ptr<Phage> p;
            p.New(random, world, &config, int_val);
            emp::Ptr<Bacterium> b;
            b.New(random, world, &config, int_val);
            b->AddSymbiont(p);
            p->uponInjection();

            double sym_piece = 40;
            double expected_return = 0;

            THEN("Phage doesn't take or give resources to the host"){
                REQUIRE(p->ProcessResources(sym_piece)==expected_return);
            }

            p.Delete();
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
            emp::Ptr<Bacterium> b;
            b.New(random, world, &config, int_val);

            WHEN("The incorporation vals are similar"){
                config.PHAGE_INC_VAL(0);

                emp::Ptr<Phage> p;
                p.New(random, world, &config, int_val);

                b->AddSymbiont(p);
                b->SetResInProcess(orig_host_resources);

                double expected_resources = 20;

                THEN("The host resources increase"){
                    REQUIRE(p->ProcessResources(sym_piece)==expected_resources);
                }
            }

            WHEN("The incorporation vals are neutral"){
                config.PHAGE_INC_VAL(0.5);

                emp::Ptr<Phage> p;
                p.New(random, world, &config, int_val);

                b->AddSymbiont(p);
                b->SetResInProcess(orig_host_resources);

                double expected_resources = 10;

                THEN("The host resources stay the same"){
                    REQUIRE(p->ProcessResources(sym_piece)==expected_resources);
                }
            }

            WHEN("The incorporation vals are far apart"){
                config.PHAGE_INC_VAL(1);

                emp::Ptr<Phage> p;
                p.New(random, world, &config, int_val);

                b->AddSymbiont(p);
                b->SetResInProcess(orig_host_resources);

                double expected_resources = 0;

                THEN("The host resources are diminished"){
                    REQUIRE(p->ProcessResources(sym_piece)==expected_resources);
                }
            }
        }
    }
}
