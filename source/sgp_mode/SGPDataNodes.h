#ifndef SGP_DATA_NODES_H
#define SGP_DATA_NODES_H

#include "SGPWorld.h"
#include "SGPHost.h"
#include "SGPSymbiont.h"
#include <cstddef>
#include <cstdint>
#include <limits>

void SGPWorld::CreateDataFiles() {
  std::string file_ending =
      "_SEED" + std::to_string(sgp_config->SEED()) + ".data";
  // track the number of each organism type
  SetUpOrgCountFile(sgp_config->FILE_PATH() + "OrganismCounts" +
    sgp_config->FILE_NAME() + file_ending)
    .SetTimingRepeat(sgp_config->DATA_INT());

  // SGP mode doesn't need int val files, and they have significant performance
  // overhead. Only the transmission file needs to be created for this mode.  
  SetUpTransmissionFile(sgp_config->FILE_PATH() + "TransmissionRates" +
                        sgp_config->FILE_NAME() + file_ending)
      .SetTimingRepeat(sgp_config->DATA_INT());
  SetupSymDonatedFile(sgp_config->FILE_PATH() + "SymDonated" +
                      sgp_config->FILE_NAME() + file_ending)
      .SetTimingRepeat(sgp_config->DATA_INT());
  SetupTasksFile(sgp_config->FILE_PATH() + "Tasks" + sgp_config->FILE_NAME() +
                  file_ending)
      .SetTimingRepeat(sgp_config->DATA_INT());
}


/**
 * Input: The address of the string representing the file to be
 * created's name
 *
 * Output: The address of the DataFile that has been created.
 *
 * Purpose: To set up the file that will be used to track 
 * organism counts in the world.
 *  This includes: (1) the host count, (2) the hosted symbiont
 * count, and (2b) the free living symbiont count, if free living
 * symbionts are permitted.
 */
emp::DataFile &SGPWorld::SetUpOrgCountFile(const std::string &filename) {
  auto& file = SetupFile(filename);
  auto& host_count = GetHostCountDataNode();
  auto& endo_sym_count = GetCountHostedSymsDataNode();
  
  file.AddVar(update, "update", "Update");
  file.AddTotal(host_count, "count", "Total number of hosts");
  file.AddTotal(endo_sym_count, "hosted_syms", "Total number of syms in a host");
  
  if (sgp_config->FREE_LIVING_SYMS()) {
    auto& free_sym_count = GetCountFreeSymsDataNode();
    file.AddTotal(free_sym_count, "free_syms", "Total number of free syms");
  }

  file.PrintHeaderKeys();
  
  return file;
}

emp::DataFile &SGPWorld::SetupTasksFile(const std::string &filename) {
  auto &file = SetupFile(filename);
  file.AddVar(update, "update", "Update");
  SetupTasksNodes();
  int i = 0;
  for (auto data : task_set) {
    file.AddTotal(data_node_host_tasks[i], "host_task_" + data.task.name,
                  "Host completions of " + data.task.name, true);
    file.AddTotal(data_node_sym_tasks[i], "sym_task_" + data.task.name,
                  "Symbiont completions of " + data.task.name, true);
    i++;
  }
  file.PrintHeaderKeys();

  return file;
}

emp::DataFile &SGPWorld::SetupSymDonatedFile(const std::string &filename) {
  auto &file = SetupFile(filename);
  file.AddVar(update, "update", "Update");
  GetSymEarnedDataNode();
  file.AddTotal(data_node_sym_earned->UnsynchronizedGetMonitor(),
                "sym_points_earned", "Points earned by symbionts", true);
  GetSymDonatedDataNode();
  file.AddFun<size_t>(
      [&]() {
        return data_node_sym_donated->UnsynchronizedGetMonitor().GetCount();
      },
      "sym_donate_calls", "Number of donate calls");
  file.AddTotal(data_node_sym_donated->UnsynchronizedGetMonitor(),
                "sym_points_donated", "Points donated by symbionts", true);
  GetSymStolenDataNode();
  file.AddFun<size_t>(
      [&]() {
        return data_node_sym_stolen->UnsynchronizedGetMonitor().GetCount();
      },
      "sym_steal_calls", "Number of steal calls");
  file.AddTotal(data_node_sym_stolen->UnsynchronizedGetMonitor(),
                "sym_points_stolen", "Points stolen by symbionts", true);
  file.PrintHeaderKeys();
  return file;
}

/**
 * Input: The address of the string representing the file to be
 * created's name
 *
 * Output: None
 *
 * Purpose: To write the count of organisms whose parents achieved
 * each combination of tasks.
 */
