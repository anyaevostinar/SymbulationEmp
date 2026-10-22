#pragma once

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

// spatial_utils contains helper functions for working with 2d toroidal grids
// that are stored in a flat vector format.
namespace spatial_utils {

/**
 * Purpose: Enumerates all 8-neighborhood grid directions.
 */
enum class GRID_DIR {
  UP_LEFT,
  UP,
  UP_RIGHT,
  RIGHT,
  DOWN_RIGHT,
  DOWN,
  DOWN_LEFT,
  LEFT
};

/**
 * Purpose: Container to hold all 8 grid directions. Useful for looping over all
 *          grid directions.
 */
emp::array<GRID_DIR, 8> grid_directions = {
  GRID_DIR::UP_LEFT,
  GRID_DIR::UP,
  GRID_DIR::UP_RIGHT,
  GRID_DIR::RIGHT,
  GRID_DIR::DOWN_RIGHT,
  GRID_DIR::DOWN,
  GRID_DIR::DOWN_LEFT,
  GRID_DIR::LEFT
};

/**
 * Input: id, grid width, grid height
 *
 * Output: XY position for given id on given grid configuration.
 *
 * Purpose: Useful when representing a 2d grid as a flat vector.
 *          E.g., imagine a 2x2 grid stored as [0, 1, 2, 3], which represents:
 *                 0 1
 *                 2 3
 *          0 is at x,y position 0,0; 1 is at x,y position 0,1; etc.
 */
emp::array<size_t, 2> GridXYFromID(size_t id, size_t grid_width, size_t grid_height) {
  emp_assert(id < grid_width * grid_height);
  emp_assert(grid_width > 0);
  return {id % grid_width, id / grid_width};
}

/**
 * Input: Length-2 array specifying an [x,y] position, grid width, grid height
 *
 * Output: Flat vector ID associated with given xy position for given grid size.
 *
 * Purpose: Useful when representing a 2d grid as a flat vector.
 *          E.g., imagine a 2x2 grid stored as [0, 1, 2, 3], which represents:
 *                 0 1
 *                 2 3
 *          0 is at x,y position 0,0; 1 is at x,y position 0,1; etc.
 */
size_t GridIDFromXY(const emp::array<size_t, 2>& pos_xy, size_t grid_width, size_t grid_height) {
  emp_assert(pos_xy[0] < grid_width);
  emp_assert(pos_xy[1] < grid_height);
  const size_t x = pos_xy[0];
  const size_t y = pos_xy[1];
  return (y * grid_width) + x;
}

/**
 * Input: xy position, direction, and grid configuration (width and height)
 *
 * Output: Grid neighbor (as XY position) in the given direction for the given
 *          grid configuration.
 *
 * Purpose: Get grid neighbor in given direction in XY format
 */
emp::array<size_t, 2> GetGridNeighbor(
  const emp::array<size_t, 2>& pos_xy,
  GRID_DIR dir,
  size_t grid_width,
  size_t grid_height
) {
  const size_t x = pos_xy[0];
  const size_t y = pos_xy[1];
  emp_assert(x < grid_width);
  emp_assert(y < grid_height);
  // Protected wrapping (avoiding size_t underflow)
  switch (dir) {
    case GRID_DIR::UP_LEFT:
      return {
        (x != 0) ? x - 1 : grid_width - 1,
        (y != 0) ? y - 1 : grid_height - 1
      };
    case GRID_DIR::UP:
      return {
        x,
        (y != 0) ? y - 1 : grid_height - 1
      };
    case GRID_DIR::UP_RIGHT:
      return {
        (x != grid_width - 1) ? x + 1 : 0,
        (y != 0) ? y - 1 : grid_height - 1
      };
    case GRID_DIR::RIGHT:
      return {
        (x != grid_width - 1) ? x + 1 : 0,
        y
      };
    case GRID_DIR::DOWN_RIGHT:
      return {
        (x != grid_width - 1) ? x + 1 : 0,
        (y != grid_height - 1) ? y + 1 : 0
      };
    case GRID_DIR::DOWN:
      return {
        x,
        (y != grid_height - 1) ? y + 1 : 0
      };
    case GRID_DIR::DOWN_LEFT:
      return {
        (x != 0) ? x - 1 : grid_width - 1,
        (y != grid_height - 1) ? y + 1 : 0
      };
    case GRID_DIR::LEFT:
      return {
        (x != 0) ? x - 1 : grid_width - 1,
        y
      };
    default:
      emp_error("Unimplemented direction");
      return {(size_t)-1, (size_t)-1};
  }
}

/**
 * Input: grid id (assumes flat format), direction, and grid configuration (width and height)
 *
 * Output: Grid neighbor (as an id) in the given direction for the given
 *          grid configuration.
 *
 * Purpose: Get grid neighbor in given direction
 */
size_t GetGridNeighbor(
  size_t grid_id,
  GRID_DIR dir,
  size_t grid_width,
  size_t grid_height
) {
  const emp::array<size_t, 2> grid_xy{GridXYFromID(grid_id, grid_width, grid_height)};
  return GridIDFromXY(
    GetGridNeighbor(grid_xy, dir, grid_width, grid_height),
    grid_width,
    grid_height
  );
}

}