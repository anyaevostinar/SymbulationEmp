#include "../../default_mode/Host.h"
#include "../../default_mode/Symbiont.h"

TEST_CASE("Symbiont SetHost, GetHost") {

    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    SymWorld w(*random);
    double int_val = 1;

    emp::Ptr<Organism> h = new Host(random, &w, &config);
    Symbiont * s = new Symbiont(random, &w, &config, int_val);

    s->SetHost(h);

    REQUIRE(s->GetHost() == h);

}

TEST_CASE("Host SetSymbionts") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    SymWorld w(*random);
    double int_val = 1;
    config.SYM_LIMIT(2);

    Host * h = new Host(random, &w, &config);
    Symbiont * s1 = new Symbiont(random, &w, &config, int_val);
    Symbiont * s2 = new Symbiont(random, &w, &config, int_val);

    emp::vector<emp::Ptr<Organism>> syms;
    syms.push_back(s1);
    syms.push_back(s2);

    h->SetSymbionts(syms);

    REQUIRE(h->GetSymbionts().size() == syms.size());

    for(size_t i = 0; i < syms.size(); i++){
        emp::vector<emp::Ptr<Organism>> host_syms = h->GetSymbionts();
        Organism * curSym = host_syms[i];
        Organism * curHost = curSym->GetHost();
        REQUIRE(curSym == (Organism *) syms[i]);
        REQUIRE(curHost == h);
    }

    bool has_sym = true;
    REQUIRE(h->HasSym() == has_sym);

}

TEST_CASE("Host SymLimit") {
    emp::Ptr<emp::Random> random;
    random.New(-1);
    SymConfigBase config;
    SymWorld w(*random);
    double int_val = 1;

    Host * h = new Host(random, &w, &config);

    emp::Ptr<Symbiont> s1;
    s1.New(random, &w, &config, int_val);
    emp::Ptr<Symbiont> s2;
    s2.New(random, &w, &config, int_val);

    emp::vector<emp::Ptr<Organism>> syms;

    syms.push_back(s1);
    h->AddSymbiont(s1);
    REQUIRE(h->GetSymbionts() == syms);

    WHEN("A symbiont is added once the SymLimit has been reached ") {
        h->AddSymbiont(s2);

        THEN("The symbiont is not added to the host's syms") {
            REQUIRE(h->GetSymbionts() == syms); // s2 is not added, sym limit was reached

        }

    }
    random.Delete();
    s1.Delete();

}

TEST_CASE("Host AddSymbiont") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    config.SYM_LIMIT(6);
    SymWorld w(*random);
    double int_val = 1;

    Host * h = new Host(random, &w, &config);

    Symbiont * s1 = new Symbiont(random, &w, &config, int_val);
    Symbiont * s2 = new Symbiont(random, &w, &config, int_val);

    emp::vector<emp::Ptr<Organism>> syms;

    syms.push_back(s1);
    h->AddSymbiont(s1);

    REQUIRE(h->GetSymbionts() == syms);
    bool has_sym = true;
    REQUIRE(h->HasSym() == has_sym);

    h->AddSymbiont(s2);
    syms.push_back(s2);
    REQUIRE(h->GetSymbionts() == syms);
}

TEST_CASE("Host AddReproSym, ClearReproSym, GetReproSymbionts") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    config.SYM_LIMIT(6);
    SymWorld w(*random);
    double int_val = 1;

    Host * h = new Host(random, &w, &config);

    Symbiont * s1 = new Symbiont(random, &w, &config, int_val);
    Symbiont * s2 = new Symbiont(random, &w, &config, int_val);

    emp::vector<emp::Ptr<Organism>> repro_syms;

    repro_syms.push_back(s1);
    h->AddReproSym(s1);

    REQUIRE(h->GetReproSymbionts() == repro_syms);

    repro_syms.push_back(s2);
    h->AddReproSym(s2);

    REQUIRE(h->GetReproSymbionts() == repro_syms);

    h->ClearReproSyms();
    repro_syms.clear();
    REQUIRE(h->GetReproSymbionts() == repro_syms);
}

