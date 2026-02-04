#include "../../default_mode/SymWorld.h"
#include "../../default_mode/Symbiont.h"
#include "../../default_mode/Host.h"
#include "../../Empirical/include/emp/math/Random.hpp"


TEST_CASE("Cure Hosts tests", "[default]"){
  GIVEN("a world") {
    // make world
    emp::Random random(17);
    SymConfigBase config;
    SymWorld world(random, &config);
    int pop_size = 3;

    emp::vector<emp::Ptr<Organism>> host_vect;
    emp::vector<emp::Ptr<Organism>> sym_vect;

    // Adding hosts and symbionts to the world
    for (int i = 0; i < pop_size; i++) {
      emp::Ptr<Host> host;
      host.New(&random, &world, &config);
      world.InjectHost(host);
      host_vect.push_back(host);
      emp::Ptr<Symbiont> sym;
      sym.New(&random, &world, &config);
      host->AddSymbiont(sym);
      sym_vect.push_back(sym);
    }
  
    WHEN("Hosts are not cured"){
      // Hosts and Sym world pop == pop_size
      REQUIRE(world.GetPop().size() == pop_size);
      REQUIRE(world.GetSymPop().size() == pop_size);
      // hosts have pointer to syms
      THEN("Hosts have pointers to symbionts"){
        for (int i = 0; i < pop_size; i++){
          REQUIRE(host_vect[i]->HasSym());
        }
      }
      THEN("syms have pointers to hosts"){
      for (int i = 0; i < pop_size; i++){
        REQUIRE(sym_vect[i]->GetHost() == host_vect[i]);
      }
      }
      THEN("syms aren't set to dead"){
        for (int i = 0; i < pop_size; i++){
          REQUIRE(sym_vect[i]->GetDead() == false);
        }
      }
    } // When (Hosts are not cured)

    WHEN("Hosts are cured by calling CureHosts()"){
      world.CureHosts();
      // syms and hosts still point to eachother, pop size is the same
      THEN("All syms are marked dead"){
        for(int i = 0; i < pop_size; i++){
          REQUIRE(sym_vect[i]->GetDead());
        }
      }

      WHEN ("Run an extra update afterwards"){
        // extra update runs through and kills syms
        world.Update();
        THEN("Hosts don't point to syms"){
          for (int i = 0; i < pop_size; i++){
            REQUIRE(host_vect[i]->HasSym() == false);
          }
        }
        // // can't check if syms are set to dead due to segmentation violation signal (syms are deleted nothing to point to)
        // THEN("syms raise errors"){
        //   for (int i = 0; i < pop_size; i++){
        //     REQUIRE_THROWS_WITH(sym_vect[i]->GetDead(), "Segmentation fault (core dumped)");
        //   }
        // } 
      
      } // WHEN (run update)
      } // WHEN(Hosts are cured)

    WHEN("Host are cured while experiment is running"){
      config.CURE(1);
      int num_updates = 1;
      int total_updates = 3;
      config.CURE_UPDATES(num_updates);
      config.UPDATES(total_updates);
      world.RunExperiment(true);
      auto world_pop = world.GetPop();

      THEN("Hosts do not have syms"){
        for (int i = 0; i < pop_size; i++){
          REQUIRE(world_pop[i]->HasSym() == false);
        }
      }
    } // WHEN (runExperiment)

    WHEN("Simulating logic in RunExperiment"){
      config.CURE(1);
      int num_updates = 1;
      int total_updates = 4;
      config.CURE_UPDATES(num_updates);
      config.UPDATES(total_updates);
      
      for(int j = 0; j < total_updates; j++) {
        if (config.CURE() && j == num_updates) {
          world.CureHosts();
        }
        // num_updates + 1, since it takes an update for deaths to process
        if (j < num_updates + 1) { // syms alive, hosts have pointers to syms
          for (int i = 0; i < pop_size; i++){
            REQUIRE(host_vect[i]->HasSym());
          }
        }
        if (j > num_updates + 1) { // syms dead, hosts don't have pointers to syms
          for (int i = 0; i < pop_size; i++){
            REQUIRE(host_vect[i]->HasSym() == false);
          }
        } 
        world.Update();
        } //outer for 
      } // WHEN (simulation of run experiement )

  } //GIVEN
} //TEST_CASE

TEST_CASE("Curing a host with multiple symbionts"){
  GIVEN("a world"){
    // make world
    emp::Random random(17);
    SymConfigBase config;
    SymWorld world(random, &config);
    config.SYM_LIMIT(2);
    // setting up hosts
    emp::Ptr<Host> host_twos;
    host_twos.New(&random, &world, &config);
    world.InjectHost(host_twos);
    // setting up symbionts
    emp::vector<emp::Ptr<Organism>> syms;
    emp::Ptr<Symbiont> sym1;
    sym1.New(&random, &world, &config);
    emp::Ptr<Symbiont> sym2;
    sym2.New(&random, &world, &config); 
    syms.push_back(sym1);
    syms.push_back(sym2);
    host_twos->SetSymbionts(syms);
        
      WHEN("Host is not cured"){  
        THEN("Host has two symbionts"){
          REQUIRE(host_twos->GetSymbionts().size() == 2);
          REQUIRE(host_twos->HasSym());
        }
      }
      WHEN("Host is cured"){
        THEN("Host doesn't have any symbionts after being cured"){
          world.CureHosts();
          world.Update();
          REQUIRE(host_twos->HasSym() == false);
        }
      }

  } // GIVEN
} // TEST_CASE