#pragma once

#include "json.hpp"

#include "emp/base/vector.hpp"

#include <string>

namespace sym_json {

// Get value from json field with default value if field doesn't exist.
template<typename RET_TYPE>
RET_TYPE GetVal(
  nlohmann::json& json,
  const std::string& field,
  RET_TYPE default_val
) {
  return (json.contains(field)) ?
    static_cast<RET_TYPE>(json[field]) :
    default_val;
}

// Get value from jsonm field. Assumes that given field exists.
template<typename RET_TYPE>
RET_TYPE GetVal(
  nlohmann::json& json,
  const std::string& field
) {
  emp_assert(json.contains(field));
  return static_cast<RET_TYPE>(json[field]);
}

bool ValidateFieldsJSON(
  const nlohmann::json& json_line,
  const emp::vector<std::string>& fields
) {
  for (const std::string& name : fields) {
    if (!json_line.contains(name)) {
      return false;
    }
  }
  return true;
}

}