TEST_CASE("Host DistribResources") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymWorld w(*random);
    SymConfigBase config;
    config.SYM_LIMIT(6);
    config.SYNERGY(5);

    WHEN("Host interaction value >= 0 and  Symbiont interaction value >= 0") {

        double host_int_val = 0.5;
        double sym_int_val = 1;

        Host * h = new Host(random, &w, &config, host_int_val);


        Symbiont * s1 = new Symbiont(random, &w, &config, sym_int_val);
        Symbiont * s2 = new Symbiont(random, &w, &config, sym_int_val);
        Symbiont * s3 = new Symbiont(random, &w, &config, sym_int_val);
        emp::vector<emp::Ptr<Organism>> syms = {s1, s2, s3};
        h->SetSymbionts(syms);

        double resources = 120;
        h->DistribResources(resources);


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
            REQUIRE(h->GetPoints() == host_points);
        }
    }


    WHEN("Host interaction value <= 0 and Symbiont interaction value < 0") {
        double host_int_val = -0.5;
        double sym_int_val = -0.1;
        double host_orig_points = 0;
        double sym_orig_points = 0;

        Host * h = new Host(random, &w, &config, host_int_val);


        Symbiont * s1 = new Symbiont(random, &w, &config, sym_int_val);
        Symbiont * s2 = new Symbiont(random, &w, &config, sym_int_val);
        Symbiont * s3 = new Symbiont(random, &w, &config, sym_int_val);
        emp::vector<emp::Ptr<Organism>> syms = {s1, s2, s3};
        h->SetSymbionts(syms);

        WHEN("Host interaction value < Symbionts' interaction value") {
            double resources = 120;
            h->DistribResources(resources);

            int num_syms = 3;
            double sym_piece = resources / num_syms; // how much resource each sym gets
            double host_defense = -1 * (host_int_val * sym_piece);
            double remaining_resources = sym_piece - host_defense;
            double host_points = remaining_resources * num_syms; // * by num_syms bc points are added during each iteration through host's syms

            THEN("Symbiont points do not change (gets nothing from host), Host points increase") {
                for( emp::Ptr<Organism> symbiont : syms) {
                    REQUIRE(symbiont->GetPoints() == sym_orig_points);
                }
                REQUIRE(h->GetPoints() == host_points);
                REQUIRE(h->GetPoints() > host_orig_points);
            }
        }


        WHEN("Host interaction value > Symbionts' interaction value") {
            double host_int_val = -0.2;
            double sym_int_val = -0.6;
            double host_orig_points = 0;
            double sym_orig_points = 0;

            Host * h = new Host(random, &w, &config, host_int_val);


            Symbiont * s1 = new Symbiont(random, &w, &config, sym_int_val);
            Symbiont * s2 = new Symbiont(random, &w, &config, sym_int_val);
            Symbiont * s3 = new Symbiont(random, &w, &config, sym_int_val);
            emp::vector<emp::Ptr<Organism>> syms = {s1, s2, s3};
            h->SetSymbionts(syms);

            double resources = 120;
            h->DistribResources(resources);

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
                REQUIRE(h->GetPoints() == host_points);
                REQUIRE(h->GetPoints() > host_orig_points);
            }
        }
    }
    WHEN("Host interaction value > 0 and Symbiont interaction value < 0, single symbiont") {
        double host_int_val = 0.1;
        double sym_int_val = -0.1;
        double host_orig_points = 0;
        double sym_orig_points = 0;

        Host * h = new Host(random, &w, &config, host_int_val);
        Symbiont * s = new Symbiont(random, &w, &config, sym_int_val, sym_orig_points);
        h->AddSymbiont(s);

        int resources = 100;
        h->DistribResources(resources);

        // int host_donation = 10; //host_int_val * resources
        int host_portion = 90;  //remaining amount
        int sym_steals = 9; //host_portion * sym_int_val * -1; new code value should be 18
        int sym_portion = 19; //sym_steals + host_donation; new code value should be 28
        host_portion = host_portion - sym_steals; //remove stolen resources from host's portion

        THEN("Symbionts points and Host points increase the correct amounts") {
            REQUIRE(s->GetPoints() == sym_orig_points+sym_portion);
            REQUIRE(h->GetPoints() == host_orig_points+host_portion);
        }

    }
    WHEN("Host interaction value > 0 and Symbiont interaction value < 0, multiple symbionts") {
        double host_int_val = 0.1;
        double sym_int_val = -0.1;
        double host_orig_points = 0;
        double sym_orig_points = 0;

        Host * h = new Host(random, &w, &config, host_int_val);


        Symbiont * s1 = new Symbiont(random, &w, &config, sym_int_val, sym_orig_points);
        Symbiont * s2 = new Symbiont(random, &w, &config, sym_int_val, sym_orig_points);
        Symbiont * s3 = new Symbiont(random, &w, &config, sym_int_val, sym_orig_points);
        emp::vector<emp::Ptr<Organism>> syms = {s1, s2, s3};
        h->SetSymbionts(syms);

        double resources = 120;
        h->DistribResources(resources);


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

            REQUIRE(h->GetPoints() == host_final_portion);
            REQUIRE(h->GetPoints() > host_orig_points);
        }

    }


    WHEN("Host interaction value < 0 and Symbiont interaction value >= 0") {
        double host_int_val = -0.1;
        double sym_int_val = 0.8;
        double host_orig_points = 0;
        double symbiont_orig_points = 0;

        Host * h = new Host(random, &w, &config, host_int_val);


        Symbiont * s1 = new Symbiont(random, &w, &config, sym_int_val);
        Symbiont * s2 = new Symbiont(random, &w, &config, sym_int_val);
        Symbiont * s3 = new Symbiont(random, &w, &config, sym_int_val);
        emp::vector<emp::Ptr<Organism>> syms = {s1, s2, s3};
        h->SetSymbionts(syms);

        double resources = 120;
        h->DistribResources(resources);

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
            REQUIRE(h->GetPoints() == host_points);
            REQUIRE(h->GetPoints() > host_orig_points);

        }
    }
}

