#ifndef SGP_DATA_NODES_C
#define SGP_DATA_NODES_C

#include "SGPWorld.h"
#include "SGPHost.h"
#include "SGPSymbiont.h"

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
  std::filesystem::path org_count_fpath = output_dir / ("OrganismCounts.csv");
  SetupOrgCountFile(org_count_fpath.string()).SetTimingRepeat(sgp_config.DATA_INT());
  // Setup transmission file
  std::filesystem::path transmission_fpath = output_dir / ("TransmissionRates.csv");
  SetUpTransmissionFile(transmission_fpath.string()).SetTimingRepeat(sgp_config.DATA_INT());
  // Setup sym donated file
  std::filesystem::path sym_donated_fpath = output_dir / ("SymDonated.csv");
  SetupSymDonatedFile(sym_donated_fpath.string()).SetTimingRepeat(sgp_config.DATA_INT());
  // Setup tasks file
  std::filesystem::path tasks_fpath = output_dir / ("Tasks.csv");
  SetupTasksFile(tasks_fpath).SetTimingRepeat(sgp_config.DATA_INT());
  // BOOKMARK
}

emp::DataFile& SGPWorld::SetupOrgCountFile(const std::string& filepath) {
  auto& file = SetupFile(filepath);
  auto& host_count = GetHostCountDataNode();
  auto& endosym_count = GetCountHostedSymsDataNode();
  auto& freeliving_sym_count = GetCountFreeSymsDataNode();

  file.AddVar(update, "update", "World update");
  file.AddTotal(host_count, "host_count", "Total number of hosts");
  file.AddTotal(endosym_count, "hosted_sym_count", "Total number of syms in a host");
  // NOTE - this was previously only on file if in free living syms mode
  //        do we want to have a variable number of columns per run?
  file.AddTotal(freeliving_sym_count, "free_sym_count", "Total number of free-living syms");
  file.PrintHeaderKeys();
  return file;
}

// TODO - update statistics as necessary
emp::DataFile& SGPWorld::SetupSymDonatedFile(const std::string& filepath) {
  // If data node already exists, delete it.
  if (data_node_sym_earned != nullptr) {
    data_node_sym_earned.Delete();
  }
  data_node_sym_earned.New();

  if (data_node_sym_donated != nullptr) {
    data_node_sym_donated.Delete();
  }
  data_node_sym_donated.New();

  if (data_node_sym_stolen != nullptr) {
    data_node_sym_stolen.Delete();
  }
  data_node_sym_stolen.New();
  // TODO - update if threading is refactored
  auto& file = SetupFile(filepath);
  file.AddVar(update, "update", "World update");
  file.AddTotal(
    data_node_sym_donated->UnsynchronizedGetMonitor(),
    "sym_points_earned",
    "Points earned by symbionts",
    true
  );
  file.AddFun<size_t>(
    [this]() {
      return data_node_sym_donated->UnsynchronizedGetMonitor().GetCount();
    },
    "sym_donate_calls",
    "Number of donate calls"
  );
  file.AddTotal(
    data_node_sym_donated->UnsynchronizedGetMonitor(),
    "sym_points_donated",
    "Points donated by symbionts",
    true
  );
  file.AddFun<size_t>(
    [this]() {
      return data_node_sym_stolen->UnsynchronizedGetMonitor().GetCount();
    },
    "sym_steal_calls",
    "Number of steal calls"
  );
  file.AddTotal(
    data_node_sym_stolen->UnsynchronizedGetMonitor(),
    "sym_points_stolen",
    "Points stolen by symbionts",
    true
  );

  file.PrintHeaderKeys();
  return file;
}

