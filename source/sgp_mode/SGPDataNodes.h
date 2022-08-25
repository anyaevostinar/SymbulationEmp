#ifndef SGP_DATA_NODES_H
#define SGP_DATA_NODES_H

#include "SGPWorld.h"

void SGPWorld::CreateDataFiles() {
  std::string file_ending =
      "_SEED" + std::to_string(my_config->SEED()) + ".data";
  // SGP mode doesn't need int val files, and they have significant performance
  // overhead. Only the transmission file needs to be created for this mode.
  SetUpTransmissionFile(my_config->FILE_PATH() + "TransmissionRates" +
                        my_config->FILE_NAME() + file_ending)
      .SetTimingRepeat(my_config->DATA_INT());
  SetupSymDonatedFile(my_config->FILE_PATH() + "SymDonated" +
                      my_config->FILE_NAME() + file_ending)
      .SetTimingRepeat(my_config->DATA_INT());
  if(my_config->TASK_TYPE() == 1){
    SetupTasksFile(my_config->FILE_PATH() + "Tasks" + my_config->FILE_NAME() +
                 file_ending)
        .SetTimingRepeat(my_config->DATA_INT());
  }else if(my_config->TASK_TYPE() == 0){
          SetupHostSquareFrequencyFile(my_config->FILE_PATH() + "Host_Square" + my_config->FILE_NAME() +
                 file_ending)
              .SetTimingRepeat(my_config->DATA_INT());
          SetupSymSquareFrequencyFile(my_config->FILE_PATH() + "Sym_Square" + my_config->FILE_NAME() +
                        file_ending)
                .SetTimingRepeat(my_config->DATA_INT());
  }
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

emp::DataFile &SGPWorld::SetupHostSquareFrequencyFile(const std::string &filename) {
  auto &file = SetupFile(filename);
  file.AddVar(update, "update", "Update");
    std::function<void(std::ostream &)> in_fun = [this](std::ostream & os){
    std::map<uint32_t, uint32_t>squareData = task_set.GetSquareFrequencyData(1);
    for (auto data : squareData){
         os << data.first;
        os << ": ";
        os << data.second;
        os << "; ";
    }
    task_set.ClearSquareFrequencyData(1);
  };
  file.Add(in_fun, "host_square_frequencies", "Host number of repeats for each square");
  file.PrintHeaderKeys();
  return file;
}

emp::DataFile &SGPWorld::SetupSymSquareFrequencyFile(const std::string &filename) {
  auto &file = SetupFile(filename);
  file.AddVar(update, "update", "Update");
  std::function<void(std::ostream &)> in_fun = [this](std::ostream & os){
      std::map<uint32_t, uint32_t>squareData = task_set.GetSquareFrequencyData(0);
      for (auto data : squareData){
         os << data.first;
        os << ": ";
        os << data.second;
        os << "; ";
    }
    task_set.ClearSquareFrequencyData(0);
  };
  file.Add(in_fun, "sym_square_frequencies", "Symbiont number of repeats for each square");
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