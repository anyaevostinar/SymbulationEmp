#ifndef SGPWORLD_H
#define SGPWORLD_H

#include "../default_mode/SymWorld.h"
#include "Tasks.h"
#include "emp/Evolve/World_structure.hpp"
#include "emp/data/DataNode.hpp"
#include "SGPConfigSetup.h"

struct StressEscapeeOffspring{
  emp::Ptr<Organism> escapee_offspring;
  size_t parent_pos;
  emp::BitSet<CPU_BITSET_LENGTH>& grandparent_tasks;

  StressEscapeeOffspring(
    emp::Ptr<Organism> sym,
    size_t loc,
    emp::BitSet<CPU_BITSET_LENGTH>& _tasks
  ) :
    escapee_offspring(sym),
    parent_pos(loc),
    grandparent_tasks(_tasks)
  {
  }
};

class SGPWorld : public SymWorld {
private:
  TaskSet task_set;
  emp::Ptr<emp::DataMonitor<int>> data_node_steal_count;
  emp::Ptr<emp::DataMonitor<int>> data_node_donate_count;


  emp::vector<emp::DataMonitor<size_t>> data_node_host_tasks;
  emp::vector<emp::DataMonitor<size_t>> data_node_sym_tasks;

  /**
  *
  * Purpose: Holds all configuration settings and points to same configuration
  * object as my_config from superclass, but with the correct subtype.
  *
  */
  emp::Ptr<SymConfigSGP> sgp_config = NULL;
public:
  emp::vector<emp::Ptr<Organism>> to_reproduce;
  emp::vector<StressEscapeeOffspring> symbiont_stress_escapee_offspring;

  SGPWorld(emp::Random &r, emp::Ptr<SymConfigSGP> _config, TaskSet task_set)
      : SymWorld(r, _config),
    task_set(task_set) {
    sgp_config = _config;
  }

  ~SGPWorld() {
    // data node deletes 
    if (data_node_steal_count) data_node_steal_count.Delete();
    if (data_node_donate_count) data_node_donate_count.Delete();

    // The vectors will delete themselves automatically
    for (auto escapee_data : symbiont_stress_escapee_offspring) {
      escapee_data.escapee_offspring.Delete();
    }
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
    if (total_res != -1) {
      total_res += sgp_config->LIMITED_RES_INFLOW();
    }

    // emp::vector<size_t> schedule = emp::GetPermutation(GetRandom(), GetSize());
    // for(size_t i : schedule) {
    //   if (IsOccupied(i) == false) continue;

    //   //else
    //   pop[i]->Process(i);
    //   if (pop[i]->GetDead()) DoDeath(i);
    // }

    for (size_t id = 0; id < GetSize(); id++) {
      if (IsOccupied(id)) {
        auto & org = pop[id];
        org->Process(id);
        if (org->GetDead()) DoDeath(id);
      }
    }


    ProcessReproductionQueue();
    
    ProcessStressEscapeeOffspring();
    // clean up the graveyard
    // TODO move to a method in SymWorld so that it can be called here
    for (size_t i = 0; i < graveyard.size(); i++) {
      graveyard[i].Delete();
    }
    graveyard.clear();
  }

  // Prototypes for setup methods
  void SetupHosts(long unsigned int *POP_SIZE) override;
  void SetupSymbionts(long unsigned int *total_syms) override;

  // Update helper methods
  void ProcessReproductionQueue();

  // Prototypes for reproduction handling methods
  emp::WorldPosition SymDoBirth(emp::Ptr<Organism> sym_baby, emp::WorldPosition parent_pos) override;
  int GetNeighborHost(size_t source_id, emp::BitSet<CPU_BITSET_LENGTH>& symbiont_tasks);
  bool TaskMatchCheck(emp::BitSet<CPU_BITSET_LENGTH>& symbiont_tasks, emp::BitSet<CPU_BITSET_LENGTH>& host_tasks);
  bool PreferentialOustingAllowed(emp::BitSet<CPU_BITSET_LENGTH>& incoming_sym_tasks, emp::Ptr<Organism> host);

  // Prototypes for sym transferring
  emp::WorldPosition SymFindHost(emp::Ptr<Organism> symbiont, emp::WorldPosition cur_pos);
  void ProcessStressEscapeeOffspring();

  // Prototype for graveyard handling method
  void SendToGraveyard(emp::Ptr<Organism> org) override;

  // Prototypes for data node methods
  emp::DataMonitor<int> &GetStealCount();
  emp::DataMonitor<int> &GetDonateCount();


  void SetupTasksNodes();

  emp::DataFile &SetUpOrgCountFile(const std::string &filename);
  emp::DataFile &SetupSymInstFile(const std::string &filename);

  emp::DataFile &SetupTasksFile(const std::string &filename);
  void WriteTaskCombinationsFile(const std::string& filename);
  void WriteOrgReproHistFile(const std::string& filename);

  void CreateDataFiles() override;
};

#endif
