#ifndef SGPWORLD_H
#define SGPWORLD_H

#include "../default_mode/SymWorld.h"
#include "Scheduler.h"
#include "Tasks.h"
#include "emp/Evolve/World_structure.hpp"
#include "emp/data/DataNode.hpp"
#include "SGPConfigSetup.h"
#include "SyncDataMonitor.h"

namespace sgpmode {

class SGPWorld : public SymWorld {
private:
  Scheduler scheduler;
  TaskSet task_set;
  emp::Ptr<SyncDataMonitor<double>> data_node_sym_donated;
  emp::Ptr<SyncDataMonitor<double>> data_node_sym_stolen;
  emp::Ptr<SyncDataMonitor<double>> data_node_sym_earned;
  emp::vector<emp::DataMonitor<size_t>> data_node_host_tasks;
  emp::vector<emp::DataMonitor<size_t>> data_node_sym_tasks;

  /**
  *
  * Purpose: Holds all configuration settings and points to same configuration
  * object as my_config from superclass, but with the correct subtype.
  *
  */
  emp::Ptr<SymConfigSGP> sgp_config = NULL;
  SGPOrganismType sgp_org_type = SGPOrganismType::DEFAULT;
  StressSymbiontType stress_sym_type = StressSymbiontType::MUTUALIST;

public:
  emp::vector<std::pair<emp::Ptr<Organism>, emp::WorldPosition>> to_reproduce;

  SGPWorld(emp::Random &r, emp::Ptr<SymConfigSGP> _config, TaskSet task_set)
      : SymWorld(r, _config), scheduler(*this, _config->THREAD_COUNT()),
    task_set(task_set) {
    sgp_config = _config;
  }

  ~SGPWorld() {
    if(data_node_sym_donated) data_node_sym_donated.Delete();
    if(data_node_sym_stolen) data_node_sym_stolen.Delete();
    if(data_node_sym_earned) data_node_sym_earned.Delete();
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
   * Output: The sgp configuration used for this world.
   *
   * Purpose: Allows accessing the world's sgp config.
   */
  const emp::Ptr<SymConfigSGP> GetConfig() const { return sgp_config; }

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
    if (sgp_config->PHYLOGENY())
      sym_sys->Update();

    // Handle resource inflow
    // NOTE - Why magic number?
    if (total_res != -1) {
      total_res += sgp_config->LIMITED_RES_INFLOW();
    }

    // Run scheduler to process orgs.
    scheduler.ProcessOrgs();

    for (auto org : to_reproduce) {
      if (!org.second.IsValid() || org.first->GetDead())
        continue;
      emp::Ptr<Organism> child = org.first->Reproduce();
      if (child->IsHost()) {
        // Host::Reproduce() doesn't take care of vertical transmission, that
        // happens here
        for (auto &sym : org.first->GetSymbionts()) {
          // don't vertically transmit if they must task match but don't
          if (sgp_config->VT_TASK_MATCH() && !TaskMatchCheck(sym, org.first)) continue;
          sym->VerticalTransmission(child);
        }
        DoBirth(child, org.second);
      } else {
        emp::WorldPosition new_pos = SymDoBirth(child, org.second);
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
      }
    }
    to_reproduce.clear();

    // clean up the graveyard
    for (size_t i = 0; i < graveyard.size(); i++) {
      graveyard[i].Delete();
    }
    graveyard.clear();
  }

  // Prototypes for setup methods
  // TODO - distinguish between world configuration and population initialization
  //  Why? Tests, different mains configure initial population differently
  void Setup() override;
  void SetupHosts(long unsigned int *POP_SIZE) override;
  void SetupSymbionts(long unsigned int *total_syms) override;
  void SetupOrgMode();
  // Internal helper function to configure scheduler.
  // Called internally on world setup.
  void SetupScheduler();

  // Prototypes for reproduction handling methods
  emp::WorldPosition SymDoBirth(emp::Ptr<Organism> sym_baby, emp::WorldPosition parent_pos) override;
  int GetNeighborHost(size_t id, emp::Ptr<Organism> symbiont);
  bool TaskMatchCheck(emp::Ptr<Organism> sym_parent, emp::Ptr<Organism> host_parent);

  // Prototype for graveyard handling method
  void SendToGraveyard(emp::Ptr<Organism> org) override;

  SGPOrganismType GetOrgType() const { return sgp_org_type; }
  StressSymbiontType GetStressSymType() const { return stress_sym_type; }

  // Prototypes for data node methods
  SyncDataMonitor<double>& GetSymDonatedDataNode();
  SyncDataMonitor<double>& GetSymStolenDataNode();
  SyncDataMonitor<double>& GetSymEarnedDataNode();
  void SetupTasksNodes();

  emp::DataFile& SetUpOrgCountFile(const std::string& filename);
  emp::DataFile& SetupSymDonatedFile(const std::string& filename);
  emp::DataFile& SetupTasksFile(const std::string& filename);
  void WriteTaskCombinationsFile(const std::string& filename);
  void WriteOrgReproHistFile(const std::string& filename);

  void CreateDataFiles() override;
};

}

#endif
