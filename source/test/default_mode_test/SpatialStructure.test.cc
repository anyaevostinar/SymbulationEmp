#include "../test_utils.h"
#include "../../default_mode/SymWorld.h"
#include "../../default_mode/Symbiont.h"
#include "../../lysis_mode/Phage.h"
#include "../../lysis_mode/LysisWorld.h"
#include "../../default_mode/Host.h"
#include "../../default_mode/WorldSetup.cc"

TEST_CASE( "Spatial structure grid mode", "[default][spatial-structure]" ) {
  using sym_world_t = test_utils::TestingWorldWrapper<SymWorld>;
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigBase config;
    sym_world_t world(random, &config);
    int width = 100;
    int height = 100;
    config.WORLD_WIDTH(width);
    config.WORLD_HEIGHT(height);
    world.Resize(width * height);
    config.FREE_LIVING_SYMS(1);
    config.MOVE_FREE_SYMS(1);
    config.SYM_HORIZ_TRANS_RES(0);
    size_t sym_limit = 10;
    config.SYM_LIMIT(sym_limit);

    WHEN("Grid is on") {
      config.SPATIAL_STRUCT_MODE("grid");
      world.SetupSpatialStructure();
      // world.SetPopStruct_Grid(width, height, false);
      // world.Setup();

      THEN("Host offspring are born next to their parents") {
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
        for (int i = 0; i < 8; i++) {
          if (world.GetPop()[possible_indices[i]] == host_baby) {
            found_baby = true;
          }
        }
        REQUIRE(found_baby == true);
      }

      WHEN("Free living symbionts are permitted") {
        THEN("Symbiont babies are horizontally transmitted to a position near their parents") {
          emp::Ptr<Organism> sym_parent = emp::NewPtr<Symbiont>(&random, &world, &config, 1);
          emp::WorldPosition sym_parent_pos = emp::WorldPosition(0, 250);
          world.AddOrgAt(sym_parent, sym_parent_pos);

          sym_parent->IndependentReproduction(sym_parent_pos);

          int possible_indices[8] = {149, 150, 151, 249, 251, 349, 350, 351};
          bool found_baby = false;
          for (int i = 0; i < 8; i++) {
            if (world.GetSymPop()[possible_indices[i]] != nullptr && world.GetSymPop()[possible_indices[i]] != sym_parent) {
              found_baby = true;
            }
          }
          REQUIRE(found_baby == true);
        }

        THEN("Symbionts randomly move to cells near their old position") {
          emp::Ptr<Organism> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, 1);
          emp::WorldPosition original_position = emp::WorldPosition(0, 898);
          world.AddOrgAt(symbiont, original_position);
          world.MoveFreeSym(original_position);

          int possible_indices[8] = {797, 798, 799, 897, 899, 997, 998, 999};
          bool found_sym = false;
          for (int i = 0; i < 8; i++) {
            if (world.GetSymPop()[possible_indices[i]] == symbiont) {
              found_sym = true;
            }
          }
          REQUIRE(found_sym == true);
        }
      }

      WHEN("Free living symbionts are not permitted") {
        config.FREE_LIVING_SYMS(0);
        THEN("Symbionts are horizontally transmitted into a neighboring host") {
          emp::Ptr<Organism> host_parent = emp::NewPtr<Host>(&random, &world, &config, 1);
          emp::Ptr<Organism> neighboring_host = emp::NewPtr<Host>(&random, &world, &config, 1);
          emp::Ptr<Organism> distant_host = emp::NewPtr<Host>(&random, &world, &config, 1);
          emp::Ptr<Organism> sym_parent = emp::NewPtr<Symbiont>(&random, &world, &config, 1);

          size_t host_parent_pos = 99;
          world.AddOrgAt(host_parent, host_parent_pos);
          world.AddOrgAt(neighboring_host, host_parent_pos + 1);
          world.AddOrgAt(distant_host, host_parent_pos + 150);
          host_parent->AddSymbiont(sym_parent);

          for (size_t sym_count = 1; sym_count <= sym_limit; sym_count++) {
            sym_parent->IndependentReproduction(emp::WorldPosition(1, host_parent_pos));
            REQUIRE(neighboring_host->GetSymbionts().size() == sym_count);
            REQUIRE(distant_host->HasSym() == false);
          }
        }
      }
    }

    WHEN("Grid is off, and population is well-mixed") {
      config.SPATIAL_STRUCT_MODE("well-mixed");
      // world.SetPopStruct_Mixed(false);
      // world.Setup();
      world.SetupSpatialStructure();
      //given the size of the world, it's very unlikely that
      //organisms will randomly be placed in a neighbor position
      THEN("Host babies are born into a random position anywhere in the world") {
        emp::Ptr<Organism> host_parent = emp::NewPtr<Host>(&random, &world, &config, 1);
        size_t host_parent_pos = 101;
        world.AddOrgAt(host_parent, host_parent_pos);

        emp::Ptr<Organism> host_baby = host_parent->Reproduce();
        world.DoBirth(host_baby, host_parent_pos);

        int possible_indices[8] = {0, 1, 2, 100, 102, 200, 201, 202};
        bool found_baby = false;
        for (int i = 0; i < 8; i++) {
          if (world.GetPop()[possible_indices[i]] == host_baby) {
            found_baby = true;
          }
        }
        REQUIRE(found_baby == false);
      }

      WHEN("Free living symbionts are permitted") {
        THEN("Symbiont babies are horizontally transmitted to a random position anywhere in the world") {
          emp::Ptr<Organism> sym_parent = emp::NewPtr<Symbiont>(&random, &world, &config, 1);
          emp::WorldPosition sym_parent_pos = emp::WorldPosition(0, 250);
          world.AddOrgAt(sym_parent, sym_parent_pos);

          sym_parent->IndependentReproduction(sym_parent_pos);


          int possible_indices[8] = {149, 150, 151, 249, 251, 349, 350, 351};
          bool found_baby = false;
          for (int i = 0; i < 8; i++) {
            if (world.GetSymPop()[possible_indices[i]] != nullptr && world.GetSymPop()[possible_indices[i]] != sym_parent) {
              found_baby = true;
            }
          }

          REQUIRE(found_baby == false);
        }
        THEN("Symbionts randomly move to cells anywhere in the world") {
          emp::Ptr<Organism> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, 1);
          emp::WorldPosition original_position = emp::WorldPosition(0, 898);
          world.AddOrgAt(symbiont, original_position);
          world.MoveFreeSym(original_position);

          int possible_indices[8] = {797, 798, 799, 897, 899, 997, 998, 999};
          bool found_sym = false;
          for (int i = 0; i < 8; i++) {
            if (world.GetSymPop()[possible_indices[i]] == symbiont) {
              found_sym = true;
            }
          }

          REQUIRE(found_sym == false);
        }
      }
      WHEN("Free living symbionts are not permitted") {
        config.FREE_LIVING_SYMS(0);
        THEN("Symbionts are horizontally transmitted into hosts anywhere in the world") {
          emp::Ptr<Organism> host_parent = emp::NewPtr<Host>(&random, &world, &config, 1);
          emp::Ptr<Organism> neighboring_host = emp::NewPtr<Host>(&random, &world, &config, 1);
          emp::Ptr<Organism> distant_host = emp::NewPtr<Host>(&random, &world, &config, 1);
          emp::Ptr<Organism> sym_parent = emp::NewPtr<Symbiont>(&random, &world, &config, 1);

          size_t host_parent_pos = 99;
          world.AddOrgAt(host_parent, host_parent_pos);
          world.AddOrgAt(neighboring_host, host_parent_pos + 1);
          world.AddOrgAt(distant_host, host_parent_pos + 150);
          host_parent->AddSymbiont(sym_parent);

          for (size_t sym_count = 1; sym_count <= sym_limit; sym_count++) {
            sym_parent->IndependentReproduction(emp::WorldPosition(1, host_parent_pos));
          }

          REQUIRE(neighboring_host->HasSym());
          REQUIRE(distant_host->HasSym());
        }
      }
    }
  }
}