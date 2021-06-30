#include "Phage.h"
#include "Host.h"

TEST_CASE("Phage Process") {

    GIVEN("A world and some phage") {

        emp::Ptr<emp::Random> random;
        random.New(5);
        SymWorld w(*random);
        SymWorld * world = &w;
        SymConfigBase config;
        config.LYSIS(1);
        double int_val = -1;

        WHEN("Lysis is enabled and the Phage's burst timer >= the burst time") {
            emp::Ptr<Phage> p;
            p.New(random, world, &config, int_val);
            emp::Ptr<Phage> p1;
            p1.New(random, world, &config, int_val);
            emp::Ptr<Phage> p2;
            p2.New(random, world, &config, int_val);

            emp::Ptr<Host> h;
            h.New(random, &w, &config);

            p->SetHost(h); 
            int time = 15;
            p->SetBurstTimer(time);

            h->AddReproSym(p1);
            h->AddReproSym(p2);

            THEN("Lysis occurs, the host dies") {
                size_t location = 2;
                // double resources_per_host_per_update = 40;
                
                p->process(location);
                
                bool host_dead = true;
                std::vector<emp::Ptr<Organism>> empty_syms = {};
                REQUIRE(h->GetDead() == host_dead);
                REQUIRE(h->GetReproSymbionts() == empty_syms);
            }

            p.Delete();
            h.Delete();
        }

        WHEN("Lysis is enabled and the Phage's burst timer < the burst time") {
            emp::Ptr<Host> h;
            h.New(random, &w, &config);

            double phage_points = 10;
            emp::Ptr<Phage> p3;
            p3.New(random, world, &config, int_val);
            emp::Ptr<Phage> p4;
            p4.New(random, world, &config, int_val);
            emp::Ptr<Phage> p5;
            p5.New(random, world, &config, int_val);
            p3->SetPoints(phage_points);

            h->AddReproSym(p4);
            h->AddReproSym(p5);
            int num_host_orig_syms = 2;
            p3->SetHost(h);

            THEN("Phage reproduces, Host gains repro symbionts, Phage loses points") {
                size_t location = 2;
                // bool host_dead = false;
                
                p3->process(location);

                REQUIRE(h->GetDead() == false);
                
                std::vector<emp::Ptr<Organism>> updated_repro_syms = h->GetReproSymbionts();

                REQUIRE( (int) updated_repro_syms.size() > num_host_orig_syms); // host gained repro syms

                REQUIRE(p3->GetPoints() < phage_points); // phage lost points


            }

            p3.Delete();
            h.Delete();

        }
        random.Delete();
    }    
}