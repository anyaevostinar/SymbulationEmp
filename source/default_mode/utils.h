#pragma once

#include "emp/datastructs/map_utils.hpp"

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

}