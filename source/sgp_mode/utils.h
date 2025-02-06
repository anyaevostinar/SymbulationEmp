#ifndef SGPMODE_UTILS_h
#define SGPMODE_UTILS_h

#include "emp/bits/Bits.hpp"

#include <algorithm>

namespace sgpmode::utils {

// TODO - write test
// Do any bits match between bits_a and bits_b?
template<size_t NUM_BITS>
bool AnyMatch(
  const emp::BitSet<NUM_BITS>& bits_a,
  const emp::BitSet<NUM_BITS>& bits_b
) {
  return bits_a.HasOverlap(bits_b);
}

// Return number of matching bits between two bit sets.
template<size_t NUM_BITS>
size_t SimpleMatchCoeff(
  const emp::BitSet<NUM_BITS>& bits_a,
  const emp::BitSet<NUM_BITS>& bits_b
) {
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

}

#endif