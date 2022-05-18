#include "../../pgg_mode/PGGHost.h"
#include "../../pgg_mode/PGGSymbiont.h"

TEST_CASE("PGGSymbiont SetHost, GetHost", "[pgg]") {

    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    PGGWorld w(*random);
    double int_val = 1;

    emp::Ptr<Organism> host = emp::NewPtr<PGGHost>(random, &w, &config);
    emp::Ptr<PGGSymbiont> symbiont = emp::NewPtr<PGGSymbiont>(random, &w, &config, int_val);

    symbiont->SetHost(host);

    REQUIRE(symbiont->GetHost() == host);

    host.Delete();
    symbiont.Delete();
}

TEST_CASE("PGGHost DistribResources", "[pgg]") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    PGGWorld w(*random);
    SymConfigBase config;
    config.SYM_LIMIT(6);
    config.SYNERGY(5);

    WHEN("Host interaction value >= 0 and  Symbiont interaction value >= 0") {

        double host_int_val = 0.5;
        double sym_int_val = 1;

        emp::Ptr<PGGHost> host = emp::NewPtr<PGGHost>(random, &w, &config, host_int_val);


        emp::Ptr<PGGSymbiont> symbiont1 = emp::NewPtr<PGGSymbiont>(random, &w, &config, sym_int_val);
        emp::Ptr<PGGSymbiont> symbiont2 = emp::NewPtr<PGGSymbiont>(random, &w, &config, sym_int_val);
        emp::Ptr<PGGSymbiont> symbiont3 = emp::NewPtr<PGGSymbiont>(random, &w, &config, sym_int_val);
        emp::vector<emp::Ptr<Organism>> syms = {symbiont1, symbiont2, symbiont3};
        host->SetSymbionts(syms);

        double resources = 120;
        host->DistribResources(resources);


        int num_syms = 3;
        double sym_piece = resources / num_syms; // how much resource each sym gets
        double host_donation = sym_piece * host_int_val;
        double host_portion = sym_piece - host_donation;
        double sym_return = (host_donation * sym_int_val) * config.SYNERGY();
        double sym_portion = host_donation - (host_donation * sym_int_val);
        host_portion += sym_return;

        double host_points = num_syms * host_portion; // * by num_syms bc points are added during each iteration through host's syms
        double sym_points = sym_portion;


        THEN("Host and Symbionts points increase") {

            for( emp::Ptr<Organism> symbiont : syms) {
                REQUIRE(symbiont->GetPoints() == sym_points);
            }
            REQUIRE(host->GetPoints() == host_points);
        }
        host.Delete();
    }


    WHEN("Host interaction value <= 0 and Symbiont interaction value < 0") {
        WHEN("Host interaction value < Symbionts' interaction value") {
            double host_int_val = -0.5;
            double sym_int_val = -0.1;
            double host_orig_points = 0;
            double sym_orig_points = 0;

            emp::Ptr<PGGHost> host = emp::NewPtr<PGGHost>(random, &w, &config, host_int_val);


            emp::Ptr<PGGSymbiont> symbiont1 = emp::NewPtr<PGGSymbiont>(random, &w, &config, sym_int_val);
            emp::Ptr<PGGSymbiont> symbiont2 = emp::NewPtr<PGGSymbiont>(random, &w, &config, sym_int_val);
            emp::Ptr<PGGSymbiont> symbiont3 = emp::NewPtr<PGGSymbiont>(random, &w, &config, sym_int_val);
            emp::vector<emp::Ptr<Organism>> syms = {symbiont1, symbiont2, symbiont3};
            host->SetSymbionts(syms);

            double resources = 120;
            host->DistribResources(resources);

            int num_syms = 3;
            double sym_piece = resources / num_syms; // how much resource each sym gets
            double host_defense = -1 * (host_int_val * sym_piece);
            double remaining_resources = sym_piece - host_defense;
            double host_points = remaining_resources * num_syms; // * by num_syms bc points are added during each iteration through host's syms

            THEN("Symbiont points do not change (gets nothing from host), Host points increase") {
                for( emp::Ptr<Organism> symbiont : syms) {
                    REQUIRE(symbiont->GetPoints() == sym_orig_points);
                }
                REQUIRE(host->GetPoints() == host_points);
                REQUIRE(host->GetPoints() > host_orig_points);
            }
            host.Delete();
        }

        WHEN("Host interaction value > Symbionts' interaction value") {
            double host_int_val = -0.2;
            double sym_int_val = -0.6;
            double host_orig_points = 0;
            double sym_orig_points = 0;

            emp::Ptr<PGGHost> host = emp::NewPtr<PGGHost>(random, &w, &config, host_int_val);


            emp::Ptr<PGGSymbiont> symbiont1 = emp::NewPtr<PGGSymbiont>(random, &w, &config, sym_int_val);
            emp::Ptr<PGGSymbiont> symbiont2 = emp::NewPtr<PGGSymbiont>(random, &w, &config, sym_int_val);
            emp::Ptr<PGGSymbiont> symbiont3 = emp::NewPtr<PGGSymbiont>(random, &w, &config, sym_int_val);
            emp::vector<emp::Ptr<Organism>> syms = {symbiont1, symbiont2, symbiont3};
            host->SetSymbionts(syms);

            double resources = 120;
            host->DistribResources(resources);

            int num_syms = 3;
            double sym_piece = resources / num_syms; // how much resource each sym gets
            double host_defense = -1 * (host_int_val * sym_piece);
            double remaining_resources = sym_piece - host_defense;
            double sym_steals = (host_int_val - sym_int_val) * remaining_resources;
            double sym_points = sym_steals;
            double host_points = (remaining_resources - sym_steals) * num_syms; // * by num_syms bc points are added during each iteration through host's syms

            THEN("Symbionts points and Host points increase") {
                for( emp::Ptr<Organism> symbiont : syms) {
                    REQUIRE(symbiont->GetPoints() == sym_points);
                    REQUIRE(symbiont->GetPoints() > sym_orig_points);
                }
                REQUIRE(host->GetPoints() == host_points);
                REQUIRE(host->GetPoints() > host_orig_points);
            }
            host.Delete();
        }
    }
    WHEN("Host interaction value > 0 and Symbiont interaction value < 0, single symbiont") {
        double host_int_val = 0.1;
        double sym_int_val = -0.1;
        double host_orig_points = 0;
        double sym_orig_points = 0;

        emp::Ptr<PGGHost> host = emp::NewPtr<PGGHost>(random, &w, &config, host_int_val);
        emp::Ptr<PGGSymbiont> symbiont = emp::NewPtr<PGGSymbiont>(random, &w, &config, sym_int_val, sym_orig_points);
        host->AddSymbiont(symbiont);

        int resources = 100;
        host->DistribResources(resources);

        // int host_donation = 10; //host_int_val * resources
        int host_portion = 90;  //remaining amount
        int sym_steals = 9; //host_portion * sym_int_val * -1; new code value should be 18
        int sym_portion = 19; //sym_steals + host_donation; new code value should be 28
        host_portion = host_portion - sym_steals; //remove stolen resources from host's portion

        THEN("Symbionts points and Host points increase the correct amounts") {
            REQUIRE(symbiont->GetPoints() == sym_orig_points+sym_portion);
            REQUIRE(host->GetPoints() == host_orig_points+host_portion);
        }
        host.Delete();
    }
    WHEN("Host interaction value > 0 and Symbiont interaction value < 0, multiple symbionts") {
        double host_int_val = 0.1;
        double sym_int_val = -0.1;
        double host_orig_points = 0;
        double sym_orig_points = 0;

        emp::Ptr<PGGHost> host = emp::NewPtr<PGGHost>(random, &w, &config, host_int_val);


        emp::Ptr<PGGSymbiont> symbiont1 = emp::NewPtr<PGGSymbiont>(random, &w, &config, sym_int_val, sym_orig_points);
        emp::Ptr<PGGSymbiont> symbiont2 = emp::NewPtr<PGGSymbiont>(random, &w, &config, sym_int_val, sym_orig_points);
        emp::Ptr<PGGSymbiont> symbiont3 = emp::NewPtr<PGGSymbiont>(random, &w, &config, sym_int_val, sym_orig_points);
        emp::vector<emp::Ptr<Organism>> syms = {symbiont1, symbiont2, symbiont3};
        host->SetSymbionts(syms);

        double resources = 120;
        host->DistribResources(resources);


        int num_syms = 3;
        // double sym_piece = 40; //resources / num_syms

        // int host_donation = 4; //host_int_val * sym_piece
        double host_portion = 36;  //remaining amount
        double sym_steals = 3.6; //host_portion * sym_int_val * -1
        double sym_portion = 7.6; //sym_steals + host_donation
        host_portion = host_portion - sym_steals; //remove stolen resources from host's portion

        double host_final_portion = host_portion * num_syms;


       THEN("Symbionts points and Host points increase") {
           for( emp::Ptr<Organism> symbiont : syms) {


               REQUIRE(symbiont->GetPoints() == sym_portion);
               REQUIRE(symbiont->GetPoints() > sym_orig_points);
            }

            REQUIRE(host->GetPoints() == host_final_portion);
            REQUIRE(host->GetPoints() > host_orig_points);
        }
        host.Delete();
    }


    WHEN("Host interaction value < 0 and Symbiont interaction value >= 0") {
        double host_int_val = -0.1;
        double sym_int_val = 0.8;
        double host_orig_points = 0;
        double symbiont_orig_points = 0;

        emp::Ptr<PGGHost> host = emp::NewPtr<PGGHost>(random, &w, &config, host_int_val);


        emp::Ptr<PGGSymbiont> symbiont1 = emp::NewPtr<PGGSymbiont>(random, &w, &config, sym_int_val);
        emp::Ptr<PGGSymbiont> symbiont2 = emp::NewPtr<PGGSymbiont>(random, &w, &config, sym_int_val);
        emp::Ptr<PGGSymbiont> symbiont3 = emp::NewPtr<PGGSymbiont>(random, &w, &config, sym_int_val);
        emp::vector<emp::Ptr<Organism>> syms = {symbiont1, symbiont2, symbiont3};
        host->SetSymbionts(syms);

        double resources = 120;
        host->DistribResources(resources);

        int num_syms = 3;
        double sym_piece = resources / num_syms;
        double host_defense = -1 * (host_int_val * sym_piece);
        double host_portion = sym_piece - host_defense;
        double sym_portion = 0;

        double sym_points = sym_portion;
        double host_points = host_portion * num_syms; // * by num_syms bc points are added during each iteration through host's syms

        THEN("Symbiont points do not change (gets nothing from host), Host points increase") {
            for( emp::Ptr<Organism> symbiont : syms) {
                REQUIRE(symbiont->GetPoints() == sym_points);
                REQUIRE(symbiont->GetPoints() == symbiont_orig_points);
            }
            REQUIRE(host->GetPoints() == host_points);
            REQUIRE(host->GetPoints() > host_orig_points);

        }
        host.Delete();
    }
}

