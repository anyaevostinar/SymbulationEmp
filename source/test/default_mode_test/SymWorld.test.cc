#include "../../default_mode/SymWorld.h"
#include "../../default_mode/Symbiont.h"
#include "../../lysis_mode/Phage.h"
#include "../../lysis_mode/LysisWorld.h"
#include "../../default_mode/Host.h"


TEST_CASE("PullResources", "[default]") {
  GIVEN(" a world ") {
    emp::Random random(19);
    SymWorld world(random);
    int full_share = 100;

    WHEN(" the resources are unlimited ") {
      world.SetLimitedRes(false);

      THEN(" organisms get as many resources as they request ") {
        REQUIRE(world.PullResources(full_share) == full_share);
      }
    }

    WHEN( " the resources are limited ") {
      world.SetLimitedRes(true);
      int original_total = 150;
      world.SetTotalRes(original_total);

      THEN(" first organism gets full share of resources, next host gets a bit, everyone else gets nothing ") {
        REQUIRE(world.PullResources(full_share) == full_share);
        REQUIRE(world.PullResources(full_share) == (original_total-full_share));
        REQUIRE(world.PullResources(full_share) == 0);
        REQUIRE(world.PullResources(full_share) == 0);
      }
    }
  }
}

TEST_CASE( "Vertical Transmission", "[default]" ) {
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

TEST_CASE( "World Capacity", "[default]" ) {
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    SymWorld w(random);

    WHEN( "hosts are added" ) {

      int n = 7532;

      //inject organisms
      for (int i = 0; i < n; i++){
        emp::Ptr<Host> new_org;
        new_org.New(&random, &w, &config, 0);
        w.AddOrgAt(new_org, w.size());
      }

      THEN( "the world's size becomes the number of hosts that were added" ) {
        REQUIRE( (int) w.GetPop().size() == n );
      }
    }
  }
}

