#ifndef SGPWORLD_H
#define SGPWORLD_H

#include "../default_mode/SymWorld.h"
#include "Scheduler.h"
#include "Tasks.h"
#include "emp/data/DataNode.hpp"

/// Helper which synchronizes access to the DataMonitor with a mutex
template <typename T, emp::data... MODS> class SyncDataMonitor {
  std::mutex mutex;
  emp::DataMonitor<T, MODS...> monitor;

public:
  /**
   * Input: None
   *
   * Output: Reference to the underlying DataMonitor, without synchronization.
   *
   * Purpose: Accesses the underlying monitor without synchronization, should
   * only be used when you're sure there's no multithreading going on.
   */
  emp::DataMonitor<T, MODS...> &UnsynchronizedGetMonitor() { return monitor; }

  /**
   * Input: An action to perform with the DataMonitor.
   *
   * Output: None
   *
   * Purpose: Calls the provided callback with the DataMonitor while holding the
   * mutex, releasing it when it returns.
   */
  template <typename F> void WithMonitor(F f) {
    std::lock_guard lock(mutex);
    f(monitor);
  }
};

class SGPWorld : public SymWorld {
private:
  Scheduler scheduler;
  TaskSet task_set;
  emp::Ptr<SyncDataMonitor<double>> data_node_sym_donated;
  emp::Ptr<SyncDataMonitor<double>> data_node_sym_earned;
  emp::vector<emp::DataMonitor<size_t>> data_node_host_tasks;
  emp::vector<emp::DataMonitor<size_t>> data_node_host_largest_square;
  emp::vector<emp::DataMonitor<size_t>> data_node_sym_largest_square;
  emp::vector<emp::DataMonitor<size_t>> data_node_sym_tasks;

public:
  emp::vector<std::pair<emp::Ptr<Organism>, emp::WorldPosition>> to_reproduce;

  SGPWorld(emp::Random &r, emp::Ptr<SymConfigBase> _config, TaskSet task_set)
      : SymWorld(r, _config), scheduler(*this, _config->THREAD_COUNT()),
        task_set(task_set) {}

  ~SGPWorld() {
    data_node_sym_donated.Delete();
    data_node_sym_earned.Delete();
    // The vectors will delete themselves automatically
  }

  /**
   * Input: None
   *
   * Output: The task set used for this world.
   *
   * Purpose: Allows accessing the world's task set.
   */
  TaskSet &GetTaskSet() { return task_set; }

  /**
   * Input: None
   *
   * Output: The configuration used for this world.
   *
   * Purpose: Allows accessing the world's config.
   */
  emp::Ptr<SymConfigBase> GetConfig() { return my_config; }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To simulate a timestep in the world, which includes calling the
   * process functions for hosts and symbionts and updating the data nodes.
   */
  void Update() {
    // These must be done here because we don't call SymWorld::Update()
    // That may change in the future
    emp::World<Organism>::Update();
    if (my_config->PHYLOGENY())
      sym_sys->Update();
    // Handle resource inflow
    if (total_res != -1) {
      total_res += my_config->LIMITED_RES_INFLOW();
    }

    scheduler.ProcessOrgs([&](emp::WorldPosition pos, Organism &org) {
      org.Process(pos);
      if (org.GetDead()) { // Check if the host died
        DoDeath(pos);
      }
    });

    for (auto org : to_reproduce) {
      if (!org.second.IsValid())
        continue;
      emp::Ptr<Organism> child = org.first->Reproduce();
      if (child->IsHost()) {
        // Host::Reproduce() doesn't take care of vertical transmission, that
        // happens here
        for (auto &sym : org.first->GetSymbionts()) {
          sym->VerticalTransmission(child);
        }
        DoBirth(child, org.second);
      } else {
        SymDoBirth(child, org.second);
      }
    }
    to_reproduce.clear();
  }

  // Prototypes for data node methods
  SyncDataMonitor<double> &GetSymDonatedDataNode();
  SyncDataMonitor<double> &GetSymEarnedDataNode();
  void SetupTasksNodes();
  void SetupOutputNodes();

  emp::DataFile &SetupSymDonatedFile(const std::string &filename);
  emp::DataFile &SetupTasksFile(const std::string &filename);
  emp::DataFile &SetupOutputFrequencyFile(const std::string &filename);

  void CreateDataFiles();
};

#endif