TEST_CASE("PGGVertical Transmission of Symbiont", "[pgg]") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    PGGWorld w(*random);
    PGGWorld * world = &w;
    SymConfigBase config;


    WHEN("When vertical transmission is enabled"){
        world->SetVertTrans(1);
        double host_int_val = .5;
        double sym_int_val = -.5;

        emp::Ptr<PGGHost> host = emp::NewPtr<PGGHost>(random, world, &config, host_int_val);
        emp::Ptr<PGGSymbiont> symbiont = emp::NewPtr<PGGSymbiont>(random, world, &config, sym_int_val);

        emp::Ptr<PGGHost> host_baby = emp::NewPtr<PGGHost>(random, world, &config, host->GetIntVal());
        long unsigned int expected_sym_size = host_baby->GetSymbionts().size() + 1;
        symbiont->VerticalTransmission(host_baby);

        THEN("Symbiont offspring are injected into host offspring") {
            REQUIRE(host_baby->GetSymbionts().size() == expected_sym_size);
        }
        host_baby.Delete();
        host.Delete();
        symbiont.Delete();
    }
    WHEN("When vertical transmission is disabled"){
        world->SetVertTrans(0);
        double host_int_val = .5;
        double sym_int_val = -.5;

        emp::Ptr<PGGHost> host = emp::NewPtr<PGGHost>(random, world, &config, host_int_val);
        emp::Ptr<PGGSymbiont> symbiont = emp::NewPtr<PGGSymbiont>(random, world, &config, sym_int_val);

        emp::Ptr<PGGHost> host_baby = emp::NewPtr<PGGHost>(random, world, &config, host->GetIntVal());
        long unsigned int expected_sym_size = host_baby->GetSymbionts().size();
        symbiont->VerticalTransmission(host_baby);

        THEN("Symbiont offspring are not injected into host offspring") {
            REQUIRE(host_baby->GetSymbionts().size() == expected_sym_size);
        }
        host_baby.Delete();
        host.Delete();
        symbiont.Delete();
    }

}

