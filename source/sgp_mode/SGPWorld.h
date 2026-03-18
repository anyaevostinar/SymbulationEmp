#ifndef SGPWORLD_H
#define SGPWORLD_H

#include "../default_mode/SymWorld.h"
//#include "Scheduler.h"
#include "SGPConfigSetup.h"
//#include "SGPHost.h"
//#include "SGPSymbiont.h"
//#include "org_type_info.h"
//#include "ReproductionQueue.h"
//#include "ProgramBuilder.h"
//#include "SGPMutator.h"
//#include "tasks/LogicTaskEnvironment.h"
//#include "hardware/SGPHardwareSpec.h"
//#include "hardware/GenomeLibrary.h"
//#include "hardware/SGPHardware.h"
#include "utils.h"

#include "emp/Evolve/World_structure.hpp"
#include "emp/data/DataNode.hpp"
#include "emp/math/Random.hpp"

#include <functional>
#include <filesystem>

namespace sgpmode {

//TODO: Configurable?
const size_t PROGRAM_LENGTH = 100;

class SGPWorld : public SymWorld {
public:



  //AEV TODO: it's own file?
  // Collection of current update statistics
  // Calculated only when CurrentUpdateInfo data file is updated.
  struct CurrentUpdateData {
    emp::vector<size_t> host_task_in_profile_counts;
    emp::vector<size_t> host_task_in_parent_org_counts;
    emp::vector<size_t> host_task_in_current_org_counts;
    emp::vector<size_t> host_generations;

    emp::vector<size_t> sym_task_in_profile_counts;
    emp::vector<size_t> sym_task_in_parent_org_counts;
    emp::vector<size_t> sym_task_in_current_org_counts;
    emp::vector<size_t> sym_generations;

    emp::vector<size_t> host_sym_profile_matches_by_task;
    emp::vector<size_t> host_sym_profile_mismatches_by_task;
    size_t host_sym_perfect_matches_total;
    size_t host_sym_any_matches_total;

    size_t num_tasks;

    std::unordered_map<emp::BitVector, size_t> host_parent_tasks_performed;
    std::unordered_map<emp::BitVector, size_t> host_current_tasks_performed;
    std::unordered_map<emp::BitVector, size_t> sym_parent_tasks_performed;
    std::unordered_map<emp::BitVector, size_t> sym_current_tasks_performed;
    // Reset Current update data, adjust task count
    void Reset(size_t task_count) {
      num_tasks = task_count;

      utils::ResizeFill(host_task_in_profile_counts, num_tasks, 0);
      utils::ResizeFill(host_task_in_parent_org_counts, num_tasks, 0);
      utils::ResizeFill(host_task_in_current_org_counts, num_tasks, 0);
      utils::ResizeFill(sym_task_in_profile_counts, num_tasks, 0);
      utils::ResizeFill(sym_task_in_parent_org_counts, num_tasks, 0);
      utils::ResizeFill(sym_task_in_current_org_counts, num_tasks, 0);
      utils::ResizeFill(host_sym_profile_matches_by_task, num_tasks, 0);
      utils::ResizeFill(host_sym_profile_mismatches_by_task, num_tasks, 0);

      host_generations.clear();
      sym_generations.clear();

      host_parent_tasks_performed.clear();
      host_current_tasks_performed.clear();
      sym_parent_tasks_performed.clear();
      sym_current_tasks_performed.clear();

      host_sym_perfect_matches_total = 0;
      host_sym_any_matches_total = 0;
    }

    // Reset current task data, use same number of tasks as before
    void Reset() {
      Reset(num_tasks);
    }

  } current_update_data;

protected:
  size_t max_world_size;
  bool setup = false;

  emp::Ptr<emp::DataMonitor<double>> data_node_sym_donated;
  emp::Ptr<emp::DataMonitor<double>> data_node_sym_stolen;
  emp::Ptr<emp::DataMonitor<double>> data_node_sym_earned;

  // Tracks host/symbiont task success counts.
  // NOTE - Managed by world instead of task set because world
  //        determines whether a task is successful
  emp::vector<emp::DataMonitor<size_t>> data_node_host_tasks;
  emp::vector<emp::DataMonitor<size_t>> data_node_sym_tasks;

  // Tracks host task successes each update. Counts reset to 0 @ begin_update_sig.
  emp::vector<size_t> host_task_successes;
  // Tracks symbiont task successes each update. Counts reset to 0 @ begin_update_sig.
  emp::vector<size_t> sym_task_successes;

  /**
    *
    * Purpose: Holds all configuration settings and references the same configuration
    * object as my_config from superclass, but with the correct subtype.
    *
    */
  SymConfigSGP& sgp_config;

  // Directory to dump output files into.
  std::filesystem::path output_dir;

  // begin_update_sig - Triggers at the beginning of an Update call.
  //  Triggers before schedule update, before processing any organisms.
  //  E.g., used for resetting any per-update data tracking.
  emp::Signal<void(void)> begin_update_sig;

  // Clear all world signals
  void ClearWorldSignals() {
    begin_update_sig.Clear();
  }

public:
  SGPWorld(
    emp::Random& rnd,
    emp::Ptr<SymConfigSGP> _config
  ) :
    SymWorld(rnd, _config),
    //scheduler(rnd),
    //prog_builder(opcode_rectifier),
    //task_env(rnd),
    //mutator(opcode_rectifier),
    sgp_config(*_config)
  { }

   /**
   * Input: None
   *
   * Output: The sgp configuration used for this world.
   *
   * Purpose: Allows accessing the world's sgp config.
   */
  // NOTE - Thoughts on holding a reference vs a pointer. Reference is a little
  //   cleaner in my opinion, but will always need to know at construction
  //   (which is how things were already setup with the pointer).
  const SymConfigSGP& GetConfig() const { return sgp_config; }
  emp::Ptr<SymConfigSGP> GetConfigPtr() { return &sgp_config; }

   /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To simulate a timestep in the world, which includes calling the
   * process functions for hosts and symbionts and updating the data nodes.
   */
  void Update() override {
    emp_assert(setup);
    begin_update_sig.Trigger();
    // Handle resource inflow
    // TODO - implement inflow configuration
    // fun_do_resource_inflow();

    // Update scheduler's evaluation order
    //scheduler.UpdateSchedule();
    // Run scheduler to process organisms
    //scheduler.Run(*this);

    // Process reproduction queue
    //DoReproduction();

    //ProcessStressEscapees();

    // Process graveyard, deletes all dead organisms.
    //ProcessGraveyard();

    // NOTE - these were previously called at the beginning of the update
    //        any specific reason to do that instead of at end?
    //        If we move this to the end of the update, file updates happen after
    //        world update logic.
    //
    // These must be done here because we don't call SymWorld::Update()
    // That may change in the future
    emp::World<Organism>::Update();
    if (sgp_config.PHYLOGENY()) {
      sym_sys->Update();
    }
  }
  

  void Run() {
    emp_assert(setup);
    emp_assert(sgp_config.UPDATES() >= 0);
    const size_t updates = sgp_config.UPDATES();
    for (size_t u = 0; u <= updates; ++u) {
      Update();
      if ((u % sgp_config.PRINT_INTERVAL()) == 0) {
        std::cout << "Update: " << u << std::endl;
      }
    }
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
  // NOTE - Can we get rid of passing these values in as pointers?
//   void SetupHosts(long unsigned int* POP_SIZE) override;
//   void SetupSymbionts(long unsigned int* total_syms) override;


};

}
#endif