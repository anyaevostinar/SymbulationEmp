#include "../../pgg_mode/PGGHost.h"
#include "../../pgg_mode/PGGSymbiont.h"

TEST_CASE( "PGG Interaction Patterns", "[pgg]" ) {
  SymConfigBase config;

  GIVEN( "a PGGworld without vertical transmission" ) {
    emp::Ptr<emp::Random> random = new emp::Random(17);
    PGGWorld w(*random);
    config.VERTICAL_TRANSMISSION(0);
    w.SetVertTrans(0);
    config.MUTATION_SIZE(0);
    config.SYM_LIMIT(500);
    config.HORIZ_TRANS(true);
    config.HOST_REPRO_RES(400);
    config.RES_DISTRIBUTE(100);
    w.SetResPerUpdate(100);
    config.SYNERGY(5);
    config.PGG(1);

    WHEN( "hostile hosts meet generous symbionts" ) {

      //inject organisms
      for (size_t i = 0; i < 10; i++){
        emp::Ptr<PGGHost> new_org = emp::NewPtr<PGGHost>(random, &w, &config, -0.1);
        w.AddOrgAt(new_org, w.size());
      }
      for (size_t i = 0; i< 10; i++){
        emp::Ptr<PGGSymbiont> new_sym = emp::NewPtr<PGGSymbiont>(random, &w, &config, 0.1);
        w.InjectSymbiont(new_sym);
      }

      //Simulate
      for(int i = 0; i < 100; i++) {
        w.Update();
      }

      THEN( "the symbionts all die" ) {
        for(size_t i = 0; i < w.GetPop().size(); i++)
          REQUIRE( !(w.GetPop()[i] && w.GetPop()[i]->HasSym()) );//We can't have a host exist with a symbiont in it.
      }
    }
  }

  GIVEN( "a PGGworld" ) {
    emp::Random random(17);
    PGGWorld w(random);
    w.SetPopStruct_Mixed();
    config.GRID(0);
    config.VERTICAL_TRANSMISSION(0.7);
    w.SetVertTrans(0.7);
    config.MUTATION_SIZE(0.002);
    config.SYM_LIMIT(500);
    config.HORIZ_TRANS(true);
    config.HOST_REPRO_RES(10);
    config.RES_DISTRIBUTE(100);
    config.SYNERGY(5);
    config.PGG(1);
    int world_size = 20000;
    w.Resize(world_size);


    WHEN( "very generous hosts meet many very hostile symbionts" ) {

      //inject organisms
      for (size_t i = 0; i < 200; i++){
        emp::Ptr<PGGHost> new_org;
        new_org.New(&random, &w, &config, 1);
        w.AddOrgAt(new_org, w.size());
      }
      for (size_t i = 0; i < 10000; i++){
        emp::Ptr<PGGSymbiont> new_sym;
        new_sym.New(&random, &w, &config, -1);
        w.InjectSymbiont(new_sym);
      }

      //Simulate
      for(int i = 0; i < 100; i++)
        w.Update();

      THEN( "the hosts cannot reproduce" ) {
          REQUIRE( w.GetNumOrgs() == 200 );
      }
    }
  }
}
