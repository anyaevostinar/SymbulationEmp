#pragma once

/*
  This file contains the SpatialStructure class that defines and manages
  spatial structure.

  This class was adapted from the SpatialStructure class written for the Ecode
  lab's chemical ecology model: https://github.com/emilydolson/chemical-ecology/blob/main/include/chemical-ecology/SpatialStructure.hpp

  The class is designed with the following trade-offs:
  - Efficient random neighbor selection
  - Efficient neighbor checking
*/

#include "emp/base/vector.hpp"
#include "emp/datastructs/vector_utils.hpp"
#include "emp/io/File.hpp"
#include "emp/math/random_utils.hpp"
#include "emp/math/Random.hpp"
#include "emp/tools/string_utils.hpp"
#include "emp/base/array.hpp"

#include <algorithm>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_set>

class SpatialStructure {
protected:

  /**
   * Purpose: Mapping of source positions ==> destination positions.
   *          Destination IDs are kept in sorted order.
   */
  emp::vector< emp::vector<size_t> > ordered_connections;

  /**
   * Purpose: Matrix specifying whether two positions are connected ([from][to]).
   */
  emp::vector< emp::vector<bool> > connection_matrix;

  /**
   * Input: None
   *
   * Output: Boolean indicating whether ordered_connections and connection_matrix
   *         are consistent with one another.
   *
   * Purpose: Internal verification that ordered_connections and connection_matrix are
   *          consistent. Used for internal asserts in debug mode.
   */
  bool VerifyConnectionConsistency() const {
    const size_t num_positions = connection_matrix.size();
    // Check that ordered_connections has same number of positions as matrix
    if (num_positions != ordered_connections.size()) {
      return false;
    }
    // Check consistency in connections across representations
    for (size_t from = 0; from < num_positions; ++from) {
      emp_assert(connection_matrix[from].size() == num_positions);
      for (size_t to = 0; to < num_positions; ++to) {
        const bool connected = connection_matrix[from][to];
        const auto& neighbors = ordered_connections[from];
        emp_assert(std::is_sorted(neighbors.begin(), neighbors.end()));
        const bool is_neighbor = std::binary_search(
          neighbors.begin(),
          neighbors.end(),
          to
        );
        if (connected != is_neighbor) {
          return false;
        }
      }
    }
    return true;
  }

public:

  /**
   * Input: Adjacency list indicating connections between vertices in a graph.
   *        Adjacency list should be given in a [from][to] format.
   *
   * Output: None
   *
   * Purpose: Configure spatial structure from mapping of "from" positions to
   *          "to" positions
   */
  void SetStructure(const emp::vector<emp::vector<size_t>>& in_struct) {
    // Configure ordered connections (copy over and sort connections)
    const size_t num_positions = in_struct.size();
    ordered_connections = in_struct;
    for (emp::vector<size_t>& neighbors : ordered_connections) {
      std::sort(
        neighbors.begin(),
        neighbors.end()
      );
    }
    // Configure connection matrix
    connection_matrix.clear();
    connection_matrix.resize(
      num_positions,
      emp::vector<bool>(num_positions, false)
    );
    for (size_t from = 0; from < num_positions; ++from) {
      const auto& neighbors = ordered_connections[from];
      for (size_t to : neighbors) {
        connection_matrix[from][to] = true;
      }
    }
    emp_assert(VerifyConnectionConsistency());
  }

  //
  /**
   * Input: Adjacency matrix indicating connections between vertices in a graph.
   *        Matrix should be given in a [from][to] format.
   *
   * Output: None
   *
   * Purpose: Configure spatial structure from a connection matrix
   *          (maps [from][to])
   */
  void SetStructure(const emp::vector<emp::vector<bool>>& in_struct) {
    // Configure connection matrix (copy from parameter)
    const size_t num_positions = in_struct.size();
    connection_matrix = in_struct;
    // Configure ordered connections
    ordered_connections.clear();
    ordered_connections.resize(
      num_positions,
      emp::vector<size_t>(0)
    );
    for (size_t from = 0; from < num_positions; ++from) {
      emp_assert(connection_matrix.size() == num_positions, "Connection matrix must be square");
      for (size_t to = 0; to < num_positions; ++to) {
        const bool connected = connection_matrix[from][to];
        if (connected) {
          ordered_connections[from].emplace_back(to);
        }
      }
    }
    emp_assert(VerifyConnectionConsistency());
  }

