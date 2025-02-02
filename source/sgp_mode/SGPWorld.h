#ifndef SGPWORLD_H
#define SGPWORLD_H

#include "../default_mode/SymWorld.h"
#include "Scheduler.h"
#include "Tasks.h"
#include "SGPConfigSetup.h"
#include "SyncDataMonitor.h"

#include "emp/Evolve/World_structure.hpp"
#include "emp/data/DataNode.hpp"


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
    emp::Ptr<Organism>, /* sym_baby_ptr */
    emp::WorldPosition /* parent_pos */
  )> before_sym_do_birth;
  emp::Signal<void(emp::WorldPosition /* sym_baby_pos */)> after_sym_do_birth;
  fun_sym_do_birth_t fun_sym_do_birth;

  // Tiggers on host birth action
  // TODO - add functions that add functions to these signals
  // TODO - switch to passing references instead of pointers
  emp::Signal<void(
    emp::Ptr<Organism>, /* host_offspring_ptr */
    emp::Ptr<Organism>, /* host_parent_ptr */
    const emp::WorldPosition&  /* parent_pos */
  )> before_host_do_birth;
  emp::Signal<void(
    const emp::WorldPosition& /* host_offspring_pos */
  )> after_host_do_birth;

  // Triggers on symbiont vertical transmission
  // TODO - shift from ptr of org to references of SGP base classes
  emp::Signal<void(
    emp::Ptr<Organism>, /**/
    emp::Ptr<Organism>,
    const emp::WorldPosition&
  )> before_sym_vert_transmission;
  emp::Signal<void(
    emp::Ptr<Organism>, /**/
    emp::Ptr<Organism>,
    const emp::WorldPosition&
  )> after_sym_vert_transmission;

  // Internal helper function to handle reproduction events each update.
  void DoReproduction();
  // Internal helper function to handle host births.
  //   Handles both host do birth and triggering vertical transmission on any
  //   symbionts within the host.
  //   Need to pass in parent pointer because parent may no longer exist at the
  //   given world position when this function is called.
  emp::WorldPosition HostDoBirth(
    emp::Ptr<Organism> host_offspring_ptr,
    emp::Ptr<Organism> host_parent_ptr,
    emp::WorldPosition parent_pos
  );

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
  TaskSet& GetTaskSet() { return task_set; }

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
  /**
   * Input: None.
   *
   * Output: None.
   *
   * Purpose: Prepare the SGPWorld for an experiment by applying the configuration settings
   * and populating the world with hosts and symbionts.
   */
  void Setup() override;
  void SetupHosts(long unsigned int* POP_SIZE) override;
  void SetupSymbionts(long unsigned int* total_syms) override;
  void SetupOrgMode();
  // Internal helper function to configure scheduler.
  // Called internally on world setup.
  void SetupScheduler(); // TODO - shift to private function (will need to refactor many tests)
  void SetupSymTransmission(); // TODO - shift to private function (will need to refactor many tests)

  // Prototypes for reproduction handling methods
  // SymDoBirth is for horizontal transmission and birthing free-living symbionts.
  emp::WorldPosition SymDoBirth(
    emp::Ptr<Organism> sym_baby,
    emp::WorldPosition parent_pos
  ) override;
  int GetNeighborHost(size_t id, emp::Ptr<Organism> symbiont);

  /**
    * Input: Pointers to a host and to a symbiont
    *
    * Output: Whether host and symbiont parent are able to accomplish
    * at least one task in common
    *
    * Purpose: To check for task matching before vertical transmission
    */
  bool TaskMatchCheck(emp::Ptr<Organism> sym_parent, emp::Ptr<Organism> host_parent);

  /**
   * Input: An organism pointer to add to the graveyard
   *
   * Output: None
   *
   * Purpose: To add organisms to the graveyard
   */
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