void SGPWorld::WriteTaskCombinationsFile(const std::string& filename) {
  std::ofstream out_file(filename);

  // loop through the whole population and keep track of the count of parents
  // who satisfy each tag combination
  std::unordered_map<std::string, int[4]> matching_task_counts;
  for (size_t i = 0; i < pop.size(); i++) {
    if (!IsOccupied(i)) continue;
    emp::Ptr<SGPHost> host = pop[i].DynamicCast<SGPHost>();
    std::string host_matching_tasks = (sgp_config->TRACK_PARENT_TASKS()) ? 
                                    host->GetCPU().state.parent_tasks_performed->ToBinaryString() : 
                                    host->GetCPU().state.tasks_performed->ToBinaryString();
    if (emp::Has(matching_task_counts, host_matching_tasks)) {
      matching_task_counts[host_matching_tasks][0]++;
    }
    else {
      matching_task_counts[host_matching_tasks][0] = 1;// { 1, 0, 0, 0 };

    }
    emp::vector<emp::Ptr<Organism>> syms = host->GetSymbionts();
    for (size_t j = 0; j < syms.size(); j++) {
      std::string sym_matching_tasks = (sgp_config->TRACK_PARENT_TASKS()) ?
        syms[j].DynamicCast<SGPSymbiont>()->GetCPU().state.parent_tasks_performed->ToBinaryString() :
        syms[j].DynamicCast<SGPSymbiont>()->GetCPU().state.tasks_performed->ToBinaryString();
      if (emp::Has(matching_task_counts, sym_matching_tasks)) {
        matching_task_counts[sym_matching_tasks][1]++;
      }
      else {
        matching_task_counts[sym_matching_tasks][1] = 1;// = { 0, 1, 0, 0 };
      }
    }
  }

  // bits construction from string takes a loop, so just loop here
  for (auto it = matching_task_counts.begin(); 
    it != matching_task_counts.end(); it++){
    for (auto interior_it = it; interior_it != matching_task_counts.end(); interior_it++) {
      if (interior_it == it) continue;
      // bit operation do and <- need to have keys be bitsets and not bitstrings
      bool can_infect = false;

      for (int i = CPU_BITSET_LENGTH - 1; i >= 0 && !can_infect; i--) {
        if (it->first[i] == interior_it->first[i] && interior_it->first[i] == '1') {
          can_infect = true;
        }
      }
      if (can_infect) {
        // [2] counts [0] hosts which can infect, [3] counts [1] symbionts which can infect
        it->second[2] += interior_it->second[0];
        it->second[3] += interior_it->second[1];
        interior_it->second[2] += it->second[0];
        interior_it->second[3] += it->second[1];
      }

    }
  }


  // write all the combinations and counts to the data file
  out_file << "parent_task_completions,host_count,symbiont_count,can_inf_hosts,can_inf_symbionts\n";
  for (auto& key_value : matching_task_counts) {
    // each "key_value" is a bitstring key with a pair (int,int) value
    out_file << key_value.first << "," << std::to_string(key_value.second[0]) << ",";
    out_file << std::to_string(key_value.second[1]) << ',' << std::to_string(key_value.second[2]);
    out_file << ',' << std::to_string(key_value.second[3]) << "\n";
  }

  out_file.close();
}

/**
 * Input: The address of the string representing the file to be
 * created's name
 *
 * Output: None
 *
 * Purpose: To write the reproductive and mutation history 
 * of all hosts and symbionts
 */
void SGPWorld::WriteOrgReproHistFile(const std::string& filename) {
  std::ofstream out_file(filename);
  out_file << "org_type,repro_count";
  for (auto task : task_set) {
    out_file << ",gained_count_" << task.task.name << ",lost_count_" << task.task.name;
  }
  out_file << "\n";

  emp::Ptr<SGPHost> host;
  emp::Ptr<SGPSymbiont> symbiont;

  for (int i = 0; i < size(); i++) {
    if (IsOccupied(i)) {
      host = pop[i].DynamicCast<SGPHost>();
      out_file << "host," << host->GetReproCount();
      for (int k = 0; k < CPU_BITSET_LENGTH; k++) {
        out_file << "," << host->GetCPU().state.task_change_gain[k] << "," << host->GetCPU().state.task_change_lose[k];
      }
      out_file << "\n";

      if (host->HasSym()) {
        emp::vector<emp::Ptr<Organism>> syms = host->GetSymbionts();
        for (size_t j = 0; j < syms.size(); j++) {
          symbiont = syms[j].DynamicCast<SGPSymbiont>();

          out_file << "sym," << symbiont->GetReproCount();
          for (int k = 0; k < CPU_BITSET_LENGTH; k++) {
            out_file << "," << symbiont->GetCPU().state.task_change_gain[k] << "," << symbiont->GetCPU().state.task_change_lose[k];
          }
          out_file << "\n";
        }
      }
    }
  }

  out_file.close();
}


void SGPWorld::SetupTasksNodes() {
  if (!data_node_host_tasks.size()) {
    data_node_host_tasks.resize(task_set.NumTasks());
    data_node_sym_tasks.resize(task_set.NumTasks());
    OnUpdate([&](auto) {
      int i = 0;
      for (auto data : task_set) {
        data_node_host_tasks[i].AddDatum(data.n_succeeds_host);
        data_node_sym_tasks[i].AddDatum(data.n_succeeds_sym);
        i++;
      }
      task_set.ResetTaskData();
    });
  }
}

SyncDataMonitor<double> &SGPWorld::GetSymEarnedDataNode() {
  if (!data_node_sym_earned) {
    data_node_sym_earned.New();
  }
  return *data_node_sym_earned;
}

SyncDataMonitor<double> &SGPWorld::GetSymDonatedDataNode() {
  if (!data_node_sym_donated) {
    data_node_sym_donated.New();
  }
  return *data_node_sym_donated;
}

SyncDataMonitor<double> &SGPWorld::GetSymStolenDataNode() {
  if (!data_node_sym_stolen) {
    data_node_sym_stolen.New();
  }
  return *data_node_sym_stolen;
}

#endif