  /**
   * Input: Vertex ids to connect
   *
   * Output: None
   *
   * Purpose: Create a connection between positions, from ==> to. Note that this
   *          forms a (one-way) directed connection.
   */
  void Connect(size_t from, size_t to) {
    // Check position validity
    emp_assert(from < GetNumPositions());
    emp_assert(to < GetNumPositions());
    // Do nothing if connection already exists
    if (IsConnected(from, to)) {
      return;
    }
    // Otherwise, connect from to to.
    connection_matrix[from][to] = true;
    auto& neighbors = ordered_connections[from];
    neighbors.insert(
      std::upper_bound(neighbors.begin(), neighbors.end(), to),
      to
    );
    emp_assert(VerifyConnectionConsistency());
  }

  /**
   * Input: Vertex ids to connect bidirectionally
   *
   * Output: None
   *
   * Purpose: Create a connection between positions, a <==> b. Note that this
   *          connects a --> b and b --> a
   */
  void ConnectBidirectional(size_t a, size_t b) {
    // Check position validity
    emp_assert(a < GetNumPositions());
    emp_assert(b < GetNumPositions());
    Connect(a, b);
    Connect(b, a);
  }

  /**
   * Input: Vertex ids to disconnect
   *
   * Output: None
   *
   * Purpose: Remove connection between positions, from ==> to. Note that this
   *          is directional. I.e., does not remove any to ==> from connection.
   */
  void Disconnect(size_t from, size_t to) {
    // Check position validity
    emp_assert(from < GetNumPositions());
    emp_assert(to < GetNumPositions());
    // Do nothing if connection does not exist
    if (!IsConnected(from, to)) {
      return;
    }
    // Otherwise, remove connection from => to
    connection_matrix[from][to] = false;
    auto& neighbors = ordered_connections[from];
    auto to_remove = std::equal_range(
      neighbors.begin(),
      neighbors.end(),
      to
    );
    neighbors.erase(to_remove.first, to_remove.second);
    emp_assert(VerifyConnectionConsistency());
  }

  /**
   * Input: Vertex ids to disconnect bidirectionally
   *
   * Output: None
   *
   * Purpose: Remove any connections between positions a and b
   */
  void DisconnectBidirectional(size_t a, size_t b) {
    // Check position validity
    emp_assert(a < GetNumPositions());
    emp_assert(b < GetNumPositions());
    Disconnect(a, b);
    Disconnect(b, a);
  }

  /**
   * Input: Vertex ids to check if connected
   *
   * Output: Boolean indicating whether "from" is connected to "to".
   *
   * Purpose: Check whether "from" is connected to "to". Note that this is directional.
   */
  bool IsConnected(size_t from, size_t to) const {
    return connection_matrix[from][to];
  }

  /**
   * Input: None
   *
   * Output: Unsigned integer indicating the number of positions in the spatial structure.
   *
   * Purpose: Get the total number of positions in the spatial structure
   */
  size_t GetNumPositions() const {
    return connection_matrix.size();
  }

  /**
   * Input: None
   *
   * Output: Connection matrix as a vector of vector of booleans.
   *
   * Purpose: Get the spatial structure in adjacency matrix form.
   */
  const emp::vector<emp::vector<bool>>& GetConnectionMatrix() const {
    return connection_matrix;
  }

  /**
   * Input: Position to get the neighbors for
   *
   * Output: const reference to list of neighbors of given position (pos)
   *
   * Purpose: Get an ordered list of neighbors for given position
   */
  const emp::vector<size_t>& GetNeighbors(size_t pos) const {
    emp_assert(pos < ordered_connections.size());
    return ordered_connections[pos];
  }

