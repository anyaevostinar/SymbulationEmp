#include "../test_utils.h"

#include "../../catch/catch.hpp"
#include "../../default_mode/SpatialStructure.h"

#include "emp/base/vector.hpp"
#include "emp/math/Random.hpp"

#include <iostream>

TEST_CASE("Can define spatial structure from a connection mapping", "[spatial-structure],[default]") {
  SpatialStructure ring_structure;

  emp::vector< emp::vector<size_t> > ring_map = {
    /* 0 -> */ {1},
    /* 1 -> */ {2},
    /* 2 -> */ {0}
  };
  ring_structure.SetStructure(ring_map);
  // ring_structure.Print(std::cout);
  // ring_structure.Print(std::cout, false);

  REQUIRE(!ring_structure.IsConnected(0, 0));
  REQUIRE(ring_structure.IsConnected(0, 1));
  REQUIRE(!ring_structure.IsConnected(0, 2));

  REQUIRE(!ring_structure.IsConnected(1, 0));
  REQUIRE(!ring_structure.IsConnected(1, 1));
  REQUIRE(ring_structure.IsConnected(1, 2));

  REQUIRE(ring_structure.IsConnected(2, 0));
  REQUIRE(!ring_structure.IsConnected(2, 1));
  REQUIRE(!ring_structure.IsConnected(2, 2));

}

TEST_CASE("Can define spatial structure from a connection matrix", "[spatial-structure],[default]") {
  SpatialStructure ring_structure;

  emp::vector< emp::vector<bool> > ring_matrix = {
    { 0, 1, 0 },
    { 0, 0, 1 },
    { 1, 0, 0 }
  };
  ring_structure.SetStructure(ring_matrix);
  // ring_structure.Print(std::cout);

  REQUIRE(!ring_structure.IsConnected(0, 0));
  REQUIRE(ring_structure.IsConnected(0, 1));
  REQUIRE(!ring_structure.IsConnected(0, 2));

  REQUIRE(!ring_structure.IsConnected(1, 0));
  REQUIRE(!ring_structure.IsConnected(1, 1));
  REQUIRE(ring_structure.IsConnected(1, 2));

  REQUIRE(ring_structure.IsConnected(2, 0));
  REQUIRE(!ring_structure.IsConnected(2, 1));
  REQUIRE(!ring_structure.IsConnected(2, 2));
}


TEST_CASE("Can make new connections and remove existing connections", "[spatial-structure],[default]") {
  SpatialStructure ring_structure;
  emp::vector< emp::vector<bool> > ring_matrix = {
    { 0, 1, 0 },
    { 0, 0, 1 },
    { 1, 0, 0 }
  };
  ring_structure.SetStructure(ring_matrix);
  // Make a connection
  ring_structure.Connect(1, 0);
  // ring_structure.Print(std::cout);

  REQUIRE(!ring_structure.IsConnected(0, 0));
  REQUIRE(ring_structure.IsConnected(0, 1));
  REQUIRE(!ring_structure.IsConnected(0, 2));

  REQUIRE(ring_structure.IsConnected(1, 0));
  REQUIRE(!ring_structure.IsConnected(1, 1));
  REQUIRE(ring_structure.IsConnected(1, 2));

  REQUIRE(ring_structure.IsConnected(2, 0));
  REQUIRE(!ring_structure.IsConnected(2, 1));
  REQUIRE(!ring_structure.IsConnected(2, 2));

  // Remove a connection
  ring_structure.Disconnect(1, 0);
  // ring_structure.Print(std::cout);

  REQUIRE(!ring_structure.IsConnected(0, 0));
  REQUIRE(ring_structure.IsConnected(0, 1));
  REQUIRE(!ring_structure.IsConnected(0, 2));

  REQUIRE(!ring_structure.IsConnected(1, 0));
  REQUIRE(!ring_structure.IsConnected(1, 1));
  REQUIRE(ring_structure.IsConnected(1, 2));

  REQUIRE(ring_structure.IsConnected(2, 0));
  REQUIRE(!ring_structure.IsConnected(2, 1));
  REQUIRE(!ring_structure.IsConnected(2, 2));
}