emp::DataFile& SGPWorld::SetupTasksFile(const std::string& filepath) {
  // NOTE - data nodes are potentially extra overhead that we're not making use of.
  data_node_host_tasks.clear();
  data_node_sym_tasks.clear();
  data_node_host_tasks.resize(task_env.GetTaskCount());
  data_node_sym_tasks.resize(task_env.GetTaskCount());

  host_task_successes.clear();
  sym_task_successes.clear();
  host_task_successes.resize(task_env.GetTaskCount(), 0);
  sym_task_successes.resize(task_env.GetTaskCount(), 0);

  // Reset task counts at beginning of every update
  begin_update_sig.AddAction(
    [this](){
      std::fill(
        host_task_successes.begin(),
        host_task_successes.end(),
        0
      );
      std::fill(
        sym_task_successes.begin(),
        sym_task_successes.end(),
        0
      );
    }
  );

  // World's on update signal happens at end of update but before data files are
  // output.
  OnUpdate(
    [this](size_t) {
      for (size_t task_i = 0; task_i < task_env.GetTaskCount(); ++task_i) {
        data_node_host_tasks[task_i].AddDatum(host_task_successes[task_i]);
        data_node_sym_tasks[task_i].AddDatum(sym_task_successes[task_i]);
      }
    }
  );

  auto& file = SetupFile(filepath);
  file.AddVar(update, "update", "World update");
  // Add columns for host tasks
  for (size_t task_i = 0; task_i < host_task_successes.size(); ++task_i) {
    const std::string& name = task_env.GetTaskSet().GetName(task_i);
    // file.AddVar(
    //   host_task_successes[task_i],
    //   "host_task_" + name,
    //   "Host completions of " + name
    // );
    // NOTE - data node is potentially extra overhead that we're not making use of.
    file.AddTotal(
      data_node_host_tasks[task_i],
      "host_task_" + name,
      "Host completions of " + name,
      true
    );
  }
  // Add columns for sym tasks
  for (size_t task_i = 0; task_i < sym_task_successes.size(); ++task_i) {
    const std::string& name = task_env.GetTaskSet().GetName(task_i);
    // file.AddVar(
    //   sym_task_successes[task_i],
    //   "sym_task_" + name,
    //   "Symbiont completions of " + name
    // );
    // NOTE - data node is potentially extra overhead that we're not making use of.
    file.AddTotal(
      data_node_sym_tasks[task_i],
      "sym_task_" + name,
      "Symbiont completions of " + name,
      true
    );
  }

  file.PrintHeaderKeys();
  return file;
}

void SGPWorld::SnapshotConfig(const std::string& filename) {
  std::filesystem::path fpath = output_dir / filename;
  emp::DataFile snapshot_file(fpath.string());
  std::function<std::string(void)> get_param;   // Parameter name
  std::function<std::string(void)> get_value;   // Parameter value
  // std::function<std::string(void)> get_source;
  // TODO - add tasks (in order)
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


  config_snapshot_entries.emplace_back(
    "tag_size",
    emp::to_string(hw_spec_t::tag_t::GetCTSize())
  );
  // NOTE - Difficult to get metric out of hw_spec_t w/out
  //        some finagling. Can do it if we want.

  // Add list of all tasks to config snapshot
  emp::vector<std::string> task_names;
  emp::vector<std::string> host_task_names;
  emp::vector<std::string> sym_task_names;
  for (size_t task_i = 0; task_i < task_env.GetTaskCount(); ++task_i) {
    task_names.emplace_back(task_env.GetTaskSet().GetName(task_i));
    if (task_env.IsHostTask(task_i)) {
      host_task_names.emplace_back(task_env.GetTaskSet().GetName(task_i));
    }
    if (task_env.IsSymTask(task_i)) {
      sym_task_names.emplace_back(task_env.GetTaskSet().GetName(task_i));
    }
  }
  config_snapshot_entries.emplace_back(
    "tasks",
    emp::to_string(task_names)
  );
  config_snapshot_entries.emplace_back(
    "host_tasks",
    emp::to_string(host_task_names)
  );
  config_snapshot_entries.emplace_back(
    "sym_tasks",
    emp::to_string(sym_task_names)
  );

  for (const auto& entry : config_snapshot_entries) {
    get_param = [&entry]() { return entry.param; };
    get_value = [&entry]() { return entry.value; };
    snapshot_file.Update();
  }

}

}

#endif