#include "../../default_mode/SymWorld.h"
#include "../../default_mode/Symbiont.h"
#include "../../lysis_mode/Phage.h"
#include "../../lysis_mode/LysisWorld.h"
#include "../../default_mode/Host.h"
#include "../../default_mode/WorldSetup.cc"
#include <fstream>
#include <cstdio>
#include <set>


TEST_CASE("Setup SpatialStructure by config string", "[default]") {
  GIVEN("a world with grid dimensions configured") {
    emp::Random random(17);
    SymConfigBase config;
    SymWorld world(random, &config);
 
    size_t width = 10;
    size_t height = 20;
    config.GRID_X(width);
    config.GRID_Y(height);

    config.POP_SIZE(0);
    config.START_MOI(0);
 
    WHEN("SpatialStructure is \"Grid\"") {
      config.SpatialStructure("Grid");
      world.Setup();
      THEN("the world is sized correctly") {
        REQUIRE(world.GetWidth() == width);
        REQUIRE(world.GetHeight() == height);
        REQUIRE(world.GetSize() == width * height);
      }
    }
 
    WHEN("SpatialStructure is \"WellMixed\"") {
      config.SpatialStructure("WellMixed");
      world.Setup();
      THEN("the world is the right size") {
        REQUIRE(world.GetSize() == width * height);
      }
    }
  }
}
 
 
TEST_CASE("Setup starting population from spatial structure", "[default]") {
  GIVEN("a world with grid dimensions configured") {
    emp::Random random(17);
    SymConfigBase config;
    SymWorld world(random, &config);
 
    size_t width = 5;
    size_t height = 5;
    config.GRID_X(width);
    config.GRID_Y(height);
    config.START_MOI(0);
 
    WHEN("POP_SIZE is -1 and the structure is a Grid") {
      config.SpatialStructure("Grid");
      config.POP_SIZE(-1);
      world.Setup();
      THEN("the world is fully populated") {

        REQUIRE(world.GetNumOrgs() == width * height);
      }
    }
 
    WHEN("POP_SIZE is -1 and the structure is WellMixed") {
      config.SpatialStructure("WellMixed");
      config.POP_SIZE(-1);
      world.Setup();
      THEN("the world is fully populated") {
        
        REQUIRE(world.GetNumOrgs() == width * height);
      }
    }
 
    WHEN("POP_SIZE is set explicitly (> -1)") {
      config.SpatialStructure("Grid");
      size_t pop = (width * height) / 2;
      config.POP_SIZE(pop);
      world.Setup();
      THEN("exactly that many hosts are created, regardless of structure size") {
        REQUIRE(world.GetNumOrgs() == pop);
      }
    }

    WHEN("POP_SIZE is set explicitly (> -1)") {
      config.SpatialStructure("WellMixed");
      size_t pop = (width * height) / 2;
      config.POP_SIZE(pop);
      world.Setup();
      THEN("exactly that many hosts are created, regardless of structure size (wellmixed)") {
        REQUIRE(world.GetNumOrgs() == pop);
      }
    }
  }
}
 
 
TEST_CASE("Grid host births", "[default]") {
  GIVEN("a world set up with a toroidal grid structure") {
    emp::Random random(17);
    SymConfigBase config;
    SymWorld world(random, &config);
 
    size_t width = 10;
    size_t height = 10;
    config.GRID_X(width);
    config.GRID_Y(height);
    config.SpatialStructure("Grid");
    config.POP_SIZE(0);   // start empty so we control placement
    config.START_MOI(0);
    world.Setup();        
 
    //  parent at index 55 (x=5, y=5). Its 4  neighbors:
    //   left=54, right=56, up=45, down=65
    size_t parent_pos = 55;
    std::set<size_t> neighbors = {45, 54, 56, 65};
 
    emp::Ptr<Organism> parent = emp::NewPtr<Host>(&random, &world, &config, 1);
    world.AddOrgAt(parent, parent_pos);
 
    THEN("repeated births all land in one of the 4 von Neumann neighbors") {
      for (int i = 0; i < 12; i++) {
        emp::Ptr<Organism> baby = parent->Reproduce();
        emp::WorldPosition baby_pos = world.DoBirth(baby, parent_pos);
        REQUIRE(baby_pos.IsValid() == true);
        REQUIRE(neighbors.count(baby_pos.GetIndex()) == 1);
      }
    }
  }
}
 
