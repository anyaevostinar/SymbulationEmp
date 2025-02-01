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
public:
  using fun_sym_do_birth_t = std::function<emp::WorldPosition(emp::Ptr<Organism>, emp::WorldPosition)>;
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

  // Triggers on symbiont do birth action.
  //  sym baby ptr, parent pos
  emp::Signal<void(
    emp::Ptr<Organism> /* sym_baby_ptr */,
    emp::WorldPosition /* parent_pos */
  )> before_sym_do_birth;

  emp::Signal<void(emp::WorldPosition /* sym_baby_pos */)> after_sym_do_birth;
  fun_sym_do_birth_t fun_sym_do_birth;


  // Internal helper function to handle reproduction events each update.
  void DoReproduction();
  // Internal helper function to delete dead organisms in graveyard.
  void ProcessGraveyard();
public:
  // TODO - Shift to protected/private. Add any necessary accessors.
  emp::vector<std::pair<emp::Ptr<Organism>, emp::WorldPosition>> to_reproduce;

  SGPWorld(emp::Random& r, emp::Ptr<SymConfigSGP> _config, TaskSet task_set)
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
    if (sgp_config->PHYLOGENY()) {
      sym_sys->Update();
    }

    // Handle resource inflow
    // NOTE - Why magic number?
    if (total_res != -1) {
      total_res += sgp_config->LIMITED_RES_INFLOW();
    }

    // Run scheduler to process orgs.
    scheduler.ProcessOrgs();

    // Processes reproduction queue (to_reproduce)
    DoReproduction();

    // Processes graveyard, deletes all dead organisms.
    ProcessGraveyard();

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
  void SetupScheduler(); // TODO - shift to private function (will need to refactor many tests)
  void SetupSymTransmission(); // TODO - shift to private function (will need to refactor many tests)

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

emp::WorldPosition SGPWorld::SymDoBirth(
  emp::Ptr<Organism> sym_baby,
  emp::WorldPosition parent_pos
) {
  // Trigger any before birth actions.
  before_sym_do_birth.Trigger(sym_baby, parent_pos);

  emp::WorldPosition sym_baby_pos = fun_sym_do_birth(sym_baby, parent_pos);

  // Trigger any after birth actions
  // NOTE - Currently triggers regardless of success. Should this only trigger on successful births?
  //   Or, have separate signals for successful / unsuccessful births?
  after_sym_do_birth.Trigger(sym_baby_pos);

  return sym_baby_pos;
}

}

#endif