TEST_CASE("Can make new bi-directional connections and remove existing connections bi-directionally", "[spatial-structure],[default]") {
  SpatialStructure structure;
  emp::vector< emp::vector<bool> > matrix = {
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 }
  };
  structure.SetStructure(matrix);
  // All nodes are disconnected
  for (size_t i = 0; i < structure.GetNumPositions(); ++i) {
    for (size_t j = i; j < structure.GetNumPositions(); ++j) {
      REQUIRE(!structure.IsConnected(i, j));
      REQUIRE(!structure.IsConnected(j, i));
    }
  }

  // Connect nodes 0, 1 bidirectionally
  structure.ConnectBidirectional(0, 1);
  REQUIRE(!structure.IsConnected(0, 2));
  REQUIRE(!structure.IsConnected(2, 0));
  REQUIRE(!structure.IsConnected(1, 2));
  REQUIRE(!structure.IsConnected(2, 1));
  REQUIRE(structure.IsConnected(0, 1));
  REQUIRE(structure.IsConnected(1, 0));

  // Disconnect nodes 0, 1 bidirectionally
  structure.DisconnectBidirectional(1, 0);
  REQUIRE(!structure.IsConnected(0, 2));
  REQUIRE(!structure.IsConnected(2, 0));
  REQUIRE(!structure.IsConnected(1, 2));
  REQUIRE(!structure.IsConnected(2, 1));
  REQUIRE(!structure.IsConnected(0, 1));
  REQUIRE(!structure.IsConnected(1, 0));

  // Regular connect then bidirectional disconnnect
  structure.Connect(0, 1);
  structure.DisconnectBidirectional(1, 0);
  REQUIRE(!structure.IsConnected(0, 2));
  REQUIRE(!structure.IsConnected(2, 0));
  REQUIRE(!structure.IsConnected(1, 2));
  REQUIRE(!structure.IsConnected(2, 1));
  REQUIRE(!structure.IsConnected(0, 1));
  REQUIRE(!structure.IsConnected(1, 0));

  // Regular connect, then bidirectional connect to connect other side
  structure.Connect(0, 1);
  structure.ConnectBidirectional(0, 1);
  REQUIRE(!structure.IsConnected(0, 2));
  REQUIRE(!structure.IsConnected(2, 0));
  REQUIRE(!structure.IsConnected(1, 2));
  REQUIRE(!structure.IsConnected(2, 1));
  REQUIRE(structure.IsConnected(0, 1));
  REQUIRE(structure.IsConnected(1, 0));
}

TEST_CASE("Can get a random neighbor", "[spatial-structure],[default]") {
  SpatialStructure structure;
  emp::Random rnd(5);

  structure.SetStructure(
    emp::vector< emp::vector<size_t> >{
      /* 0 -> */ {1, 2, 3},
      /* 1 -> */ {4},
      /* 2 -> */ {},
      /* 3 -> */ {},
      /* 4 -> */ {}
    }
  );

  for (size_t i = 0; i < 100; ++i) {
    const size_t neighbor = structure.GetRandomNeighbor(rnd, 0).value();
    REQUIRE(emp::Has(structure.GetNeighbors(0), neighbor));
  }

  auto result = structure.GetRandomNeighbor(rnd, 1);
  REQUIRE(result.value() == 4);

  result = structure.GetRandomNeighbor(rnd, 2);
  REQUIRE(!result);

}

TEST_CASE("Can read structure from csv that lists edges", "[spatial-structure],[default]") {
  const std::string csv_path = "source/test/data/spatial-structure-edges.csv";

  SpatialStructure structure;

  structure.LoadStructureFromEdgeCSV(csv_path);
  // structure.Print(std::cout);
  REQUIRE(structure.GetNumPositions() == 5);
  REQUIRE(structure.GetNeighbors(0) == emp::vector<size_t>{1, 2, 3});
  REQUIRE(structure.GetNeighbors(1) == emp::vector<size_t>{0, 2});
  REQUIRE(structure.GetNeighbors(2) == emp::vector<size_t>{0});
  REQUIRE(structure.GetNeighbors(3) == emp::vector<size_t>{});
  REQUIRE(structure.GetNeighbors(4) == emp::vector<size_t>{});

}