TEST_CASE("LoadFile births respect a custom structure", "[default]") {
  GIVEN("a world set up from a loaded directed-ring structure") {
    emp::Random random(17);
    SymConfigBase config;
    SymWorld world(random, &config);
 
    //a 4x4 connection matrix
    const std::string matrix_path = "./test-spatial-ring.dat";
    {
      std::ofstream out(matrix_path);
      out << "0,1,0,0\n"
          << "0,0,1,0\n"
          << "0,0,0,1\n"
          << "1,0,0,0\n";
    }
 

    config.SpatialStructure("LoadFile");
    config.LoadFile(matrix_path);
    config.POP_SIZE(0);   // start empty so we control placement
    config.START_MOI(0);
    world.Setup();        
 
    THEN("the world is sized to the loaded structure") {
      REQUIRE(world.GetSize() == 4);
    }
 
    THEN("a host at position 0 always reproduces into its single neighbor, 1") {
      emp::Ptr<Organism> parent = emp::NewPtr<Host>(&random, &world, &config, 1);
      world.AddOrgAt(parent, 0);
 
      emp::Ptr<Organism> baby = parent->Reproduce();
      emp::WorldPosition baby_pos = world.DoBirth(baby, 0);
 
      REQUIRE(baby_pos.IsValid() == true);
      REQUIRE(baby_pos.GetIndex() == 1);
      REQUIRE(world.GetPop()[1] == baby);
    }
 
  }
}

TEST_CASE("LoadFile undirected line graph", "[default]") {
  GIVEN("an undirected line 0-1-2-3 loaded from a symmetric matrix") {
    emp::Random random(17);
    SymConfigBase config;
    SymWorld world(random, &config);
    const std::string path = "./test-spatial-line.dat";
 
    
    SetupWorldFromMatrixFile(world, config, path,
      "0,1,0,0\n"  
      "1,0,1,0\n"  
      "0,1,0,1\n"  
      "0,0,1,0\n",  
      4);
 
    WHEN("a host at endpoint 0 reproduces") {
      THEN("every child lands at its single neighbor, 1") {
        for (int i = 0; i < 10; i++) {
          emp::Ptr<Organism> parent = emp::NewPtr<Host>(&random, &world, &config, 1);
          world.AddOrgAt(parent, 0);
          emp::Ptr<Organism> baby = parent->Reproduce();
          emp::WorldPosition pos = world.DoBirth(baby, 0);
          REQUIRE(pos.IsValid() == true);
          REQUIRE(pos.GetIndex() == 1);
        }
      }
    }

    WHEN("a host at endpoint 3 reproduces") {
      THEN("every child lands at its single neighbor, 2") {
        for (int i = 0; i < 10; i++) {
          emp::Ptr<Organism> parent = emp::NewPtr<Host>(&random, &world, &config, 1);
          world.AddOrgAt(parent, 3);
          emp::Ptr<Organism> baby = parent->Reproduce();
          emp::WorldPosition pos = world.DoBirth(baby, 3);
          REQUIRE(pos.IsValid() == true);
          REQUIRE(pos.GetIndex() == 2);
        }
      }
    }

    WHEN("a host at middle position 1 reproduces") {
      THEN("every child lands at one of its two neighbors, 0 or 2") {
        std::set<size_t> neighbors = {0, 2};
        for (int i = 0; i < 10; i++) {
          emp::Ptr<Organism> parent = emp::NewPtr<Host>(&random, &world, &config, 1);
          world.AddOrgAt(parent, 1);
          emp::Ptr<Organism> baby = parent->Reproduce();
          emp::WorldPosition pos = world.DoBirth(baby, 1);
          REQUIRE(pos.IsValid() == true);
          REQUIRE(neighbors.count(pos.GetIndex()) == 1);
        }
      }
    }

    WHEN("a host at middle position 2 reproduces") {
      THEN("every child lands at one of its two neighbors, 1 or 3") {
        std::set<size_t> neighbors = {1, 3};
        for (int i = 0; i < 10; i++) {
          emp::Ptr<Organism> parent = emp::NewPtr<Host>(&random, &world, &config, 1);
          world.AddOrgAt(parent, 2);
          emp::Ptr<Organism> baby = parent->Reproduce();
          emp::WorldPosition pos = world.DoBirth(baby, 2);
          REQUIRE(pos.IsValid() == true);
          REQUIRE(neighbors.count(pos.GetIndex()) == 1);
        }
      }
    }
  }


}

