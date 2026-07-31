#pragma once

#include "json.hpp"

#include "emp/base/vector.hpp"

#include <string>

namespace sym_json {

/**
 * Purpose: Convenience function for more easily getting a value from a json field
 *          with a default return value if the field doesn't exist.
 *
 * Input: json object to access, field to access in json object, and a default
 *        value to return if the field doesn't exist in the json object.
 *
 * Output: Either the accessed field value or the provided default value.
 *
 */
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

/**
 * Purpose: Convenience function for more easily getting a value from a json field.
 *          This version of the GetVal function assumes that the field exists.
 *
 * Input: json object to access, field to access in json object
 *
 * Output: The accessed field value
 *
 */
template<typename RET_TYPE>
RET_TYPE GetVal(
  nlohmann::json& json,
  const std::string& field
) {
  emp_assert(json.contains(field));
  return static_cast<RET_TYPE>(json[field]);
}

/**
 * Purpose: Validate that specified strings exist as fields inside of a json object.
 *          This function is useful for asserting that required/expected fields
 *          exist.
 *
 * Input: json object to check, fields to check in json object
 *
 * Output: Boolean indicating whether all given fields are contained in the json
 *         object.
 *
 */
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