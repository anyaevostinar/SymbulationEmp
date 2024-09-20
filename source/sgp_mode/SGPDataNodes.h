#ifndef SGP_DATA_NODES_H
#define SGP_DATA_NODES_H

#include "SGPWorld.h"
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