TEST_CASE( "Interaction Patterns", "[default]" ) {
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
      int width = 10;
      int height = 1;
      int world_size = width * height;
      w.Resize(width, height);
      for (int i = 0; i < world_size; i++){
        emp::Ptr<Symbiont> new_sym = emp::NewPtr<Symbiont>(random, &w, &config, 0.1);
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




  GIVEN( "a world" ) {
    emp::Random random(17);
    SymWorld w(random);
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

    WHEN( "very generous hosts meet many very hostile symbionts" ) {

      //inject organisms
      for (size_t i = 0; i < 200; i++){
        emp::Ptr<Host> new_org;
        new_org.New(&random, &w, &config, 1);
        w.AddOrgAt(new_org, w.size());
      }

      int width = 100;
      int height = 200;
      w.Resize(width, height);

      for (size_t i = 0; i < 10000; i++){
        emp::Ptr<Symbiont> new_sym;
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

TEST_CASE( "Hosts injected correctly", "[default]" ) {
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    SymWorld w(random);

    WHEN( "host added with interaction value 1" ) {
      //inject organism
      emp::Ptr<Host> new_org1;
      new_org1.New(&random, &w, &config, 1);
      w.AddOrgAt(new_org1, 0);

      THEN( "host has interaction value of 1" ) {
        REQUIRE( w.GetOrg(0).GetIntVal() == 1 );
      }
    }
    WHEN( "host added with interaction value -1" ) {
      //inject organism
      emp::Ptr<Host> new_org1;
      new_org1.New(&random, &w, &config, -1);
      w.AddOrgAt(new_org1, 0);

      THEN( "host has interaction value of -1" ) {
        REQUIRE( w.GetOrg(0).GetIntVal() == -1 );
      }
    }
    WHEN( "host added with interaction value 0" ) {
      //inject organism
      emp::Ptr<Host> new_org1;
      new_org1.New(&random, &w, &config, 0);
      w.AddOrgAt(new_org1, 0);

      THEN( "host has interaction value of 0" ) {
        REQUIRE( w.GetOrg(0).GetIntVal() == 0 );
      }
    }
  }
}

TEST_CASE( "InjectSymbiont", "[default]" ){
  GIVEN( "a world" ){
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    SymWorld w(random);
    int world_size = 4;

    WHEN( "free living syms are not allowed" ){
      w.Resize(world_size);
      config.FREE_LIVING_SYMS(0);
      w.SetFreeLivingSyms(false);

      emp::Ptr<Organism> host = emp::NewPtr<Host>(&random, &w, &config, int_val);
      w.AddOrgAt(host, 0);
      emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(&random, &w, &config, int_val);

      THEN( "syms are injected into a random host" ){
        w.InjectSymbiont(sym);
        emp::vector<emp::Ptr<Organism>> host_syms = host->GetSymbionts();

        REQUIRE(host_syms.size() == 1);
        REQUIRE(host_syms.at(0) == sym);
      }
    }
    WHEN( "free living syms are allowed" ){
      world_size = 1000;
      w.Resize(world_size);
      config.FREE_LIVING_SYMS(1);
      w.SetFreeLivingSyms(true);


      THEN( "syms can be injected into a random empty cell" ){

        REQUIRE(w.GetNumOrgs() == 0);

        size_t sym_count = 100;

        for(size_t i = 0; i < sym_count; i++){
          w.InjectSymbiont(emp::NewPtr<Symbiont>(&random, &w, &config, int_val));
        }
        //since spot of injection is random, a few symbionts
        //will get overwritten, and thus # injected != # remaining in world
        REQUIRE(w.GetNumOrgs() < (sym_count + 1));
        REQUIRE(w.GetNumOrgs() > (sym_count - 10));
      }
    }
  }
}

TEST_CASE( "DoBirth", "[default]" ){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    SymWorld w(random);
    int world_size = 4;
    w.Resize(world_size);
    w.SetFreeLivingSyms(true);
    emp::Ptr<Organism> h2 = emp::NewPtr<Host>(&random, &w, &config, int_val);
    w.AddOrgAt(h2, 3);
    emp::Ptr<Organism> host = emp::NewPtr<Host>(&random, &w, &config, int_val);

    WHEN( "born into an empty spot" ){
      THEN( "occupies that spot" ){
        w.DoBirth(host, 2);

        REQUIRE(w.GetNumOrgs() == 2);
        bool host_isborn = false;
        for(size_t i = 0; i < 4; i++){
          if(&w.GetOrg(i) == host) {
            host_isborn = true;
            break;
          }
        }
        REQUIRE(host_isborn == true);
      }
    }
    WHEN( "born into a spot occupied by another host" ){
      THEN( "kills that host and replaces it" ){
        emp::Ptr<Organism> other_host = emp::NewPtr<Host>(&random, &w, &config, int_val);
        w.AddOrgAt(other_host, 0);
        w.DoBirth(host, 2);

        REQUIRE(w.GetNumOrgs() == 2);

        bool host_isborn = false;
        bool otherhost_isdead = true;
        for(size_t i = 0; i < 4; i++){
          if(w.GetPop()[i] == host) {
            host_isborn = true;
          } else if (w.GetPop()[i] != nullptr && w.GetPop()[i] != h2){
            otherhost_isdead = false;
          }
        }
        REQUIRE(w.GetNumOrgs() == 2);
        REQUIRE(host_isborn == true);
        REQUIRE(otherhost_isdead == true);
      }
    }
  }
}

TEST_CASE( "SymDoBirth", "[default]" ) {
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    SymWorld w(random);
    size_t world_size = 4;
    w.Resize(world_size);

    WHEN( "free living symbionts are not allowed" ) {
      config.FREE_LIVING_SYMS(0);
      w.SetFreeLivingSyms(false);

      WHEN( "there is a valid neighbouring host" ){
        emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &w, &config, int_val);
        w.AddOrgAt(host, 0);

        emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(&random, &w, &config, int_val);
        w.SymDoBirth(sym, 1);

        emp::vector<emp::Ptr<Organism>> syms = host->GetSymbionts();
        emp::Ptr<Organism> host_sym = syms[0];

        THEN( "the sym is inserted into the valid neighbouring host" ){
          REQUIRE(host_sym == sym);
          REQUIRE(w.GetNumOrgs() == 1);
        }
      }

      WHEN( "there is no valid neighbouring host" ){
        emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(&random, &w, &config, int_val);
        w.SymDoBirth(sym, 1);

        THEN( "the sym is killed" ){
          //the world should be empty
          REQUIRE(w.GetNumOrgs() == 0);
        }
      }
    }


    WHEN( "free living symbionts are allowed"){
      config.FREE_LIVING_SYMS(1);
      w.SetFreeLivingSyms(true);

      THEN("it might be inserted into an empty cell"){
        w.SymDoBirth(emp::NewPtr<Symbiont>(&random, &w, &config, int_val), emp::WorldPosition(0, 1));
        REQUIRE(w.GetNumOrgs() == 1);
      }

      THEN("it may be inserted into an occupied cell, overwriting the previous occupant"){
        world_size = 2;
        w.Resize(world_size);

        for(size_t i = 0; i < world_size; i++){
          w.AddOrgAt(emp::NewPtr<Symbiont>(&random, &w, &config, int_val), emp::WorldPosition(0, i));
        }
        REQUIRE(w.GetNumOrgs() == world_size);

        emp::WorldPosition parent_pos = emp::WorldPosition(0, 1);
        emp::Ptr<Organism> new_symbiont = emp::NewPtr<Symbiont>(&random, &w, &config, int_val);
        w.SymDoBirth(new_symbiont, parent_pos);

        bool new_sym_born = false;
        for(size_t i = 0; i < world_size; i++){
          if(w.GetSymAt(i) == new_symbiont){
            new_sym_born = true;
          }
        }

        REQUIRE(w.GetNumOrgs() == world_size);
        REQUIRE(new_sym_born == true);
      }
    }
  }
}

TEST_CASE( "Update", "[default]" ){
  GIVEN("a world"){
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    SymWorld w(random);
    int world_size = 4;
    w.Resize(world_size);
    int res_per_update = 10;
    config.RES_DISTRIBUTE(res_per_update);
    int num_updates = 5;

    emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &w, &config, int_val);

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
        int res_before_update = host->GetPoints();
        w.Update();
        int res_after_update = host->GetPoints();
        int res_change = res_after_update - res_before_update;
        REQUIRE(res_per_update == res_change);
      }
    }


    WHEN("free living syms are allowed"){
      res_per_update = 80;
      config.RES_DISTRIBUTE(res_per_update);
      config.FREE_SYM_RES_DISTRIBUTE(res_per_update);
      world_size = 16;
      w.Resize(world_size);
      w.SetFreeLivingSyms(1);
      config.FREE_LIVING_SYMS(1);
      config.MOVE_FREE_SYMS(1);

      WHEN("there are no syms in the world"){
        THEN("hosts process normally"){
          w.AddOrgAt(host, 0);
          int orig_points = host->GetPoints();
          w.Update();

          REQUIRE(host->GetPoints() - orig_points == res_per_update);
        }
      }

      config.LYSIS(0);
      config.HORIZ_TRANS(1);
      world_size = 9;
      w.Resize(world_size);

      THEN("if only syms in the world they can get resources and reproduce"){
        emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(&random, &w, &config, int_val);
        w.SymDoBirth(sym, 0);
        REQUIRE(w.GetNumOrgs() == 1);

        for(int i = 0; i < num_updates; i++){
          w.Update();
        }
        //the sym has reproduced at least once
        REQUIRE(w.GetNumOrgs() > 1);

        int num_pop_elements = 0;
        for(int i = 0; i < world_size; i++){
          if(w.GetPop()[i]){
            num_pop_elements++;
          }
        }
        REQUIRE(num_pop_elements == 0);

        host.Delete(); //since it wasn't added to the world, have to delete it manually
      }
      THEN("hosts and syms can mingle in the environment"){
        w.AddOrgAt(host, 0);
        w.AddOrgAt(emp::NewPtr<Symbiont>(&random, &w, &config, int_val), emp::WorldPosition(0,1));
        for(int i = 0; i < num_updates; i++){
          w.Update();
        }

        //the organisms have done something
        REQUIRE(w.GetNumOrgs() > 2);

        int free_sym_count = 0;
        int hosted_sym_count = 0;
        int host_count = 0;
        for(int i = 0; i < world_size; i++){
          if(w.GetPop()[i]){
            host_count++;
            hosted_sym_count += w.GetOrg(i).GetSymbionts().size();
          }
          if(w.GetSymPop()[i]){
            free_sym_count++;
          }
        }
        //there should be at least one free sym, hosted sym, and host
        REQUIRE(free_sym_count > 0);
        REQUIRE(hosted_sym_count > 0);
        REQUIRE(host_count > 0);
      }
    }
  }
}

