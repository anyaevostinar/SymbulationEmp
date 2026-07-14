#ifndef SYM_UTILS_h
#define SYM_UTILS_h
#pragma once

#include "emp/datastructs/map_utils.hpp"
#include "emp/base/vector.hpp"
#include "emp/math/Random.hpp"
#include "emp/math/random_utils.hpp"
#include "emp/base/assert.hpp"
#include "emp/bits/Bits.hpp"
#include "emp/datastructs/map_utils.hpp"

#include <algorithm>
#include <unordered_map>

namespace utils {

/**
 * Input: Two bitsets
 *
 * Output: Boolean indicating if any positions in the two input bitsets have
 *         matching ones
 *
 * Purpose: Check whether any positions in the two input bitsets have matching ones
 */
template<size_t NUM_BITS>
bool AnyMatchingOnes(
  const emp::BitSet<NUM_BITS>& bits_a,
  const emp::BitSet<NUM_BITS>& bits_b
) {
  return bits_a.HasOverlap(bits_b);
}

/**
 * Input: Two bit vectors
 *
 * Output: Boolean indicating if any positions in the two input bit vectors have
 *         matching ones
 *
 * Purpose: Check whether any positions in the two input bit vectors have matching
 *          ones
 */
bool AnyMatchingOnes(
  const emp::BitVector& bits_a,
  const emp::BitVector& bits_b
) {
  return bits_a.HasOverlap(bits_b);
}

/**
 * Input: Two bit sets
 *
 * Output: Number of positions that have a 1 in that position across both input
 *         bit sets.
 *
 * Purpose: Return number of matching ones between two bit sets.
 */
template<size_t NUM_BITS>
size_t MatchingOnesCount(
  const emp::BitSet<NUM_BITS>& bits_a,
  const emp::BitSet<NUM_BITS>& bits_b
) {
  return bits_a.AND(bits_b).CountOnes();
}

/**
 * Input: Two bit vectors
 *
 * Output: Number of positions that have a 1 in that position across both input
 *         bit vectors.
 *
 * Purpose: Return number of matching ones between two bit vectors.
 */
size_t MatchingOnesCount(
  const emp::BitVector& bits_a,
  const emp::BitVector& bits_b
) {
  emp_assert(bits_a.GetSize() == bits_b.GetSize());
  return bits_a.AND(bits_b).CountOnes();
}

/**
 * Input: Bit vector and new size
 *
 * Output: None
 *
 * Purpose: Resize the bit vector to a specified size, and set all positions to 0
 */
void ResizeClear(emp::BitVector& in, size_t new_size) {
  in.Resize(new_size);
  in.Clear();
}

/**
 * Input: Container and new size
 *
 * Output: None
 *
 * Purpose: Resize the container to a specified size, and set all positions to
 *          a given fill value.
 */
template<typename CONTAINER_T, typename FILL_T>
void ResizeFill(CONTAINER_T& container, size_t new_size, FILL_T fill_val) {
  container.resize(new_size);
  std::fill(container.begin(), container.end(), fill_val);
}


/**
 * Input: unordered map and an item
 *
 * Output: None
 *
 * Purpose: Helper function for counting occurrences. If item is already in
 *          the counting map, increment the count. If not, add to map and set count
 *          to 1.
 */
template<typename CONTAINER_T>
void AddToCountingMap(
  std::unordered_map<CONTAINER_T, size_t>& counting_map,
  const CONTAINER_T& item
) {
  const bool new_item = !emp::Has(counting_map, item);
  if (new_item) {
    counting_map[item] = 1;
  } else {
    ++(counting_map[item]);
  }
}

namespace internal {
  auto value_selector = [](const auto& pair) { return pair.second; };
}

/**
 * Input: unordered map to collect values from and a vector to fill with values
 *
 * Output: None
 *
 * Purpose: Collect map values into the given values vector. Will overwrite the
 *          contents of values.
 */
template<typename KEY_T, typename VALUE_T>
void CollectMapValues(
  const std::unordered_map<KEY_T, VALUE_T>& map,
  emp::vector<VALUE_T>& values
) {
    values.resize(map.size());
    std::transform(
      map.begin(),
      map.end(),
      values.begin(),
      internal::value_selector
    );
}

/**
 * Input: Mapping of string to a config options enum, name of config, and config value.
 *
 * Output: None
 *
 * Purpose: Validate whether given cfg_input is specified in the cfg_mapping.
 *          If not, print out a helpful message and then exit.
 */
template<typename ENUM_CFG_OPTIONS>
void ValidateConfigMode(
  const std::unordered_map<std::string, ENUM_CFG_OPTIONS>& cfg_mapping,
  const std::string& cfg_name,
  const std::string& cfg_input
) {
  const bool valid_cfg = emp::Has(cfg_mapping, cfg_input);
  if (!valid_cfg) {
    std::cout << "Invalid input for " << cfg_name << " (" << cfg_input << ")." << std::endl;
    std::cout << "  Valid options include: " << std::endl;
    for (const auto& [option, mode] : cfg_mapping) {
      std::cout << "    " << option << std::endl;
    }
    exit(-1);
  }
}

/**
 * Input: Random number generator, start of ordering, end of ordering.
 *
 * Output: Vector with randomized ordering: [start:end) in shuffled order.
 *
 * Purpose: Generate a shuffled ordering [start:end)
 */
emp::vector<size_t> GenerateRandomOrdering(
  emp::Random& random,
  size_t first,
  size_t end
) {
  emp_assert(end >= first);
  const size_t num_positions = end - first;
  emp::vector<size_t> positions(num_positions);
  std::iota(
    positions.begin(),
    positions.end(),
    first
  );
  emp::Shuffle(random, positions);
  return positions;
}

/**
 * Input: Random number generator, length of ordering.
 *
 * Output: Vector with randomized ordering: [0:end) in shuffled order.
 *
 * Purpose: Generate a shuffled ordering [0:end)
 */
emp::vector<size_t> GenerateRandomOrdering(
  emp::Random& random,
  size_t end
) {
  emp_assert(end >= 0);
  return GenerateRandomOrdering(random, 0, end);
}

}

#endif