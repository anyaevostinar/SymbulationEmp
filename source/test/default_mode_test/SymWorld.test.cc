#include "../../default_mode/SymWorld.h"
#include "../../default_mode/Symbiont.h"
#include "../../lysis_mode/Phage.h"
#include "../../lysis_mode/LysisWorld.h"
#include "../../default_mode/Host.h"


TEST_CASE("PullResources", "[default]") {
  GIVEN(" a world ") {
    emp::Random random(19);
    SymConfigBase config;
    SymWorld world(random, &config);
    int full_share = 100;

    WHEN(" the resources are unlimited ") {
      config.LIMITED_RES_TOTAL(-1);

      THEN(" organisms get as many resources as they request ") {
        REQUIRE(world.PullResources(full_share) == full_share);
      }
    }

    WHEN( " the resources are limited ") {
      int original_total = 150;
      config.LIMITED_RES_TOTAL(original_total);
      SymWorld world(random, &config);

      THEN(" first organism gets full share of resources, next host gets a bit, everyone else gets nothing ") {
        REQUIRE(world.PullResources(full_share) == full_share);
        REQUIRE(world.PullResources(full_share) == (original_total-full_share));
        REQUIRE(world.PullResources(full_share) == 0);
        REQUIRE(world.PullResources(full_share) == 0);
      }
    }
  }
}

TEST_CASE("Limited resources inflow", "[default]") {
  GIVEN(" a world ") {
    emp::Random random(11);
    SymConfigBase config;
    int full_share = 100;

    WHEN( " the resources are limited and inflow is zero ") {
      int original_total = 150;
      config.LIMITED_RES_TOTAL(original_total);
      config.LIMITED_RES_INFLOW(0);
      SymWorld world(random, &config);
      // Update() calls GetPermutation() which crashes if the size is zero
      world.Resize(1);

      THEN(" first organism gets full share of resources, next host gets a bit, everyone else gets nothing ") {
        REQUIRE(world.PullResources(full_share) == full_share);
        REQUIRE(world.PullResources(full_share) == (original_total-full_share));
        REQUIRE(world.PullResources(full_share) == 0);
        REQUIRE(world.PullResources(full_share) == 0);
        AND_WHEN(" the world is updated ") {
          world.Update();
          THEN(" no organisms get points since the total is still zero ") {
            REQUIRE(world.PullResources(full_share) == 0);
            REQUIRE(world.PullResources(full_share) == 0);
            REQUIRE(world.PullResources(full_share) == 0);
            REQUIRE(world.PullResources(full_share) == 0);
          }
        }
      }
    }

    WHEN( " the resources are limited and inflow is turned on ") {
      int original_total = 150;
      int inflow = 25;
      config.LIMITED_RES_TOTAL(original_total);
      config.LIMITED_RES_INFLOW(inflow);
      SymWorld world(random, &config);
      // Update() calls GetPermutation() which crashes if the size is zero
      world.Resize(1);

      THEN(" first organism gets full share of resources, next host gets a bit, everyone else gets nothing ") {
        REQUIRE(world.PullResources(full_share) == full_share);
        REQUIRE(world.PullResources(full_share) == (original_total-full_share));
        REQUIRE(world.PullResources(full_share) == 0);
        REQUIRE(world.PullResources(full_share) == 0);
        AND_WHEN(" the world is updated ") {
          world.Update();
          THEN(" the first organism gets some new resources, everyone else gets nothing ") {
            REQUIRE(world.PullResources(full_share) == inflow);
            REQUIRE(world.PullResources(full_share) == 0);
            REQUIRE(world.PullResources(full_share) == 0);
            REQUIRE(world.PullResources(full_share) == 0);
          }
        }
      }
    }
  }
}

