#include "../../pgg_mode/PGGHost.h"
#include "../../pgg_mode/PGGSymbiont.h"

TEST_CASE( "PGG Interaction Patterns", "[pgg]" ) {
  SymConfigBase config;

  GIVEN( "a PGGworld without vertical transmission" ) {
    emp::Ptr<emp::Random> random = new emp::Random(17);
    PGGWorld world(*random, &config);
    config.VERTICAL_TRANSMISSION(0);
    config.MUTATION_SIZE(0);
    config.SYM_LIMIT(500);
    config.HORIZ_TRANS(true);
    config.HOST_REPRO_RES(400);
    config.RES_DISTRIBUTE(100);
    config.SYNERGY(5);
    config.PGG(1);

    WHEN( "hostile hosts meet generous symbionts" ) {

      //inject organisms
      for (size_t i = 0; i < 10; i++){
        emp::Ptr<PGGHost> new_org = emp::NewPtr<PGGHost>(random, &world, &config, -0.1);
        world.AddOrgAt(new_org, world.size());
      }
      for (size_t i = 0; i< 10; i++){
        emp::Ptr<PGGSymbiont> new_sym = emp::NewPtr<PGGSymbiont>(random, &world, &config, 0.1);
        world.InjectSymbiont(new_sym);
      }

      //Simulate
      for(int i = 0; i < 100; i++) {
        world.Update();
      }

      THEN( "the symbionts all die" ) {
        for(size_t i = 0; i < world.GetPop().size(); i++)
          REQUIRE( !(world.GetPop()[i] && world.GetPop()[i]->HasSym()) );//We can't have a host exist with a symbiont in it.
      }
    }
  }

  GIVEN( "a PGGworld" ) {
    emp::Random random(17);
    PGGWorld world(random, &config);
    world.SetPopStruct_Mixed();
    config.GRID(0);
    config.VERTICAL_TRANSMISSION(0.7);
    config.VERTICAL_TRANSMISSION(0.7);
    config.MUTATION_SIZE(0.002);
    config.SYM_LIMIT(500);
    config.HORIZ_TRANS(true);
    config.HOST_REPRO_RES(10);
    config.RES_DISTRIBUTE(100);
    config.SYNERGY(5);
    config.PGG(1);
    int world_size = 20000;
    world.Resize(world_size);


    WHEN( "very generous hosts meet many very hostile symbionts" ) {

      //inject organisms
      for (size_t i = 0; i < 200; i++){
        emp::Ptr<PGGHost> new_org;
        new_org.New(&random, &world, &config, 1);
        world.AddOrgAt(new_org, world.size());
      }
      for (size_t i = 0; i < 10000; i++){
        emp::Ptr<PGGSymbiont> new_sym;
        new_sym.New(&random, &world, &config, -1);
        world.InjectSymbiont(new_sym);
      }

      //Simulate
      for(int i = 0; i < 100; i++)
        world.Update();

      THEN( "the hosts cannot reproduce" ) {
          REQUIRE( world.GetNumOrgs() == 200 );
      }
    }
  }
}

TEST_CASE("PGG SetupSymbionts", "[pgg]") {
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigBase config;
    PGGWorld world(random, &config);

    size_t world_size = 6;
    world.Resize(world_size);
    config.FREE_LIVING_SYMS(1);

    size_t num_to_add = 2;
    world.SetupSymbionts(&num_to_add);

    size_t num_added = world.GetNumOrgs();
    REQUIRE(num_added == num_to_add);

    emp::Ptr<Organism> symbiont;
    for (size_t i = 0; i < world_size; i++) {
      symbiont = world.GetSymAt(i);
      if (symbiont) {
        REQUIRE(symbiont->GetName() == "PGGSymbiont");
        REQUIRE(symbiont->GetDonation() == config.PGG_DONATE());
      }
    }
  }
}

TEST_CASE("PGG SetupHosts", "[pgg]") {
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigBase config;
    PGGWorld world(random, &config);

    size_t num_to_add = 5;
    world.SetupHosts(&num_to_add);
    size_t num_added = world.GetNumOrgs();
    REQUIRE(num_added == num_to_add);

    emp::Ptr<Organism> host = world.GetPop()[0];
    REQUIRE(host != nullptr);
    REQUIRE(host->GetName() == "PGGHost");
  }
}
