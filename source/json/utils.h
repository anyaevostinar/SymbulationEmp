#pragma once

#include "json.hpp"

#include "emp/base/vector.hpp"

#include <string>

namespace sym_json {

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

// @AML review: renamed, fixed inner loop
bool ValidateFieldsJSON(
  const nlohmann::json& json_line,
  const emp::vector<std::string>& fields
) {
  // @AML review: Can use const string reference to avoid copying string here
  for (const std::string& name : fields) {
    if (!json_line.contains(name)) {
      return false;
    }
  }
  return true;
}

}