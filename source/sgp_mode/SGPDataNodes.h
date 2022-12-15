#ifndef SGP_DATA_NODES_H
#define SGP_DATA_NODES_H

#include "SGPWorld.h"
#include <cstddef>
#include <cstdint>
#include <limits>

void SGPWorld::CreateDataFiles() {
  std::string file_ending =
      "_SEED" + std::to_string(my_config->SEED()) + ".data";
  // track the number of each organism type
  SetUpOrgCountFile(my_config->FILE_PATH() + "OrganismCounts" +
    my_config->FILE_NAME() + file_ending)
    .SetTimingRepeat(my_config->DATA_INT());

  // SGP mode doesn't need int val files, and they have significant performance
  // overhead. Only the transmission file needs to be created for this mode.  
  SetUpTransmissionFile(my_config->FILE_PATH() + "TransmissionRates" +
                        my_config->FILE_NAME() + file_ending)
      .SetTimingRepeat(my_config->DATA_INT());
  SetupSymDonatedFile(my_config->FILE_PATH() + "SymDonated" +
                      my_config->FILE_NAME() + file_ending)
      .SetTimingRepeat(my_config->DATA_INT());
  if (my_config->TASK_TYPE() == 1) {
    SetupTasksFile(my_config->FILE_PATH() + "Tasks" + my_config->FILE_NAME() +
                   file_ending)
        .SetTimingRepeat(my_config->DATA_INT());
  } else if (my_config->TASK_TYPE() == 0) {
    SetupHostSquareFrequencyFile(my_config->FILE_PATH() + "Host_Square" +
                                 my_config->FILE_NAME() + file_ending)
        .SetTimingRepeat(my_config->DATA_INT());
    SetupSymSquareFrequencyFile(my_config->FILE_PATH() + "Sym_Square" +
                                my_config->FILE_NAME() + file_ending)
        .SetTimingRepeat(my_config->DATA_INT());
  }
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
  
  if (my_config->FREE_LIVING_SYMS()) {
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

emp::DataFile &
SGPWorld::SetupHostSquareFrequencyFile(const std::string &filename) {
  auto &file = SetupFile(filename);
  file.AddVar(update, "update", "Update");
  file.Add(
      [&](std::ostream &os) {
        for (auto [val, count] : data_node_host_squares) {
          os << val << ": " << count << "; ";
        }
        data_node_host_squares.clear();
      },
      "host_square_frequencies", "Host number of repeats for each square");
  file.PrintHeaderKeys();
  return file;
}

emp::DataFile &
SGPWorld::SetupSymSquareFrequencyFile(const std::string &filename) {
  auto &file = SetupFile(filename);
  file.AddVar(update, "update", "Update");
  file.Add(
      [&](std::ostream &os) {
        for (auto [val, count] : data_node_sym_squares) {
          os << val << ": " << count << "; ";
        }
        data_node_sym_squares.clear();
      },
      "sym_square_frequencies", "Sym number of repeats for each square");
  file.PrintHeaderKeys();

  return file;
}

emp::DataFile &SGPWorld::SetupSymDonatedFile(const std::string &filename) {
  auto &file = SetupFile(filename);
  file.AddVar(update, "update", "Update");
  GetHostEarnedDataNode();
  file.AddTotal(data_node_host_earned->UnsynchronizedGetMonitor(),
    "host_points_earned", "Points earned by hosts", true);

  std::string prefix = ""; std::string suffix = "by symbionts";
  if (my_config->FREE_LIVING_SYMS() == 1) {
    prefix = "hosted_";
    suffix = "by hosted symbionts";
  }

  GetHostedSymEarnedDataNode();
  file.AddTotal(data_node_hosted_sym_earned->UnsynchronizedGetMonitor(),
      (prefix + "sym_points_earned"), ("Points earned " + suffix), true);
  GetHostedSymDonatedDataNode();
  file.AddFun<size_t>(
      [&]() {
        return data_node_hosted_sym_donated->UnsynchronizedGetMonitor().GetCount();
      },
      (prefix + "sym_donate_calls"), ("Number of donate calls " + suffix));
  file.AddTotal(data_node_hosted_sym_donated->UnsynchronizedGetMonitor(),
                (prefix + "sym_points_donated"), ("Points donated " + suffix), true);
  GetHostedSymStolenDataNode();
  file.AddFun<size_t>(
      [&]() {
        return data_node_hosted_sym_stolen->UnsynchronizedGetMonitor().GetCount();
      },
      (prefix + "sym_steal_calls"), ("Number of steal calls " + suffix));
  file.AddTotal(data_node_hosted_sym_stolen->UnsynchronizedGetMonitor(),
                (prefix + "sym_points_stolen"), ("Points stolen " + suffix), true);

  if (my_config->FREE_LIVING_SYMS() == 1) {
    GetFreeSymEarnedDataNode();
    file.AddTotal(data_node_free_sym_earned->UnsynchronizedGetMonitor(),
      "free_sym_points_earned", "Points earned by free living symbionts", true);
    GetFreeSymDonatedDataNode();
    file.AddFun<size_t>(
      [&]() {
        return data_node_free_sym_donated->UnsynchronizedGetMonitor().GetCount();
      },
      "free_sym_donate_calls", "Number of donate calls by free living symbionts");
    file.AddTotal(data_node_free_sym_donated->UnsynchronizedGetMonitor(),
      "free_sym_points_donated", "Points donated by free living symbionts", true);
    GetFreeSymStolenDataNode();
    file.AddFun<size_t>(
      [&]() {
        return data_node_free_sym_stolen->UnsynchronizedGetMonitor().GetCount();
      },
      "free_sym_steal_calls", "Number of steal calls by free living symbionts");
    file.AddTotal(data_node_free_sym_stolen->UnsynchronizedGetMonitor(),
      "free_sym_points_stolen", "Points stolen by free living symbionts", true);
  }


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

SyncDataMonitor<double> &SGPWorld::GetHostEarnedDataNode() {
  if (!data_node_host_earned) {
    data_node_host_earned.New();
  }
  return *data_node_host_earned;
}

SyncDataMonitor<double> &SGPWorld::GetHostedSymEarnedDataNode() {
  if (!data_node_hosted_sym_earned) {
    data_node_hosted_sym_earned.New();
  }
  return *data_node_hosted_sym_earned;
}

SyncDataMonitor<double> &SGPWorld::GetHostedSymDonatedDataNode() {
  if (!data_node_hosted_sym_donated) {
    data_node_hosted_sym_donated.New();
  }
  return *data_node_hosted_sym_donated;
}

SyncDataMonitor<double> &SGPWorld::GetHostedSymStolenDataNode() {
  if (!data_node_hosted_sym_stolen) {
    data_node_hosted_sym_stolen.New();
  }
  return *data_node_hosted_sym_stolen;
}

SyncDataMonitor<double>& SGPWorld::GetFreeSymEarnedDataNode() {
  if (!data_node_free_sym_earned) {
    data_node_free_sym_earned.New();
  }
  return *data_node_free_sym_earned;
}

SyncDataMonitor<double>& SGPWorld::GetFreeSymDonatedDataNode() {
  if (!data_node_free_sym_donated) {
    data_node_free_sym_donated.New();
  }
  return *data_node_free_sym_donated;
}

SyncDataMonitor<double>& SGPWorld::GetFreeSymStolenDataNode() {
  if (!data_node_free_sym_stolen) {
    data_node_free_sym_stolen.New();
  }
  return *data_node_free_sym_stolen;
}

#endif