TEST_CASE( "MoveFreeSym", "[default]" ){
  GIVEN("free living syms are allowed"){
    emp::Random random(14);
    SymConfigBase config;
    SymWorld w(random);
    w.SetFreeLivingSyms(true);
    int int_val = 0;
    int world_size = 4;
    w.Resize(world_size);
    size_t host_pos = 0;
    emp::WorldPosition sym_pos = emp::WorldPosition(0, host_pos);

    emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(&random, &w, &config, int_val);
    w.AddOrgAt(sym, sym_pos);
    WHEN("there is a parallel host and the sym wants to infect"){
      emp::Ptr<Organism> host = emp::NewPtr<Host>(&random, &w, &config, int_val);
      w.AddOrgAt(host, host_pos);
      REQUIRE(w.GetNumOrgs() == 2);
      REQUIRE(host->HasSym() == false);

      WHEN("the infection fails"){
        config.SYM_INFECTION_FAILURE_RATE(1);
        emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(&random, &w, &config, int_val);
        w.AddOrgAt(sym, sym_pos);
        REQUIRE(w.GetNumOrgs() == 2);
        THEN("the sym is deleted"){
          w.MoveFreeSym(sym_pos);
          REQUIRE(w.GetNumOrgs() == 1);
          REQUIRE(!host->HasSym());
        }
      }
      WHEN("the infection does not fail"){
        THEN("the sym moves into the host"){
          w.MoveFreeSym(sym_pos);
          REQUIRE(w.GetNumOrgs() == 1);
          REQUIRE(host->HasSym());
          REQUIRE(host->GetSymbionts()[0] == sym);
        }
      }
    }
    WHEN("the sym does not want to/can't infect a parallel host"){
      size_t sym_id = 0;
      WHEN("moving is turned on"){
        w.SetMoveFreeSyms(1);
        sym->SetInfectionChance(0);
        THEN("the sym moves to a random spot in the free world"){
          REQUIRE(w.GetSymPop()[sym_id] == sym); //there should be a sym at pos 0
          w.MoveFreeSym(sym_pos);

          size_t new_sym_id = 2;
          emp::Ptr<Organism> new_sym = w.GetSymPop()[new_sym_id];
          REQUIRE(sym == new_sym);
          REQUIRE(w.GetSymPop()[sym_id] == nullptr);
        }
      }
      WHEN("moving is turned off"){
        w.SetMoveFreeSyms(0);
        THEN("the sym doesn't move"){
          REQUIRE(w.GetSymPop()[sym_id] == sym);
          w.MoveFreeSym(sym_pos);
          emp::Ptr<Organism> new_sym = w.GetSymPop()[sym_id];
          REQUIRE(sym == new_sym);
        }
      }
    }
  }
}