TEST_CASE("Can read structure from matrix file", "[spatial-structure],[default]") {
  const std::string mat_path = "source/test/data/spatial-structure-matrix.dat";

  SpatialStructure structure;

  structure.LoadStructureFromMatrix(mat_path);
  // structure.Print(std::cout);
  // structure.Print(std::cout, false);
  REQUIRE(structure.GetNumPositions() == 5);
  REQUIRE(structure.GetNeighbors(0) == emp::vector<size_t>{1, 2, 3});
  REQUIRE(structure.GetNeighbors(1) == emp::vector<size_t>{0, 2});
  REQUIRE(structure.GetNeighbors(2) == emp::vector<size_t>{0});
  REQUIRE(structure.GetNeighbors(3) == emp::vector<size_t>{});
  REQUIRE(structure.GetNeighbors(4) == emp::vector<size_t>{});

}

TEST_CASE("Can configure toroidal grid structure", "[spatial-structure],[default]") {
  SpatialStructure structure;
  ConfigureToroidalGrid(structure, 2, 2);
  REQUIRE(structure.GetNeighbors(0) == emp::vector<size_t>{1, 2});
  REQUIRE(structure.GetNeighbors(1) == emp::vector<size_t>{0, 3});
  REQUIRE(structure.GetNeighbors(2) == emp::vector<size_t>{0, 3});
  REQUIRE(structure.GetNeighbors(3) == emp::vector<size_t>{1, 2});

  ConfigureToroidalGrid(structure, 3, 3);
  // structure.Print(std::cout);
  REQUIRE(structure.GetNeighbors(0) == emp::vector<size_t>{1, 2, 3, 6});
  REQUIRE(structure.GetNeighbors(1) == emp::vector<size_t>{0, 2, 4, 7});
  REQUIRE(structure.GetNeighbors(2) == emp::vector<size_t>{0, 1, 5, 8});
  REQUIRE(structure.GetNeighbors(3) == emp::vector<size_t>{0, 4, 5, 6});
  REQUIRE(structure.GetNeighbors(4) == emp::vector<size_t>{1, 3, 5, 7});
  REQUIRE(structure.GetNeighbors(5) == emp::vector<size_t>{2, 3, 4, 8});
  REQUIRE(structure.GetNeighbors(6) == emp::vector<size_t>{0, 3, 7, 8});
  REQUIRE(structure.GetNeighbors(7) == emp::vector<size_t>{1, 4, 6, 8});
  REQUIRE(structure.GetNeighbors(8) == emp::vector<size_t>{2, 5, 6, 7});

  ConfigureToroidalGrid(structure, 4, 3);
  REQUIRE(structure.GetNeighbors(11) == emp::vector<size_t>{3, 7, 8, 10});
  // structure.Print(std::cout);

  ConfigureToroidalGrid(structure, 3, 4);
  REQUIRE(structure.GetNeighbors(11) == emp::vector<size_t>{2, 8, 9, 10});
  // structure.Print(std::cout);
}

TEST_CASE("Can configure fully connected structure", "[spatial-structure],[default]") {
  SpatialStructure structure;

  ConfigureFullyConnected(structure, 1);
  REQUIRE(structure.GetNumPositions() == 1);
  REQUIRE(structure.GetNeighbors(0) == emp::vector<size_t>{});

  ConfigureFullyConnected(structure, 2);
  REQUIRE(structure.GetNumPositions() == 2);
  REQUIRE(structure.GetNeighbors(0) == emp::vector<size_t>{1});
  REQUIRE(structure.GetNeighbors(1) == emp::vector<size_t>{0});

  ConfigureFullyConnected(structure, 4);
  REQUIRE(structure.GetNumPositions() == 4);
  REQUIRE(structure.GetNeighbors(0) == emp::vector<size_t>{1, 2, 3});
  REQUIRE(structure.GetNeighbors(1) == emp::vector<size_t>{0, 2, 3});
  REQUIRE(structure.GetNeighbors(2) == emp::vector<size_t>{0, 1, 3});
  REQUIRE(structure.GetNeighbors(3) == emp::vector<size_t>{0, 1, 2});
}