TEST_CASE("Vertical Transmission of Symbiont") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymWorld w(*random);
    SymWorld * world = &w;
    SymConfigBase config;


    WHEN("When vertical transmission is enabled"){
        world->SetVertTrans(1);
        double host_int_val = .5;
        double sym_int_val = -.5;

        emp::Ptr<Host> h = new Host(random, world, &config, host_int_val);
        emp::Ptr<Symbiont> s = new Symbiont(random, world, &config, sym_int_val);

        emp::Ptr<Host> host_baby = emp::NewPtr<Host>(random, world, &config, h->GetIntVal());
        long unsigned int expected_sym_size = host_baby->GetSymbionts().size() + 1;
        s->VerticalTransmission(host_baby);

        THEN("Symbiont offspring are injected into host offspring") {
            REQUIRE(host_baby->GetSymbionts().size() == expected_sym_size);
        }
    }
    WHEN("When vertical transmission is disabled"){
        world->SetVertTrans(0);
        double host_int_val = .5;
        double sym_int_val = -.5;

        emp::Ptr<Host> h = new Host(random, world, &config, host_int_val);
        emp::Ptr<Symbiont> s = new Symbiont(random, world, &config, sym_int_val);

        emp::Ptr<Host> host_baby = emp::NewPtr<Host>(random, world, &config, h->GetIntVal());
        long unsigned int expected_sym_size = host_baby->GetSymbionts().size();
        s->VerticalTransmission(host_baby);

        THEN("Symbiont offspring are not injected into host offspring") {
            REQUIRE(host_baby->GetSymbionts().size() == expected_sym_size);
        }
    }
}
