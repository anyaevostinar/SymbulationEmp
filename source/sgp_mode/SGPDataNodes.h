#ifndef SGP_DATA_NODES_H
#define SGP_DATA_NODES_H

#include "SGPWorld.h"

void SGPWorld::CreateDataFiles() {
  std::string file_ending =
      "_SEED" + std::to_string(my_config->SEED()) + ".data";
  SymWorld::CreateDateFiles();
  SetupSymDonatedFile(my_config->FILE_PATH() + "SymDonated" +
                      my_config->FILE_NAME() + file_ending)
      .SetTimingRepeat(my_config->DATA_INT());
  SetupTasksFile(my_config->FILE_PATH() + "Tasks" + my_config->FILE_NAME() +
                 file_ending)
      .SetTimingRepeat(my_config->DATA_INT());
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
  file.AddTotal(data_node_sym_donated->UnsynchronizedGetMonitor(),
                "sym_points_donated", "Points donated by symbionts", true);
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

#endif