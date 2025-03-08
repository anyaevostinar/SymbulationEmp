#ifndef SGP_DATA_NODES_C
#define SGP_DATA_NODES_C

#include "SGPWorld.h"
#include "SGPHost.h"
#include "SGPSymbiont.h"
#include "spec.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <filesystem>

namespace sgpmode {

void SGPWorld::CreateDataFiles() {
  // Configure output directory
  output_dir.clear();
  output_dir = sgp_config.FILE_PATH();
  // If setup has not been run, create output directory.
  if (!setup) {
    std::filesystem::create_directory(output_dir);
  }

  // NOTE - discuss typical data organization for symbulation
  //        TODO - update file endings post discussion

  // Setup organism count file
  // std::filesystem::path org_count_fpath = output_dir / ("OrganismCounts.csv");
  // SetUpOrgCountFile(org_count_fpath.string()).SetTimingRepeat(sgp_config.DATA_INT());
  // Setup transmission file
  std::filesystem::path transmission_fpath = output_dir / ("TransmissionRates.csv");
  SetUpTransmissionFile(transmission_fpath.string()).SetTimingRepeat(sgp_config.DATA_INT());
  // Setup

}


void SGPWorld::SnapshotConfig(const std::string& filename) {
  std::filesystem::path fpath = output_dir / filename;
  emp::DataFile snapshot_file(fpath.string());
  std::function<std::string(void)> get_param;   // Parameter name
  std::function<std::string(void)> get_value;   // Parameter value
  // std::function<std::string(void)> get_source;
  snapshot_file.AddFun<std::string>(
    [&get_param]() { return get_param(); },
    "parameter"
  );
  snapshot_file.AddFun<std::string>(
    [&get_value]() { return get_value(); },
    "value"
  );
  snapshot_file.PrintHeaderKeys();

  // Snapshot config values
  for (const auto& entry : sgp_config) {
    get_param = [&entry]() { return entry.first; };
    get_value = [&entry]() { return emp::to_string(entry.second->GetValue()); };
    snapshot_file.Update();
  }

  // NOTE - discuss any addition configuration settings that it might be useful
  //        to have dumped in a snapshot file.

  // NOTE - Add any additional custom things here.
  config_snapshot_entries.emplace_back(
    "tag_size",
    emp::to_string(hw_spec_t::tag_t::GetCTSize())
  );

  // NOTE - Difficult to get metric out of hw_spec_t w/out
  //        some finagling. Can do it if we want.

  for (const auto& entry : config_snapshot_entries) {
    get_param = [&entry]() { return entry.param; };
    get_value = [&entry]() { return entry.value; };
    snapshot_file.Update();
  }

}

}

#endif