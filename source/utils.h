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

// TODO - write test
// Do any bits match between bits_a and bits_b?
template<size_t NUM_BITS>
bool AnyMatchingOnes(
  const emp::BitSet<NUM_BITS>& bits_a,
  const emp::BitSet<NUM_BITS>& bits_b
) {
  return bits_a.HasOverlap(bits_b);
}

bool AnyMatchingOnes(
  const emp::BitVector& bits_a,
  const emp::BitVector& bits_b
) {
  return bits_a.HasOverlap(bits_b);
}

// Return number of matching bits between two bit sets.
template<size_t NUM_BITS>
size_t MatchingOnesCount(
  const emp::BitSet<NUM_BITS>& bits_a,
  const emp::BitSet<NUM_BITS>& bits_b
) {
  return bits_a.AND(bits_b).CountOnes();
}

// Return number of matching bits between two bit sets.
size_t MatchingOnesCount(
  const emp::BitVector& bits_a,
  const emp::BitVector& bits_b
) {
  emp_assert(bits_a.GetSize() == bits_b.GetSize());
  return bits_a.AND(bits_b).CountOnes();
}

void ResizeClear(emp::BitVector& in, size_t new_size) {
  in.Resize(new_size);
  in.Clear();
}

template<typename CONTAINER_T, typename FILL_T>
void ResizeFill(CONTAINER_T& container, size_t new_size, FILL_T fill_val) {
  container.resize(new_size);
  std::fill(container.begin(), container.end(), fill_val);
}


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

// Collect map values into values vector
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

emp::vector<size_t> GenerateRandomOrdering(
  emp::Random& random,
  size_t end
) {
  emp_assert(end >= 0);
  return GenerateRandomOrdering(random, 0, end);
}

}

#endif