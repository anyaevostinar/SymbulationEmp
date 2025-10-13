#ifndef SGPMODE_UTILS_h
#define SGPMODE_UTILS_h

#include "emp/bits/Bits.hpp"
#include "emp/datastructs/map_utils.hpp"

#include <algorithm>
#include <unordered_map>

namespace sgpmode::utils {

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

}

#endif