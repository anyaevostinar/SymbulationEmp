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
      for (size_t i = 0; i < n; i++){
        emp::Ptr<Host> new_org;
        new_org = new Host(&random, &w, &config, 0);
        w.AddOrgAt(new_org, w.size());
      }

      THEN( "the world's size becomes the number of hosts that were added" ) {
        REQUIRE( w.getPop().size() == n );
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

TEST_CASE( "SymDoBirth" ) {
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    SymWorld w(random);
    w.Resize(2,2);

    WHEN( "free living phage are not allowed" ) {
      config.FREE_LIVING_PHAGE(0);
      w.SetFreeLivingPhage(false);

      WHEN( "there is a valid neighbouring host" ){
        emp::Ptr<Host> host = new Host(&random, &w, &config, int_val);
        w.AddOrgAt(host, 0);

        emp::Ptr<Organism> sym = new Symbiont(&random, &w, &config, int_val);
        w.SymDoBirth(sym, 1);

        emp::vector<emp::Ptr<Organism>> syms = host->GetSymbionts();
        emp::Ptr<Organism> host_sym = syms[0];

        THEN( "the sym is inserted into the valid neighbouring host" ){
          REQUIRE(host_sym == sym);
          REQUIRE(w.GetNumOrgs() == 1);
        }
      }

      WHEN( "there is no valid neighbouring host" ){
        emp::Ptr<Organism> sym = new Symbiont(&random, &w, &config, int_val);
        w.SymDoBirth(sym, 1);

        THEN( "the sym is killed" ){
          //the world should be empty
          REQUIRE(w.GetNumOrgs() == 0);
        }
      }
    }


    WHEN( "free living phage are allowed"){
      config.FREE_LIVING_PHAGE(1);
      w.SetFreeLivingPhage(true);
      config.SYM_LIMIT(3);

      emp::Ptr<Host> host1 = new Host(&random, &w, &config, int_val);
      emp::Ptr<Host> host2 = new Host(&random, &w, &config, int_val);
      emp::Ptr<Host> host3 = new Host(&random, &w, &config, int_val);

      emp::Ptr<Symbiont> sym1 = new Symbiont(&random, &w, &config, int_val);
      emp::Ptr<Symbiont> sym2 = new Symbiont(&random, &w, &config, int_val);
      emp::Ptr<Symbiont> sym3 = new Symbiont(&random, &w, &config, int_val);

      WHEN("sym is inserted into an empty world"){
        THEN("it occupies some empty cell"){
          w.SymDoBirth(sym1, 0);
          REQUIRE(w.GetNumOrgs() == 1);
        }
      }
      WHEN("sym is inserted into a not-empty world"){
        THEN("it might be inserted into a neighbouring host"){
          w.AddOrgAt(host1, 0);
          w.AddOrgAt(host2, 2);

          w.SymDoBirth(sym1, 0);
          w.SymDoBirth(sym2, 1);
          w.SymDoBirth(sym3, 2);

          emp::vector<emp::Ptr<Organism>> syms1 = host1->GetSymbionts();
          emp::vector<emp::Ptr<Organism>> syms2 = host2->GetSymbionts();

          int totalSyms = syms1.size() + syms2.size();

          REQUIRE(w.GetNumOrgs() == 2);
          REQUIRE(syms1.size() == 1);
          REQUIRE(syms2.size() == 2);
          REQUIRE(totalSyms == 3);
        }
        THEN("it might be inserted into an empty cell"){
          w.AddOrgAt(host1, 0);
          w.SymDoBirth(sym1, 2);

          REQUIRE(w.GetNumOrgs() == 2);
        }
        THEN("it might be insterted into a cell with a sym, killing and replacing it"){
          w.SymDoBirth(sym1, 0);
          w.SymDoBirth(sym2, 1);
          emp::Ptr<Organism> oldSym = new Symbiont(&random, &w, &config, int_val);
          w.SymDoBirth(oldSym, 0);

          emp::Ptr<Organism> newSym = &w.GetOrg(2);

          REQUIRE(w.GetNumOrgs() == 2);
          REQUIRE(newSym == oldSym);
        }
      }
      WHEN("host is birthed into spot containing free living sym"){
        THEN("host absorbs that sym"){
          emp::Ptr<Organism> oldSym = new Symbiont(&random, &w, &config, int_val);
          emp::Ptr<Organism> oldHost = new Host(&random, &w, &config, int_val);
          w.AddOrgAt(sym1, 0);
          w.AddOrgAt(sym2, 1);
          w.AddOrgAt(oldSym, 2);
          w.DoBirth(oldHost, 1);

          emp::Ptr<Organism> newOrg = &w.GetOrg(2);

          REQUIRE(oldHost == newOrg);
          REQUIRE(oldHost->GetSymbionts().size() == 1);
          REQUIRE(oldHost->GetSymbionts().at(0) == oldSym);
          REQUIRE(w.GetNumOrgs() == 3);
        }
      }
    }
  }
}

TEST_CASE( "Update" ){
  GIVEN("a world"){
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    SymWorld w(random);
    w.Resize(2,2);
    w.SetLimitedRes(0);
    int resPerUpdate = 10;
    w.SetResPerUpdate(resPerUpdate);


    emp::Ptr<Host> host = new Host(&random, &w, &config, int_val);

    WHEN("free living syms are not allowed"){
      w.AddOrgAt(host, 0);

      WHEN("a host is dead"){
        THEN("it is removed from the world"){
          host->SetDead();
          REQUIRE(w.GetNumOrgs() == 1);

          w.Update();
          REQUIRE(w.GetNumOrgs() == 0);
        }
      }
      THEN("hosts process normally"){
        int resBeforeUpdate = host->GetPoints();
        w.Update();
        int resAfterUpdate = host->GetPoints();
        int resChange = resAfterUpdate - resBeforeUpdate;
        REQUIRE(resPerUpdate == resChange);
      }
    }
    WHEN("free living syms are allowed"){
      w.SetFreeLivingPhage(1);
      WHEN("there are no syms in the world"){
        THEN("hosts process normally"){
          w.AddOrgAt(host, 0);
          REQUIRE(w.GetNumOrgs() == 1);

          w.Update();
          REQUIRE(host->GetPoints() == resPerUpdate);
        }
      }
      WHEN("there are syms in the world"){
        config.LYSIS(1);
        w.SetLimitedRes(0);
        int burst_time = 5;
        config.BURST_TIME(burst_time);
        int resPerUpdate = 500;
        w.SetResPerUpdate(resPerUpdate);
        w.Resize(4,4);

        THEN("if only syms nothing changes"){
          w.SymDoBirth(new Symbiont(&random, &w, &config, int_val), 0);
          for(int i = 0; i <= burst_time; i++){
            w.Update();
          }
          REQUIRE(w.GetNumOrgs() == 1);
        }
        THEN("hosts and syms can mingle in the environment"){
          w.AddOrgAt(host, 0);
          w.SymDoBirth(new Symbiont(&random, &w, &config, int_val), 1);

          for(int i = 0; i <= burst_time; i++){
            w.Update();
          }

          REQUIRE(w.GetNumOrgs() == 5);
        }
      }
    }
  }
}
