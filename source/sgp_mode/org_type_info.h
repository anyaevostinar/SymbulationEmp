#ifndef SGPMODE_ORG_TYPE_INFO_h
#define SGPMODE_ORG_TYPE_INFO_h

#include "emp/datastructs/map_utils.hpp"
#include "emp/tools/string_utils.hpp"

#include <unordered_map>
#include <string>

namespace sgpmode::org_info {

const size_t DEFAULT_STACK_SIZE_LIMIT = 16;

enum class SGPOrganismType { DEFAULT = 0 };
enum class StressSymbiontType { MUTUALIST = 0, PARASITE, NEUTRAL };
enum class HealthSymbiontType { MUTUALIST = 0, PARASITE, NEUTRAL };

// Mapping from commandline string configuration to organism type.
std::unordered_map<std::string, SGPOrganismType> sgp_org_type_map = {
  {"default", SGPOrganismType::DEFAULT}
};

// Mapping from commandline stress symbiont type string to stress symbiont type.
std::unordered_map<std::string, StressSymbiontType> sgp_stress_sym_type_map = {
  {"mutualist", StressSymbiontType::MUTUALIST},
  {"parasite", StressSymbiontType::PARASITE},
  {"neutral", StressSymbiontType::NEUTRAL}
};

// Mapping from commandline health symbiont type string to health symbiont type.
std::unordered_map<std::string, HealthSymbiontType> sgp_health_sym_type_map = {
  {"mutualist", HealthSymbiontType::MUTUALIST},
  {"parasite", HealthSymbiontType::PARASITE},
  {"neutral", HealthSymbiontType::NEUTRAL}
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

bool IsValidHealthSymType(const std::string& type_str) {
  return emp::Has(sgp_health_sym_type_map, type_str);
}

HealthSymbiontType GetHealthSymType(const std::string& type_str) {
  emp_assert(IsValidHealthSymType(type_str));
  return sgp_health_sym_type_map[type_str];
}

}

#endif