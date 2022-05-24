#include "../../lysis_mode/Phage.h"
#include "../../lysis_mode/Bacterium.h"

TEST_CASE("Phage Process", "[lysis]") {

    GIVEN("A world and some phage") {

        emp::Ptr<emp::Random> random;
        random.New(5);
        SymConfigBase config;
        LysisWorld w(*random, &config);
        LysisWorld * world = &w;
        config.LYSIS(1);
        double int_val = -1;

        WHEN("Lysis is enabled and the Phage's burst timer >= the burst time") {
            emp::Ptr<Phage> phage;
            phage.New(random, world, &config, int_val);
            emp::Ptr<Phage> phage1;
            phage1.New(random, world, &config, int_val);
            emp::Ptr<Phage> phage2;
            phage2.New(random, world, &config, int_val);

            emp::Ptr<Bacterium> bacterium;
            bacterium.New(random, world, &config);

            phage->SetHost(bacterium);
            int time = 15;
            phage->SetBurstTimer(time);

            bacterium->AddReproSym(phage1);
            bacterium->AddReproSym(phage2);

            THEN("Lysis occurs, the host dies") {
                size_t location = 2;
                // double resources_per_host_per_update = 40;

                phage->Process(location);

                bool host_dead = true;
                std::vector<emp::Ptr<Organism>> empty_syms = {};
                REQUIRE(bacterium->GetDead() == host_dead);
                REQUIRE(bacterium->GetReproSymbionts() == empty_syms);
            }

            phage.Delete();
            bacterium.Delete();
        }

        WHEN("Lysis is enabled and the Phage's burst timer < the burst time") {
            emp::Ptr<Bacterium> bacterium;
            bacterium.New(random, world, &config);

            double phage_points = 10;
            emp::Ptr<Phage> phage3;
            phage3.New(random, world, &config, int_val);
            emp::Ptr<Phage> phage4;
            phage4.New(random, world, &config, int_val);
            emp::Ptr<Phage> phage5;
            phage5.New(random, world, &config, int_val);
            phage3->SetPoints(phage_points);

            bacterium->AddReproSym(phage4);
            bacterium->AddReproSym(phage5);
            int num_host_orig_syms = 2;
            phage3->SetHost(bacterium);

            THEN("Phage reproduces, Host gains repro symbionts, Phage loses points") {
                size_t location = 2;
                // bool host_dead = false;

                phage3->Process(location);

                REQUIRE(bacterium->GetDead() == false);

                std::vector<emp::Ptr<Organism>> updated_repro_syms = bacterium->GetReproSymbionts();

                REQUIRE( (int) updated_repro_syms.size() > num_host_orig_syms); // host gained repro syms

                REQUIRE(phage3->GetPoints() < phage_points); // phage lost points


            }

            phage3.Delete();
            bacterium.Delete();

        }
        random.Delete();
    }
}

