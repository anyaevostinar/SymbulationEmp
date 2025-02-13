#ifndef SGPMODE_SPEC_h
#define SGPMODE_SPEC_h

#include <unordered_map>
#include <string>

// TODO - rename, "spec" overlaps too much with hardwdare spec naming; confusing as-is
namespace sgpmode::spec {

// TODO - refactor tasks, move this.
const size_t NUM_TASKS = 9;
const size_t DEFAULT_STACK_SIZE_LIMIT = 16;

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