TEST_CASE( "ExtractSym", "[default]" ){
  GIVEN("a world"){
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    SymWorld w(random);
    int world_size = 4;
    w.Resize(world_size);
    size_t sym_index = 1;
    emp::WorldPosition sym_pos = emp::WorldPosition(0, sym_index);
    emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(&random, &w, &config, int_val);

    w.AddOrgAt(sym, sym_pos);
    REQUIRE(w.GetSymPop()[sym_index] == sym);
    REQUIRE(w.GetNumOrgs() == 1);

    emp::Ptr<Organism> new_org = w.ExtractSym(sym_index);
    REQUIRE(sym == new_org);
    REQUIRE(w.GetNumOrgs() == 0);
    REQUIRE(w.GetSymPop()[sym_index] == nullptr);

    sym.Delete();
  }
}

TEST_CASE( "MoveIntoNewFreeWorldPos", "[default]" ){
  GIVEN("free living syms are allowed"){
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    SymWorld w(random);
    int world_size = 4;
    w.Resize(world_size);

    emp::Ptr<Organism> parent_sym = emp::NewPtr<Symbiont>(&random, &w, &config, int_val);
    emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(&random, &w, &config, int_val);

    size_t orig_pos = 3;
    emp::WorldPosition orig_sym_pos = emp::WorldPosition(0, orig_pos);
    w.AddOrgAt(parent_sym, orig_sym_pos);

    REQUIRE(w.GetNumOrgs() == 1);

    w.MoveIntoNewFreeWorldPos(sym, orig_sym_pos);
    REQUIRE(w.GetNumOrgs() == 2);
    REQUIRE(w.GetSymPop()[orig_pos] == parent_sym);

    bool sym_exists = false;
    for(int i = 0; i < 2; i++) {
      if (w.GetSymPop()[i] == sym) {
        sym_exists = true;
        break;
      }
    }
    REQUIRE(sym_exists == true);
    //TODO: CHECK MOVING OUT OF HOST
  }
}

