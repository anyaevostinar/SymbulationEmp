#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPWorldSetup.cc"
#include "../../sgp_mode/utils.h"

#include "../../catch/catch.hpp"


TEST_CASE("Cure Hosts tests", "[sgp]"){
  GIVEN("a world") {
    // make world
    emp::Random random(61);
    sgpmode::SymConfigSGP config;
    sgpmode::SGPWorld world(random, &config);

    // making host sym pairs
    sgpmode::SGPHost host1(&random, &world, &config);    
    emp::Ptr<sgpmode::SGPSymbiont> sym1 = emp::NewPtr<sgpmode::SGPSymbiont> (&random, &world, &config);
    host1->AddSymbiont(sym1);
    world.AddOrgAt(emp::NewPtr<sgpmode::SGPHost>(host1), 0);

    // emp::Ptr<sgpmode::SGPHost> host2 = emp::NewPtr<sgpmode::SGPHost>(&random, &world, &config);
    // emp::Ptr<sgpmode::SGPSymbiont> sym2 = emp::NewPtr<sgpmode::SGPSymbiont> (&random, &world, &config);
    // host1->AddSymbiont(sym2);
    // world.AddOrgAt(host2, 1);

    // emp::Ptr<sgpmode::SGPHost> host3 = emp::NewPtr<sgpmode::SGPHost>(&random, &world, &config);
    // emp::Ptr<sgpmode::SGPSymbiont> sym3 = emp::NewPtr<sgpmode::SGPSymbiont> (&random, &world, &config);
    // host1->AddSymbiont(sym3);
    // world.AddOrgAt(host3, 2);


    // basic test
    WHEN("Hosts are not cured"){
      // Hosts and Sym world pop == pop_size
      REQUIRE(world.GetPop().size() == 3);
      REQUIRE(world.GetSymPop().size() == 3);
      // hosts have pointer to syms
      THEN("Hosts have symbionts"){
        REQUIRE(host1->HasSym());
      }
      THEN("syms have pointers to hosts"){
        REQUIRE(sym1->GetHost() == host1);
      }
      THEN("syms aren't set to dead"){
        REQUIRE(sym1->GetDead() == false);
      }
    } // When (Hosts are not cured)
    

  } //GIVEN
} //TEST_CASE