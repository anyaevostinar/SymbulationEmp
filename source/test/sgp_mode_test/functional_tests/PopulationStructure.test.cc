/*
  These tests are adapted from tests for custom population structure in
    default_mode_test/PopulationStructure.test.cc.
  Rationale: sgp mode overrides many SymWorld functions, so it's valuable to
    check that custom spatial structure behaves as expected (i.e., consistent
    with SymWorld)
*/

#include "../../test_utils.h"

#include "../../../default_mode/SymWorld.h"
#include "../../../default_mode/WorldSetup.cc"
#include "../../../default_mode/DataNodes.h"
#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorld.cc"
#include "../../../sgp_mode/SGPWorldSetup.cc"
#include "../../../sgp_mode/SGPWorldData.cc"
#include "../../../sgp_mode/SGPW_InteractionMechanismSetup.cc"
#include "../../../sgp_mode/SGPW_TaskProfileSetup.cc"
#include "../../../sgp_mode/ProgramBuilder.h"

#include "../../../catch/catch.hpp"


TEST_CASE( "Spatial structure grid mode (sgp mode)", "[sgp][spatial-structure]" ) {
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
  using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;
  GIVEN("a world") {
    emp::Random random(17);
    sgpmode::SymConfigSGP config;
    world_t world(random, &config);
    const size_t width = 100;
    const size_t height = 100;
    config.WORLD_WIDTH(width);
    config.WORLD_HEIGHT(height);
    config.TASK_IO_BANK_SIZE(1);
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.FREE_LIVING_SYMS(1);
    config.MOVE_FREE_SYMS(1);
    config.SYM_HORIZ_TRANS_RES(0);
    config.HORIZONTAL_TRANSMISSION_COMPATIBILITY_MODE("always");
    size_t sym_limit = 10;
    config.SYM_LIMIT(sym_limit);

    WHEN("Grid is on") {
      config.INIT_POP_SIZE(0);
      config.SPATIAL_STRUCT_MODE("grid");
      world.Setup();
      auto& prog_builder = world.GetProgramBuilder();

      THEN("the world is sized widthxheight") {
        REQUIRE(world.GetWidth() == width);
        REQUIRE(world.GetHeight() == height);
        REQUIRE(world.GetSize() == width * height);
      }

      THEN("Host offspring are born next to their parents") {
        emp::Ptr<Organism> host_parent = emp::NewPtr<sgp_host_t>(
          &random,
          &world,
          &config,
          prog_builder.CreateReproProgram(10)
        );
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
    }

    WHEN("Grid is on and free living symbionts are permitted") {
      config.INIT_POP_SIZE(0);
      config.FREE_LIVING_SYMS(1);
      config.MOVE_FREE_SYMS(1);
      config.SPATIAL_STRUCT_MODE("grid");
      world.Setup();
      THEN("Symbiont babies are horizontally transmitted to a position near their parents") {
        emp::Ptr<Organism> sym_parent = emp::NewPtr<sgp_sym_t>(&random, &world, &config, 1);
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
        emp::Ptr<Organism> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config, 1);
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
      config.INIT_POP_SIZE(0);
      config.FREE_LIVING_SYMS(0);
      config.MOVE_FREE_SYMS(1);
      config.SPATIAL_STRUCT_MODE("grid");
      config.CYCLES_PER_UPDATE(0);
      config.HORIZ_TRANS(1);
      config.SYM_HORIZ_TRANS_RES(0);
      config.FIND_NEIGHBOR_HOST_ATTEMPTS(40); // Make sure we sample enough neighbors
      world.Setup();
      THEN("Symbionts are horizontally transmitted into a neighboring host") {
        emp::Ptr<sgp_host_t> host_parent = emp::NewPtr<sgp_host_t>(&random, &world, &config, 1);
        emp::Ptr<sgp_host_t> neighboring_host = emp::NewPtr<sgp_host_t>(&random, &world, &config, 1);
        emp::Ptr<sgp_host_t> distant_host = emp::NewPtr<sgp_host_t>(&random, &world, &config, 1);
        emp::Ptr<sgp_sym_t> sym_parent = emp::NewPtr<sgp_sym_t>(&random, &world, &config, 1);

        size_t host_parent_pos = 0;
        world.AddOrgAt(host_parent, host_parent_pos);
        world.AddOrgAt(neighboring_host, host_parent_pos + 1);
        world.AddOrgAt(distant_host, host_parent_pos + 150);
        host_parent->AddSymbiont(sym_parent);
        REQUIRE(host_parent->GetSymbionts().size() == 1);
        REQUIRE(neighboring_host->GetSymbionts().size() == 0);
        REQUIRE(distant_host->GetSymbionts().size() == 0);

        sym_parent->AttemptIndependentReproduction(emp::WorldPosition(1, host_parent_pos));
        world.Update(); // Process reproduction queue
        REQUIRE(neighboring_host->GetSymbionts().size() == 1);
        REQUIRE(distant_host->HasSym() == false);
      }
    }

    WHEN("Population is well-mixed") {
      config.INIT_POP_SIZE(0);
      config.SPATIAL_STRUCT_MODE("well-mixed");
      world.Setup();
      auto& prog_builder = world.GetProgramBuilder();

      THEN("the world is sized as width * height") {
        REQUIRE(world.GetSize() == width * height);
      }

      //given the size of the world, it's very unlikely that
      //organisms will randomly be placed in a neighbor position
      THEN("Host babies are born into a random position anywhere in the world") {
        emp::Ptr<Organism> host_parent = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateReproProgram(10));
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

    }
    WHEN("Population is well-mixed and free living symbionts are permitted") {
      config.INIT_POP_SIZE(0);
      config.SPATIAL_STRUCT_MODE("well-mixed");
      config.FREE_LIVING_SYMS(1);
      config.MOVE_FREE_SYMS(1);
      config.CYCLES_PER_UPDATE(0);
      config.HORIZ_TRANS(1);
      config.SYM_HORIZ_TRANS_RES(0);
      world.Setup();

      THEN("Symbiont babies are horizontally transmitted to a random position anywhere in the world") {
        emp::Ptr<Organism> sym_parent = emp::NewPtr<sgp_sym_t>(&random, &world, &config, 1);
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
        emp::Ptr<Organism> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config, 1);
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
    WHEN("Population is well-mixed and free living symbionts are not permitted") {
      config.INIT_POP_SIZE(0);
      config.SPATIAL_STRUCT_MODE("well-mixed");
      config.FREE_LIVING_SYMS(0);
      config.MOVE_FREE_SYMS(1);
      config.CYCLES_PER_UPDATE(0);
      config.HORIZ_TRANS(1);
      config.SYM_HORIZ_TRANS_RES(0);
      config.WORLD_WIDTH(5);
      config.WORLD_HEIGHT(5);
      config.FIND_NEIGHBOR_HOST_ATTEMPTS(40); // Make sure we sample enough neighbors
      world.Setup();
      THEN("Symbionts are horizontally transmitted into hosts anywhere in the world") {
        emp::Ptr<sgp_host_t> host_parent = emp::NewPtr<sgp_host_t>(&random, &world, &config, 1);
        emp::Ptr<sgp_host_t> distant_host = emp::NewPtr<sgp_host_t>(&random, &world, &config, 1);
        emp::Ptr<sgp_sym_t> sym_parent = emp::NewPtr<sgp_sym_t>(&random, &world, &config, 1);

        size_t host_parent_pos = 0;
        world.AddOrgAt(host_parent, host_parent_pos);
        world.AddOrgAt(distant_host, host_parent_pos + 3);
        host_parent->AddSymbiont(sym_parent);
        REQUIRE(host_parent->HasSym());
        sym_parent->AttemptIndependentReproduction(emp::WorldPosition(1, host_parent_pos));
        world.Update();
        REQUIRE(host_parent->HasSym());
        REQUIRE(distant_host->HasSym());
      }
    }
  }
}

TEST_CASE("Spatial structure loaded from files (sgp mode)", "[sgp][spatial-structure]") {
  emp::Random random(17);
  sgpmode::SymConfigSGP config;
  // NOTE: width x height aren't used for spatial structure mode (will check this)
  const size_t width = 100;
  const size_t height = 100;
  config.WORLD_WIDTH(width);
  config.WORLD_HEIGHT(height);
  config.FREE_LIVING_SYMS(1);
  config.MOVE_FREE_SYMS(1);
  config.SYM_HORIZ_TRANS_RES(0);
  config.TASK_IO_BANK_SIZE(1);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  size_t sym_limit = 10;
  config.SYM_LIMIT(sym_limit);
  config.INIT_POP_SIZE(0);
  sgpmode::SGPWorld world(random, &config);

  GIVEN("a world") {
    WHEN("in load spatial structure mode") {
      config.SPATIAL_STRUCT_MODE("load");
      THEN("load from edges csv should work as expected") {
        config.SPATIAL_STRUCT_LOAD_MODE("edges");
        config.SPATIAL_STRUCT_CFG_PATH("source/test/data/spatial-structure-edges.csv");
        world.Setup();
        REQUIRE(world.IsCustomPopStruct());
        REQUIRE(world.GetSize() == 5);
        const auto& structure = world.GetCustomPopStructure();
        const emp::vector<emp::vector<size_t>> expected_connections = {
          /* 0: */ {1, 2, 3},
          /* 1: */ {0, 2},
          /* 2: */ {0},
          /* 3: */ {},
          /* 4: */ {}
        };
        // Check connections
        REQUIRE(test_utils::spatial::VerifyStructure(expected_connections, structure));
      }
      THEN("load from matrix file should work as expected") {
        config.SPATIAL_STRUCT_LOAD_MODE("matrix");
        config.SPATIAL_STRUCT_CFG_PATH("source/test/data/spatial-structure-matrix.dat");
        world.Setup();
        REQUIRE(world.IsCustomPopStruct());
        REQUIRE(world.GetSize() == 5);
        const auto& structure = world.GetCustomPopStructure();
        const emp::vector<emp::vector<size_t>> expected_connections = {
          /* 0: */ {1, 2, 3},
          /* 1: */ {0, 2},
          /* 2: */ {0},
          /* 3: */ {},
          /* 4: */ {}
        };
        // Check connections
        REQUIRE(test_utils::spatial::VerifyStructure(expected_connections, structure));
      }
    }
  }
}

TEST_CASE("World uses custom spatial structure (sgp mode)", "[sgp][spatial-structure]") {
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
  using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;
  emp::Random random(17);
  sgpmode::SymConfigSGP config;
  // NOTE: width x height aren't used for spatial structure mode (will check this)
  const size_t width = 100;
  const size_t height = 100;
  size_t sym_limit = 10;
  config.WORLD_WIDTH(width);
  config.WORLD_HEIGHT(height);
  config.FREE_LIVING_SYMS(0);
  config.START_MOI(0);
  config.MOVE_FREE_SYMS(1);
  config.SYM_HORIZ_TRANS_RES(0);
  config.SYM_LIMIT(sym_limit);
  config.TASK_IO_BANK_SIZE(1);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.SPATIAL_STRUCT_MODE("load");
  config.SPATIAL_STRUCT_LOAD_MODE("edges");
  config.SPATIAL_STRUCT_CFG_PATH("source/test/data/chain-edges.csv");
  const emp::vector<emp::vector<size_t>> expected_connections = {
    /* 0: */ {1, 4},
    /* 1: */ {0, 2},
    /* 2: */ {1, 3},
    /* 3: */ {2, 4},
    /* 4: */ {0, 3}
  };
  GIVEN("a world with custom spatial structure") {
    world_t world(random, &config);
    THEN("population initializations should work as expected") {
      WHEN("INIT_POP_SIZE = -1 should initialize a full population") {
        config.INIT_POP_SIZE(-1);
        world.Setup(); // Setup spatial structure + initial population
        REQUIRE(world.IsCustomPopStruct());
        REQUIRE(world.GetSize() == 5);
        REQUIRE(world.GetNumOrgs() == 5);
      }
      WHEN("INIT_POP_SIZE != -1 should initialize population accordingly") {
        config.INIT_POP_SIZE(3);
        world.Setup();
        REQUIRE(world.GetSize() == 5);
        REQUIRE(world.GetNumOrgs() == 3);
      }
    }
    THEN("spatial structure should be used for neighbors") {
      config.INIT_POP_SIZE(-1);
      world.Setup();
      const auto& structure = world.GetCustomPopStructure();
      for (size_t pop_i = 0; pop_i < world.GetSize(); ++pop_i) {
        emp::vector<size_t> world_neighbors(world.GetValidNeighborOrgIDs(pop_i));
        std::sort(world_neighbors.begin(), world_neighbors.end());
        REQUIRE(structure.GetNeighbors(pop_i) == expected_connections[pop_i]);
        REQUIRE(world_neighbors == expected_connections[pop_i]);
      }
    }
    WHEN("hosts reproduce") {
      config.INIT_POP_SIZE(0);
      world.Setup();
      size_t parent_pos = 2;
      std::set<size_t> neighbors = {1, 3};
      emp::Ptr<Organism> parent = emp::NewPtr<sgp_host_t>(&random, &world, &config, 0);
      world.AddOrgAt(parent, parent_pos);
      THEN("offspring should be placed in neighboring locations") {
        // Repeated births should land in valid neighbor positions
        for (size_t i = 0; i < 20; ++i) {
          emp::Ptr<Organism> offspring = parent->Reproduce();
          emp::WorldPosition offspring_pos = world.DoBirth(offspring, parent_pos);
          REQUIRE(offspring_pos.IsValid());
          REQUIRE(emp::Has(neighbors, offspring_pos.GetIndex()));
        }
      }
    }
    WHEN("a hosted symbiont transmits horizontally") {
      config.INIT_POP_SIZE(0);
      size_t sym_limit = 4;
      config.SYM_LIMIT(sym_limit);
      world.Setup();
      emp::Ptr<sgp_host_t> host_parent = emp::NewPtr<sgp_host_t>(&random, &world, &config, 1);
      emp::Ptr<sgp_host_t> neighboring_host = emp::NewPtr<sgp_host_t>(&random, &world, &config, 1);
      emp::Ptr<sgp_host_t> distant_host = emp::NewPtr<sgp_host_t>(&random, &world, &config, 1);
      emp::Ptr<sgp_sym_t> sym_parent = emp::NewPtr<sgp_sym_t>(&random, &world, &config, 1);
      // Chain structure: 0-1-2-3-4-0
      size_t host_parent_pos = 1;
      world.AddOrgAt(host_parent, host_parent_pos);
      world.AddOrgAt(neighboring_host, 0);   // Location 0 is connected to location 1
      world.AddOrgAt(distant_host, 3);       // Location 3 is not connected to location 1
      // host loc: 1, 0
      REQUIRE(world.GetCustomPopStructure().IsConnected(1, 0));
      REQUIRE(!world.GetCustomPopStructure().IsConnected(1, 3));
      // Parent sym should be set to: emp::WorldPosition(syms.size(), location.GetIndex())
      host_parent->AddSymbiont(sym_parent);
      THEN("offspring only enter the neighboring host, never the distant host") {
        REQUIRE(neighboring_host->GetSymbionts().size() == 0);
        REQUIRE(distant_host->GetSymbionts().size() == 0);
        size_t neighbor_sym_count = 0;
        for (size_t sym_count = 0; sym_count < sym_limit; sym_count++) {
          emp::Ptr<Organism> sym_offspring = sym_parent->Reproduce();
          world.SymDoBirth(sym_offspring, sym_parent->GetLocation());
          ++neighbor_sym_count;
          REQUIRE(neighboring_host->GetSymbionts().size() == neighbor_sym_count);
          REQUIRE(!distant_host->HasSym());
        }
      }
    }
    WHEN("freeliving symbionts are moved") {
      config.INIT_POP_SIZE(0);
      config.FREE_LIVING_SYMS(1);
      config.MOVE_FREE_SYMS(1);
      world.Setup();
      REQUIRE(world.GetSymPop().size() == 5);
      size_t world_pos = 0;
      emp::WorldPosition sym_pos = emp::WorldPosition(0, world_pos);
      THEN("freeliving symbionts should move into valid neighboring locations") {
        emp::Ptr<Organism> sym = emp::NewPtr<sgp_sym_t>(&random, &world, &config, 0);
        world.AddOrgAt(sym, sym_pos);
        world.MoveFreeSym(sym_pos);
        // look for where the sym moved to, but shouldn't be original cell
        const bool found_sym = world.GetSymPop()[4] == sym || world.GetSymPop()[1] == sym;
        REQUIRE(found_sym);
        size_t new_sym_pos = (world.GetSymPop()[4] == sym) ? 4 : 1;
        for (size_t pos_i = 0; pos_i < world.GetSymPop().size(); ++pos_i) {
          if (pos_i == new_sym_pos) continue;
          REQUIRE(world.GetSymPop()[pos_i] == nullptr);
        }
      }
    }
  }
}

TEST_CASE("Organism in isolated position neither reproduces nor receives offspring (sgp mode)", "[sgp][spatial-structure]") {
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;

  GIVEN("A world with a custom structure that isolates position 0") {
    emp::Random random(17);
    sgpmode::SymConfigSGP config;
    config.FREE_LIVING_SYMS(0);
    config.START_MOI(0);
    config.MOVE_FREE_SYMS(1);
    config.SYM_HORIZ_TRANS_RES(0);
    config.SPATIAL_STRUCT_MODE("load");
    config.SPATIAL_STRUCT_LOAD_MODE("matrix");
    config.SPATIAL_STRUCT_CFG_PATH("source/test/data/isolated-pos-0-matrix.dat");
    config.INIT_POP_SIZE(0);
    config.START_MOI(0);
    config.TASK_IO_BANK_SIZE(1);
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    world_t world(random, &config);
    world.Setup();

    WHEN("a host at the isolated position 0 reproduces") {
      emp::Ptr<Organism> host_parent = emp::NewPtr<sgp_host_t>(&random, &world, &config, 1);
      world.AddOrgAt(host_parent, 0);

      emp::Ptr<Organism> host_offspring = host_parent->Reproduce();
      emp::WorldPosition new_pos = world.DoBirth(host_offspring, 0);
      THEN("the offspring has no valid position to be born into") {
        REQUIRE(new_pos.IsValid() == false);
      }
    }

    WHEN("hosts at the connected positions 1, 2, and 3 reproduce") {
      size_t parent_positions[3] = {1, 2, 3};
      int neighbors[3][2] = { {2, 3}, {1, 3}, {1, 2} };

      THEN("each offspring is born at a neighbor, never at the isolated position 0") {
        for (int p = 0; p < 3; p++) {
          emp::Ptr<Organism> host_parent = emp::NewPtr<sgp_host_t>(&random, &world, &config, 1);
          world.AddOrgAt(host_parent, parent_positions[p]);

          emp::Ptr<Organism> host_offspring = host_parent->Reproduce();
          world.DoBirth(host_offspring, parent_positions[p]);

          bool found_offspring = false;
          for (int i = 0; i < 2; i++) {
            if (world.GetPop()[neighbors[p][i]] == host_offspring) {
              found_offspring = true;
            }
          }
          REQUIRE(found_offspring == true);
          REQUIRE(world.GetPop()[0] != host_offspring);
        }
      }
    }
  }
}
