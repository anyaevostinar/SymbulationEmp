#ifndef SGPMODE_ORG_TYPE_INFO_h
#define SGPMODE_ORG_TYPE_INFO_h

#include "emp/datastructs/map_utils.hpp"
#include "emp/tools/string_utils.hpp"

#include <unordered_map>
#include <string>

namespace sgpmode::org_info {

enum class SGPOrganismType { DEFAULT = 0, HEALTH, STRESS, NUTRIENT };
enum class StressSymbiontType { MUTUALIST = 0, PARASITE, NEUTRAL };

// Mapping from commandline string configuration to organism type.
std::unordered_map<std::string, SGPOrganismType> sgp_org_type_map = {
  {"default", SGPOrganismType::DEFAULT},
  {"health", SGPOrganismType::HEALTH},
  {"stress", SGPOrganismType::STRESS},
  {"nutrient", SGPOrganismType::NUTRIENT}
};

// Mapping from commandline stress symbiont type string to stress symbiont type.
std::unordered_map<std::string, StressSymbiontType> sgp_stress_sym_type_map = {
  {"mutualist", StressSymbiontType::MUTUALIST},
  {"parasite", StressSymbiontType::PARASITE},
  {"neutral", StressSymbiontType::NEUTRAL}
};

bool IsValidOrganismType(const std::string& type_str) {
  return emp::Has(sgp_org_type_map, type_str);
}

SGPOrganismType GetOrganismType(const std::string& type_str) {
  emp_assert(IsValidOrganismType(type_str));
  return sgp_org_type_map[type_str];
}

bool IsValidStressSymType(const std::string& type_str) {
  return emp::Has(sgp_stress_sym_type_map, type_str);
}

StressSymbiontType GetStressSymType(const std::string& type_str) {
  emp_assert(IsValidStressSymType(type_str));
  return sgp_stress_sym_type_map[type_str];
}

}

#endif