  /**
   * Input: Random number generator and position to get a random neighbor for.
   *
   * Output: Random neighbor of given position.
   *
   * Purpose: Returns a random neighbor of given position. If no valid neighbors,
              returns nullopt.
   */
  std::optional<size_t> GetRandomNeighbor(emp::Random& rnd, size_t pos) const {
    emp_assert(pos < GetNumPositions()) ;
    const auto& neighbors = ordered_connections[pos];
    if (neighbors.empty()) {
      return std::nullopt;
    }
    const size_t neighbor = neighbors[rnd.GetUInt(neighbors.size())];
    emp_assert(neighbor < GetNumPositions());
    return { neighbor };
  }

  /**
   * Input: Path to file containing spatial structure to load.
   *
   * Output: None
   *
   * Purpose: Loads spatial structure from csv file specified by filepath.
              File should have "from" and "to" columns (labeled in header).
              All other columns are ignored.
              See source/test/data/spatial-structure-edges.csv for an example
              of the expected format.
   */
  void LoadStructureFromEdgeCSV(const std::string& filepath) {
    emp::File file(filepath);
    // Read header
    std::string header_str = file.front();
    emp::vector<std::string> line;
    emp::slice(header_str, line, ',');
    emp_assert(emp::Has(line, {"from"}));
    emp_assert(emp::Has(line, {"to"}));

    // Get position of from and to columns
    const size_t from_idx = (size_t)emp::FindValue(line, {"from"});
    size_t to_idx = (size_t)emp::FindValue(line, {"to"});

    // Read all node names
    std::unordered_set<std::string> node_name_set;
    emp::vector<std::string> node_names;
    emp::vector< std::pair<std::string, std::string> > edges;
    for (size_t i = 1; i < file.GetNumLines(); ++i) {
      std::string line_str = file[i];
      emp::slice(line_str, line, ',');
      const std::string from_str = line[from_idx];
      const std::string to_str = line[to_idx];
      const bool valid_from = from_str != "NONE" && from_str != "";
      const bool valid_to = to_str != "NONE" && to_str != "";
      if (valid_from) {
        node_name_set.emplace(from_str);
      }
      if (valid_to) {
        node_name_set.emplace(to_str);
      }
      if (valid_from && valid_to) {
        edges.emplace_back(from_str, to_str);
      }
    }
    // Order node names
    for (const auto& val : node_name_set) {
      node_names.emplace_back(val);
    }
    const size_t num_positions = node_names.size();
    std::sort(node_names.begin(), node_names.end());
    // Create mapping from names to position
    std::unordered_map<std::string, size_t> name_to_position;
    for (size_t pos = 0; pos < num_positions; ++pos) {
      name_to_position[node_names[pos]] = pos;
    }
    // Begin reading edges
    emp::vector< emp::vector<size_t> > connections(
      num_positions,
      emp::vector<size_t>(0)
    );
    for (const auto& pair : edges) {
      const size_t from = name_to_position[pair.first];
      const size_t to = name_to_position[pair.second];
      connections[from].emplace_back(to);
    }
    SetStructure(connections);
  }

  /**
   * Input: Path to file containing spatial structure to load.
   *
   * Output: None
   *
   * Purpose: Load spatial structure from matrix file format.
              See source/test/data/spatial-structure-matrix.dat for example
              expected format.
   */
  void LoadStructureFromMatrix(const std::string& filepath) {
    emp::File file(filepath);
    file.RemoveEmpty();
    emp::vector< emp::vector<bool> > matrix;
    emp::vector< std::string > line_components;
    for (size_t i = 0; i < file.GetNumLines(); ++i) {
      std::string line_str = file[i];
      // Remove all whitespace
      emp::remove_whitespace(line_str);
      // Handle comments
      line_str = emp::string_get(line_str, "#");
      // If line is now empty, skip
      if (line_str == emp::empty_string()) {
        continue;
      }
      // If here, this line should represent a row
      emp::slice(line_str, line_components, ',');
      matrix.emplace_back(
        emp::vector<bool>(
          line_components.size(),
          false
        )
      );
      for (size_t to = 0; to < line_components.size(); ++to) {
        matrix.back()[to] = line_components[to] != "0";
      }
    }
    SetStructure(matrix);
  }

