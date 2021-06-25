#include "Phage.h"
#include "Host.h"

TEST_CASE("Phage Process") {

    emp::Ptr<emp::Random> random = new emp::Random(5);
    SymWorld w(*random);
    SymWorld * world = &w;
    SymConfigBase config;
    config.LYSIS(1);
    double int_val = -1;
    emp::Ptr<Phage> p = new Phage(random, world, &config, int_val);
    emp::Ptr<Phage> p1 = new Phage(random, world, &config, int_val);
    emp::Ptr<Phage> p2 = new Phage(random, world, &config, int_val);
    Host * h = new Host(random, &w, &config);

    WHEN("Lysis is enabled and the Phage's burst timer >= the burst time") {

        p->SetHost(h); 
        int time = 15;
        p->SetBurstTimer(time);

        h->AddReproSym(p1);
        h->AddReproSym(p2);

        THEN("Lysis occurs, the host dies") {
            size_t location = 2;
            double resources_per_host_per_update = 40;
            
            p->process(0,location);
            
            bool host_dead = true;
            std::vector<emp::Ptr<Organism>> empty_syms = {};
            REQUIRE(h->GetDead() == host_dead);
            REQUIRE(h->GetReproSymbionts() == empty_syms);
        }
    }

    WHEN("Lysis is enabled and the Phage's burst timer < the burst time") {
        emp::Ptr<emp::Random> random = new emp::Random(5);
        SymWorld w(*random);
        SymWorld * world = &w;
        SymConfigBase config;
        config.LYSIS(1);
        double int_val = -1;
        double phage_points = 10;
        emp::Ptr<Phage> p = new Phage(random, world, &config, int_val);
        emp::Ptr<Phage> p1 = new Phage(random, world, &config, int_val);
        emp::Ptr<Phage> p2 = new Phage(random, world, &config, int_val);
        Host * h = new Host(random, &w, &config);
        p->SetPoints(phage_points);

        h->AddReproSym(p1);
        h->AddReproSym(p2);
        std::vector<emp::Ptr<Organism>> repro_syms = {p1, p2}; 
        int num_host_orig_syms = 2;
        p->SetHost(h);

        THEN("Phage reproduces, Host gains repro symbionts, Phage loses points") {
            size_t location = 2;
            bool host_dead = false;
            
            p->process(0,location);

            REQUIRE(h->GetDead() == false);
            
            std::vector<emp::Ptr<Organism>> updated_repro_syms = h->GetReproSymbionts();

            REQUIRE( updated_repro_syms.size() > num_host_orig_syms); // host gained repro syms

            REQUIRE(p->GetPoints() < phage_points); // phage lost points


        }

    }
    
}