TEST_CASE( "Resize", "[default]" ){
  GIVEN("a world"){
    emp::Random random(17);
    SymWorld w(random);

    size_t pop_size = w.GetPop().size();
    size_t sym_pop_size = w.GetSymPop().size();
    REQUIRE(pop_size == sym_pop_size);
    REQUIRE(pop_size == 0);

    size_t width = 3;
    size_t height = 3;
    w.Resize(width, height);
    pop_size = w.GetPop().size();
    sym_pop_size = w.GetSymPop().size();
    REQUIRE(pop_size == sym_pop_size);
    REQUIRE(pop_size == (width * height));

    size_t world_size = 11;
    w.Resize(world_size);
    pop_size = w.GetPop().size();
    sym_pop_size = w.GetSymPop().size();
    REQUIRE(pop_size == sym_pop_size);
    REQUIRE(pop_size == world_size);
  }
}

TEST_CASE( "AddOrgAt", "[default]" ){
  //adding hosts to the world should be covered by Empirical tests,
  //so here we'll test adding a sym
  GIVEN("a world"){
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    SymWorld w(random);
    int world_size = 4;
    w.Resize(world_size);
    emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(&random, &w, &config, int_val);

    WHEN("a sym is added into an empty spot"){
      THEN("it occupies that spot"){
        REQUIRE(w.GetNumOrgs() == 0);
        REQUIRE(w.GetSymPop()[0] == nullptr);
        w.AddOrgAt(sym, 0);
        REQUIRE(w.GetNumOrgs() == 1);
        REQUIRE(w.GetSymPop()[0] == sym);
      }
    }
    WHEN("a sym is added into an occupied spot"){
      THEN("it replaces the occupying sym"){
        emp::Ptr<Organism> old_sym = emp::NewPtr<Symbiont>(&random, &w, &config, int_val);
        w.AddOrgAt(old_sym, 0);
        REQUIRE(w.GetNumOrgs() == 1);
        REQUIRE(w.GetSymPop()[0] == old_sym);

        w.AddOrgAt(sym, 0);
        REQUIRE(w.GetNumOrgs() == 1);
        REQUIRE(w.GetSymPop()[0] == sym);
      }
    }
    WHEN("a sym is added to an out of bounds pos"){
      THEN("pop and sym_pop are expanded to fit it"){
        REQUIRE(w.GetSymPop().size() == 4);
        w.AddOrgAt(sym, emp::WorldPosition(0,7));
        REQUIRE(w.GetSymPop().size() == w.GetPop().size());
        REQUIRE(w.GetSymPop().size() == 8);
      }
    }
  }
}

TEST_CASE( "GetSymAt", "[default]" ){
  GIVEN("a world"){
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    int world_size = 4;
    SymWorld w(random);
    w.Resize(world_size);


    WHEN("a request is made for an in-bounds sym"){
      emp::Ptr<Organism> sym1 = emp::NewPtr<Symbiont>(&random, &w, &config, int_val);
      emp::Ptr<Organism> sym2 = emp::NewPtr<Symbiont>(&random, &w, &config, int_val);
      w.AddOrgAt(sym1, 0);
      w.AddOrgAt(sym2, emp::WorldPosition(0,1));
      THEN("the sym at that position is returned"){
        REQUIRE(w.GetSymAt(0) == sym1);
        REQUIRE(w.GetSymAt(1) == sym2);
        REQUIRE(w.GetSymAt(2) == nullptr);
      }
    }
    WHEN("a request is made for an out-of-bounds sym"){
      THEN("an exception is thrown"){
        REQUIRE_THROWS(w.GetSymAt(4));
      }
    }
  }
}

