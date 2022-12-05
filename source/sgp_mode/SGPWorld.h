#ifndef SGPWORLD_H
#define SGPWORLD_H

#include "../default_mode/SymWorld.h"
#include "Scheduler.h"
#include "Tasks.h"
#include "emp/Evolve/World_structure.hpp"
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
  emp::Ptr<SyncDataMonitor<double>> data_node_sym_stolen;
  emp::Ptr<SyncDataMonitor<double>> data_node_sym_earned;
  emp::vector<emp::DataMonitor<size_t>> data_node_host_tasks;
  emp::vector<emp::DataMonitor<size_t>> data_node_sym_tasks;

public:
  std::map<uint32_t, size_t> data_node_host_squares;
  std::map<uint32_t, size_t> data_node_sym_squares;
  std::mutex squares_mutex;

  emp::vector<std::pair<emp::Ptr<Organism>, emp::WorldPosition>> to_reproduce;

  SGPWorld(emp::Random &r, emp::Ptr<SymConfigBase> _config, TaskSet task_set)
      : SymWorld(r, _config), scheduler(*this, _config->THREAD_COUNT()),
        task_set(task_set) {}

  ~SGPWorld() {
    data_node_sym_donated.Delete();
    data_node_sym_stolen.Delete();
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
   * Output: None
   *
   * Purpose: To simulate a timestep in the world, which includes calling the
   * process functions for hosts and symbionts and updating the data nodes.
   */
  void Update() override {
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
      if (org.IsHost()) {
        org.Process(pos);
        if (org.GetDead()) DoDeath(pos);
      }
      else {
        //have to check for death first, because it might have moved
       // process takes worldposition, dosymdeath takes popid
        if (org.GetDead()) DoSymDeath(pos.GetPopID());
        else org.Process(pos);
        if(IsSymPopOccupied(pos.GetPopID()) && org.GetDead()) DoSymDeath(pos.GetPopID());
        
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
      } else if (emp::Ptr<BaseSymbiont> sym = child.DynamicCast<BaseSymbiont>()) {
        emp::WorldPosition new_pos = SymDoBirth(sym, org.second);
        // Because we're not calling HorizontalTransmission, we need to adjust
        // these data nodes here
        emp::DataMonitor<int> &data_node_attempts_horiztrans =
            GetHorizontalTransmissionAttemptCount();
        data_node_attempts_horiztrans.AddDatum(1);

        emp::DataMonitor<int> &data_node_successes_horiztrans =
            GetHorizontalTransmissionSuccessCount();
        if (new_pos.IsValid()) {
          data_node_successes_horiztrans.AddDatum(1);
        }
      } else throw "not a host or symbiont";
    }
    to_reproduce.clear();
  }

  // Prototypes for setup methods
  void SetupHosts(long unsigned int *POP_SIZE) override;
  void SetupSymbionts(long unsigned int *total_syms) override;

  // Prototypes for data node methods
  SyncDataMonitor<double> &GetSymDonatedDataNode();
  SyncDataMonitor<double> &GetSymStolenDataNode();
  SyncDataMonitor<double> &GetSymEarnedDataNode();
  void SetupTasksNodes();

  emp::DataFile &SetUpOrgCountFile(const std::string &filename);
  emp::DataFile &SetupSymDonatedFile(const std::string &filename);
  emp::DataFile &SetupTasksFile(const std::string &filename);
  emp::DataFile &SetupHostSquareFrequencyFile(const std::string &filename);
  emp::DataFile &SetupSymSquareFrequencyFile(const std::string &filename);

  void CreateDataFiles() override;
};

#endif