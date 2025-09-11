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
  // Setup current update information file
  std::filesystem::path cur_update_info_fpath = output_dir / ("CurrentUpdateInfo.csv");
  SetupCurrentUpdateInfoFile(cur_update_info_fpath).SetTimingRepeat(sgp_config.DATA_INT());
  // TODO - add prefun to above file

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
// TODO - What about sym earned?
// TODO - split this setup function up
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
  // NOTE - The commented out code below was in original, but looks redundant?
  // file.AddTotal(
  //   data_node_sym_donated,
  //   "sym_points_earned",
  //   "Points earned by symbionts",
  //   true
  // );
  file.AddFun<size_t>(
    [this]() {
      return data_node_sym_donated->GetCount();
    },
    "sym_donate_calls",
    "Number of donate calls"
  );
  file.AddTotal(
    *data_node_sym_donated,
    "sym_points_donated",
    "Points donated by symbionts",
    true
  );

  file.AddFun<size_t>(
    [this]() {
      return data_node_sym_stolen->GetCount();
    },
    "sym_steal_calls",
    "Number of steal calls"
  );
  file.AddTotal(
    *data_node_sym_stolen,
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

emp::DataFile& SGPWorld::SetupCurrentUpdateInfoFile(const std::string& filepath) {
  auto& file = SetupFile(filepath);

  // Configure current_update_data
  const size_t num_tasks = task_env.GetTaskCount();
  current_update_data.Reset(num_tasks);

  file.AddPreFun(
    [this]() {
      CollectCurrentUpdateData();
    }
  );

  file.AddVar(update, "update", "World update");

  // Repeated for loops here to make output file easier to see tasks being completed
  // at a glance
  for (size_t task_i = 0; task_i < num_tasks; ++task_i) {
    const std::string& name = task_env.GetTaskSet().GetName(task_i);
    file.AddFun<size_t>(
      [this, task_i]() { return current_update_data.host_task_in_profile_counts[task_i]; },
       name + "_in_host_profile_counts"
    );
  }
  for (size_t task_i = 0; task_i < num_tasks; ++task_i) {
    const std::string& name = task_env.GetTaskSet().GetName(task_i);
    file.AddFun<size_t>(
      [this, task_i]() { return current_update_data.sym_task_in_profile_counts[task_i]; },
      name + "_in_sym_profile_counts"
    );
  }

  for (size_t task_i = 0; task_i < num_tasks; ++task_i) {
    const std::string& name = task_env.GetTaskSet().GetName(task_i);
    file.AddFun<size_t>(
      [this, task_i]() { return current_update_data.host_task_in_parent_org_counts[task_i]; },
      name + "_in_host_parent_org_counts"
    );
  }
  for (size_t task_i = 0; task_i < num_tasks; ++task_i) {
    const std::string& name = task_env.GetTaskSet().GetName(task_i);
    file.AddFun<size_t>(
      [this, task_i]() { return current_update_data.sym_task_in_parent_org_counts[task_i]; },
      name + "_in_sym_parent_org_counts"
    );
  }

  for (size_t task_i = 0; task_i < num_tasks; ++task_i) {
    const std::string& name = task_env.GetTaskSet().GetName(task_i);
    file.AddFun<size_t>(
      [this, task_i]() { return current_update_data.host_task_in_current_org_counts[task_i]; },
      name + "_in_host_current_org_counts"
    );
  }

  for (size_t task_i = 0; task_i < num_tasks; ++task_i) {
    const std::string& name = task_env.GetTaskSet().GetName(task_i);
    file.AddFun<size_t>(
      [this, task_i]() { return current_update_data.sym_task_in_current_org_counts[task_i]; },
      name + "_in_sym_current_org_counts"
    );
  }

  for (size_t task_i = 0; task_i < num_tasks; ++task_i) {
    const std::string& name = task_env.GetTaskSet().GetName(task_i);
    file.AddFun<size_t>(
      [this, task_i]() { return current_update_data.host_sym_profile_matches_by_task[task_i]; },
      name + "_host_sym_profile_matches"
    );
  }
  for (size_t task_i = 0; task_i < num_tasks; ++task_i) {
    const std::string& name = task_env.GetTaskSet().GetName(task_i);
    file.AddFun<size_t>(
      [this, task_i]() { return current_update_data.host_sym_profile_mismatches_by_task[task_i]; },
      name + "_host_sym_profile_mismatches"
    );
  }

  file.AddVar(
    current_update_data.host_sym_perfect_matches_total,
    "host_sym_perfect_matches_total"
  );
  file.AddVar(
    current_update_data.host_sym_any_matches_total,
    "host_sym_any_matches_total"
  );

  file.PrintHeaderKeys();
  return file;
}

void SGPWorld::CollectCurrentUpdateData() {
  // Reset current update data
  current_update_data.Reset();

  for (size_t pop_id = 0; pop_id < max_world_size; ++pop_id) {
    if (IsOccupied(pop_id)) {
      // Occupied by host
      auto& org = GetOrg(pop_id);
      emp_assert(org.IsHost());
      sgp_host_t& host = static_cast<sgp_host_t&>(org);
      // (1) Update host task counts
      const auto& host_task_profile = fun_get_host_task_profile(host);
      auto& host_cpu_state = host.GetHardware().GetCPUState();
      for (size_t task_i = 0; task_i < task_env.GetTaskCount(); ++task_i) {
        current_update_data.host_task_in_profile_counts[task_i] += (size_t)host_task_profile.Get(task_i);
        current_update_data.host_task_in_parent_org_counts[task_i] += (size_t)host_cpu_state.GetParentTaskPerformed(task_i);
        current_update_data.host_task_in_current_org_counts[task_i] += (size_t)host_cpu_state.GetTaskPerformed(task_i);
      }
      // (2) Update endosymbiont task counts + matching/mismatching info
      emp::vector<emp::Ptr<Organism>>& endosyms = host.GetSymbionts();
      for (size_t sym_i = 0; sym_i < endosyms.size(); ++sym_i) {
        emp::Ptr<sgp_sym_t> endosym_ptr = static_cast<sgp_sym_t*>(endosyms[sym_i].Raw());
        const auto& endosym_task_profile = fun_get_sym_task_profile(*endosym_ptr);
        auto& endosym_cpu_state = endosym_ptr->GetHardware().GetCPUState();
        bool any_match = false;
        bool all_match = true;
        for (size_t task_i = 0; task_i < task_env.GetTaskCount(); ++task_i) {
          // Update sym task counts
          current_update_data.sym_task_in_profile_counts[task_i] += (size_t)endosym_task_profile.Get(task_i);
          current_update_data.sym_task_in_parent_org_counts[task_i] += (size_t)endosym_cpu_state.GetParentTaskPerformed(task_i);
          current_update_data.sym_task_in_current_org_counts[task_i] += (size_t)endosym_cpu_state.GetTaskPerformed(task_i);
          // Update matches/mismatches with host
          const bool task_match = (endosym_task_profile.Get(task_i) && host_task_profile.Get(task_i));
          const bool task_mismatch = (endosym_task_profile.Get(task_i) != host_task_profile.Get(task_i));
          current_update_data.host_sym_profile_matches_by_task[task_i] += (size_t)task_match;
          current_update_data.host_sym_profile_mismatches_by_task[task_i] += (size_t)task_mismatch;
          any_match = any_match || task_match; // NOTE - Task match requires that both are doing the task
          all_match = all_match && !task_mismatch;
        }
        // Update perfect / any matches totals
        current_update_data.host_sym_perfect_matches_total += (size_t)all_match;
        current_update_data.host_sym_any_matches_total += (size_t)any_match;
      }
    }

    if (IsSymPopOccupied(pop_id)) {
      // Occupied by free-living symbiont
      emp_assert(false, "Not implemented yet");
    }
  }


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