TEST_CASE( "DoSymDeath", "[default]" ){
  GIVEN("a world"){
    emp::Random random(17);
    SymConfigBase config;
    SymWorld w(random);
    int world_size = 4;
    w.Resize(world_size);
    emp::Ptr<Organism> s = emp::NewPtr<Symbiont>(&random, &w, &config, 1);
    size_t sym_position = 1;
    w.AddOrgAt(s, emp::WorldPosition(0, sym_position));

    WHEN("A sym is deleted from a position"){
      THEN("It is no longer included in the count of organisms in the world"){
        REQUIRE(w.GetNumOrgs() == 1);
        w.DoSymDeath(sym_position);
        REQUIRE(w.GetNumOrgs() == 0);
      }
      THEN("It no longer occupies a spot in the world"){
        emp::Ptr<Organism> world_sym = w.GetSymAt(sym_position);
        REQUIRE(world_sym == s);
        w.DoSymDeath(sym_position);

        emp::Ptr<Organism> world_sym_deleted = w.GetSymAt(sym_position);
        REQUIRE(world_sym_deleted == nullptr);
      }
    }
  }
}

TEST_CASE( "Host Phylogeny", "[default]" ){
  emp::Random random(17);
  SymConfigBase config;
  config.MUTATION_SIZE(0.09);
  config.MUTATION_RATE(1);
  config.PHYLOGENY(1);
  int int_val = 0;
  SymWorld w(random);
  int world_size = 4;
  w.Resize(world_size);
  w.SetTrackPhylogeny(1);
  w.SetNumPhyloBins(20);


  emp::Ptr<Organism> host = emp::NewPtr<Host>(&random, &w, &config, int_val);
  emp::Ptr<emp::Systematics<Organism,int>> host_sys = w.GetHostSys();

  //ORGANISMS ADDED TO SYSTEMATICS
  WHEN("an organism is added to the world"){
    WHEN("the cell it's added to is occupied"){
      size_t pos = 0;
      int_val = -1;

      emp::Ptr<Organism> occupying_host = emp::NewPtr<Host>(&random, &w, &config, int_val);
      w.AddOrgAt(occupying_host, pos);
      size_t expected_occupying_taxon_info = 0;
      size_t taxon_info = host_sys->GetTaxonAt(pos)->GetInfo();

      REQUIRE(w.GetNumOrgs() == 1);
      REQUIRE(host_sys->GetNumActive() == 1);
      REQUIRE(expected_occupying_taxon_info == taxon_info);

      w.AddOrgAt(host, pos);
      taxon_info = host_sys->GetTaxonAt(pos)->GetInfo();
      size_t expected_taxon_info = 10;

      THEN("the occupying organism is removed from the systematic"){
        REQUIRE(w.GetNumOrgs() == 1);
        REQUIRE(host_sys->GetNumActive() == 1);
        REQUIRE(expected_taxon_info == taxon_info);
      }
    }

    WHEN("the cell it's added to is empty"){
      size_t pos = 0;

      REQUIRE(w.GetNumOrgs() == 0);
      REQUIRE(host_sys->GetNumActive() == 0);

      w.AddOrgAt(host, pos);
      size_t taxon_info = host_sys->GetTaxonAt(pos)->GetInfo();
      size_t expected_taxon_info = 10;

      THEN("the organism is tracked by the systematic"){
        REQUIRE(w.GetNumOrgs() == 1);
        REQUIRE(host_sys->GetNumActive() == 1);
        REQUIRE(expected_taxon_info == taxon_info);
      }
    }


    //ORGANISMS ADDED TO TAXA
    THEN("the organsim is added to the correct taxon"){
      size_t pos = 0;

      //taxon info 0
      double int_val = -1;
      int expected_taxon_info = 0;
      w.AddOrgAt(emp::NewPtr<Host>(&random, &w, &config, int_val), pos);
      int taxon_info = host_sys->GetTaxonAt(pos)->GetInfo();
      REQUIRE(expected_taxon_info == taxon_info);

      //taxon info 0
      int_val = -0.98;
      expected_taxon_info = 0;
      w.AddOrgAt(emp::NewPtr<Host>(&random, &w, &config, int_val), pos);
      taxon_info = host_sys->GetTaxonAt(pos)->GetInfo();
      REQUIRE(expected_taxon_info == taxon_info);

      //taxon info 1
      int_val = -0.9;
      expected_taxon_info = 1;
      w.AddOrgAt(emp::NewPtr<Host>(&random, &w, &config, int_val), pos);
      taxon_info = host_sys->GetTaxonAt(pos)->GetInfo();
      REQUIRE(expected_taxon_info == taxon_info);

      //taxon info 2
      int_val = -0.8;
      expected_taxon_info = 2;
      w.AddOrgAt(emp::NewPtr<Host>(&random, &w, &config, int_val), pos);
      taxon_info = host_sys->GetTaxonAt(pos)->GetInfo();
      REQUIRE(expected_taxon_info == taxon_info);

      //taxon info 16
      int_val = 0.65 ;
      expected_taxon_info = 16;
      w.AddOrgAt(emp::NewPtr<Host>(&random, &w, &config, int_val), pos);
      taxon_info = host_sys->GetTaxonAt(pos)->GetInfo();
      REQUIRE(expected_taxon_info == taxon_info);

      //taxon info 19
      int_val = 0.9;
      expected_taxon_info = 19;
      w.AddOrgAt(emp::NewPtr<Host>(&random, &w, &config, int_val), pos);
      taxon_info = host_sys->GetTaxonAt(pos)->GetInfo();
      REQUIRE(expected_taxon_info == taxon_info);

      //taxon info 19
      int_val = 1;
      expected_taxon_info = 19;
      w.AddOrgAt(emp::NewPtr<Host>(&random, &w, &config, int_val), pos);
      taxon_info = host_sys->GetTaxonAt(pos)->GetInfo();
      REQUIRE(expected_taxon_info == taxon_info);

      //true even if the number of bins changes!
      w.SetNumPhyloBins(2);
      //taxon info 1
      int_val = 1;
      expected_taxon_info = 1;
      w.AddOrgAt(emp::NewPtr<Host>(&random, &w, &config, int_val), pos);
      taxon_info = host_sys->GetTaxonAt(pos)->GetInfo();
      REQUIRE(expected_taxon_info == taxon_info);

      //taxon info 1
      int_val = 0;
      expected_taxon_info = 1;
      w.AddOrgAt(emp::NewPtr<Host>(&random, &w, &config, int_val), pos);
      taxon_info = host_sys->GetTaxonAt(pos)->GetInfo();
      REQUIRE(expected_taxon_info == taxon_info);

      //taxon info 1
      int_val = -0.001;
      expected_taxon_info = 0;
      w.AddOrgAt(emp::NewPtr<Host>(&random, &w, &config, int_val), pos);
      taxon_info = host_sys->GetTaxonAt(pos)->GetInfo();
      REQUIRE(expected_taxon_info == taxon_info);

      host.Delete();
    }
  }

  //ORGANISMS AND RELATIONSHIPS TRACKED
  WHEN("Several generations pass"){
    THEN("The phylogenetic relationships are tracked and accurate"){
      world_size = 10;
      w.Resize(world_size);
      int num_descendants = 4;
      //add the first host
      w.AddOrgAt(host, 0);

      //populate the world with descendents with various interaction values
      //Can't use num_descendants for the following array sizes because some
      //compilers don't allow it
      double int_vals[4] = {0.1, -0.05, -0.2, 0.14};
      //bins: parent org in 10, then in int_vals order: 11, 9, 8, 11
      size_t parents[4] = {0, 1, 1, 3};
      for(int i = 0; i < num_descendants; i++){
        w.AddOrgAt(emp::NewPtr<Host>(&random, &w, &config, int_vals[i]), (i+1), parents[i]);
      }

      char lineages[][30] = {"Lineage:\n10\n",
                             "Lineage:\n11\n10\n",
                             "Lineage:\n9\n11\n10\n",
                             "Lineage:\n8\n11\n10\n",
                             "Lineage:\n11\n8\n11\n10\n",
                           };


      for(int i = 0; i < (num_descendants+1); i++){
        std::stringstream result;
        host_sys->PrintLineage(host_sys->GetTaxonAt(i), result);
        REQUIRE(result.str() == lineages[i]);
      }
    }
  }
}

