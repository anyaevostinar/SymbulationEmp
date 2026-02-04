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
    int pop_size = 5;

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
      // All syms from world.GetSymPop() are nullptrs
      THEN("All syms are nullptrs"){
        // sym_pop[i] still points to an address
        emp::World<Organism>::pop_t world_syms = world.GetSymPop();
        for(int i = 0; i < pop_size; i++){
          REQUIRE(world_syms[i] != nullptr);
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
        // world.CureHosts();
        // extra update runs through and kills syms
        world.Update();
        THEN("Syms population is zero"){
          // sym pop size is still the same. FAILS
          REQUIRE(world.GetSymPop().size() == 0);
        }
        THEN("Hosts don't point to syms"){
          for (int i = 0; i < pop_size; i++){
            REQUIRE(host_vect[i]->HasSym() == false);
          }
        }
        // can't check if syms are set to dead due to segmentation violation signal (syms are deleted nothing to point to)
        // THEN("syms are set to dead"){
        //   for (int i = 0; i < pop_size; i++){
        //     REQUIRE(sym_vect[i]->GetDead());
        //   }
        // } 
        // All syms from world.GetSymPop() are nullptrs
        THEN("Syms are nullptrs"){
        // sym_pop[i] still points to an address
          emp::World<Organism>::pop_t world_syms = world.GetSymPop();
          for(int i = 0; i < pop_size; i++){
            REQUIRE(world_syms[i] == nullptr);
          }
        }
      } // WHEN (run update)
      } // WHEN(Hosts are cured)

    WHEN("Host are cured while experiment is running"){
      config.CURE(1);
      int num_updates = 1;
      int total_updates = 4;
      config.CURE_UPDATES(num_updates);
      config.UPDATES(total_updates);
      world.RunExperiment(true);

      THEN("Sym population is 0"){
        // FAILS
        REQUIRE(world.GetSymPop().size() == 0);
      }
      THEN("Hosts do not have syms"){
        for (int i = 0; i < pop_size; i++){
          REQUIRE(host_vect[i]->HasSym() == false);
        }
      }
    } // WHEN (runExperiment)

    // WHEN("Simulating logic in RunExperiment"){
    //   config.CURE(1);
    //   int num_updates = 1;
    //   int total_updates = 4;
    //   config.CURE_UPDATES(num_updates);
    //   config.UPDATES(total_updates);
    //   emp::World<Organism>::pop_t world_syms = world.GetSymPop();
      
    //   for(int j = 0; j < total_updates; j++) {
    //     if (config.CURE() && j == num_updates) {
    //       world.CureHosts();
    //     }
    //     if (j < num_updates + 1) { // syms alive, still point to address
    //       REQUIRE(world_syms[j] != nullptr);
    //     }
    //     if (j > num_updates + 1) { // syms killed, don't point to address
    //       REQUIRE(world_syms[j] == nullptr);
    //     } 
    //     world.Update();
    //     } //outer for 
    //   } // WHEN (simulation of run experiement )
  } //GIVEN
} //TEST_CASE

