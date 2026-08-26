#ifndef SGPMODE_ORG_TYPE_INFO_h
#define SGPMODE_ORG_TYPE_INFO_h

#include "emp/datastructs/map_utils.hpp"
#include "emp/tools/string_utils.hpp"

#include <unordered_map>
#include <string>

namespace sgpmode::org_info {

const size_t DEFAULT_STACK_SIZE_LIMIT = 16;

enum class SGPOrganismType { DEFAULT = 0 };
enum class StressSymbiontType { MUTUALIST = 0, PARASITE, NEUTRAL, INTERACTION_VALUE_BASED };
enum class HealthSymbiontType { MUTUALIST = 0, PARASITE, NEUTRAL, INTERACTION_VALUE_BASED };
enum class NutrientSymbiontType { MUTUALIST = 0, PARASITE, NEUTRAL, INTERACTION_VALUE_BASED };

// Mapping from commandline string configuration to organism type.
std::unordered_map<std::string, SGPOrganismType> sgp_org_type_map = {
  {"default", SGPOrganismType::DEFAULT}
};

// Mapping from commandline stress symbiont type string to stress symbiont type.
std::unordered_map<std::string, StressSymbiontType> sgp_stress_sym_type_map = {
  {"mutualist", StressSymbiontType::MUTUALIST},
  {"parasite", StressSymbiontType::PARASITE},
  {"neutral", StressSymbiontType::NEUTRAL},
  {"interaction-value", StressSymbiontType::INTERACTION_VALUE_BASED}
};

// Mapping from commandline health symbiont type string to health symbiont type.
std::unordered_map<std::string, HealthSymbiontType> sgp_health_sym_type_map = {
  {"mutualist", HealthSymbiontType::MUTUALIST},
  {"parasite", HealthSymbiontType::PARASITE},
  {"neutral", HealthSymbiontType::NEUTRAL},
  {"interaction-value", HealthSymbiontType::INTERACTION_VALUE_BASED}
};

// Mapping from commandline nutrient symbiont type string to nutrient symbiont type.
std::unordered_map<std::string, NutrientSymbiontType> sgp_nutrient_sym_type_map = {
  {"mutualist", NutrientSymbiontType::MUTUALIST},
  {"parasite", NutrientSymbiontType::PARASITE},
  {"neutral", NutrientSymbiontType::NEUTRAL},
  {"interaction-value", NutrientSymbiontType::INTERACTION_VALUE_BASED}
};

/**
   * Input: A String.
   *
   * Output: A Boolean of whether the string provided is a type of organism.
   *
   * Purpose: Checking if the string is a correct organism type before attempting to access it
   */
bool IsValidOrganismType(const std::string& type_str) {
  return emp::Has(sgp_org_type_map, type_str);
}

/**
   * Input: A String.
   *
   * Output: The SGPOrganismType associated with that string
   *
   * Purpose: Access the organism type object from its name
   */
SGPOrganismType GetOrganismType(const std::string& type_str) {
  emp_assert(IsValidOrganismType(type_str));
  return sgp_org_type_map[type_str];
}

/**
   * Input: A String.
   *
   * Output: A Boolean of whether the string provided is a type of stress symbiont.
   *
   * Purpose: Checking if the string is a correct stress symbiont type before attempting to access it
   */
bool IsValidStressSymType(const std::string& type_str) {
  return emp::Has(sgp_stress_sym_type_map, type_str);
}

/**
   * Input: A String.
   *
   * Output: The StressSymbiontType associated with that string
   *
   * Purpose: Access the stress symbiont type object from its name
   */
StressSymbiontType GetStressSymType(const std::string& type_str) {
  emp_assert(IsValidStressSymType(type_str));
  return sgp_stress_sym_type_map[type_str];
}

/**
   * Input: A String.
   *
   * Output: A Boolean of whether the string provided is a type of health symbiont.
   *
   * Purpose: Checking if the string is a correct health symbiont type before attempting to access it
   */
bool IsValidHealthSymType(const std::string& type_str) {
  return emp::Has(sgp_health_sym_type_map, type_str);
}

/**
   * Input: A String.
   *
   * Output: The HealthSymbiontType associated with that string
   *
   * Purpose: Access the health symbiont type object from its name
   */
HealthSymbiontType GetHealthSymType(const std::string& type_str) {
  emp_assert(IsValidHealthSymType(type_str));
  return sgp_health_sym_type_map[type_str];
}

/**
   * Input: A String.
   *
   * Output: A Boolean of whether the string provided is a type of nutrient symbiont.
   *
   * Purpose: Checking if the string is a correct nutrient symbiont type before attempting to access it
   */
bool IsValidNutrientSymType(const std::string& type_str) {
  return emp::Has(sgp_nutrient_sym_type_map, type_str);
}

/**
   * Input: A String.
   *
   * Output: The NutrientSymbiontType associated with that string
   *
   * Purpose: Access the nutrient symbiont type object from its name
   */
NutrientSymbiontType GetNutrientSymType(const std::string& type_str) {
  emp_assert(IsValidNutrientSymType(type_str));
  return sgp_nutrient_sym_type_map[type_str];
}

}

#endif