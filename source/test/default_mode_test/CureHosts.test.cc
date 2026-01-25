#include "../../default_mode/SymWorld.h"
#include "../../default_mode/Symbiont.h"
#include "../../default_mode/Host.h"
#include "../../Empirical/include/emp/math/Random.hpp"


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
      THEN ('Hosts still have symbionts') {
        REQUIRE(host1->HasSym());
        REQUIRE(host2->HasSym());
        REQUIRE(host3->HasSym());
      }
    }

    WHEN ('Call CureHost()') {
      world.CureHosts();
      THEN ('Hosts are cure') {
        REQUIRE(host1->HasSym() == false);
        REQUIRE(host2->HasSym() == false);
        REQUIRE(host3->HasSym() == false);
      }
    }
  } //GIVEN
} //TEST_CASE

TEST_CASE("CureHostIntegration", "[default]") {
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigBase config;
    SymWorld world(random, &config);
    int int_val = 0;
    // int total_updates = 10;
    int num_updates = 1;
    config.CURE(1);
    config.CURE_UPDATES(num_updates);
    config.UPDATES(2);

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

    WHEN('No experiment run with config.CURE(1)')
      REQUIRE(host1->HasSym());
      REQUIRE(host2->HasSym());
      REQUIRE(host3->HasSym());
    WHEN("Hosts are cured of all symbionts on a specific update") {
      //Simulate
      world.RunExperiment(true);
      REQUIRE(host1->HasSym() == false);
      REQUIRE(host2->HasSym() == false);
      REQUIRE(host3->HasSym() == false);
    }
  } //GIVEN
} //TEST_CASE


TEST_CASE("Simulation of RunExperiment CureHosts logic", "[default]"){
  GIVEN("a world"){
    emp::Random random(17);
    SymConfigBase config;
    SymWorld world(random, &config);
    
    // num_updates and total_updates values == or > 100 can cause fail or cause errors
    int int_val = 0;
    int num_updates = 3;
    config.CURE_UPDATES(num_updates);
    int total_updates = 5; //config.UPDATES

    // Injecting hosts and symbionts into the world
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


    WHEN("Run Updates with config.CURE(1)") {
      config.CURE(1);
      for(int i = 0; i < total_updates; i++) {
        world.Update();
        if (config.CURE() && i == num_updates) {
          world.CureHosts();
        }
        if (i < num_updates) {
          REQUIRE(host1->HasSym());
          REQUIRE(host2->HasSym());
          REQUIRE(host3->HasSym());
        }
        if (i >= num_updates) {
          REQUIRE(host1->HasSym() == false);
          REQUIRE(host2->HasSym() == false);
          REQUIRE(host3->HasSym() == false);
        }
      }
    } //WHEN

    WHEN("CURE(0) and CURE_UPDATES(num_updates > 0)"){
      config.CURE(0);
      for(int i = 0; i < total_updates; i++) {
        world.Update();
        if (config.CURE() && i == num_updates) {
          world.CureHosts();
        }
        REQUIRE(host1->HasSym());
        REQUIRE(host2->HasSym());
        REQUIRE(host3->HasSym());
      }
     } //WHEN
  } //GIVEN
} //TEST_CASE

TEST_CASE("Adding hosts and symbionts in a loop", "[default]"){
  GIVEN("a world") {
    // make world
    emp::Random random(17);
    SymConfigBase config;
    SymWorld world(random, &config);
    int int_val = 0;
    int pop_size = 10;

    for (int i = 0; i < pop_size; i++) {
    emp::Ptr<Host> new_org;
    new_org.New(&random, &world, &config, int_val);
    world.InjectHost(new_org);
    emp::Ptr<Symbiont> sym;
    sym.New(&random, &world, &config, int_val);
    world.InjectSymbiont(sym);
  }
  REQUIRE(world.GetPop().size() == 10);
  REQUIRE(world.GetSymPop().size() == 10);


  } //GIVEN
} //TEST_CASE

// TEST_CASE("Problems with CureHosts()", "[default]") {
//   GIVEN("a world") {
//     // make world
//     emp::Random random(17);
//     SymConfigBase config;
//     SymWorld world(random, &config);
//     int int_val = 0;

//     // host and sym 1
//     emp::Ptr<Organism> host1 = emp::NewPtr<Host>(&random, &world, &config, int_val);
//     world.AddOrgAt(host1, 0);
//     emp::Ptr<Organism> sym1 = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
//     host1->AddSymbiont(sym1);

//     // host and sym 2
//     emp::Ptr<Organism> host2 = emp::NewPtr<Host>(&random, &world, &config, int_val);
//     world.AddOrgAt(host2, 1);
//     emp::Ptr<Organism> sym2 = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
//     host2->AddSymbiont(sym2);

//     // host and sym 3
//     emp::Ptr<Organism> host3 = emp::NewPtr<Host>(&random, &world, &config, int_val);
//     world.AddOrgAt(host3, 2);
//     emp::Ptr<Organism> sym3 = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
//     host3->AddSymbiont(sym3);

//     WHEN("CureHosts() is called") {
//       world.CureHosts();
//       THEN("All symbionts should be dead") {
//         REQUIRE(sym1->GetDead());
//         REQUIRE(sym2->GetDead());
//         REQUIRE(sym3->GetDead());
//       }
//       THEN("Symbiont population should be 0"){
//         REQUIRE(world.GetSymPop().size() == 0);
//       }
//       THEN("Syms shoud not have a host"){
//         REQUIRE(sym1->GetHost() != host1);
//         REQUIRE(sym2->GetHost() != host2);
//         REQUIRE(sym3->GetHost() != host3);
//       }
//     }
//   } //GIVEN
// } //TEST_CASE