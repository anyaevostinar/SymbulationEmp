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
  SetupTasksFile(sgp_config->FILE_PATH() + "Tasks" + sgp_config->FILE_NAME() +
                  file_ending)
      .SetTimingRepeat(sgp_config->DATA_INT());
  SetupSymInstFile(sgp_config->FILE_PATH() + "SymInstCount" +
                        sgp_config->FILE_NAME() + file_ending)
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


emp::DataFile &SGPWorld::SetupSymInstFile(const std::string &filename) {
  auto &file = SetupFile(filename);
  file.AddVar(update, "update", "Update");
  GetStealCount();
  file.AddTotal(GetStealCount(),
                "sym_steal_ran", "Amount of times steal instructions were ran", true);
  
  GetDonateCount();
  file.AddTotal(GetDonateCount(),
                "sym_donate_ran", "Amount of times donate instructions were ran", true);
  
  file.PrintHeaderKeys();
  return file;
}

/**
 * Input: The address of the file object
 *
 * Output: None
 *
 * Purpose: To add the stress repro escapee attempts and successes 
 * as columns in the transmission file.
 */
void SGPWorld::SetupTransmissionFileColumns(emp::DataFile& file) {
  SymWorld::SetupTransmissionFileColumns(file);
  if (sgp_config->INTERACTION_MECHANISM() == STRESS && sgp_config->SYMBIONT_TYPE() == PARASITE && sgp_config->PARASITE_NUM_OFFSPRING_ON_STRESS_INTERACTION() > 0) {
    emp::DataMonitor<size_t>& escapee_attempts = GetStressEscapeeOffspringAttemptCount();
    emp::DataMonitor<size_t>& escapee_successes = GetStressEscapeeOffspringSuccessCount();

    file.AddTotal(escapee_attempts, "stress_escapee_offspring_attempts", "Total number of stress offspring who attempted to escape", true);
    file.AddTotal(escapee_successes, "stress_escapee_offspring_successes", "Total number of stress offspring who succeeded in escaping (they were validly placed)", true);
  }
}

/**
 * Input: The address of the string representing the file to be
 * created's name
 *
 * Output: None
 *
 * Purpose: To write the count of organisms who achieved or whose parents 
 * achieved each combination of tasks.
 */
