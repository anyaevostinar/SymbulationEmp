#ifndef SGPMODE_ORG_TYPE_INFO_h
#define SGPMODE_ORG_TYPE_INFO_h

#include "emp/datastructs/map_utils.hpp"
#include "emp/tools/string_utils.hpp"

#include <unordered_map>
#include <string>

namespace sgpmode::org_info {

const size_t DEFAULT_STACK_SIZE_LIMIT = 16;

enum class SGPOrganismType { DEFAULT = 0 };

// Mapping from commandline string configuration to organism type.
std::unordered_map<std::string, SGPOrganismType> sgp_org_type_map = {
  {"default", SGPOrganismType::DEFAULT}
};

bool IsValidOrganismType(const std::string& type_str) {
  return emp::Has(sgp_org_type_map, type_str);
}

SGPOrganismType GetOrganismType(const std::string& type_str) {
  emp_assert(IsValidOrganismType(type_str));
  return sgp_org_type_map[type_str];
}

}

#endif