TEST_CASE( "Symbiont Phylogeny", "[default]" ){
  emp::Random random(17);
  SymConfigBase config;
  config.MUTATION_SIZE(0.09);
  config.MUTATION_RATE(1);
  config.FREE_LIVING_SYMS(1);
  config.PHYLOGENY(1);
  int int_val = 0;
  SymWorld w(random);
  int world_size = 20;
  w.Resize(world_size);
  w.SetTrackPhylogeny(1);
  w.SetFreeLivingSyms(1);
  w.SetNumPhyloBins(20);

  emp::Ptr<emp::Systematics<Organism,int>> sym_sys = w.GetSymSys();

  WHEN("symbionts are added to the world"){
    THEN("they get added to the correct taxonomic bins"){
      REQUIRE(sym_sys->GetNumActive() == 0);
      size_t count = 8;
      //Can't use count for the following array sizes because some
      //compilers don't allow it
      double int_vals[8] = {-1, -0.9, -0.82, 0, 0.5, 0.65, 0.9, 1};
      int taxon_infos[8] = {0, 1, 1, 10, 15, 16, 19, 19};

      emp::Ptr<Organism> syms[count];
      emp::Ptr<Organism> sym;

      for(size_t i = 0; i < count; i++){
        sym = emp::NewPtr<Symbiont>(&random, &w, &config, int_vals[i]);
        w.InjectSymbiont(sym);
        REQUIRE(sym->GetTaxon()->GetInfo() == taxon_infos[i]);
      }
    }
  }

  WHEN("symbionts are deleted"){
    THEN("they are no longer tracked by the sym systematic"){
      world_size = 1;
      w.Resize(world_size);
      REQUIRE(sym_sys->GetNumActive() == 0);
      emp::Ptr<Organism> symbiont = emp::NewPtr<Symbiont>(&random, &w, &config, int_val);
      w.InjectSymbiont(symbiont);
      REQUIRE(sym_sys->GetNumActive() == 1);
      w.DoSymDeath(0);
      REQUIRE(sym_sys->GetNumActive() == 0);
    }
  }

  WHEN("generations pass"){
    config.MUTATION_SIZE(1);
    config.MUTATION_RATE(1);
    config.PHYLOGENY(1);
    size_t num_syms = 4;

    emp::Ptr<Organism> syms[num_syms];
    syms[0] = emp::NewPtr<Symbiont>(&random, &w, &config, 0);
    w.AddSymToSystematic(syms[0]);

    for(size_t i = 1; i < num_syms; i++){
      syms[i] = syms[i-1]->Reproduce();
    }

    THEN("Their lineages are tracked"){
      char lineages[][30] = {"Lineage:\n10\n",
                             "Lineage:\n16\n10\n",
                             "Lineage:\n19\n16\n10\n",
                             "Lineage:\n16\n19\n16\n10\n",
                           };

      for(size_t i = 0; i < num_syms; i++){
        std::stringstream result;
        sym_sys->PrintLineage(syms[i]->GetTaxon(), result);
        REQUIRE(result.str() == lineages[i]);
      }
      syms[0].Delete();
      syms[1].Delete();
    }

    THEN("Their birth and destruction dates are tracked"){
      //all curr syms should have orig times of 0
      for(size_t i = 0; i < num_syms; i++){
        REQUIRE(syms[i]->GetTaxon()->GetOriginationTime() == 0);
      }
      w.Update();

      //after update, times should now be 1
      emp::Ptr<emp::Taxon<int>> dest_tax = syms[0]->GetTaxon();
      syms[0].Delete();
      REQUIRE(dest_tax->GetDestructionTime() == 1);

      //another update, times 2
      w.Update();
      dest_tax = syms[1]->GetTaxon();
      syms[1].Delete();
      REQUIRE(dest_tax->GetDestructionTime() == 2);
    }

    syms[2].Delete();
    syms[3].Delete();
  }
}
