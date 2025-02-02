#ifndef UTILS_h
#define UTILS_h

#include "emp/bits/Bits.hpp"

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

}

#endif