#include "../test_utils.h"

#include "../../default_mode/SymWorld.h"
#include "../../default_mode/Symbiont.h"
#include "../../lysis_mode/Phage.h"
#include "../../lysis_mode/LysisWorld.h"
#include "../../default_mode/Host.h"
#include "../../default_mode/WorldSetup.cc"


TEST_CASE("Well-Mixed Neighbor doesn't include focal org", "[default]") {
  GIVEN(" a world ") {
    emp::Random random(17);
    SymConfigBase config;
    config.WORLD_WIDTH(2);
    config.WORLD_HEIGHT(2);
    config.INIT_POP_SIZE(4);
    config.SPATIAL_STRUCT_MODE("well-mixed"); // make sure the world is well-mixed
    SymWorld world(random, &config);
    world.Setup();
    bool self_neighbor = false;


    WHEN(" focal position is 0 ") {
      emp::WorldPosition focal_pos(0);
      WHEN("we repeatedly get a neighbor ") {
        for (int i = 0; i < 100; i++) {
          emp::WorldPosition neighbor_pos = world.GetRandomNeighborPos(focal_pos);
          if (neighbor_pos.GetIndex() == focal_pos.GetIndex()) {
            std::cout << "neighbor pos: " << neighbor_pos.GetIndex() << std::endl;
            self_neighbor = true;
            break;
          }
        }

        THEN(" the neighbor is never the focal position ") {
          REQUIRE(self_neighbor == false);
        }
      }
    }

    WHEN(" focal position is end of population ") {
      emp::WorldPosition focal_pos(config.INIT_POP_SIZE() - 1);
      WHEN("we repeatedly get a neighbor ") {
        for (int i = 0; i < 100; i++) {
          emp::WorldPosition neighbor_pos = world.GetRandomNeighborPos(focal_pos);
          if (neighbor_pos.GetIndex() == focal_pos.GetIndex()) {
            self_neighbor = true;
            break;
          }
        }
        THEN(" the neighbor is never the focal position ") {
          REQUIRE(self_neighbor == false);
        }
      }
    }
  }
}

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

    WHEN( "the vertical transmission rate is 0" ) {
      config.VERTICAL_TRANSMISSION(0);

      THEN( "there is never vertical transmission" ) {
        REQUIRE( world.WillTransmit() == false );
        REQUIRE( world.WillTransmit() == false );
        REQUIRE( world.WillTransmit() == false );
        REQUIRE( world.WillTransmit() == false );
        REQUIRE( world.WillTransmit() == false );
      }
    }

    WHEN( "the vertical transmission rate is 1" ) {
      config.VERTICAL_TRANSMISSION(1);

      THEN( "there is always vertical transmission" ) {
        REQUIRE( world.WillTransmit() == true );
        REQUIRE( world.WillTransmit() == true );
        REQUIRE( world.WillTransmit() == true );
        REQUIRE( world.WillTransmit() == true );
        REQUIRE( world.WillTransmit() == true );
      }
    }

    WHEN( "the vertical transmission rate is .5" ) {
      config.VERTICAL_TRANSMISSION(.5);

      THEN( "there is sometimes vertical transmission" ) {
        bool yes = false;
        bool no = false;
        for (int i = 0; i < 128; i++)//Odds of failure should be 1 in 170141183460469231731687303715884105728
          if (world.WillTransmit())
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
    // Configure empty well-mixed environment
    test_utils::SetEmptyWellMixed(config);

    SymWorld world(random, &config);
    world.Setup();

    WHEN( "hosts are added" ) {

      int n = 7532;

      //inject organisms
      for (int i = 0; i < n; i++) {
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
    emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(17);
    SymWorld world(*random, &config);
    test_utils::SetEmptyWellMixed(config);
    config.VERTICAL_TRANSMISSION(0);
    config.VERTICAL_TRANSMISSION(0);
    config.MUTATION_SIZE(0);
    config.SYM_LIMIT(500);
    config.HORIZ_TRANS(true);
    config.HOST_REPRO_RES(400);
    config.RES_DISTRIBUTE(100);
    config.SYNERGY(5);
    world.Setup();

    WHEN( "hostile hosts meet generous symbionts" ) {

      //inject organisms
      for (size_t i = 0; i < 10; i++) {
        emp::Ptr<Host> new_org = emp::NewPtr<Host>(random, &world, &config, -0.1);
        world.AddOrgAt(new_org, world.size());
      }
      int width = 10;
      int height = 1;
      int world_size = width * height;
      world.Resize(width, height);
      for (int i = 0; i < world_size; i++) {
        emp::Ptr<Symbiont> new_sym = emp::NewPtr<Symbiont>(random, &world, &config, 0.1);
        world.InjectSymbiont(new_sym);
      }
      //Simulate
      for (int i = 0; i < 100; i++) {
        world.Update();
      }

      THEN( "the symbionts all die" ) {

        for (size_t i = 0; i < world.GetPop().size(); i++)
          REQUIRE( !(world.GetPop()[i] && world.GetPop()[i]->HasSym()) );//We can't have a host exist with a symbiont in it.
      }
    }
    random.Delete();
  }




  GIVEN( "a world" ) {
    emp::Random random(17);
    SymWorld world(random, &config);
    world.SetPopStruct_Mixed();
    config.SPATIAL_STRUCT_MODE("well-mixed");
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
      for (size_t i = 0; i < 200; i++) {
        emp::Ptr<Host> new_org;
        new_org.New(&random, &world, &config, 1);
        world.AddOrgAt(new_org, world.size());
      }

      int width = 100;
      int height = 200;
      world.Resize(width, height);

      for (size_t i = 0; i < 10000; i++) {
        emp::Ptr<Symbiont> new_sym;
        new_sym.New(&random, &world, &config, -1);
        world.InjectSymbiont(new_sym);
      }

      //Simulate
      for (int i = 0; i < 100; i++)
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
    test_utils::SetEmptyWellMixed(config);
    SymWorld world(random, &config);
    world.Setup();

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

TEST_CASE( "InjectSymbiont", "[default]" ) {
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    test_utils::SetEmptyWellMixed(config);
    int int_val = 0;
    SymWorld world(random, &config);
    world.Setup();
    int world_size = 4;

    WHEN( "free living syms are not allowed" ) {
      world.Resize(world_size);
      config.FREE_LIVING_SYMS(0);

      emp::Ptr<Organism> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      world.AddOrgAt(host, 0);
      emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);

      THEN( "syms are injected into a random host" ) {
        world.InjectSymbiont(sym);
        emp::vector<emp::Ptr<Organism>> host_syms = host->GetSymbionts();

        REQUIRE(host_syms.size() == 1);
        REQUIRE(host_syms.at(0) == sym);
      }
    }
    WHEN( "free living syms are allowed" ) {
      world_size = 1000;
      world.Resize(world_size);
      config.FREE_LIVING_SYMS(1);


      THEN( "syms can be injected into a random empty cell" ) {

        REQUIRE(world.GetNumOrgs() == 0);

        size_t sym_count = 100;

        for (size_t i = 0; i < sym_count; i++) {
          world.InjectSymbiont(emp::NewPtr<Symbiont>(&random, &world, &config, int_val));
        }
        //since spot of injection is random, a few symbionts
        //will get overwritten, and thus # injected != # remaining in world
        REQUIRE(world.GetNumOrgs() < (sym_count + 1));
        REQUIRE(world.GetNumOrgs() > (sym_count - 10));
        world.CleanupGraveyard();
      }
    }
  }
}

TEST_CASE( "DoBirth", "[default]" ) {
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    test_utils::SetEmptyWellMixed(config);
    int int_val = 0;
    SymWorld world(random, &config);
    world.Setup();
    int world_size = 4;
    world.Resize(world_size);
    config.FREE_LIVING_SYMS(1);
    emp::Ptr<Organism> h2 = emp::NewPtr<Host>(&random, &world, &config, int_val);
    world.AddOrgAt(h2, 3);
    emp::Ptr<Organism> host = emp::NewPtr<Host>(&random, &world, &config, int_val);

    WHEN( "born into an empty spot" ) {
      THEN( "occupies that spot" ) {
        world.DoBirth(host, 2);

        REQUIRE(world.GetNumOrgs() == 2);
        bool host_isborn = false;
        for (size_t i = 0; i < 4; i++) {
          if (&world.GetOrg(i) == host) {
            host_isborn = true;
            break;
          }
        }
        REQUIRE(host_isborn == true);
      }
    }
    WHEN( "born into a spot occupied by another host" ) {
      THEN( "kills that host and replaces it" ) {
        emp::Ptr<Organism> other_host = emp::NewPtr<Host>(&random, &world, &config, int_val);
        world.AddOrgAt(other_host, 0);
        world.DoBirth(host, 2);

        REQUIRE(world.GetNumOrgs() == 2);

        bool host_isborn = false;
        bool otherhost_isdead = true;
        for (size_t i = 0; i < 4; i++) {
          if (world.GetPop()[i] == host) {
            host_isborn = true;
          } else if (world.GetPop()[i] != nullptr && world.GetPop()[i] != h2) {
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
    test_utils::SetEmptyWellMixed(config);

    int int_val = 0;
    SymWorld world(random, &config);
    world.Setup();

    size_t world_size = 4;
    world.Resize(world_size);
    world.SetPopStruct_Mixed();

    emp::WorldPosition new_pos;

    WHEN( "free living symbionts are not allowed" ) {
      config.FREE_LIVING_SYMS(0);

      WHEN( "there is a valid neighbouring host" ) {
        size_t host_pos = 1;
        emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
        world.AddOrgAt(host, host_pos);

        emp::Ptr<Host> uninfected_host = emp::NewPtr<Host>(&random, &world, &config, int_val);
        world.AddOrgAt(uninfected_host, host_pos + 1);

        WHEN("there is room in the neighboring host and free failure due to size constraints is off") {
          config.FREE_HT_FAILURE(0);
          config.SYM_LIMIT(2);

          emp::WorldPosition parent_sym_pos = emp::WorldPosition(1, host_pos);
          emp::Ptr<Organism> parent_symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
          host->AddSymbiont(parent_symbiont);
          emp::Ptr<Organism> new_symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
          new_pos = world.SymDoBirth(new_symbiont, parent_sym_pos);

          emp::vector<emp::Ptr<Organism>> syms = uninfected_host->GetSymbionts();
          emp::Ptr<Organism> host_sym = syms[0];

          THEN( "the sym is inserted into the valid neighbouring host" ) {
            REQUIRE(host_sym == new_symbiont);
            REQUIRE(world.GetNumOrgs() == 2);
            REQUIRE(new_pos.IsValid() == true);
            REQUIRE(parent_symbiont->GetPoints() == 0);

            REQUIRE(new_pos.GetIndex() == 1);
            REQUIRE(new_pos.GetPopID() == host_pos + 1);
            REQUIRE(new_symbiont->GetPoints() == 0);
          }
        }
        WHEN("there is room in the host and free failure due to size constraints is on") {
          config.FREE_HT_FAILURE(1);
          config.SYM_LIMIT(2);

          emp::Ptr<Organism> symbiont_parent = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
          host->AddSymbiont(symbiont_parent);

          // there MUST be a symbiont parent in the free failure condition (seg fault otherwise)
          size_t starting_resources = 20;
          size_t horiz_trans_res_required = 10;
          config.SYM_HORIZ_TRANS_RES(horiz_trans_res_required);

          symbiont_parent->SetPoints(starting_resources);
          symbiont_parent->IndependentReproduction(emp::WorldPosition(1, host_pos));

          THEN("the sym child is inserted in the neighboring host and the parent spends points") {
            REQUIRE(symbiont_parent->GetPoints() == 0);
            REQUIRE(host->GetSymbionts().size() == 1);
            REQUIRE(uninfected_host->GetSymbionts().size() == 1);
            REQUIRE(host->GetSymbionts().at(0) == symbiont_parent);
            REQUIRE(uninfected_host->GetSymbionts().at(0)->GetPoints() == 0);
          }
        }
        WHEN("there is no room in the neighbor host and free failure due to size constraints is on") {
          uninfected_host->AddSymbiont(emp::NewPtr<Symbiont>(&random, &world, &config, int_val));
          config.FREE_HT_FAILURE(1);
          config.SYM_LIMIT(1);

          emp::Ptr<Organism> symbiont_parent = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
          host->AddSymbiont(symbiont_parent);

          // there MUST be a symbiont parent in the free failure condition (seg fault otherwise)
          size_t starting_resources = 20;
          size_t horiz_trans_res_required = 10;
          config.SYM_HORIZ_TRANS_RES(horiz_trans_res_required);

          symbiont_parent->SetPoints(starting_resources);
          symbiont_parent->IndependentReproduction(emp::WorldPosition(1, host_pos));

          THEN("the sym child is inserted nowhere and the parent spends no points") {
            REQUIRE(symbiont_parent->GetPoints() == starting_resources);
            REQUIRE(host->GetSymbionts().size() == 1);
            REQUIRE(host->GetSymbionts().at(0) == symbiont_parent);
          }
        }
        WHEN("there is no room in the host and free failure due to size constraints is off") {
          config.FREE_HT_FAILURE(0);
          config.SYM_LIMIT(1);

          emp::Ptr<Organism> symbiont_parent = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
          host->AddSymbiont(symbiont_parent);

          // there MUST be a symbiont parent in the free failure condition (seg fault otherwise)
          size_t starting_resources = 20;
          size_t horiz_trans_res_required = 10;
          config.SYM_HORIZ_TRANS_RES(horiz_trans_res_required);

          symbiont_parent->SetPoints(starting_resources);
          symbiont_parent->IndependentReproduction(emp::WorldPosition(1, host_pos));

          THEN("the sym child is inserted nowhere and the parent's points get set to 0") {
            REQUIRE(symbiont_parent->GetPoints() == 0);
            REQUIRE(host->GetSymbionts().size() == 1);
            REQUIRE(host->GetSymbionts().at(0) == symbiont_parent);
          }
        }
      }

      WHEN( "there is no valid neighbouring host" ) {

        new_pos = world.SymDoBirth(emp::NewPtr<Symbiont>(&random, &world, &config, int_val), 2);

        THEN( "the sym is killed" ) {
          //the world should be empty
          REQUIRE(world.GetNumOrgs() == 0);
          REQUIRE(new_pos.IsValid() == false);
        }
      }
    }


    WHEN( "free living symbionts are allowed") {
      config.FREE_LIVING_SYMS(1);
      world_size = 2;
      world.Resize(world_size);

      THEN("it might be inserted into an empty cell") {
        emp::WorldPosition parent_pos = emp::WorldPosition(0, 1);

        new_pos = world.SymDoBirth(emp::NewPtr<Symbiont>(&random, &world, &config, int_val), parent_pos);

        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(new_pos.IsValid() == true);
        REQUIRE(new_pos.GetIndex() == 0);
        REQUIRE(new_pos.GetPopID() == 0);
      }

      THEN("it may be inserted into an occupied cell, overwriting the previous occupant") {
        for (size_t i = 0; i < world_size; i++) {
          world.AddOrgAt(emp::NewPtr<Symbiont>(&random, &world, &config, int_val), emp::WorldPosition(0, i));
        }
        REQUIRE(world.GetNumOrgs() == world_size);

        emp::WorldPosition parent_pos = emp::WorldPosition(0, 1);
        emp::Ptr<Organism> new_symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
        new_pos = world.SymDoBirth(new_symbiont, parent_pos);

        bool new_sym_born = false;
        for (size_t i = 0; i < world_size; i++) {
          if (world.GetSymAt(i) == new_symbiont) {
            new_sym_born = true;
          }
        }
        world.CleanupGraveyard();
        REQUIRE(world.GetNumOrgs() == world_size);
        REQUIRE(new_sym_born == true);
        REQUIRE(new_pos.IsValid() == true);
        REQUIRE(world.IsInboundsPos(new_pos) == true);
        world.CleanupGraveyard();
      }

      THEN("it might not find a valid cell and get deleted") {
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

TEST_CASE( "Update without free living symbionts", "[default]" ) {
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigBase config;
    // Configure empty well-mixed environment
    config.SPATIAL_STRUCT_MODE("well-mixed");
    config.WORLD_WIDTH(0);
    config.WORLD_HEIGHT(0);
    config.INIT_POP_SIZE(0);

    int int_val = 0;
    SymWorld world(random, &config);
    world.Setup();
    int world_size = 4;
    world.Resize(world_size);
    int res_per_update = 10;
    config.RES_DISTRIBUTE(res_per_update);

    emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);

    world.AddOrgAt(host, 0);

    WHEN("a host is dead") {
      THEN("it is removed from the world") {
        host->SetDead();
        REQUIRE(world.GetNumOrgs() == 1);

        world.Update();
        REQUIRE(world.GetNumOrgs() == 0);
      }
    }
    THEN("hosts process normally") {
      int res_before_update = host->GetPoints();
      world.Update();
      int res_after_update = host->GetPoints();
      int res_change = res_after_update - res_before_update;
      REQUIRE(res_per_update == res_change);
    }
  }
}

TEST_CASE( "Update with free living symbionts", "[default]" ) {
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigBase config;
    // Configure empty well-mixed environment
    config.SPATIAL_STRUCT_MODE("well-mixed");
    config.WORLD_WIDTH(0);
    config.WORLD_HEIGHT(0);
    config.INIT_POP_SIZE(0);

    int int_val = 0;
    SymWorld world(random, &config);
    world.Setup();
    int world_size = 4;
    world.Resize(world_size);
    int res_per_update = 10;
    config.RES_DISTRIBUTE(res_per_update);
    int num_updates = 20;

    emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);

    res_per_update = 80;
    config.RES_DISTRIBUTE(res_per_update);
    config.FREE_SYM_RES_DISTRIBUTE(res_per_update);
    world_size = 16;
    world.Resize(world_size);

    config.FREE_LIVING_SYMS(1);
    config.MOVE_FREE_SYMS(1);

    WHEN("there are no syms in the world") {
      THEN("hosts process normally") {
        world.AddOrgAt(host, 0);
        int orig_points = host->GetPoints();
        world.Update();

        REQUIRE(host->GetPoints() - orig_points == res_per_update);
      }
    }

    WHEN("there are only syms (no hosts) in the world") {
      world_size = 9;
      world.Resize(world_size);
      THEN("if only syms in the world they can get resources and reproduce") {
        emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
        world.SymDoBirth(sym, 0);
        REQUIRE(world.GetNumOrgs() == 1);

        for (int i = 0; i < num_updates; i++) {
          world.Update();
        }
        //the sym has reproduced at least once
        REQUIRE(world.GetNumOrgs() > 1);

        int num_pop_elements = 0;
        for (int i = 0; i < world_size; i++) {
          if (world.GetPop()[i]) {
            num_pop_elements++;
          }
        }

        host.Delete(); //since it wasn't added to the world, have to delete it manually
      }
    }

    WHEN("there are both hosts and syms in the world") {
      world_size = 9;
      world.Resize(world_size);
      THEN("hosts and syms can mingle in the environment") {
        world.AddOrgAt(host, 0);
        world.AddOrgAt(emp::NewPtr<Symbiont>(&random, &world, &config, int_val), emp::WorldPosition(0,1));
        for (int i = 0; i < num_updates; i++) {
          world.Update();
        }

        //the organisms have done something
        REQUIRE(world.GetNumOrgs() > 2);

        int free_sym_count = 0;
        int hosted_sym_count = 0;
        int host_count = 0;
        for (int i = 0; i < world_size; i++) {
          if (world.GetPop()[i]) {
            host_count++;
            hosted_sym_count += world.GetOrg(i).GetSymbionts().size();
          }
          if (world.GetSymPop()[i]) {
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

TEST_CASE( "MoveFreeSym", "[default]" ) {
  GIVEN("free living syms are allowed") {
    emp::Random random(14);
    SymConfigBase config;
    // Configure empty well-mixed environment
    config.SPATIAL_STRUCT_MODE("well-mixed");
    config.WORLD_WIDTH(0);
    config.WORLD_HEIGHT(0);
    config.INIT_POP_SIZE(0);
    config.FREE_LIVING_SYMS(1);
    SymWorld world(random, &config);
    int int_val = 0;
    size_t world_size = 4;
    world.Resize(world_size);
    size_t host_pos = 0;
    emp::WorldPosition sym_pos = emp::WorldPosition(0, host_pos);

    emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
    world.AddOrgAt(sym, sym_pos);
    WHEN("there is a parallel host and the sym wants to infect") {
      emp::Ptr<Organism> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      world.AddOrgAt(host, host_pos);
      REQUIRE(world.GetNumOrgs() == 2);
      REQUIRE(host->HasSym() == false);

      WHEN("the infection fails") {
        config.SYM_INFECTION_FAILURE_RATE(1);
        emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
        world.AddOrgAt(sym, sym_pos);
        REQUIRE(world.GetNumOrgs() == 2);
        THEN("the sym is deleted") {
          world.MoveFreeSym(sym_pos);
          world.CleanupGraveyard();
          REQUIRE(world.GetNumOrgs() == 1);
          REQUIRE(!host->HasSym());
        }
      }
      WHEN("the infection does not fail") {
        THEN("the sym moves into the host") {
          world.MoveFreeSym(sym_pos);
          REQUIRE(world.GetNumOrgs() == 1);
          REQUIRE(host->HasSym());
          REQUIRE(host->GetSymbionts()[0] == sym);
        }
      }
    }
    WHEN("the sym does not want to/can't infect a parallel host") {
      size_t sym_id = 0;
      WHEN("moving is turned on") {
        config.MOVE_FREE_SYMS(1);
        sym->SetInfectionChance(0);
        THEN("the sym moves to a random spot in the free world") {
          REQUIRE(world.GetSymPop()[sym_id] == sym); //there should be a sym at pos 0
          world.MoveFreeSym(sym_pos);
          // look for where the sym moved to, but shouldn't be original cell
          bool found_sym = false;
          for (size_t i=1; i < world_size; i++) {
            if (world.GetSymPop()[i] == sym) {
              found_sym = true;
              break;
            }
          }
          REQUIRE(found_sym);
          REQUIRE(world.GetSymPop()[sym_id] == nullptr);
        }
      }
      WHEN("moving is turned off") {
        config.MOVE_FREE_SYMS(0);
        THEN("the sym doesn't move") {
          REQUIRE(world.GetSymPop()[sym_id] == sym);
          world.MoveFreeSym(sym_pos);
          emp::Ptr<Organism> new_sym = world.GetSymPop()[sym_id];
          REQUIRE(sym == new_sym);
        }
      }
    }
  }
}

TEST_CASE( "ExtractSym", "[default]" ) {
  GIVEN("a world") {
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

TEST_CASE( "MoveIntoNewFreeWorldPos", "[default]" ) {
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigBase config;
    SymWorld world(random, &config);
    int int_val = 0;
    int world_size = 4;
    world.Resize(world_size);

    emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);

    size_t orig_pos = 3;
    emp::WorldPosition orig_sym_pos = emp::WorldPosition(0, orig_pos);

    WHEN("A symbiont is successfully moved into a new symbiont world position") {
      emp::Ptr<Organism> parent_sym = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      world.AddOrgAt(parent_sym, orig_sym_pos);
      REQUIRE(world.GetNumOrgs() == 1);

      emp::WorldPosition new_pos = world.MoveIntoNewFreeWorldPos(sym, orig_sym_pos);
      THEN("It returns its new symbiont world position and can be found there") {
        REQUIRE(world.GetNumOrgs() == 2); //it didn't overwrite the parent
        REQUIRE(world.GetSymAt(new_pos.GetPopID()) == sym);
        REQUIRE(new_pos.IsValid() == true);
      }
    }

    WHEN("A symbiont unsuccessfully attempts to move into a new symbiont world position") {
      world_size = 0; // forcing an invalid new position
      world.Resize(world_size);
      emp::WorldPosition new_pos = world.MoveIntoNewFreeWorldPos(sym, orig_sym_pos);
      THEN("It returns an invalid WorldPosition and is not present in the symbiont world") {
        REQUIRE(world.GetNumOrgs() == 0); //the parent
        REQUIRE(new_pos.IsValid() == false);
      }
    }
    //TODO: CHECK MOVING OUT OF HOST
  }
}

TEST_CASE( "Resize", "[default]" ) {
  GIVEN("a world") {
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

TEST_CASE( "AddOrgAt", "[default]" ) {
  //adding hosts to the world should be covered by Empirical tests,
  //so here we'll test adding a sym
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigBase config;
    test_utils::SetEmptyWellMixed(config);
    int int_val = 0;
    SymWorld world(random, &config);
    world.Setup();
    int world_size = 4;
    world.Resize(world_size);
    emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);

    WHEN("a sym is added into an empty spot") {
      THEN("it occupies that spot") {
        REQUIRE(world.GetNumOrgs() == 0);
        REQUIRE(world.GetSymPop()[0] == nullptr);
        world.AddOrgAt(sym, 0);
        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(world.GetSymPop()[0] == sym);
      }
    }
    WHEN("a sym is added into an occupied spot") {
      THEN("it replaces the occupying sym") {
        emp::Ptr<Organism> old_sym = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
        world.AddOrgAt(old_sym, 0);
        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(world.GetSymPop()[0] == old_sym);

        world.AddOrgAt(sym, 0);
        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(world.GetSymPop()[0] == sym);
        world.CleanupGraveyard();
      }
    }
    WHEN("a sym is added to an out of bounds pos") {
      THEN("pop and sym_pop are expanded to fit it") {
        REQUIRE(world.GetSymPop().size() == 4);
        world.AddOrgAt(sym, emp::WorldPosition(0,7));
        REQUIRE(world.GetSymPop().size() == world.GetPop().size());
        REQUIRE(world.GetSymPop().size() == 8);
      }
    }
  }
}

TEST_CASE( "GetSymAt", "[default]" ) {
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    int world_size = 4;
    SymWorld world(random, &config);
    world.Resize(world_size);


    WHEN("a request is made for an in-bounds sym") {
      emp::Ptr<Organism> sym1 = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      emp::Ptr<Organism> sym2 = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      world.AddOrgAt(sym1, 0);
      world.AddOrgAt(sym2, emp::WorldPosition(0,1));
      THEN("the sym at that position is returned") {
        REQUIRE(world.GetSymAt(0) == sym1);
        REQUIRE(world.GetSymAt(1) == sym2);
        REQUIRE(world.GetSymAt(2) == nullptr);
      }
    }
    WHEN("a request is made for an out-of-bounds sym") {
      THEN("an exception is thrown") {
        REQUIRE_THROWS(world.GetSymAt(4));
      }
    }
  }
}

TEST_CASE( "DoSymDeath", "[default]" ) {
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigBase config;
    SymWorld world(random, &config);
    int world_size = 4;
    world.Resize(world_size);
    emp::Ptr<Organism> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, 1);
    size_t sym_position = 1;
    world.AddOrgAt(symbiont, emp::WorldPosition(0, sym_position));

    WHEN("A sym is deleted from a position") {
      THEN("It is no longer included in the count of organisms in the world") {
        REQUIRE(world.GetNumOrgs() == 1);
        world.DoSymDeath(sym_position);
        REQUIRE(world.GetNumOrgs() == 0);
      }
      THEN("It no longer occupies a spot in the world") {
        emp::Ptr<Organism> world_sym = world.GetSymAt(sym_position);
        REQUIRE(world_sym == symbiont);
        world.DoSymDeath(sym_position);

        emp::Ptr<Organism> world_sym_deleted = world.GetSymAt(sym_position);
        REQUIRE(world_sym_deleted == nullptr);
      }
    }
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

TEST_CASE( "No mutation updates", "[default] ") {
  GIVEN("a world with 1 mutation update and 1 non-mutation update") {
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
    emp::Ptr<Organism> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
    emp::Ptr<Organism> host = emp::NewPtr<Host>(&random, &world, &config, int_val);

    WHEN("A host and symbiont reproduce at first") {
      emp::Ptr<Organism> mut_sym_baby = symbiont->Reproduce();
      emp::Ptr<Organism> mut_host_baby = host->Reproduce();

      THEN("Mutation rate and size are still 1") {
        REQUIRE(config.MUTATION_RATE() == 1);
        REQUIRE(config.MUTATION_SIZE() == 1);
      }
      THEN("Host and symbiont offspring are mutated") {
        REQUIRE(mut_sym_baby->GetIntVal() > int_val - 0.00001);
        REQUIRE(mut_host_baby->GetIntVal() > int_val - 0.00001);
      }
      mut_sym_baby.Delete();
      mut_host_baby.Delete();
    }
    WHEN("The experiment runs and host and symbiont reproduce after") {
      world.RunExperiment(false);
      emp::Ptr<Organism> no_mut_sym_baby = symbiont->Reproduce();
      emp::Ptr<Organism> no_mut_host_baby = host->Reproduce();

      THEN("Mutation sizes and rates are all 0") {
        REQUIRE(config.MUTATION_RATE() == 0);
        REQUIRE(config.MUTATION_SIZE() == 0);
        REQUIRE(config.HOST_MUTATION_SIZE() == 0);
        REQUIRE(config.HOST_MUTATION_RATE() == 0);
        REQUIRE(config.MUTATE_LYSIS_CHANCE() == 0);
        REQUIRE(config.MUTATE_INDUCTION_CHANCE() == 0);
        REQUIRE(config.MUTATE_INC_VAL() == 0);
      }
      THEN("Host and symbiont offspring aren't mutated") {
        REQUIRE(no_mut_sym_baby->GetIntVal() < int_val + 0.00001);
        REQUIRE(no_mut_sym_baby->GetIntVal() > int_val - 0.00001);
        REQUIRE(no_mut_host_baby->GetIntVal() < int_val + 0.00001);
        REQUIRE(no_mut_host_baby->GetIntVal() > int_val - 0.00001);
      }
      no_mut_sym_baby.Delete();
      no_mut_host_baby.Delete();
    }
    symbiont.Delete();
    host.Delete();
  }
}

TEST_CASE( "IsInboundsPos", "[default]" ) {
  GIVEN("a world") {
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

TEST_CASE("InjectHost", "[default]") {
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigBase config;
    SymWorld world(random, &config);
    int int_val = 0;

    WHEN("Spatial structure is turned off") {
      config.SPATIAL_STRUCT_MODE("well-mixed");
      THEN("Hosts are placed side-by-side in the world") {
        REQUIRE(world.GetNumOrgs() == 0);

        emp::Ptr<Organism> host0 = emp::NewPtr<Host>(&random, &world, &config, int_val);
        emp::Ptr<Organism> host1 = emp::NewPtr<Host>(&random, &world, &config, int_val);
        emp::Ptr<Organism> host2 = emp::NewPtr<Host>(&random, &world, &config, int_val);

        world.InjectHost(host0);
        world.InjectHost(host1);
        world.InjectHost(host2);

        REQUIRE(world.GetNumOrgs() == 3);
        REQUIRE(world.GetPop()[0] == host0);
        REQUIRE(world.GetPop()[1] == host1);
        REQUIRE(world.GetPop()[2] == host2);
      }
    }
    WHEN("Spatial structure is turned on") {
      config.SPATIAL_STRUCT_MODE("grid");
      world.Resize(9); // world should be resized before injecting hosts if spatial structure is on
      THEN("Hosts are placed randomly in the world") {
        REQUIRE(world.GetNumOrgs() == 0);

        emp::Ptr<Organism> host0 = emp::NewPtr<Host>(&random, &world, &config, int_val);
        emp::Ptr<Organism> host1 = emp::NewPtr<Host>(&random, &world, &config, int_val);
        emp::Ptr<Organism> host2 = emp::NewPtr<Host>(&random, &world, &config, int_val);

        world.InjectHost(host0);
        world.InjectHost(host1);
        world.InjectHost(host2);

        REQUIRE(world.GetNumOrgs() == 3);
        bool hosts_sequentially_placed = world.GetPop()[0] == host0 && world.GetPop()[1] == host1 && world.GetPop()[2] == host2;
        REQUIRE(hosts_sequentially_placed == false);
      }
    }
  }
}

TEST_CASE("Setup", "[default]") {
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigBase config;
    SymWorld world(random, &config);

    size_t width = 10;
    size_t height = 20;
    config.WORLD_WIDTH(width);
    config.WORLD_HEIGHT(height);

    WHEN("Grid is on") {
      config.SPATIAL_STRUCT_MODE("grid");
      world.Setup();
      THEN("World size, width, and height are set correctly") {
        REQUIRE(world.GetWidth() == width);
        REQUIRE(world.GetHeight() == height);
        REQUIRE(world.GetSize() == width * height);
      }
    }

    WHEN("Config option POP_SIZE is -1") {
      config.INIT_POP_SIZE(-1);
      world.Setup();
      THEN("The world has full starting population") {
        REQUIRE(world.GetNumOrgs() == width * height);
      }
    }

    WHEN("Config option POP_SIZE is greater than -1") {
      size_t pop_size = (width * height) / 2;
      config.INIT_POP_SIZE(pop_size);
      world.Setup();
      THEN("The world has a partial starting population") {
        REQUIRE(world.GetNumOrgs() == pop_size);
      }
    }

    WHEN("A world is populated with hosts") {
      double smoi = 0.02;
      config.START_MOI(smoi);
      config.SYM_LIMIT(10);
      config.INIT_POP_SIZE(-1);
      emp::DataMonitor<int>& hosted_sym_count_node = world.GetCountHostedSymsDataNode();
      world.Setup();
      world.Update();

      size_t num_syms = hosted_sym_count_node.GetTotal();

      THEN("The world is populated with a proportional number of symbionts") {
        REQUIRE(world.GetNumOrgs() == width * height);
        REQUIRE(num_syms == smoi * width * height);
      }
    }
  }
}

TEST_CASE("SetupSymbionts", "[default]") {
  using sym_world_t = test_utils::TestingWorldWrapper<SymWorld>;
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigBase config;
    sym_world_t world(random, &config);

    size_t world_size = 6;

    WHEN("SetupSymbionts is called and FLS is on") {
      world.Resize(world_size);
      config.FREE_LIVING_SYMS(1);
      size_t num_to_add = 2;
      world.SetupSymbionts(&num_to_add);

      THEN("The specified number of symbionts are added to the world") {
        size_t num_added = world.GetNumOrgs();
        REQUIRE(num_added == num_to_add);

        emp::Ptr<Organism> symbiont;
        for (size_t i = 0; i < world_size; i++) {
          symbiont = world.GetSymAt(i);
          if (symbiont) break;
        }
        REQUIRE(symbiont->GetName() == "Symbiont");
      }
    }
  }
}

TEST_CASE("SetupHosts", "[default]") {
  using sym_world_t = test_utils::TestingWorldWrapper<SymWorld>;
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigBase config;
    sym_world_t world(random, &config);
    // Need to manually setup world spatial structure before adding any hosts/symbionts
    test_utils::SetEmptyWellMixed(config);
    world.SetupSpatialStructure();

    WHEN("SetupHosts is called") {
      size_t num_to_add = 5;


      THEN("The specified number of hosts are added to the world") {
        world.SetupHosts(&num_to_add);

        size_t num_added = world.GetNumOrgs();
        REQUIRE(num_added == num_to_add);

        emp::Ptr<Organism> host = world.GetPop()[0];
        REQUIRE(host != nullptr);
        REQUIRE(host->GetName() == "Host");
      }
    }
  }
}

TEST_CASE("IsSymPopOccupied", "[default]") {
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigBase config;
    test_utils::SetEmptyWellMixed(config);
    SymWorld world(random, &config);
    world.Setup();
    int world_size = 4;
    world.Resize(world_size);
    bool is_sp_occupied;
    int int_val = 0;

    WHEN("The passed location is out of bounds") {
      is_sp_occupied = world.IsSymPopOccupied(world_size);
      THEN("Returns false") {
        REQUIRE(is_sp_occupied == false);
      }
    }
    WHEN("The passed location is in bounds but does not contain a symbiont") {
      is_sp_occupied = world.IsSymPopOccupied(world_size - 1);
      THEN("Returns false") {
        REQUIRE(is_sp_occupied == false);
      }
    }
    WHEN("The passed location is in bounds and contains a symbiont") {
      emp::Ptr<Organism> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      world.AddOrgAt(symbiont, emp::WorldPosition(0, world_size - 1));
      is_sp_occupied = world.IsSymPopOccupied(world_size - 1);
      THEN("Returns true") {
        REQUIRE(is_sp_occupied == true);
      }
    }
  }
}

TEST_CASE("SendToGraveyard", "[default]") {
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigBase config;
    test_utils::SetEmptyWellMixed(config);
    SymWorld world(random, &config);
    world.Setup();
    emp::Ptr<Organism> host = emp::NewPtr<Host>(&random, &world, &config, 0);
    WHEN("SendToGraveyard is called") {
      REQUIRE(world.GetGraveyard().size() == 0);
      THEN("The graveyard increases in size") {
        world.SendToGraveyard(host);
        REQUIRE(world.GetGraveyard().size() == 1);
      }
    }
    host.Delete();
  }
}
