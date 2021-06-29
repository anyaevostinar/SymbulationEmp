#include "SymWorld.h"
#include "Symbiont.h"
#include "Phage.h"
#include "Host.h"


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
    SymConfigBase config;
    SymWorld w(random);

    WHEN( "hosts are added" ) {

      int n = 7532;

      //inject organisms
      for (int i = 0; i < n; i++){
        emp::Ptr<Host> new_org;
        new_org = new Host(&random, &w, &config, 0);
        w.AddOrgAt(new_org, w.size());
      }
        
      THEN( "the world's size becomes the number of hosts that were added" ) {
        REQUIRE( (int) w.getPop().size() == n );
      }
    }
  }
}



TEST_CASE( "Interaction Patterns" ) {
  SymConfigBase config;

  GIVEN( "a world without vertical transmission" ) {
    emp::Ptr<emp::Random> random = new emp::Random(17);
    SymWorld w(*random);
    config.VERTICAL_TRANSMISSION(0);
    w.SetVertTrans(0);
    config.MUTATION_SIZE(0);
    config.SYM_LIMIT(500);
    config.HORIZ_TRANS(true);
    config.HOST_REPRO_RES(400);
    config.RES_DISTRIBUTE(100);
    w.SetResPerUpdate(100);
    config.SYNERGY(5);

    WHEN( "hostile hosts meet generous symbionts" ) {

      //inject organisms
      for (size_t i = 0; i < 10; i++){
        emp::Ptr<Host> new_org = emp::NewPtr<Host>(random, &w, &config, -0.1);
        w.AddOrgAt(new_org, w.size());
      }
      for (size_t i = 0; i< 10; i++){
        emp::Ptr<Symbiont> new_sym = emp::NewPtr<Symbiont>(random, &w, &config, 0.1);
        w.InjectSymbiont(new_sym);
      }
      
      //Simulate
      for(int i = 0; i < 100; i++) {
        w.Update();
      }

      THEN( "the symbionts all die" ) {
        for(size_t i = 0; i < w.getPop().size(); i++)
          REQUIRE( !(w.getPop()[i] && w.getPop()[i]->HasSym()) );//We can't have a host exist with a symbiont in it.
      }
    }
  }




  GIVEN( "a world" ) {
    emp::Ptr<emp::Random> random = new emp::Random(17);
    SymWorld w(*random);
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
    w.Resize(100, 200);
    

    WHEN( "very generous hosts meet many very hostile symbionts" ) {

      //inject organisms
      for (size_t i = 0; i < 200; i++){
        Host *new_org;
        new_org = new Host(random, &w, &config, 1);
        w.AddOrgAt(new_org, w.size());
      }
      for (size_t i = 0; i < 10000; i++){
        emp::Ptr<Symbiont> new_sym = new Symbiont(random, &w, &config, -1);
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

TEST_CASE( "Hosts injected correctly" ) {
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    SymWorld w(random);

    WHEN( "host added with interaction value 1" ) {
      //inject organism
      emp::Ptr<Host> new_org1;
      new_org1 = new Host(&random, &w, &config, 1);
      w.AddOrgAt(new_org1, 0);
      
      THEN( "host has interaction value of 1" ) {
        REQUIRE( w.GetOrg(0).GetIntVal() == 1 );
      }
    }
    WHEN( "host added with interaction value -1" ) {
      //inject organism
      emp::Ptr<Host> new_org1;
      new_org1 = new Host(&random, &w, &config, -1);
      w.AddOrgAt(new_org1, 0);
      
      THEN( "host has interaction value of -1" ) {
        REQUIRE( w.GetOrg(0).GetIntVal() == -1 );
      }
    }
    WHEN( "host added with interaction value 0" ) {
      //inject organism
      emp::Ptr<Host> new_org1;
      new_org1 = new Host(&random, &w, &config, 0);
      w.AddOrgAt(new_org1, 0);
      
      THEN( "host has interaction value of 0" ) {
        REQUIRE( w.GetOrg(0).GetIntVal() == 0 );
      }
    }
  }
}