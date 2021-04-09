#include "Host.h"
#include "Symbiont.h"

TEST_CASE("Host SetSymbionts") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    SymWorld w(*random);
    double int_val = 1;

    Host * h = new Host(random, &w, &config);
    Symbiont * s1 = new Symbiont(random, &w, &config, int_val);
    Symbiont * s2 = new Symbiont(random, &w, &config, int_val);
    
    emp::vector<emp::Ptr<Organism>> syms;
    syms.push_back(s1);
    syms.push_back(s2);

    h->SetSymbionts(syms);
    REQUIRE(h->GetSymbionts() == syms);

    bool has_sym = true;
    REQUIRE(h->HasSym() == has_sym);

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

    WHEN("Host interaction value > 0 and Symbiont interaction value < 0") {
         double host_int_val = 0.6;
        double sym_int_val = -0.2;
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
        double sym_piece = resources / num_syms;
        double host_donation = 0;
        double host_portion = 0;
        double sym_portion = 0;
        double host_points = 0;
        

       THEN("Symbionts points and Host points increase") {
           for( emp::Ptr<Organism> symbiont : syms) {
               host_donation = host_int_val * sym_piece; 
               host_portion = sym_piece - host_donation; 
               sym_piece -= host_donation; 
                
               double sym_steals = -1.0 * (sym_piece * sym_int_val); 
               host_portion -= sym_steals; 
               sym_portion = host_donation + sym_steals; 

               REQUIRE(symbiont->GetPoints() == sym_portion); 
               REQUIRE(symbiont->GetPoints() > sym_orig_points);

               host_points += host_portion; // portion gets added to points in each iteration 
            }

            REQUIRE(h->GetPoints() == host_points); 
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