#ifndef SGP_DATA_NODES_C
#define SGP_DATA_NODES_C

#include "SGPWorld.h"
#include "SGPHost.h"
#include "SGPSymbiont.h"

#include "emp/datastructs/map_utils.hpp"
#include "emp/math/info_theory.hpp"
#include "emp/math/stats.hpp"

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
  SetupTransmissionFile(transmission_fpath.string()).SetTimingRepeat(sgp_config.DATA_INT());
  // // Setup sym donated file
  // std::filesystem::path sym_donated_fpath = output_dir / ("SymDonated.csv");
  // SetupSymDonatedFile(sym_donated_fpath.string()).SetTimingRepeat(sgp_config.DATA_INT());
  // Setup tasks file
  std::filesystem::path tasks_fpath = output_dir / ("Tasks.csv");
  SetupTasksFile(tasks_fpath).SetTimingRepeat(sgp_config.DATA_INT());
  // Setup current update information file
  std::filesystem::path cur_update_info_fpath = output_dir / ("CurrentUpdateInfo.csv");
  SetupCurrentUpdateInfoFile(cur_update_info_fpath).SetTimingRepeat(sgp_config.DATA_INT());
  // Setup file for symbiont interaction values
  std::filesystem::path sym_int_vals_fpath = output_dir / ("SymbiontInteractionValues.csv");
  SetupSymbiontInteractionValuesFile(sym_int_vals_fpath).SetTimingRepeat(sgp_config.DATA_INT());
}