TEST_CASE("PGGSymbiont  PGGHost Pool Interaction", "[pgg]"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    PGGWorld w(*random);
    SymConfigBase config;
    config.SYM_LIMIT(3);
    config.SYNERGY(5);
    config.PGG_SYNERGY(1.1);

    double host_int_val = 1;
    double sym_int_val = 0;
    double donation = 0.1;
    double donation2 = 0.2;

    emp::Ptr<PGGHost> host = emp::NewPtr<PGGHost>(random, &w, &config, host_int_val);


    emp::Ptr<PGGSymbiont> symbiont1 = emp::NewPtr<PGGSymbiont>(random, &w, &config, sym_int_val,donation);
    emp::Ptr<PGGSymbiont> symbiont2 = emp::NewPtr<PGGSymbiont>(random, &w, &config, sym_int_val,donation2);
    emp::vector<emp::Ptr<Organism>> syms = {symbiont1, symbiont2};
    host->SetSymbionts(syms);
    double resources = 120;
    host->DistribResources(resources);


    double host_portion = 0;  //remaining amount
    double host_pool = 0; // no remaining pool
    double s1_final_source = 54+9*1.1;
    double s2_final_source = 48+9*1.1;


    REQUIRE(symbiont1->GetPoints() == s1_final_source);
    REQUIRE(symbiont2->GetPoints() == s2_final_source);
    REQUIRE(host->GetPoints() == host_portion);
    REQUIRE(host->GetPool() == host_pool);
    host.Delete();
}

TEST_CASE("PGGSym Dead and Removal", "[pgg]") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    PGGWorld w(*random);
    PGGWorld * world = &w;
    SymConfigBase config;
    config.SYM_LIMIT(2);


    double host_int_val = .5;
    double sym_int_val = -.5;

    emp::Ptr<PGGHost> host = emp::NewPtr<PGGHost>(random, world, &config, host_int_val);
    emp::Ptr<PGGSymbiont> p = emp::NewPtr<PGGSymbiont>(random, world, &config, sym_int_val);

    host->AddSymbiont(p);
    p->SetDead();

    long unsigned int expected_sym_size = 0;
    host->Process(0);
    REQUIRE(host->GetSymbionts().size() == expected_sym_size);
    host.Delete();
}
