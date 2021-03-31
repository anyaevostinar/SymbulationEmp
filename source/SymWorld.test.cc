#include "SymWorld.h"
#include "Symbiont.h"
#include "Phage.h"


TEST_CASE("PullResources") {
  GIVEN(" a world ") {
    emp::Random random(19);
    SymWorld world(random);
    int full_share = 100;
    world.SetResPerUpdate(full_share);

    WHEN(" the resources are unlimited ") {
      world.SetLimitedRes(false);

      THEN(" hosts get the full share of resources ") {
        REQUIRE(world.PullResources() == full_share);
      }
    }

    WHEN( " the resources are limited ") {
      world.SetLimitedRes(true);
      int original_total = 150;
      world.SetTotalRes(original_total);

      THEN(" first host gets full share of resources, next host gets a bit, everyone else gets nothing ") {
        REQUIRE(world.PullResources() == full_share);
        REQUIRE(world.PullResources() == (original_total-full_share));
        REQUIRE(world.PullResources() == 0);
        REQUIRE(world.PullResources() == 0);
      }
    }
  }
}

TEST_CASE( "Vertical Transmission" ) {
  GIVEN( "a world" ) {
    emp::Random random(17); 
    SymWorld w(random);

    WHEN( "the vertical taransmission rate is 0" ) {
      w.SetVertTrans(0);
        
      THEN( "there is never vertical transmission" ) {
        REQUIRE( w.WillTransmit() == false );
        REQUIRE( w.WillTransmit() == false );
        REQUIRE( w.WillTransmit() == false );
        REQUIRE( w.WillTransmit() == false );
        REQUIRE( w.WillTransmit() == false );
      }
    }

    WHEN( "the vertical taransmission rate is 1" ) {
      w.SetVertTrans(1);
      
      THEN( "there is always vertical transmission" ) {
        REQUIRE( w.WillTransmit() == true );
        REQUIRE( w.WillTransmit() == true );
        REQUIRE( w.WillTransmit() == true );
        REQUIRE( w.WillTransmit() == true );
        REQUIRE( w.WillTransmit() == true );
      }
    }

    WHEN( "the vertical taransmission rate is .5" ) {
      w.SetVertTrans(.5);
      
      THEN( "there is sometimes vertical transmission" ) {
        bool yes = false;
        bool no = false;
        for(int i = 0; i < 128; i++)//Odds of failure should be 1 in 170141183460469231731687303715884105728
          if(w.WillTransmit())
            yes = true;
          else
            no = true;
        REQUIRE( yes == true );
        REQUIRE( no == true );
      }
    }
  }
}



TEST_CASE( "World Capacity" ) {
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymWorld w(random);

    WHEN( "hosts are added" ) {

      int n = 7532;

      //inject organisms
      for (size_t i = 0; i < n; i++){
        Host *new_org;
        new_org = new Host(0);
        w.Inject(*new_org);
      }
        
      THEN( "the world's size becomes the number of hosts that were added" ) {
        REQUIRE( w.getPop().size() == n );
      }
    }
  }
}



TEST_CASE( "Interaction Patterns" ) {
  SymConfigBase config;
  emp::Ptr<SymConfigBase> config_ptr = &config;

  GIVEN( "a world without vertical transmission" ) {
    emp::Ptr<emp::Random> random = new emp::Random(17);
    SymWorld w(*random);
    w.SetVertTrans(0);
    w.SetMutRate(.002);
    w.SetSymLimit(500);
    w.SetHTransBool(true);
    w.SetHostRepro(400);
    w.SetResPerUpdate(100);
    w.SetSynergy(5);

    WHEN( "hostile hosts meet generous symbionts" ) {

      //inject organisms
      for (size_t i = 0; i < 1000; i++){
        Host *new_org;
        new_org = new Host(random, -.1);
        w.Inject(*new_org);
      
        emp::Ptr<Symbiont> new_sym = new Symbiont(random, &w, config_ptr, .1);
        w.InjectSymbiont(new_sym);
      }
      
      //Simulate
      for(int i = 0; i < 100; i++)//Burst time + 1
        w.Update();

      THEN( "the symbionts all die" ) {
        for(size_t i = 0; i < w.getPop().size(); i++)
          REQUIRE( !(w.getPop()[i] && w.getPop()[i]->HasSym()) );//We can't have a host exist with a symbiont in it.
      }
    }
  }


  GIVEN( "a world" ) {
    emp::Ptr<emp::Random> random = new emp::Random(17);
    SymWorld w(*random);
    w.SetPopStruct_Mixed(); // added this. still failing test
    w.SetVertTrans(.7);
    w.SetMutRate(.002);
    w.SetSymLimit(500);
    w.SetHTransBool(true);
    w.SetHostRepro(10);
    w.SetResPerUpdate(100);
    w.SetSynergy(5);
    w.Resize(100, 200);
    

    WHEN( "very generous hosts meet many very hostile symbionts" ) {

      //inject organisms
      for (size_t i = 0; i < 200; i++){
        Host *new_org;
        new_org = new Host(random, 1);
        w.Inject(*new_org);
      }
      for (size_t i = 0; i < 10000; i++){//Odds of failure should be 1 in 29387493568128248844
        emp::Ptr<Symbiont> new_sym = new Symbiont(random, &w, config_ptr, -1);
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