emp::DataFile& SGPWorld::SetupOrgCountFile(const std::string& filepath) {
  auto& file = SetupFile(filepath);
  // NOTE: Not using GetHostCountDataNode() to create to avoid adding
  //       counting loop to OnUpdate. Just call counting loop before outputting
  //       count file line.
  // (1) Create host count data node
  if (data_node_hostcount != nullptr) {
    data_node_hostcount.Delete();
  }
  data_node_hostcount.New();
  // (2) Create hosted syms data node
  if (data_node_hostedsymcount != nullptr) {
    data_node_hostedsymcount.Delete();
  }
  data_node_hostedsymcount.New();
  // (3) Create free syms data node
  if (data_node_freesymcount != nullptr) {
    data_node_freesymcount.Delete();
  }
  data_node_freesymcount.New();

  // auto& host_count = GetHostCountDataNode();
  // auto& endosym_count = GetCountHostedSymsDataNode();
  // auto& freeliving_sym_count = GetCountFreeSymsDataNode();

  // Calculate counts before updating file.
  // NOTE - this happens less frequently than what base class's data nodes would
  //        be updated!
  file.AddPreFun(
    [this]() {
      // Update: host count node, hosted syms node, free syms node
      data_node_hostcount->Reset();
      data_node_hostedsymcount->Reset();
      data_node_freesymcount->Reset();
      for (size_t pop_i = 0; pop_i < pop.size(); ++pop_i) {
        if (IsOccupied(pop_i)) {
          data_node_hostcount->AddDatum(1);
          data_node_hostedsymcount->AddDatum(pop[pop_i]->GetSymbionts().size());
        }
        if (sym_pop[pop_i]) {
          data_node_freesymcount->AddDatum(1);
        }
      }
    }
  );

  file.AddVar(update, "update", "World update");
  file.AddTotal(*data_node_hostcount, "host_count", "Total number of hosts");
  file.AddTotal(*data_node_hostedsymcount, "hosted_sym_count", "Total number of syms in a host");
  // NOTE - this was previously only on file if in free living syms mode
  //        do we want to have a variable number of columns per run?
  file.AddTotal(*data_node_freesymcount, "free_sym_count", "Total number of free-living syms");
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

emp::DataFile& SGPWorld::SetupSymbiontInteractionValuesFile(const std::string& filepath) {
  // NOTE - Writing a new function here instead of using SetupSymIntValFile in base class
  //        to optimize timing of looping over population to collect sym interaction values
  //        Base class version loops every update. Don't need to loop so often.
  //        Can just collect data before updating output file.
  auto& file = SetupFile(filepath);
  // Setup symintval data node
  // Not using GetSymIntValDataNode to allow optimization of when we
  // scan population for these data.
  if (data_node_symintval != nullptr) {
    data_node_symintval.Delete();
  }
  data_node_symintval.New();
  // Create symcount node
  if (data_node_symcount != nullptr) {
    data_node_symcount.Delete();
  }
  data_node_symcount.New();
  // SetupBins pulled from GetSymIntValDataNode() funciton in base class.
  data_node_symintval->SetupBins(-1.0, 1.1, 21);

  file.AddPreFun(
    [this]() {
      // Assert that organism count data nodes have been updated for this update.
      // Should happen for OrgCounts data file
      data_node_symintval->Reset();
      data_node_symcount->Reset();
      for (size_t pop_i = 0; pop_i < pop.size(); ++pop_i) {
        if (IsOccupied(pop_i)) {
          emp::vector<emp::Ptr<Organism>>& syms = pop[pop_i]->GetSymbionts();
          data_node_symcount->AddDatum(syms.size());
          for (size_t sym_i = 0; sym_i < syms.size(); ++sym_i) {
            data_node_symintval->AddDatum(syms[sym_i]->GetIntVal());
          }
        }
        if (sym_pop[pop_i]) {
          data_node_symintval->AddDatum(sym_pop[pop_i]->GetIntVal());
          data_node_symcount->AddDatum(1);
        }
      }
    }
  );

  file.AddVar(update, "update", "Update");
  file.AddMean(*data_node_symintval, "mean_intval", "Average symbiont interaction value");
  file.AddTotal(*data_node_symcount, "count", "Total number of symbionts");

  //interaction val histogram
  file.AddHistBin(*data_node_symintval, 0, "Hist_-1", "Count for histogram bin -1 to <-0.9");
  file.AddHistBin(*data_node_symintval, 1, "Hist_-0.9", "Count for histogram bin -0.9 to <-0.8");
  file.AddHistBin(*data_node_symintval, 2, "Hist_-0.8", "Count for histogram bin -0.8 to <-0.7");
  file.AddHistBin(*data_node_symintval, 3, "Hist_-0.7", "Count for histogram bin -0.7 to <-0.6");
  file.AddHistBin(*data_node_symintval, 4, "Hist_-0.6", "Count for histogram bin -0.6 to <-0.5");
  file.AddHistBin(*data_node_symintval, 5, "Hist_-0.5", "Count for histogram bin -0.5 to <-0.4");
  file.AddHistBin(*data_node_symintval, 6, "Hist_-0.4", "Count for histogram bin -0.4 to <-0.3");
  file.AddHistBin(*data_node_symintval, 7, "Hist_-0.3", "Count for histogram bin -0.3 to <-0.2");
  file.AddHistBin(*data_node_symintval, 8, "Hist_-0.2", "Count for histogram bin -0.2 to <-0.1");
  file.AddHistBin(*data_node_symintval, 9, "Hist_-0.1", "Count for histogram bin -0.1 to <0.0");
  file.AddHistBin(*data_node_symintval, 10, "Hist_0.0", "Count for histogram bin 0.0 to <0.1");
  file.AddHistBin(*data_node_symintval, 11, "Hist_0.1", "Count for histogram bin 0.1 to <0.2");
  file.AddHistBin(*data_node_symintval, 12, "Hist_0.2", "Count for histogram bin 0.2 to <0.3");
  file.AddHistBin(*data_node_symintval, 13, "Hist_0.3", "Count for histogram bin 0.3 to <0.4");
  file.AddHistBin(*data_node_symintval, 14, "Hist_0.4", "Count for histogram bin 0.4 to <0.5");
  file.AddHistBin(*data_node_symintval, 15, "Hist_0.5", "Count for histogram bin 0.5 to <0.6");
  file.AddHistBin(*data_node_symintval, 16, "Hist_0.6", "Count for histogram bin 0.6 to <0.7");
  file.AddHistBin(*data_node_symintval, 17, "Hist_0.7", "Count for histogram bin 0.7 to <0.8");
  file.AddHistBin(*data_node_symintval, 18, "Hist_0.8", "Count for histogram bin 0.8 to <0.9");
  file.AddHistBin(*data_node_symintval, 19, "Hist_0.9", "Count for histogram bin 0.9 to 1.0");

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

  file.AddFun<double>(
    [this]() -> double {
      const double avg_gen = (current_update_data.host_generations.size()) ?
        emp::Mean(current_update_data.host_generations) :
        0;
      return avg_gen;
    },
    "host_mean_generations"
  );

  file.AddFun<double>(
    [this]() -> double {
      const double var_gen = (current_update_data.host_generations.size()) ?
        emp::Variance(current_update_data.host_generations) :
        0;
      return var_gen;
    },
    "host_variance_generations"
  );

  file.AddFun<double>(
    [this]() -> double {
      const double avg_gen = (current_update_data.sym_generations.size()) ?
        emp::Mean(current_update_data.sym_generations) :
        0;
      return avg_gen;
    },
    "sym_mean_generations"
  );

  file.AddFun<double>(
    [this]() -> double {
      const double var_gen = (current_update_data.sym_generations.size()) ?
        emp::Variance(current_update_data.sym_generations) :
        0;
      return var_gen;
    },
    "sym_variance_generations"
  );

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

  // Record task profile diversity measures
  file.AddFun<size_t>(
    [this]() -> size_t {
      return current_update_data.host_parent_tasks_performed.size();
    },
    "host_parent_num_task_sets",
    "How many distinct task sets represented among all host parent tasks performed?"
  );
  file.AddFun<double>(
    [this]() -> double {
      const auto& task_set_counts = current_update_data.host_parent_tasks_performed;
      emp::vector<size_t> counts(task_set_counts.size());
      utils::CollectMapValues(task_set_counts, counts);
      // Calculate entropy
      return emp::Entropy(counts);
    },
    "host_parent_entropy_task_sets",
    "Entropy of distinct task sets represented among all host parent tasks performed."
  );

  file.AddFun<size_t>(
    [this]() -> size_t {
      return current_update_data.host_current_tasks_performed.size();
    },
    "host_current_num_task_sets",
    "How many distinct task sets represented among all host current tasks performed?"
  );
  file.AddFun<double>(
    [this]() -> double {
      const auto& task_set_counts = current_update_data.host_current_tasks_performed;
      emp::vector<size_t> counts(task_set_counts.size());
      utils::CollectMapValues(task_set_counts, counts);
      // Calculate entropy
      return emp::Entropy(counts);
    },
    "host_current_entropy_task_sets",
    "Entropy of distinct task sets represented among all host current tasks performed."
  );

  file.AddFun<size_t>(
    [this]() -> size_t {
      return current_update_data.sym_parent_tasks_performed.size();
    },
    "sym_parent_num_task_sets",
    "How many distinct task sets represented among all sym parent tasks performed?"
  );
  file.AddFun<double>(
    [this]() -> double {
      const auto& task_set_counts = current_update_data.sym_parent_tasks_performed;
      emp::vector<size_t> counts(task_set_counts.size());
      utils::CollectMapValues(task_set_counts, counts);
      // Calculate entropy
      return emp::Entropy(counts);
    },
    "sym_parent_entropy_task_sets",
    "Entropy of distinct task sets represented among all host parent tasks performed."
  );

  file.AddFun<size_t>(
    [this]() -> size_t {
      return current_update_data.sym_current_tasks_performed.size();
    },
    "sym_current_num_task_sets",
    "How many distinct task sets represented among all sym current tasks performed?"
  );
  file.AddFun<double>(
    [this]() -> double {
      const auto& task_set_counts = current_update_data.sym_current_tasks_performed;
      emp::vector<size_t> counts(task_set_counts.size());
      utils::CollectMapValues(task_set_counts, counts);
      // Calculate entropy
      return emp::Entropy(counts);
    },
    "sym_current_entropy_task_sets",
    "Entropy of distinct task sets represented among all sym current tasks performed."
  );

  file.PrintHeaderKeys();
  return file;
}

// NOTE - This isn't setup to handle free-living symbionts!
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
      // (1.5) Update host generations
      current_update_data.host_generations.emplace_back(
        host.GetReproCount()
      );
      for (size_t task_i = 0; task_i < task_env.GetTaskCount(); ++task_i) {
        current_update_data.host_task_in_profile_counts[task_i] += (size_t)host_task_profile.Get(task_i);
        current_update_data.host_task_in_parent_org_counts[task_i] += (size_t)host_cpu_state.GetParentTaskPerformed(task_i);
        current_update_data.host_task_in_current_org_counts[task_i] += (size_t)host_cpu_state.GetTaskPerformed(task_i);
      }
      // - Update counts of host parent/current task performance profiles
      utils::AddToCountingMap(
        current_update_data.host_parent_tasks_performed,
        host_cpu_state.GetParentTasksPerformed()
      );
      utils::AddToCountingMap(
        current_update_data.host_current_tasks_performed,
        host_cpu_state.GetTasksPerformed()
      );
      // (2) Update endosymbiont task counts + matching/mismatching info
      emp::vector<emp::Ptr<Organism>>& endosyms = host.GetSymbionts();
      for (size_t sym_i = 0; sym_i < endosyms.size(); ++sym_i) {
        emp::Ptr<sgp_sym_t> endosym_ptr = static_cast<sgp_sym_t*>(endosyms[sym_i].Raw());
        // Update symbiont generations
        current_update_data.sym_generations.emplace_back(
          endosym_ptr->GetReproCount()
        );
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
        // Update endosymbiont parent/current task performance counts
        utils::AddToCountingMap(
          current_update_data.sym_parent_tasks_performed,
          endosym_cpu_state.GetParentTasksPerformed()
        );
        utils::AddToCountingMap(
          current_update_data.sym_current_tasks_performed,
          endosym_cpu_state.GetTasksPerformed()
        );
      }
    }

    if (IsSymPopOccupied(pop_id)) {
      // Occupied by free-living symbiont
      emp_assert(false, "Not implemented yet");
    }
  }

  // Calculate diversity of task profiles
  // entropy_host_parent_task_sets
  // entropy_host_current_task_sets
  // entropy_sym_parent_task_sets
  // entropy_sym_current_task_sets

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