  /**
   * Input: Stream to put output in, boolean indicating whether to print as mapping
   *        (from => to) or matrix format.
   *
   * Output: None
   *
   * Purpose: Print spatial structure connectivity. Defaults to mapping format.
   */
  void Print(std::ostream& os = std::cout, bool as_mapping = true) const {
    if (as_mapping) {
      PrintConnectionMapping(os);
    } else {
      PrintConnectionMatrix(os);
    }
  }

  /**
   * Input: Output stream to print into.
   *
   * Output: None
   *
   * Purpose: Print spatial structure connectivity as connection matrix.
   */
  void PrintConnectionMatrix(std::ostream& os = std::cout) const {
    emp_assert(VerifyConnectionConsistency());
    const size_t num_positions = GetNumPositions();
    for (size_t from = 0; from < num_positions; ++from) {
      for (size_t to = 0; to < num_positions; ++to) {
        if (to) os << ",";
        os << (size_t)connection_matrix[from][to];
      }
      os << std::endl;
    }
  }

  /**
   * Input: Output stream to print into.
   *
   * Output: None.
   *
   * Purpose: Print mapping of "from" positions to "to" positions
   */
  void PrintConnectionMapping(std::ostream& os = std::cout) const {
    emp_assert(VerifyConnectionConsistency());
    const size_t num_positions = GetNumPositions();
    for (size_t from = 0; from < num_positions; ++from) {
      const auto& neighbors = ordered_connections[from];
      os << from << ":";
      for (size_t i = 0; i < neighbors.size(); ++i) {
        if (i) os << ",";
        os << " " << neighbors[i];
      }
      os << std::endl;
    }
  }

}; // End SpatialStructure class

// -- Simple structure configurations --

/**
 * Input: Spatial structure object to configure, width and height of the grid to
 *        create.
 *
 * Output: None.
 *
 * Purpose: Configure structure object into a width x height toroidal grid.
 */
void ConfigureToroidalGrid(SpatialStructure& structure, size_t width, size_t height) {
  emp_assert(width > 0, "Width must be greater than 0");
  emp_assert(height > 0, "Height must be greater than 0");

  // Build 2D toroidal grid
  const size_t grid_size = width * height;
  emp::vector< emp::vector<size_t> > grid_connections(
    grid_size,
    emp::vector<size_t>(0)
  );
  for (size_t pos = 0; pos < grid_size; ++pos) {
    // Get x,y cooridinates associated with current position
    const size_t pos_x = pos % width;
    const size_t pos_y = pos / width;
    // Calculate horizontal neighbors, handle wrap-around
    const size_t left_pos = (pos_x != 0) ? (pos - 1) : (pos - 1) + width;
    const size_t right_pos = (pos_x != width - 1) ? (pos + 1) : (pos + 1) - width;
    // Calculate vertical neighbors, handle wrap-around
    const size_t up_pos = (pos_y != 0) ? (pos - width) : grid_size - (width - pos_x); // (pos - width) + (width * height);
    const size_t down_pos = (pos_y != height - 1) ? (pos + width) : (pos + width) - grid_size;
    // Add connections (remove duplicates)
    std::set<size_t> neighbor_set = {left_pos, right_pos, up_pos, down_pos};
    std::copy(
      neighbor_set.begin(),
      neighbor_set.end(),
      std::back_inserter(grid_connections[pos])
    );
  }

  structure.SetStructure(grid_connections);
}


/**
 * Input:
 *
 * Output:
 *
 * Purpose: Build well-mixed structure of given size. Note: It can be expensive
 *          to explicitly represent a fully connected graph. Generally much better
 *          to represent as a flat vector and implicitly assume all nodes are
 *          connected to one another.
 */
void ConfigureFullyConnected(SpatialStructure& structure, size_t size) {
  emp_assert(size > 0, "Size must be greater than 0");

  // Create matrix with all possible edges
  emp::vector< emp::vector<bool> > matrix(
    size,
    emp::vector<bool>(size, true)
  );

  // Remove self-connections
  for (size_t i = 0; i < size; ++i) {
    matrix[i][i] = false;
  }

  structure.SetStructure(matrix);
}