void SGPWorld::WriteTaskCombinationsFile(const std::string& filename) {
  std::ofstream out_file(filename);

  // loop through the whole population and keep track of the count of parents
  // who satisfy each tag combination
  std::unordered_map<std::string, int[4]> matching_task_counts; 
  for (size_t i = 0; i < pop.size(); i++) {
    if (!IsOccupied(i)) continue;
    emp::Ptr<SGPHost> host = pop[i].DynamicCast<SGPHost>();

    // Host parent 
    std::string host_parent_matching_tasks = host->GetCPU().state.parent_tasks_performed->ToBinaryString();
    if (emp::Has(matching_task_counts, host_parent_matching_tasks)) {
      matching_task_counts[host_parent_matching_tasks][0]++;
    }
    else {
      matching_task_counts[host_parent_matching_tasks][0] = 1;// { 1, 0, 0, 0 };

    }

    // Host self
    std::string host_matching_tasks = host->GetCPU().state.tasks_performed->ToBinaryString();
    if (emp::Has(matching_task_counts, host_matching_tasks)) {
      matching_task_counts[host_matching_tasks][2]++;
    }
    else {
      matching_task_counts[host_matching_tasks][2] = 1;// { 0, 0, 1, 0 };
    }

    emp::vector<emp::Ptr<Organism>> syms = host->GetSymbionts();
    for (size_t j = 0; j < syms.size(); j++) {
      emp::Ptr<SGPSymbiont> sym = syms[j].DynamicCast<SGPSymbiont>();

      // Symbiont parent
      std::string sym_parent_matching_tasks = sym->GetCPU().state.parent_tasks_performed->ToBinaryString();
      if (emp::Has(matching_task_counts, sym_parent_matching_tasks)) {
        matching_task_counts[sym_parent_matching_tasks][1]++;
      }
      else {
        matching_task_counts[sym_parent_matching_tasks][1] = 1;// = { 0, 1, 0, 0 };
      }

      // Symbiont self
      std::string sym_matching_tasks = sym->GetCPU().state.tasks_performed->ToBinaryString();
      if (emp::Has(matching_task_counts, sym_matching_tasks)) {
        matching_task_counts[sym_matching_tasks][3]++;
      }
      else {
        matching_task_counts[sym_matching_tasks][3] = 1;// = { 0, 0, 0, 1 };
      }
    }
  }

  // write all the combinations and counts to the data file
  out_file << "task_completions,host_parent_count,sym_parent_count,host_count,symbiont_count\n";
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
    out_file << ",lineage_gained_count_" << task.task.name << ",lineage_lost_count_" << task.task.name << ",toward_partner_count_" << task.task.name << ",from_partner_count_" << task.task.name;
  }
  out_file << "\n";

  emp::Ptr<SGPHost> host;
  emp::Ptr<SGPSymbiont> symbiont;

  for (size_t i = 0; i < size(); i++) {
    if (IsOccupied(i)) {
      host = pop[i].DynamicCast<SGPHost>();
      out_file << "host," << host->GetReproCount();
      for (int k = 0; k < CPU_BITSET_LENGTH; k++) {
        out_file << "," << host->GetCPU().state.task_change_gain[k] << "," << host->GetCPU().state.task_change_lose[k] << "," << host->GetCPU().state.task_toward_partner[k] << "," << host->GetCPU().state.task_from_partner[k];
      }
      out_file << "\n";

      if (host->HasSym()) {
        emp::vector<emp::Ptr<Organism>> syms = host->GetSymbionts();
        for (size_t j = 0; j < syms.size(); j++) {
          symbiont = syms[j].DynamicCast<SGPSymbiont>();

          out_file << "sym," << symbiont->GetReproCount();
          for (int k = 0; k < CPU_BITSET_LENGTH; k++) {
            out_file << "," << symbiont->GetCPU().state.task_change_gain[k] << "," << symbiont->GetCPU().state.task_change_lose[k] << "," << symbiont->GetCPU().state.task_toward_partner[k] << "," << symbiont->GetCPU().state.task_from_partner[k];
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


emp::DataMonitor<int> &SGPWorld::GetStealCount() {
  if (!data_node_steal_count) {
    data_node_steal_count.New();
  }
  return *data_node_steal_count;
}

emp::DataMonitor<int> &SGPWorld::GetDonateCount() {
  if (!data_node_donate_count) {
    data_node_donate_count.New();
  }
  return *data_node_donate_count;
}

/**
 * Input: None
 *
 * Output: A reference to the data node storing the count of
 * stress escapee offspring (successful or not)
 *
 * Purpose: To set up and return the data node storing the count
 * of attempted stress escapee offspring
 */
emp::DataMonitor<size_t>& SGPWorld::GetStressEscapeeOffspringAttemptCount() {
  if (!data_node_stress_escapee_offspring_attempt_count) {
    data_node_stress_escapee_offspring_attempt_count.New();
  }
  return *data_node_stress_escapee_offspring_attempt_count;
}

/**
 * Input: None
 *
 * Output: A reference to the data node storing the count of successful
 * stress escapee offspring
 *
 * Purpose: To set up and return the data node storing the count
 * of successful stress escapee offspring
 */
emp::DataMonitor<size_t>& SGPWorld::GetStressEscapeeOffspringSuccessCount() {
  if (!data_node_stress_escapee_offspring_success_count) {
    data_node_stress_escapee_offspring_success_count.New();
  }
  return *data_node_stress_escapee_offspring_success_count;
}
#endif