TEST_CASE("Phage Vertical Transmission", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    LysisWorld w(*random, &config);
    LysisWorld * world = &w;
    config.LYSIS(1);

    WHEN("phage is lytic"){
        config.LYSIS_CHANCE(1);

        WHEN("Vertical Transmission is enabled"){
            config.VERTICAL_TRANSMISSION(1);
            double host_int_val = .5;
            double sym_int_val = -.5;

            emp::Ptr<Bacterium> bacterium = emp::NewPtr<Bacterium>(random, world, &config, host_int_val);
            emp::Ptr<Phage> phage = emp::NewPtr<Phage>(random, world, &config, sym_int_val);
            bacterium->AddSymbiont(phage);

            emp::Ptr<Bacterium> host_baby = emp::NewPtr<Bacterium>(random, world, &config, bacterium->GetIntVal());
            long unsigned int expected_sym_size = host_baby->GetSymbionts().size();
            phage->VerticalTransmission(host_baby);

            THEN ("Phage does not vertically transmit") {
                REQUIRE(host_baby->GetSymbionts().size() == expected_sym_size);
            }
            bacterium.Delete();
            host_baby.Delete();
        }
        WHEN("Vertical Transmission is disabled"){
            config.VERTICAL_TRANSMISSION(0);
            double host_int_val = .5;
            double sym_int_val = -.5;

            emp::Ptr<Bacterium> bacterium = emp::NewPtr<Bacterium>(random, world, &config, host_int_val);
            emp::Ptr<Phage> phage = emp::NewPtr<Phage>(random, world, &config, sym_int_val);
            bacterium->AddSymbiont(phage);

            emp::Ptr<Bacterium> host_baby = emp::NewPtr<Bacterium>(random, world, &config, bacterium->GetIntVal());
            long unsigned int expected_sym_size = host_baby->GetSymbionts().size();
            phage->VerticalTransmission(host_baby);

            THEN ("Phage does not vertically transmit") {
                REQUIRE(host_baby->GetSymbionts().size() == expected_sym_size);
            }
            bacterium.Delete();
            host_baby.Delete();
        }

    }

    WHEN("phage is lysogenic"){
        config.LYSIS_CHANCE(0);

        WHEN("Vertical Transmission is enabled"){
            config.VERTICAL_TRANSMISSION(1);
            double host_int_val = .5;
            double sym_int_val = -.5;

            emp::Ptr<Bacterium> bacterium = emp::NewPtr<Bacterium>(random, world, &config, host_int_val);
            emp::Ptr<Phage> phage = emp::NewPtr<Phage>(random, world, &config, sym_int_val);
            bacterium->AddSymbiont(phage);

            emp::Ptr<Bacterium> host_baby = emp::NewPtr<Bacterium>(random, world, &config, bacterium->GetIntVal());
            long unsigned int expected_sym_size = host_baby->GetSymbionts().size() +1;
            phage->VerticalTransmission(host_baby);

            THEN ("Phage does vertically transmit") {
                REQUIRE(host_baby->GetSymbionts().size() == expected_sym_size);
            }
            bacterium.Delete();
            host_baby.Delete();
        }
         WHEN("Vertical Transmission is disabled"){
            config.VERTICAL_TRANSMISSION(0);
            double host_int_val = .5;
            double sym_int_val = -.5;

            emp::Ptr<Bacterium> bacterium = emp::NewPtr<Bacterium>(random, world, &config, host_int_val);
            emp::Ptr<Phage> phage = emp::NewPtr<Phage>(random, world, &config, sym_int_val);
            bacterium->AddSymbiont(phage);

            emp::Ptr<Bacterium> host_baby = emp::NewPtr<Bacterium>(random, world, &config, bacterium->GetIntVal());
            long unsigned int expected_sym_size = host_baby->GetSymbionts().size()+1;
            phage->VerticalTransmission(host_baby);

            THEN ("Phage does vertically transmit") {
                REQUIRE(host_baby->GetSymbionts().size() == expected_sym_size);
            }
            bacterium.Delete();
            host_baby.Delete();
        }

    }

}

TEST_CASE("Host phage death and removal from syms list", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(6);
    SymConfigBase config;
    LysisWorld w(*random, &config);
    LysisWorld *world = &w;
    config.SYM_LIMIT(2);

    WHEN("there is a single lysogenic phage and it is dead"){
        config.LYSIS_CHANCE(0);
        double host_int_val = .5;
        double sym_int_val = -.5;

        emp::Ptr<Bacterium> bacterium = emp::NewPtr<Bacterium>(random, world, &config, host_int_val);
        emp::Ptr<Phage> phage = emp::NewPtr<Phage>(random, world, &config, sym_int_val);

        bacterium->AddSymbiont(phage);
        phage->SetDead();

        long unsigned int expected_sym_size = 0;
        bacterium->Process(0);

        THEN("phage is removed from syms list"){
            REQUIRE(bacterium->GetSymbionts().size() == expected_sym_size);
        }
        bacterium.Delete();
    }

    WHEN("There are multiple lysogenic phage and only one dies"){
        config.LYSIS_CHANCE(0);
        double host_int_val = .5;
        double sym_int_val = -.5;

        emp::Ptr<Bacterium> bacterium = emp::NewPtr<Bacterium>(random, world, &config, host_int_val);
        emp::Ptr<Organism> phage1 = emp::NewPtr<Phage>(random, world, &config, sym_int_val);
        emp::Ptr<Organism> phage2 = emp::NewPtr<Phage>(random, world, &config, 0.0);

        bacterium->AddSymbiont(phage1);
        bacterium->AddSymbiont(phage2);
        phage1->SetDead();

        long unsigned int expected_sym_size = 1;
        bacterium->Process(0);

        THEN("Only the dead phage is removed from the syms list"){
            REQUIRE(bacterium->GetSymbionts().size() == expected_sym_size);

            emp::Ptr<Organism> curSym = bacterium->GetSymbionts()[0];
            REQUIRE(curSym->GetIntVal() == phage2->GetIntVal());
            REQUIRE(curSym == phage2);
        }
        bacterium.Delete();
    }
}

