#pragma once

namespace sym_json {

template<typename RET_TYPE>
RET_TYPE GetVal(
  json_t& json,
  const std::string& field,
  RET_TYPE default_val
) {
  return (json.contains(field)) ?
    static_cast<RET_TYPE>(json[field]) :
    default_val;
}

}