TEST_CASE( "Vertical Transmission", "[default]" ) {
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    SymWorld world(random, &config);

    WHEN( "the vertical taransmission rate is 0" ) {
      config.VERTICAL_TRANSMISSION(0);

      THEN( "there is never vertical transmission" ) {
        REQUIRE( world.WillTransmit() == false );
        REQUIRE( world.WillTransmit() == false );
        REQUIRE( world.WillTransmit() == false );
        REQUIRE( world.WillTransmit() == false );
        REQUIRE( world.WillTransmit() == false );
      }
    }

    WHEN( "the vertical taransmission rate is 1" ) {
      config.VERTICAL_TRANSMISSION(1);

      THEN( "there is always vertical transmission" ) {
        REQUIRE( world.WillTransmit() == true );
        REQUIRE( world.WillTransmit() == true );
        REQUIRE( world.WillTransmit() == true );
        REQUIRE( world.WillTransmit() == true );
        REQUIRE( world.WillTransmit() == true );
      }
    }

    WHEN( "the vertical taransmission rate is .5" ) {
      config.VERTICAL_TRANSMISSION(.5);

      THEN( "there is sometimes vertical transmission" ) {
        bool yes = false;
        bool no = false;
        for(int i = 0; i < 128; i++)//Odds of failure should be 1 in 170141183460469231731687303715884105728
          if(world.WillTransmit())
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

    SymWorld world(random, &config);

    WHEN( "hosts are added" ) {

      int n = 7532;

      //inject organisms
      for (int i = 0; i < n; i++){
        emp::Ptr<Host> new_org;
        new_org.New(&random, &world, &config, 0);
        world.AddOrgAt(new_org, world.size());
      }

      THEN( "the world's size becomes the number of hosts that were added" ) {
        REQUIRE( (int) world.GetPop().size() == n );
      }
    }
  }
}

TEST_CASE( "Interaction Patterns", "[default]" ) {
  SymConfigBase config;

  GIVEN( "a world without vertical transmission" ) {
    emp::Ptr<emp::Random> random = new emp::Random(17);
    SymWorld world(*random, &config);
    config.VERTICAL_TRANSMISSION(0);
    config.VERTICAL_TRANSMISSION(0);
    config.MUTATION_SIZE(0);
    config.SYM_LIMIT(500);
    config.HORIZ_TRANS(true);
    config.HOST_REPRO_RES(400);
    config.RES_DISTRIBUTE(100);
    config.SYNERGY(5);

    WHEN( "hostile hosts meet generous symbionts" ) {

      //inject organisms
      for (size_t i = 0; i < 10; i++){
        emp::Ptr<Host> new_org = emp::NewPtr<Host>(random, &world, &config, -0.1);
        world.AddOrgAt(new_org, world.size());
      }
      int width = 10;
      int height = 1;
      int world_size = width * height;
      world.Resize(width, height);
      for (int i = 0; i < world_size; i++){
        emp::Ptr<Symbiont> new_sym = emp::NewPtr<Symbiont>(random, &world, &config, 0.1);
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




  GIVEN( "a world" ) {
    emp::Random random(17);
    SymWorld world(random, &config);
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

    WHEN( "very generous hosts meet many very hostile symbionts" ) {

      //inject organisms
      for (size_t i = 0; i < 200; i++){
        emp::Ptr<Host> new_org;
        new_org.New(&random, &world, &config, 1);
        world.AddOrgAt(new_org, world.size());
      }

      int width = 100;
      int height = 200;
      world.Resize(width, height);

      for (size_t i = 0; i < 10000; i++){
        emp::Ptr<Symbiont> new_sym;
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

TEST_CASE( "Hosts injected correctly", "[default]" ) {
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    SymWorld world(random, &config);

    WHEN( "host added with interaction value 1" ) {
      //inject organism
      emp::Ptr<Host> new_org1;
      new_org1.New(&random, &world, &config, 1);
      world.AddOrgAt(new_org1, 0);

      THEN( "host has interaction value of 1" ) {
        REQUIRE( world.GetOrg(0).GetIntVal() == 1 );
      }
    }
    WHEN( "host added with interaction value -1" ) {
      //inject organism
      emp::Ptr<Host> new_org1;
      new_org1.New(&random, &world, &config, -1);
      world.AddOrgAt(new_org1, 0);

      THEN( "host has interaction value of -1" ) {
        REQUIRE( world.GetOrg(0).GetIntVal() == -1 );
      }
    }
    WHEN( "host added with interaction value 0" ) {
      //inject organism
      emp::Ptr<Host> new_org1;
      new_org1.New(&random, &world, &config, 0);
      world.AddOrgAt(new_org1, 0);

      THEN( "host has interaction value of 0" ) {
        REQUIRE( world.GetOrg(0).GetIntVal() == 0 );
      }
    }
  }
}

TEST_CASE( "InjectSymbiont", "[default]" ){
  GIVEN( "a world" ){
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    SymWorld world(random, &config);
    int world_size = 4;

    WHEN( "free living syms are not allowed" ){
      world.Resize(world_size);
      config.FREE_LIVING_SYMS(0);

      emp::Ptr<Organism> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      world.AddOrgAt(host, 0);
      emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);

      THEN( "syms are injected into a random host" ){
        world.InjectSymbiont(sym);
        emp::vector<emp::Ptr<Organism>> host_syms = host->GetSymbionts();

        REQUIRE(host_syms.size() == 1);
        REQUIRE(host_syms.at(0) == sym);
      }
    }
    WHEN( "free living syms are allowed" ){
      world_size = 1000;
      world.Resize(world_size);
      config.FREE_LIVING_SYMS(1);


      THEN( "syms can be injected into a random empty cell" ){

        REQUIRE(world.GetNumOrgs() == 0);

        size_t sym_count = 100;

        for(size_t i = 0; i < sym_count; i++){
          world.InjectSymbiont(emp::NewPtr<Symbiont>(&random, &world, &config, int_val));
        }
        //since spot of injection is random, a few symbionts
        //will get overwritten, and thus # injected != # remaining in world
        REQUIRE(world.GetNumOrgs() < (sym_count + 1));
        REQUIRE(world.GetNumOrgs() > (sym_count - 10));
      }
    }
  }
}

TEST_CASE( "DoBirth", "[default]" ){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    SymWorld world(random, &config);
    int world_size = 4;
    world.Resize(world_size);
    config.FREE_LIVING_SYMS(1);
    emp::Ptr<Organism> h2 = emp::NewPtr<Host>(&random, &world, &config, int_val);
    world.AddOrgAt(h2, 3);
    emp::Ptr<Organism> host = emp::NewPtr<Host>(&random, &world, &config, int_val);

    WHEN( "born into an empty spot" ){
      THEN( "occupies that spot" ){
        world.DoBirth(host, 2);

        REQUIRE(world.GetNumOrgs() == 2);
        bool host_isborn = false;
        for(size_t i = 0; i < 4; i++){
          if(&world.GetOrg(i) == host) {
            host_isborn = true;
            break;
          }
        }
        REQUIRE(host_isborn == true);
      }
    }
    WHEN( "born into a spot occupied by another host" ){
      THEN( "kills that host and replaces it" ){
        emp::Ptr<Organism> other_host = emp::NewPtr<Host>(&random, &world, &config, int_val);
        world.AddOrgAt(other_host, 0);
        world.DoBirth(host, 2);

        REQUIRE(world.GetNumOrgs() == 2);

        bool host_isborn = false;
        bool otherhost_isdead = true;
        for(size_t i = 0; i < 4; i++){
          if(world.GetPop()[i] == host) {
            host_isborn = true;
          } else if (world.GetPop()[i] != nullptr && world.GetPop()[i] != h2){
            otherhost_isdead = false;
          }
        }
        REQUIRE(world.GetNumOrgs() == 2);
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
    SymWorld world(random, &config);
    size_t world_size = 4;
    world.Resize(world_size);

    emp::WorldPosition new_pos;

    WHEN( "free living symbionts are not allowed" ) {
      config.FREE_LIVING_SYMS(0);

      WHEN( "there is a valid neighbouring host" ){
        size_t host_pos = 1;
        emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
        world.AddOrgAt(host, host_pos);

        emp::Ptr<Organism> new_symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
        new_pos = world.SymDoBirth(new_symbiont, 1);

        emp::vector<emp::Ptr<Organism>> syms = host->GetSymbionts();
        emp::Ptr<Organism> host_sym = syms[0];

        THEN( "the sym is inserted into the valid neighbouring host" ){
          REQUIRE(host_sym == new_symbiont);
          REQUIRE(world.GetNumOrgs() == 1);
          REQUIRE(new_pos.IsValid() == true);

          REQUIRE(new_pos.GetIndex() == 1);
          REQUIRE(new_pos.GetPopID() == host_pos);
        }
      }

      WHEN( "there is no valid neighbouring host" ){

        new_pos = world.SymDoBirth(emp::NewPtr<Symbiont>(&random, &world, &config, int_val), 1);

        THEN( "the sym is killed" ){
          //the world should be empty
          REQUIRE(world.GetNumOrgs() == 0);
          REQUIRE(new_pos.IsValid() == false);
        }
      }
    }


    WHEN( "free living symbionts are allowed"){
      config.FREE_LIVING_SYMS(1);
      world_size = 2;
      world.Resize(world_size);

      THEN("it might be inserted into an empty cell"){
        emp::WorldPosition parent_pos = emp::WorldPosition(0, 1);

        new_pos = world.SymDoBirth(emp::NewPtr<Symbiont>(&random, &world, &config, int_val), parent_pos);

        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(new_pos.IsValid() == true);
        REQUIRE(new_pos.GetIndex() == 0);
        REQUIRE(new_pos.GetPopID() == 0);
      }

      THEN("it may be inserted into an occupied cell, overwriting the previous occupant"){
        for(size_t i = 0; i < world_size; i++){
          world.AddOrgAt(emp::NewPtr<Symbiont>(&random, &world, &config, int_val), emp::WorldPosition(0, i));
        }
        REQUIRE(world.GetNumOrgs() == world_size);

        emp::WorldPosition parent_pos = emp::WorldPosition(0, 1);
        emp::Ptr<Organism> new_symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
        new_pos = world.SymDoBirth(new_symbiont, parent_pos);

        bool new_sym_born = false;
        for(size_t i = 0; i < world_size; i++){
          if(world.GetSymAt(i) == new_symbiont){
            new_sym_born = true;
          }
        }

        REQUIRE(world.GetNumOrgs() == world_size);
        REQUIRE(new_sym_born == true);
        REQUIRE(new_pos.IsValid() == true);
        REQUIRE(world.IsInboundsPos(new_pos) == true);
      }

      THEN("it might not find a valid cell and get deleted"){
        world_size = 0;
        world.Resize(0);
        emp::WorldPosition parent_pos = emp::WorldPosition(0, 1);
        emp::Ptr<Organism> new_symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
        new_pos = world.SymDoBirth(new_symbiont, parent_pos);

        REQUIRE(new_pos.IsValid() == false);
        REQUIRE(world.GetNumOrgs() == 0);
      }
    }
  }
}

TEST_CASE( "Update without free living symbionts", "[default]" ){
  GIVEN("a world"){
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    SymWorld world(random, &config);
    int world_size = 4;
    world.Resize(world_size);
    int res_per_update = 10;
    config.RES_DISTRIBUTE(res_per_update);

    emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);

    world.AddOrgAt(host, 0);

    WHEN("a host is dead"){
      THEN("it is removed from the world"){
        host->SetDead();
        REQUIRE(world.GetNumOrgs() == 1);

        world.Update();
        REQUIRE(world.GetNumOrgs() == 0);
      }
    }
    THEN("hosts process normally"){
      int res_before_update = host->GetPoints();
      world.Update();
      int res_after_update = host->GetPoints();
      int res_change = res_after_update - res_before_update;
      REQUIRE(res_per_update == res_change);
    }
  }
}

TEST_CASE( "Update with free living symbionts", "[default]" ){
  GIVEN("a world"){
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    SymWorld world(random, &config);
    int world_size = 4;
    world.Resize(world_size);
    int res_per_update = 10;
    config.RES_DISTRIBUTE(res_per_update);
    int num_updates = 5;

    emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);

    res_per_update = 80;
    config.RES_DISTRIBUTE(res_per_update);
    config.FREE_SYM_RES_DISTRIBUTE(res_per_update);
    world_size = 16;
    world.Resize(world_size);

    config.FREE_LIVING_SYMS(1);
    config.MOVE_FREE_SYMS(1);

    WHEN("there are no syms in the world"){
      THEN("hosts process normally"){
        world.AddOrgAt(host, 0);
        int orig_points = host->GetPoints();
        world.Update();

        REQUIRE(host->GetPoints() - orig_points == res_per_update);
      }
    }

    WHEN("there are only syms (no hosts) in the world"){
      world_size = 9;
      world.Resize(world_size);
      THEN("if only syms in the world they can get resources and reproduce"){
        emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
        world.SymDoBirth(sym, 0);
        REQUIRE(world.GetNumOrgs() == 1);

        for(int i = 0; i < num_updates; i++){
          world.Update();
        }
        //the sym has reproduced at least once
        REQUIRE(world.GetNumOrgs() > 1);

        int num_pop_elements = 0;
        for(int i = 0; i < world_size; i++){
          if(world.GetPop()[i]){
            num_pop_elements++;
          }
        }

        host.Delete(); //since it wasn't added to the world, have to delete it manually
      }
    }

    WHEN("there are both hosts and syms in the world"){
      world_size = 9;
      world.Resize(world_size);
      THEN("hosts and syms can mingle in the environment"){
        world.AddOrgAt(host, 0);
        world.AddOrgAt(emp::NewPtr<Symbiont>(&random, &world, &config, int_val), emp::WorldPosition(0,1));
        for(int i = 0; i < num_updates; i++){
          world.Update();
        }

        //the organisms have done something
        REQUIRE(world.GetNumOrgs() > 2);

        int free_sym_count = 0;
        int hosted_sym_count = 0;
        int host_count = 0;
        for(int i = 0; i < world_size; i++){
          if(world.GetPop()[i]){
            host_count++;
            hosted_sym_count += world.GetOrg(i).GetSymbionts().size();
          }
          if(world.GetSymPop()[i]){
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
    SymWorld world(random, &config);
    config.FREE_LIVING_SYMS(1);
    int int_val = 0;
    int world_size = 4;
    world.Resize(world_size);
    size_t host_pos = 0;
    emp::WorldPosition sym_pos = emp::WorldPosition(0, host_pos);

    emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
    world.AddOrgAt(sym, sym_pos);
    WHEN("there is a parallel host and the sym wants to infect"){
      emp::Ptr<Organism> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      world.AddOrgAt(host, host_pos);
      REQUIRE(world.GetNumOrgs() == 2);
      REQUIRE(host->HasSym() == false);

      WHEN("the infection fails"){
        config.SYM_INFECTION_FAILURE_RATE(1);
        emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
        world.AddOrgAt(sym, sym_pos);
        REQUIRE(world.GetNumOrgs() == 2);
        THEN("the sym is deleted"){
          world.MoveFreeSym(sym_pos);
          REQUIRE(world.GetNumOrgs() == 1);
          REQUIRE(!host->HasSym());
        }
      }
      WHEN("the infection does not fail"){
        THEN("the sym moves into the host"){
          world.MoveFreeSym(sym_pos);
          REQUIRE(world.GetNumOrgs() == 1);
          REQUIRE(host->HasSym());
          REQUIRE(host->GetSymbionts()[0] == sym);
        }
      }
    }
    WHEN("the sym does not want to/can't infect a parallel host"){
      size_t sym_id = 0;
      WHEN("moving is turned on"){
        config.MOVE_FREE_SYMS(1);
        sym->SetInfectionChance(0);
        THEN("the sym moves to a random spot in the free world"){
          REQUIRE(world.GetSymPop()[sym_id] == sym); //there should be a sym at pos 0
          world.MoveFreeSym(sym_pos);

          size_t new_sym_id = 2;
          emp::Ptr<Organism> new_sym = world.GetSymPop()[new_sym_id];
          REQUIRE(sym == new_sym);
          REQUIRE(world.GetSymPop()[sym_id] == nullptr);
        }
      }
      WHEN("moving is turned off"){
        config.MOVE_FREE_SYMS(0);
        THEN("the sym doesn't move"){
          REQUIRE(world.GetSymPop()[sym_id] == sym);
          world.MoveFreeSym(sym_pos);
          emp::Ptr<Organism> new_sym = world.GetSymPop()[sym_id];
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
    SymWorld world(random, &config);
    int world_size = 4;
    world.Resize(world_size);
    size_t sym_index = 1;
    emp::WorldPosition sym_pos = emp::WorldPosition(0, sym_index);
    emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);

    world.AddOrgAt(sym, sym_pos);
    REQUIRE(world.GetSymPop()[sym_index] == sym);
    REQUIRE(world.GetNumOrgs() == 1);

    emp::Ptr<Organism> new_org = world.ExtractSym(sym_index);
    REQUIRE(sym == new_org);
    REQUIRE(world.GetNumOrgs() == 0);
    REQUIRE(world.GetSymPop()[sym_index] == nullptr);

    sym.Delete();
  }
}

TEST_CASE( "MoveIntoNewFreeWorldPos", "[default]" ){
  GIVEN("a world"){
    emp::Random random(17);
    SymConfigBase config;
    SymWorld world(random, &config);
    int int_val = 0;
    int world_size = 4;
    world.Resize(world_size);

    emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);

    size_t orig_pos = 3;
    emp::WorldPosition orig_sym_pos = emp::WorldPosition(0, orig_pos);

    WHEN("A symbiont is successfully moved into a new symbiont world position"){
      emp::Ptr<Organism> parent_sym = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      world.AddOrgAt(parent_sym, orig_sym_pos);
      REQUIRE(world.GetNumOrgs() == 1);

      emp::WorldPosition new_pos = world.MoveIntoNewFreeWorldPos(sym, orig_sym_pos);
      THEN("It returns its new symbiont world position and can be found there"){
        REQUIRE(world.GetNumOrgs() == 2); //it didn't overwrite the parent
        REQUIRE(world.GetSymAt(new_pos.GetPopID()) == sym);
        REQUIRE(new_pos.IsValid() == true);
      }
    }

    WHEN("A symbiont unsuccessfully attempts to move into a new symbiont world position"){
      world_size = 0; // forcing an invalid new position
      world.Resize(world_size);
      emp::WorldPosition new_pos = world.MoveIntoNewFreeWorldPos(sym, orig_sym_pos);
      THEN("It returns an invalid WorldPosition and is not present in the symbiont world"){
        REQUIRE(world.GetNumOrgs() == 0); //the parent
        REQUIRE(new_pos.IsValid() == false);
      }
    }
    //TODO: CHECK MOVING OUT OF HOST
  }
}

TEST_CASE( "Resize", "[default]" ){
  GIVEN("a world"){
    SymConfigBase config;
    emp::Random random(17);
    SymWorld world(random, &config);

    size_t pop_size = world.GetPop().size();
    size_t sym_pop_size = world.GetSymPop().size();
    REQUIRE(pop_size == sym_pop_size);
    REQUIRE(pop_size == 0);

    size_t width = 3;
    size_t height = 3;
    world.Resize(width, height);
    pop_size = world.GetPop().size();
    sym_pop_size = world.GetSymPop().size();
    REQUIRE(pop_size == sym_pop_size);
    REQUIRE(pop_size == (width * height));

    size_t world_size = 11;
    world.Resize(world_size);
    pop_size = world.GetPop().size();
    sym_pop_size = world.GetSymPop().size();
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
    SymWorld world(random, &config);
    int world_size = 4;
    world.Resize(world_size);
    emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);

    WHEN("a sym is added into an empty spot"){
      THEN("it occupies that spot"){
        REQUIRE(world.GetNumOrgs() == 0);
        REQUIRE(world.GetSymPop()[0] == nullptr);
        world.AddOrgAt(sym, 0);
        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(world.GetSymPop()[0] == sym);
      }
    }
    WHEN("a sym is added into an occupied spot"){
      THEN("it replaces the occupying sym"){
        emp::Ptr<Organism> old_sym = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
        world.AddOrgAt(old_sym, 0);
        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(world.GetSymPop()[0] == old_sym);

        world.AddOrgAt(sym, 0);
        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(world.GetSymPop()[0] == sym);
      }
    }
    WHEN("a sym is added to an out of bounds pos"){
      THEN("pop and sym_pop are expanded to fit it"){
        REQUIRE(world.GetSymPop().size() == 4);
        world.AddOrgAt(sym, emp::WorldPosition(0,7));
        REQUIRE(world.GetSymPop().size() == world.GetPop().size());
        REQUIRE(world.GetSymPop().size() == 8);
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
    SymWorld world(random, &config);
    world.Resize(world_size);


    WHEN("a request is made for an in-bounds sym"){
      emp::Ptr<Organism> sym1 = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      emp::Ptr<Organism> sym2 = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      world.AddOrgAt(sym1, 0);
      world.AddOrgAt(sym2, emp::WorldPosition(0,1));
      THEN("the sym at that position is returned"){
        REQUIRE(world.GetSymAt(0) == sym1);
        REQUIRE(world.GetSymAt(1) == sym2);
        REQUIRE(world.GetSymAt(2) == nullptr);
      }
    }
    WHEN("a request is made for an out-of-bounds sym"){
      THEN("an exception is thrown"){
        REQUIRE_THROWS(world.GetSymAt(4));
      }
    }
  }
}

TEST_CASE( "DoSymDeath", "[default]" ){
  GIVEN("a world"){
    emp::Random random(17);
    SymConfigBase config;
    SymWorld world(random, &config);
    int world_size = 4;
    world.Resize(world_size);
    emp::Ptr<Organism> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, 1);
    size_t sym_position = 1;
    world.AddOrgAt(symbiont, emp::WorldPosition(0, sym_position));

    WHEN("A sym is deleted from a position"){
      THEN("It is no longer included in the count of organisms in the world"){
        REQUIRE(world.GetNumOrgs() == 1);
        world.DoSymDeath(sym_position);
        REQUIRE(world.GetNumOrgs() == 0);
      }
      THEN("It no longer occupies a spot in the world"){
        emp::Ptr<Organism> world_sym = world.GetSymAt(sym_position);
        REQUIRE(world_sym == symbiont);
        world.DoSymDeath(sym_position);

        emp::Ptr<Organism> world_sym_deleted = world.GetSymAt(sym_position);
        REQUIRE(world_sym_deleted == nullptr);
      }
    }
  }
}

TEST_CASE( "Spatial structure", "[default]" ){
  GIVEN("a world"){
    emp::Random random(17);
    SymConfigBase config;
    SymWorld world(random, &config);
    int width = 100;
    int height = 100;
    config.GRID_X(width);
    config.GRID_Y(height);
    world.Resize(width * height);
    config.FREE_LIVING_SYMS(1);
    config.MOVE_FREE_SYMS(1);
    config.SYM_HORIZ_TRANS_RES(0);
    size_t sym_limit = 10;
    config.SYM_LIMIT(sym_limit);

    WHEN("Grid is on"){
      config.GRID(1);
      world.SetPopStruct_Grid(width, height, false);

      THEN("Host babies are born next to their parents"){
        emp::Ptr<Organism> host_parent = emp::NewPtr<Host>(&random, &world, &config, 1);
        size_t host_parent_pos = 101;
        world.AddOrgAt(host_parent, host_parent_pos);

        emp::Ptr<Organism> host_baby = host_parent->Reproduce();
        world.DoBirth(host_baby, host_parent_pos);

         // We have parent position 101, so we expect baby to be in one of 8 surrounding cells
         //     | 000 | 001 | 002 | ...
         //     | 100 |*101*| 102 | ...
         //     | 200 | 201 | 202 | ...

        int possible_indices[8] = {0, 1, 2, 100, 102, 200, 201, 202};
        bool found_baby = false;
        for(int i = 0; i < 8; i++){
          if(world.GetPop()[possible_indices[i]] == host_baby){
            found_baby = true;
          }
        }

        REQUIRE(found_baby == true);
      }
      WHEN("Free living symbionts are permitted"){
        THEN("Symbiont babies are horizontally transmitted to a position near their parents"){
          emp::Ptr<Organism> sym_parent = emp::NewPtr<Symbiont>(&random, &world, &config, 1);
          emp::WorldPosition sym_parent_pos = emp::WorldPosition(0, 250);
          world.AddOrgAt(sym_parent, sym_parent_pos);

          sym_parent->HorizontalTransmission(sym_parent_pos);


          int possible_indices[8] = {149, 150, 151, 249, 251, 349, 350, 351};
          bool found_baby = false;
          for(int i = 0; i < 8; i++){
            if(world.GetSymPop()[possible_indices[i]] != nullptr && world.GetSymPop()[possible_indices[i]] != sym_parent){
              found_baby = true;
            }
          }

          REQUIRE(found_baby == true);
        }
        THEN("Symbionts randomly move to cells near their old position"){
          emp::Ptr<Organism> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, 1);
          emp::WorldPosition original_position = emp::WorldPosition(0, 898);
          world.AddOrgAt(symbiont, original_position);
          world.MoveFreeSym(original_position);

          int possible_indices[8] = {797, 798, 799, 897, 899, 997, 998, 999};
          bool found_sym = false;
          for(int i = 0; i < 8; i++){
            if(world.GetSymPop()[possible_indices[i]] == symbiont){
              found_sym = true;
            }
          }

          REQUIRE(found_sym == true);
        }
      }
      WHEN("Free living symbionts are not permitted"){
        config.FREE_LIVING_SYMS(0);
        THEN("Symbionts are horizontally transmitted into a neighboring host"){
          emp::Ptr<Organism> host_parent = emp::NewPtr<Host>(&random, &world, &config, 1);
          emp::Ptr<Organism> neighboring_host = emp::NewPtr<Host>(&random, &world, &config, 1);
          emp::Ptr<Organism> distant_host = emp::NewPtr<Host>(&random, &world, &config, 1);
          emp::Ptr<Organism> sym_parent = emp::NewPtr<Symbiont>(&random, &world, &config, 1);

          size_t host_parent_pos = 99;
          world.AddOrgAt(host_parent, host_parent_pos);
          world.AddOrgAt(neighboring_host, host_parent_pos + 1);
          world.AddOrgAt(distant_host, host_parent_pos + 150);
          host_parent->AddSymbiont(sym_parent);

          for(size_t sym_count = 1; sym_count <= sym_limit; sym_count++){
            sym_parent->HorizontalTransmission(emp::WorldPosition(1, host_parent_pos));
            REQUIRE(neighboring_host->GetSymbionts().size() == sym_count);
            REQUIRE(distant_host->HasSym() == false);
          }
        }
      }
    }

    WHEN("Grid is off"){
      config.GRID(0);
      world.SetPopStruct_Mixed(false);
      //given the size of the world, it's very unlikely that
      //organisms will randomly be placed in a neighbor position
      THEN("Host babies are born into a random position anywhere in the world"){
        emp::Ptr<Organism> host_parent = emp::NewPtr<Host>(&random, &world, &config, 1);
        size_t host_parent_pos = 101;
        world.AddOrgAt(host_parent, host_parent_pos);

        emp::Ptr<Organism> host_baby = host_parent->Reproduce();
        world.DoBirth(host_baby, host_parent_pos);

        int possible_indices[8] = {0, 1, 2, 100, 102, 200, 201, 202};
        bool found_baby = false;
        for(int i = 0; i < 8; i++){
          if(world.GetPop()[possible_indices[i]] == host_baby){
            found_baby = true;
          }
        }

        REQUIRE(found_baby == false);
      }
      WHEN("Free living symbionts are permitted"){
        THEN("Symbiont babies are horizontally transmitted to a random position anywhere in the world"){
          emp::Ptr<Organism> sym_parent = emp::NewPtr<Symbiont>(&random, &world, &config, 1);
          emp::WorldPosition sym_parent_pos = emp::WorldPosition(0, 250);
          world.AddOrgAt(sym_parent, sym_parent_pos);

          sym_parent->HorizontalTransmission(sym_parent_pos);


          int possible_indices[8] = {149, 150, 151, 249, 251, 349, 350, 351};
          bool found_baby = false;
          for(int i = 0; i < 8; i++){
            if(world.GetSymPop()[possible_indices[i]] != nullptr && world.GetSymPop()[possible_indices[i]] != sym_parent){
              found_baby = true;
            }
          }

          REQUIRE(found_baby == false);
        }
        THEN("Symbionts randomly move to cells anywhere in the world"){
          emp::Ptr<Organism> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, 1);
          emp::WorldPosition original_position = emp::WorldPosition(0, 898);
          world.AddOrgAt(symbiont, original_position);
          world.MoveFreeSym(original_position);

          int possible_indices[8] = {797, 798, 799, 897, 899, 997, 998, 999};
          bool found_sym = false;
          for(int i = 0; i < 8; i++){
            if(world.GetSymPop()[possible_indices[i]] == symbiont){
              found_sym = true;
            }
          }

          REQUIRE(found_sym == false);
        }
      }
      WHEN("Free living symbionts are not permitted"){
        config.FREE_LIVING_SYMS(0);
        THEN("Symbionts are horizontally transmitted into hosts anywhere in the world"){
          emp::Ptr<Organism> host_parent = emp::NewPtr<Host>(&random, &world, &config, 1);
          emp::Ptr<Organism> neighboring_host = emp::NewPtr<Host>(&random, &world, &config, 1);
          emp::Ptr<Organism> distant_host = emp::NewPtr<Host>(&random, &world, &config, 1);
          emp::Ptr<Organism> sym_parent = emp::NewPtr<Symbiont>(&random, &world, &config, 1);

          size_t host_parent_pos = 99;
          world.AddOrgAt(host_parent, host_parent_pos);
          world.AddOrgAt(neighboring_host, host_parent_pos + 1);
          world.AddOrgAt(distant_host, host_parent_pos + 150);
          host_parent->AddSymbiont(sym_parent);

          for(size_t sym_count = 1; sym_count <= sym_limit; sym_count++){
            sym_parent->HorizontalTransmission(emp::WorldPosition(1, host_parent_pos));
          }

          REQUIRE(neighboring_host->HasSym());
          REQUIRE(distant_host->HasSym());
        }
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
  config.NUM_PHYLO_BINS(20);
  int int_val = 0;
  SymWorld world(random, &config);
  int world_size = 4;
  world.Resize(world_size);


  emp::Ptr<Organism> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
  emp::Ptr<emp::Systematics<Organism,int>> host_sys = world.GetHostSys();

  //ORGANISMS ADDED TO SYSTEMATICS
  WHEN("an organism is added to the world"){
    WHEN("the cell it's added to is occupied"){
      size_t pos = 0;
      int_val = -1;

      emp::Ptr<Organism> occupying_host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      world.AddOrgAt(occupying_host, pos);
      size_t expected_occupying_taxon_info = 0;
      size_t taxon_info = host_sys->GetTaxonAt(pos)->GetInfo();

      REQUIRE(world.GetNumOrgs() == 1);
      REQUIRE(host_sys->GetNumActive() == 1);
      REQUIRE(expected_occupying_taxon_info == taxon_info);

      world.AddOrgAt(host, pos);
      taxon_info = host_sys->GetTaxonAt(pos)->GetInfo();
      size_t expected_taxon_info = 10;

      THEN("the occupying organism is removed from the systematic"){
        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(host_sys->GetNumActive() == 1);
        REQUIRE(expected_taxon_info == taxon_info);
      }
    }

    WHEN("the cell it's added to is empty"){
      size_t pos = 0;

      REQUIRE(world.GetNumOrgs() == 0);
      REQUIRE(host_sys->GetNumActive() == 0);

      world.AddOrgAt(host, pos);
      size_t taxon_info = host_sys->GetTaxonAt(pos)->GetInfo();
      size_t expected_taxon_info = 10;

      THEN("the organism is tracked by the systematic"){
        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(host_sys->GetNumActive() == 1);
        REQUIRE(expected_taxon_info == taxon_info);
      }
    }

    WHEN("there are 20 taxonomic bins"){

      size_t count = 7;
      size_t pos = 0;

      double int_vals[7] = {-1, -0.98, -0.9, -0.8, 0.65, 0.9, 1};
      int expected_taxon_infos[7] = {0, 0, 1, 2, 16, 19, 19};

      THEN("it will be placed in the correct bin"){
        for(size_t i = 0; i < count; i++){
          world.AddOrgAt(emp::NewPtr<Host>(&random, &world, &config, int_vals[i]), pos);
          int taxon_info = host_sys->GetTaxonAt(pos)->GetInfo();
          REQUIRE(taxon_info == expected_taxon_infos[i]);
        }
      }
      host.Delete();
    }

    WHEN("there are 2 taxonomic bins"){
      config.NUM_PHYLO_BINS(2);

      size_t count = 3;
      size_t pos = 0;

      double int_vals[3] = {1, 0, -0.001};
      int expected_taxon_infos[3] = {1, 1, 0};

      THEN("it will be placed in the correct bin"){
        for(size_t i = 0; i < count; i++){
          world.AddOrgAt(emp::NewPtr<Host>(&random, &world, &config, int_vals[i]), pos);
          int taxon_info = host_sys->GetTaxonAt(pos)->GetInfo();
          REQUIRE(taxon_info == expected_taxon_infos[i]);
        }
      }
      host.Delete();
    }
  }

  //ORGANISMS AND RELATIONSHIPS TRACKED
  WHEN("Several generations pass"){
    THEN("The phylogenetic relationships are tracked and accurate"){
      world_size = 10;
      world.Resize(world_size);
      int num_descendants = 4;
      //add the first host
      world.AddOrgAt(host, 0);

      //populate the world with descendents with various interaction values
      //Can't use num_descendants for the following array sizes because some
      //compilers don't allow it
      double int_vals[4] = {0.1, -0.05, -0.2, 0.14};
      size_t parents[4] = {0, 1, 1, 3};
      for(int i = 0; i < num_descendants; i++){
        world.AddOrgAt(emp::NewPtr<Host>(&random, &world, &config, int_vals[i]), (i+1), parents[i]);
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
  config.NUM_PHYLO_BINS(20);
  int int_val = 0;
  SymWorld world(random, &config);
  int world_size = 20;
  world.Resize(world_size);

  emp::Ptr<emp::Systematics<Organism,int>> sym_sys = world.GetSymSys();

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
        sym = emp::NewPtr<Symbiont>(&random, &world, &config, int_vals[i]);
        world.InjectSymbiont(sym);
        REQUIRE(sym->GetTaxon()->GetInfo() == taxon_infos[i]);
      }
    }
  }

  WHEN("symbionts are deleted"){
    THEN("they are no longer tracked by the sym systematic"){
      world_size = 1;
      world.Resize(world_size);
      REQUIRE(sym_sys->GetNumActive() == 0);
      emp::Ptr<Organism> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      world.InjectSymbiont(symbiont);
      REQUIRE(sym_sys->GetNumActive() == 1);
      world.DoSymDeath(0);
      REQUIRE(sym_sys->GetNumActive() == 0);
    }
  }

  WHEN("generations pass"){
    config.MUTATION_SIZE(1);
    config.MUTATION_RATE(1);
    config.PHYLOGENY(1);
    size_t num_syms = 4;

    emp::Ptr<Organism> syms[num_syms];
    syms[0] = emp::NewPtr<Symbiont>(&random, &world, &config, 0);
    world.AddSymToSystematic(syms[0]);

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
      world.Update();

      //after update, times should now be 1
      emp::Ptr<emp::Taxon<int>> dest_tax = syms[0]->GetTaxon();
      syms[0].Delete();
      REQUIRE(dest_tax->GetDestructionTime() == 1);

      //another update, times 2
      world.Update();
      dest_tax = syms[1]->GetTaxon();
      syms[1].Delete();
      REQUIRE(dest_tax->GetDestructionTime() == 2);
    }

    syms[2].Delete();
    syms[3].Delete();
  }
}

TEST_CASE( "SetMutationZero", "[default]") {
  GIVEN("World first created with all mutation settings at 1") {
    emp::Random random(17);
    SymConfigBase config;
    config.MUTATION_SIZE(1);
    config.MUTATION_RATE(1);
    config.HOST_MUTATION_SIZE(1);
    config.HOST_MUTATION_RATE(1);
    config.MUTATE_LYSIS_CHANCE(1);
    config.MUTATE_INDUCTION_CHANCE(1);
    config.MUTATE_INC_VAL(1);
    config.EFFICIENCY_MUT_RATE(1);
    config.INT_VAL_MUT_RATE(1);
    config.HORIZ_MUTATION_SIZE(1);
    config.HORIZ_MUTATION_RATE(1);
    SymWorld world(random, &config);

    REQUIRE(config.MUTATION_SIZE() == 1);
    REQUIRE(config.MUTATION_RATE() == 1);
    REQUIRE(config.HOST_MUTATION_SIZE() == 1);
    REQUIRE(config.HOST_MUTATION_RATE() == 1);
    REQUIRE(config.MUTATE_LYSIS_CHANCE() == 1);
    REQUIRE(config.MUTATE_INDUCTION_CHANCE() == 1);
    REQUIRE(config.MUTATE_INC_VAL() == 1);
    REQUIRE(config.EFFICIENCY_MUT_RATE() == 1);
    REQUIRE(config.INT_VAL_MUT_RATE() == 1);
    REQUIRE(config.HORIZ_MUTATION_SIZE() == 1);
    REQUIRE(config.HORIZ_MUTATION_RATE() == 1);

    WHEN("SetMutationZero method called") {
      world.SetMutationZero();
      THEN("Mutation size and rate both 0") {
        REQUIRE(config.MUTATION_SIZE() == 0);
        REQUIRE(config.MUTATION_RATE() == 0);
        REQUIRE(config.HOST_MUTATION_SIZE() == 0);
        REQUIRE(config.HOST_MUTATION_RATE() == 0);
        REQUIRE(config.MUTATE_LYSIS_CHANCE() == 0);
        REQUIRE(config.MUTATE_INDUCTION_CHANCE() == 0);
        REQUIRE(config.MUTATE_INC_VAL() == 0);
        REQUIRE(config.EFFICIENCY_MUT_RATE() == 0);
        REQUIRE(config.INT_VAL_MUT_RATE() == 0);
        REQUIRE(config.HORIZ_MUTATION_SIZE() == 0);
        REQUIRE(config.HORIZ_MUTATION_RATE() == 0);
      }
    }
  }
}

TEST_CASE( "No mutation updates", "[default] "){
  GIVEN("a world with 1 mutation update and 1 non-mutation update"){
    emp::Random random(17);
    SymConfigBase config;
    config.MUTATION_SIZE(1);
    config.MUTATION_RATE(1);
    config.UPDATES(1);
    config.NO_MUT_UPDATES(1);
    SymWorld world(random, &config);
    double int_val = 0.4;
    int world_size = 100;
    world.Resize(world_size);
    emp::Ptr<Organism> symbiont = new Symbiont(&random, &world, &config, int_val);
    emp::Ptr<Organism> host = new Host(&random, &world, &config, int_val);

    WHEN("A host and symbiont reproduce at first"){
      emp::Ptr<Organism> mut_sym_baby = symbiont->Reproduce();
      emp::Ptr<Organism> mut_host_baby = host->Reproduce();

      THEN("Mutation rate and size are still 1"){
        REQUIRE(config.MUTATION_RATE() == 1);
        REQUIRE(config.MUTATION_SIZE() == 1);
      }
      THEN("Host and symbiont offspring are mutated"){
        REQUIRE(mut_sym_baby->GetIntVal() > int_val - 0.00001);
        REQUIRE(mut_host_baby->GetIntVal() > int_val - 0.00001);
      }
    }
    WHEN("The experiment runs and host and symbiont reproduce after"){
      world.RunExperiment(false);
      emp::Ptr<Organism> no_mut_sym_baby = symbiont->Reproduce();
      emp::Ptr<Organism> no_mut_host_baby = host->Reproduce();

      THEN("Mutation sizes and rates are all 0"){
        REQUIRE(config.MUTATION_RATE() == 0);
        REQUIRE(config.MUTATION_SIZE() == 0);
        REQUIRE(config.HOST_MUTATION_SIZE() == 0);
        REQUIRE(config.HOST_MUTATION_RATE() == 0);
        REQUIRE(config.MUTATE_LYSIS_CHANCE() == 0);
        REQUIRE(config.MUTATE_INDUCTION_CHANCE() == 0);
        REQUIRE(config.MUTATE_INC_VAL() == 0);
      }
      THEN("Host and symbiont offspring aren't mutated"){
        REQUIRE(no_mut_sym_baby->GetIntVal() < int_val + 0.00001);
        REQUIRE(no_mut_sym_baby->GetIntVal() > int_val - 0.00001);
        REQUIRE(no_mut_host_baby->GetIntVal() < int_val + 0.00001);
        REQUIRE(no_mut_host_baby->GetIntVal() > int_val - 0.00001);
      }
    }
  }
}

TEST_CASE( "IsInboundsPos", "[default]" ){
  GIVEN("a world"){
    emp::Random random(17);
    SymConfigBase config;
    SymWorld world(random, &config);
    int world_size = 4;
    world.Resize(world_size);

    size_t valid_pos = 3;
    REQUIRE(world.IsInboundsPos(valid_pos) == true);

    emp::WorldPosition valid_pos_wp = emp::WorldPosition(3,0);
    REQUIRE(world.IsInboundsPos(valid_pos_wp) == true);

    valid_pos_wp = emp::WorldPosition(0,3);
    REQUIRE(world.IsInboundsPos(valid_pos_wp) == true);

    emp::WorldPosition invalid_pos = emp::WorldPosition(4,3);
    REQUIRE(world.IsInboundsPos(invalid_pos) == false);

    invalid_pos = emp::WorldPosition(0,4);
    REQUIRE(world.IsInboundsPos(invalid_pos) == false);
  }
}