TEST_CASE("Phage LysisBurst", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(6);
    SymConfigBase config;
    LysisWorld w(*random, &config);
    LysisWorld * world = &w;

    config.LYSIS(1);
    config.GRID_X(2);
    config.GRID_Y(1);
    config.SYM_LIMIT(10);
    int location = 0;

    double int_val = 0;
    emp::Ptr<Phage> phage = emp::NewPtr<Phage>(random, world, &config, int_val);

    GIVEN("create two hosts and add both to world as neighbors, add phage offspring to the original host's repro syms"){
        emp::Ptr<Bacterium> orig_bacterium = emp::NewPtr<Bacterium>(random, world, &config, int_val);
        emp::Ptr<Bacterium> new_bacterium = emp::NewPtr<Bacterium>(random, world, &config, int_val);
        orig_bacterium->AddSymbiont(phage);
        world->AddOrgAt(orig_bacterium, 0);
        world->AddOrgAt(new_bacterium, 1);

        emp::Ptr<Organism> p_baby1 = phage->Reproduce();
        emp::Ptr<Organism> p_baby2 = phage->Reproduce();
        orig_bacterium->AddReproSym(p_baby1);
        orig_bacterium->AddReproSym(p_baby2);

        WHEN("call the burst method so we can check injection") {
            int original_sym_num = new_bacterium->GetSymbionts().size() + orig_bacterium->GetSymbionts().size();
            phage->LysisBurst(location);

            THEN("the repro syms go into the other host as symbionts and the original host dies"){
                int new_sym_num = new_bacterium->GetSymbionts().size() + orig_bacterium->GetSymbionts().size();
                REQUIRE(new_sym_num == original_sym_num+2);
                REQUIRE(size(orig_bacterium->GetReproSymbionts()) == 0);
                REQUIRE(orig_bacterium->GetDead() == true);
            }
        }
    }
}

TEST_CASE("Phage LysisStep", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(9);
    SymConfigBase config;
    LysisWorld w(*random, &config);
    LysisWorld * world = &w;

    double sym_repro_points = 5.0;
    config.LYSIS(1);
    config.SYM_LYSIS_RES(sym_repro_points);

    double int_val = 0;
    emp::Ptr<Phage> phage = emp::NewPtr<Phage>(random, world, &config, int_val);
    emp::Ptr<Bacterium> bacterium = emp::NewPtr<Bacterium>(random, world, &config, int_val);
    bacterium->AddSymbiont(phage);

    WHEN("The phage doesn't have enough resources to reproduce"){
        double repro_syms_size_pre_process = size(bacterium->GetReproSymbionts());
        double orig_points = 3.0;
        double expected_points = 3.0;
        phage->SetPoints(orig_points);
        double orig_burst_time = 0.0;
        phage->SetBurstTimer(orig_burst_time);

        phage->LysisStep();
        THEN("The burst timer is incremented but no offspring are created"){
            REQUIRE(phage->GetBurstTimer() > orig_burst_time);
            REQUIRE(size(bacterium->GetReproSymbionts()) == repro_syms_size_pre_process);
            REQUIRE(phage->GetPoints() == expected_points);
        }
    }

    WHEN("The phage does have enough resources to reproduce"){
        double expected_repro_syms_size_post_process = size(bacterium->GetReproSymbionts()) + 1; //one offspring created
        double orig_points = sym_repro_points;//symbiont given enough resources to produce one offspring
        double expected_points = 0.0;
        phage->SetPoints(orig_points);
        double orig_burst_time = 0.0;
        phage->SetBurstTimer(orig_burst_time);

        phage->LysisStep();
        THEN("The burst timer is incremented and offspring are created"){
            REQUIRE(phage->GetBurstTimer() > orig_burst_time);
            REQUIRE(size(bacterium->GetReproSymbionts()) == expected_repro_syms_size_post_process);
            REQUIRE(phage->GetPoints() == expected_points);
        }
    }

    bacterium.Delete();
}