TEST_CASE("isolated pos neither reproduces nor receives offspring", "[default]") {
  GIVEN("A world with a custom structure that isolates position 0") {
    emp::Random random(17);
    SymConfigBase config;
    SymWorld world(random, &config);
    const std::string path = "./test-spatial-isolated-zero.dat";
 
    
    {
      std::ofstream out(path);
      out << "0,0,0,0\n"   
          << "0,0,1,1\n"   
          << "0,1,0,1\n"   
          << "0,1,1,0\n";  
    }
   
    config.SpatialStructure("LoadFile");
    config.LoadFile(path);
    config.POP_SIZE(0);
    config.START_MOI(0);
    world.Setup();
 
    WHEN("a host at the isolated position 0 reproduces") {
      emp::Ptr<Organism> host_parent = emp::NewPtr<Host>(&random, &world, &config, 1);
      world.AddOrgAt(host_parent, 0);
 
      emp::Ptr<Organism> host_baby = host_parent->Reproduce();
      emp::WorldPosition new_pos = world.DoBirth(host_baby, 0);
 
      THEN("the offspring has no valid position to be born into") {
        REQUIRE(new_pos.IsValid() == false);
      }
    }
 
    WHEN("hosts at the connected positions 1, 2, and 3 reproduce") {
      size_t parent_positions[3] = {1, 2, 3};
      int neighbors[3][2] = { {2, 3}, {1, 3}, {1, 2} };
 
      THEN("each baby is born at a neighbor, never at the isolated position 0") {
        for (int p = 0; p < 3; p++) {
          emp::Ptr<Organism> host_parent = emp::NewPtr<Host>(&random, &world, &config, 1);
          world.AddOrgAt(host_parent, parent_positions[p]);
 
          emp::Ptr<Organism> host_baby = host_parent->Reproduce();
          world.DoBirth(host_baby, parent_positions[p]);
 
          bool found_baby = false;
          for (int i = 0; i < 2; i++) {
            if (world.GetPop()[neighbors[p][i]] == host_baby) {
              found_baby = true;
            }
          }
          REQUIRE(found_baby == true);
          REQUIRE(world.GetPop()[0] != host_baby);
        }
      }
    }
  }
}

TEST_CASE("LoadFile hosted symbiont transmits only into structural neighbor hosts", "[default]") {
  GIVEN("a world loaded from a structure where cell 1's only neighbor is cell 0") {
    emp::Random random(17);
    SymConfigBase config;
    SymWorld world(random, &config);
    const std::string path = "./test-spatial-ht-neighbor.dat";
 
    size_t sym_limit = 4;
    config.FREE_LIVING_SYMS(0);
    config.SYM_HORIZ_TRANS_RES(0);
    config.SYM_LIMIT(sym_limit);
 

    {
      std::ofstream out(path);
      out << "0,1,0,0\n"   
          << "1,0,0,0\n"  
          << "0,0,0,1\n"   
          << "0,0,1,0\n";  
    }
    
    config.SpatialStructure("LoadFile");
    config.LoadFile(path);
    config.POP_SIZE(0);
    config.START_MOI(0);
    world.Setup();
 
    WHEN("a hosted symbiont repeatedly transmits horizontally") {
      emp::Ptr<Organism> host_parent = emp::NewPtr<Host>(&random, &world, &config, 1);
      emp::Ptr<Organism> neighboring_host = emp::NewPtr<Host>(&random, &world, &config, 1);
      emp::Ptr<Organism> distant_host = emp::NewPtr<Host>(&random, &world, &config, 1);
      emp::Ptr<Organism> sym_parent = emp::NewPtr<Symbiont>(&random, &world, &config, 1);
 
      size_t host_parent_pos = 1;
      world.AddOrgAt(host_parent, host_parent_pos);
      world.AddOrgAt(neighboring_host, 0);   // cell 0 is host_parent's only neighbor
      world.AddOrgAt(distant_host, 2);       // cell 2 is not connected to cell 1
      host_parent->AddSymbiont(sym_parent);
 
      THEN("offspring only enter the neighboring host, never the distant host") {
        for (size_t sym_count = 1; sym_count <= sym_limit; sym_count++) {
          sym_parent->HorizontalTransmission(emp::WorldPosition(1, host_parent_pos));
          REQUIRE(neighboring_host->GetSymbionts().size() == sym_count);
          REQUIRE(distant_host->HasSym() == false);
        }
      }
    }
 
    std::remove(path.c_str());
  }
}
