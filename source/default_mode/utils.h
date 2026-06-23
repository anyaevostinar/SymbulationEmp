#pragma once

#include "emp/datastructs/map_utils.hpp"
#include "emp/base/vector.hpp"
#include "emp/math/Random.hpp"
#include "emp/math/random_utils.hpp"
#include "emp/base/assert.hpp"

#include <algorithm>

namespace utils {

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
  emp_assert(end > first);
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
  emp_assert(end > 0);
  return GenerateRandomOrdering(random, 0, end);
}



}