#include "../../default_mode/SymWorld.h"
#include "../../default_mode/Symbiont.h"
#include "../../lysis_mode/Phage.h"
#include "../../lysis_mode/LysisWorld.h"
#include "../../default_mode/Host.h"


TEST_CASE("CureHost", "[default]"){
  GIVEN("a world") {
    // make world
    emp::Random random(17);
    SymConfigBase config;
    SymWorld world(random, &config);
    int int_val = 0;

    // host and sym 1
    emp::Ptr<Organism> host1 = emp::NewPtr<Host>(&random, &world, &config, int_val);
    world.AddOrgAt(host1, 0);
    emp::Ptr<Organism> sym1 = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
    host1->AddSymbiont(sym1);

    // host and sym 2
    emp::Ptr<Organism> host2 = emp::NewPtr<Host>(&random, &world, &config, int_val);
    world.AddOrgAt(host2, 1);
    emp::Ptr<Organism> sym2 = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
    host2->AddSymbiont(sym2);

    // host and sym 3
    emp::Ptr<Organism> host3 = emp::NewPtr<Host>(&random, &world, &config, int_val);
    world.AddOrgAt(host3, 2);
    emp::Ptr<Organism> sym3 = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
    host3->AddSymbiont(sym3);

    WHEN ('Host are not cured') {
      REQUIRE(host1->HasSym());
      REQUIRE(host2->HasSym());
      REQUIRE(host3->HasSym());
    }
    WHEN ('Hosts are cured') {
      world.CureHosts();
      REQUIRE(host1->HasSym() == false);
      REQUIRE(host2->HasSym() == false);
      REQUIRE(host3->HasSym() == false);
    }
  } //GIVEN
} //TEST_CASE

TEST_CASE("CureHostIntegration", "[default]") {
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigBase config;
    SymWorld world(random, &config);
    int int_val = 0;
    int total_updates = 10;
    int cure_updates = 3;
    config.CURE(1);
    config.CURE_UPDATES(cure_updates);

    // Adding hosts and syms
    // host and sym 1
    emp::Ptr<Organism> host1 = emp::NewPtr<Host>(&random, &world, &config, int_val);
    world.AddOrgAt(host1, 0);
    emp::Ptr<Organism> sym1 = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
    host1->AddSymbiont(sym1);

    // host and sym 2
    emp::Ptr<Organism> host2 = emp::NewPtr<Host>(&random, &world, &config, int_val);
    world.AddOrgAt(host2, 1);
    emp::Ptr<Organism> sym2 = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
    host2->AddSymbiont(sym2);

    // host and sym 3
    emp::Ptr<Organism> host3 = emp::NewPtr<Host>(&random, &world, &config, int_val);
    world.AddOrgAt(host3, 2);
    emp::Ptr<Organism> sym3 = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
    host3->AddSymbiont(sym3);

    WHEN("Hosts are cured of all symbionts on a specific update") {
      //Simulate
      for(int i = 0; i < total_updates; i++) {
        world.Update();
        std::cout<<"Update"<<i<<std::endl;
        // 3
        std::cout<<world.GetSymPop().size()<<std::endl;
        // 3
        std::cout<<world.GetPop().size()<<std::endl;
        // REQUIRE(world.GetSymPop().size() > 0);
        REQUIRE(host1->HasSym());
        REQUIRE(host2->HasSym());
        REQUIRE(host3->HasSym());
        if(i > cure_updates) {
          REQUIRE(host1->HasSym() == false);
          REQUIRE(host2->HasSym() == false);
          REQUIRE(host3->HasSym() == false);
          // REQUIRE(world.GetSymPop().size() == 0);
        }
      }
    }
  } //GIVEN
} //TEST_CASE



  // making hosts in loop
  //   std::vector<std::string> host_vector = {h1, h2, h3, h4, h5}

  //   // inject hosts
  //   for (int i = 0; i < 5; i++) {
  //     emp::Ptr<Organism> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
  //     world.AddOrgAt(host, i);
      
  //     // inject syms
  //     emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
  //     host->AddSymbiont(sym);
  //   }

  //   // checking each host has a sym
  //   for (int i = 0; i < 5; i++) {
  //       REQUIRE(host->HasSym()); }