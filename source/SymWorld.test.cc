#include "SymWorld.h"

TEST_CASE( "Vertical Transmission" ) {

  GIVEN( "a world" ) {
    emp::Random random(-1);
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
    emp::Random random(-1);
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
  GIVEN( "a world without vertical transmission" ) {
    emp::Random random(-1);
    SymWorld w(random);
    w.SetVertTrans(0);
    w.SetMutRate(.002);
    w.SetSymLimit(500);
    w.SetLysisBool(true);
    w.SetHTransBool(true);
    w.SetBurstSize(500);
    w.SetBurstTime(20);
    w.SetHostRepro(400);
    w.SetSymHRes(.1);
    w.SetSymLysisRes(.001);
    w.SetResPerUpdate(100);
    w.SetSynergy(5);

    WHEN( "hostile hosts meet generous symbionts" ) {

      //inject organisms
      for (size_t i = 0; i < 1000; i++){
        Host *new_org;
        new_org = new Host(-.1);
        w.Inject(*new_org);
      
        Symbiont new_sym; 
        new_sym = *(new Symbiont(.1));
        w.InjectSymbiont(new_sym);
      }
      
      //Simulate
      for(int i = 0; i < 21; i++)//Burst time + 1
        w.Update();

      THEN( "the symbionts all die" ) {
        for(size_t i = 0; i < w.getPop().size(); i++)
          REQUIRE( !(w.getPop()[i] && w.getPop()[i]->HasSym()) );//We can't have a host exist with a symbiont in it.
      }
    }
  }


  GIVEN( "a world" ) {
    emp::Random random(-1);
    SymWorld w(random);
    w.SetVertTrans(.7);
    w.SetMutRate(.002);
    w.SetSymLimit(500);
    w.SetLysisBool(true);
    w.SetHTransBool(true);
    w.SetBurstSize(500);
    w.SetBurstTime(50);
    w.SetHostRepro(10);
    w.SetSymHRes(3);
    w.SetSymLysisRes(3);
    w.SetResPerUpdate(100);
    w.SetSynergy(5);

    WHEN( "very generous hosts meet many very hostile symbionts" ) {

      //inject organisms
      for (size_t i = 0; i < 200; i++){
        Host *new_org;
        new_org = new Host(1);
        w.Inject(*new_org);
      }
      for (size_t i = 0; i < 10000; i++){//Odds of failure should be 1 in 29387493568128248844
        Symbiont new_sym; 
        new_sym = *(new Symbiont(-1));
        w.InjectSymbiont(new_sym);
      }
      
      //Simulate
      for(int i = 0; i < 51; i++)
        w.Update();

      THEN( "the hosts all die" ) {
        for(size_t i = 0; i < w.getPop().size(); i++)
          REQUIRE( !w.getPop()[i] );
      }
    }
  }
}

