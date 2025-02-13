#ifndef SGPMODE_ORG_TYPE_INFO_h
#define SGPMODE_ORG_TYPE_INFO_h

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

}

#endif