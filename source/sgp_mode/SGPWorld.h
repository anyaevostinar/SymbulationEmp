#ifndef SGPWORLD_H
#define SGPWORLD_H

#include "../default_mode/SymWorld.h"
#include "Scheduler.h"
#include "SGPConfigSetup.h"
#include "SyncDataMonitor.h"
#include "spec.h"
#include "SGPHost.h"
#include "SGPSymbiont.h"
#include "org_type_info.h"
#include "ReproductionQueue.h"
#include "ProgramBuilder.h"
#include "SGPMutator.h"
#include "tasks/LogicTaskEnvironment.h"
#include "hardware/SGPHardwareSpec.h"
#include "hardware/GenomeLibrary.h"
#include "hardware/SGPHardware.h"

#include "emp/Evolve/World_structure.hpp"
#include "emp/data/DataNode.hpp"
#include "emp/math/Random.hpp"

#include <functional>

namespace sgpmode {

// TODO - do we want this to be configurable?
const size_t PROGRAM_LENGTH = 100;

// TODO - init necessary hardware state on organism birth (e.g., stack limit)
class SGPWorld : public SymWorld {
public:
  using sgp_cpu_peripheral_t = CPUState<SGPWorld>;
  using hw_spec_t = SGPHardwareSpec<Library, sgp_cpu_peripheral_t, SGPWorld>;
  using sgp_host_t = SGPHost<hw_spec_t>;
  using sgp_sym_t = SGPSymbiont<hw_spec_t>;
  using tag_t = typename hw_spec_t::tag_t;
  using sgp_hw_t = SGPHardware<hw_spec_t>;
  using sgp_prog_t = typename sgp_hw_t::program_t;
  using task_env_t = tasks::LogicTaskEnvironment;
  using task_reqs_t = typename task_env_t::TaskReqInfo;
  using mutator_t = SGPMutator<sgp_prog_t>;

  using fun_sym_do_birth_t = std::function<emp::WorldPosition(
    emp::Ptr<sgp_sym_t>, /* symbiont baby ptr */
    const emp::WorldPosition&     /* parent_position */
  )>;

  // NOTE - better name?
  // TODO - switch to using references to SGPHost, etc.
  using fun_can_attempt_vert_trans_t = std::function<bool(
    sgp_sym_t&, /* symbiont_ptr */
    sgp_host_t&, /* host_offspring_ptr (trans to) */
    sgp_host_t&, /* host_parent_ptr (trans from) */
    const emp::WorldPosition&  /* parent_pos */
  )>;

  using fun_compatibility_check_t = std::function<bool(
    const sgp_host_t&,
    const sgp_sym_t&
    // emp::Ptr<Organism>, /* host */
    // emp::Ptr<Organism> /* symbiont */
  )>;

  using fun_do_resource_inflow_t = std::function<void(void)>;

  // using fun_process_endosym_t = std::function<void(
  //   sgp_sym_t&,                /* endosymbiont */
  //   const emp::WorldPosition&, /* sym pos */
  //   sgp_host_t&                /* host */
  // )>;

  using org_mode_t = typename org_info::SGPOrganismType;
  using stress_sym_mode_t = typename org_info::StressSymbiontType;

  tag_t START_TAG;

protected:
  // TODO - scheduler could be SGP scheduler? It will only work with SGPWorld anyway?
  Scheduler scheduler;
  size_t max_world_size; // Maximum number of locations in the world
  ReproductionQueue repro_queue;
  ProgramBuilder<hw_spec_t> prog_builder;
  tasks::LogicTaskEnvironment task_env;
  mutator_t mutator;


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
  //  emp::Ptr<SymConfigSGP> sgp_config = nullptr;
  SymConfigSGP& sgp_config;

  // What kind of SGP organism type to use?
  org_mode_t sgp_org_type = org_mode_t::DEFAULT;
  // If using stress organisms, what kind of stress?
  stress_sym_mode_t stress_sym_type = stress_sym_mode_t::MUTUALIST;

  // Function to check compatibility between host and symbiont
  // - Used to check eligibility for vertical / horizontal transmission, etc.
  fun_compatibility_check_t fun_host_sym_compatibility_check;

  // --- Symbiont birth signals / functors ---
  emp::Signal<void(
    emp::Ptr<sgp_sym_t>, /* sym_baby_ptr */
    const emp::WorldPosition&     /* parent_pos */
  )> before_sym_do_birth_sig;
  emp::Signal<void(const emp::WorldPosition& /* sym_baby_pos */)> after_sym_do_birth_sig;
  fun_sym_do_birth_t fun_sym_do_birth;

  // --- Symbiont vertical transmission signals / functors ---
  // TODO - Need to extract whether vertical transmission is successful or not.
  emp::Signal<void(
    emp::Ptr<sgp_sym_t>,       /* sym_ptr - symbiont producing offspring */
    emp::Ptr<sgp_host_t>,           /* host_offspring_ptr - transmission to */
    emp::Ptr<sgp_host_t>,           /* host_parent_ptr - transmission from */
    const emp::WorldPosition&    /* host_parent_pos */
  )> before_sym_vert_transmission_sig;
  emp::Signal<void(
    emp::Ptr<sgp_sym_t>,         /* sym_offspring_ptr */
    emp::Ptr<sgp_sym_t>,         /* sym_parent_ptr */
    emp::Ptr<sgp_host_t>,        /* host_offspring_ptr */
    emp::Ptr<sgp_host_t>,        /* host_parent_ptr */
    const emp::WorldPosition&,  /* host_parent_pos */
    bool                        /* vertical transmission success */
  )> after_sym_vert_transmission_sig;
  // Checks if symbiont vertical transmission is successful
  fun_can_attempt_vert_trans_t fun_can_attempt_vert_trans;

  // --- Host birth signals / functors ---
  // TODO - add functions that add functions to these signals
  emp::Signal<void(
    sgp_host_t&,        /* host_offspring_ptr */
    sgp_host_t&,        /* host_parent_ptr */
    const emp::WorldPosition&  /* parent_pos */
  )> before_host_do_birth_sig;
  emp::Signal<void(
    const emp::WorldPosition& /* host_offspring_pos */
  )> after_host_do_birth_sig;

  // --- Host process signals / functors ---
  // TODO - document timing
  emp::Signal<void(
    sgp_host_t&
  )> before_host_process_sig;
  emp::Signal<void(
    sgp_host_t&
  )> after_host_process_sig;
  emp::Signal<void(
    sgp_host_t&
  )> after_host_cpu_step_sig;
  emp::Signal<void(
    sgp_host_t&
  )> after_host_cpu_exec_sig;
  // fun_process_endosym_t fun_process_endosym; // NOTE - not used at the moment

  // --- Free-living symbiont signals / functors ---
  emp::Signal<void(
    sgp_sym_t&                 /* sym */
  )> before_freeliving_sym_process_sig;
  emp::Signal<void(
    sgp_sym_t&                 /* sym */
  )> after_freeliving_sym_process_sig;
  emp::Signal<void(
    sgp_sym_t&                 /* sym */
  )> after_freeliving_sym_cpu_step_sig;
  emp::Signal<void(
    sgp_sym_t&                 /* sym */
  )> after_freeliving_sym_cpu_exec_sig;

  // --- Endosymbiont process signals / functors ---
  emp::Signal<void(
    const emp::WorldPosition&, /* sym_pos */
    sgp_sym_t&,                /* sym */
    sgp_host_t&                /* host */
  )> before_endosym_process_sig;
  emp::Signal<void(
    const emp::WorldPosition&, /* sym_pos */
    sgp_sym_t&,                /* sym */
    sgp_host_t&                /* host */
  )> after_endosym_process_sig;
  emp::Signal<void(
    const emp::WorldPosition&, /* sym_pos */
    sgp_sym_t&,                /* sym */
    sgp_host_t&                /* host */
  )> after_endosym_cpu_step_sig;
  emp::Signal<void(
    const emp::WorldPosition&, /* sym_pos */
    sgp_sym_t&,                /* sym */
    sgp_host_t&                /* host */
  )> after_endosym_cpu_exec_sig;


  // --- Environment signals/functors ---
  // fun_do_resource_inflow_t fun_do_resource_inflow;

  std::function<std::optional<emp::WorldPosition>(
    size_t,                 /* Parent's host location id in world (pops[0][id])*/
    emp::Ptr<sgp_sym_t>     /* Pointer to symbiont parent (producing the sym offspring) */
  )> fun_find_host_for_horizontal_trans;

  // ---- Internal helper functions ----
  void DoReproduction();

  void HostAttemptRepro(const emp::WorldPosition& pos, sgp_host_t& host);

  void EndosymAttemptRepro(
    const emp::WorldPosition& pos,
    sgp_sym_t& sym,
    sgp_host_t& host
  );

  void FreeLivingSymAttemptRepro(
    const emp::WorldPosition& pos,
    sgp_sym_t& sym
  );

  // Internal helper function to handle host births.
  //   Handles both host do birth and triggering vertical transmission on any
  //   symbionts within the host.
  //   Need to pass in parent pointer because parent may no longer exist at the
  //   given world position when this function is called.
  emp::WorldPosition HostDoBirth(
    emp::Ptr<Organism> host_offspring_ptr,
    emp::Ptr<Organism> host_parent_ptr,
    const emp::WorldPosition& parent_pos
  );

  emp::WorldPosition FreeLivingSymDoBirth(
    emp::Ptr<sgp_sym_t> sym_baby_ptr,
    const emp::WorldPosition& parent_pos
  );

  emp::WorldPosition SymAttemptHorizontalTrans(
    emp::Ptr<sgp_sym_t> sym_baby_ptr,
    const emp::WorldPosition& parent_pos
  );

  // Attempt vertical transmission from host parent to host offspring.
  // Pass in pointers to play nice with Symbiont.h's VerticalTransmission function.
  bool EndosymAttemptVertTransmission(
    emp::Ptr<sgp_sym_t> endosym_ptr,                  /* Endosymbiont attempting transmission */
    emp::Ptr<sgp_host_t> host_offspring_ptr,          /* Host offspring (transmit to) */
    emp::Ptr<sgp_host_t> host_parent_ptr,             /* Host parent (transmit from) */
    const emp::WorldPosition& parent_pos /* Parent location */
  );

  // Internal helper function to delete dead organisms in graveyard.
  void ProcessGraveyard();

  // --- Internal setup helper functions ---.
  // Called internally on world setup.
  void SetupOrgMode();
  void SetupPopStructure();
  void SetupScheduler();           // TODO - shift to private function (will need to refactor tests)
  void SetupReproduction();
  void SetupSymReproduction();     // TODO - shift to private function (will need to refactor tests)
  void SetupHostReproduction();    // TODO - shift to private function (will need to refactor tests)
  void SetupHostSymInteractions(); // TODO - shift to private function (will need to refactor tests)
  void SetupTaskEnvironment();
  void SetupMutator();

  // Clear all world signals
  void ClearWorldSignals() {
    before_sym_do_birth_sig.Clear();
    after_sym_do_birth_sig.Clear();
    before_sym_vert_transmission_sig.Clear();
    after_sym_vert_transmission_sig.Clear();
    before_host_do_birth_sig.Clear();
    after_host_do_birth_sig.Clear();
    before_host_process_sig.Clear();
    after_host_process_sig.Clear();
    after_host_cpu_step_sig.Clear();
    before_freeliving_sym_process_sig.Clear();
    after_freeliving_sym_process_sig.Clear();
    after_freeliving_sym_cpu_step_sig.Clear();
    before_endosym_process_sig.Clear();
    after_endosym_process_sig.Clear();
    after_endosym_cpu_step_sig.Clear();
  }

  void AssignNewEnvIO(sgp_cpu_peripheral_t& cpu_state) {
    const size_t env_id = GetRandom().GetUInt(task_env.GetIOBank().GetSize());
    const auto& task_io = task_env.GetIOBank().GetIO(env_id);
    cpu_state.SetTaskEnvID(env_id);
    cpu_state.SetInputs(task_io.input_buffer);
  }

  // NOTE - could make this a functor to allow runtime configuration or differences
  //        between different kinds of organisms
  // NOTE - Other conditions that we want to check?
  bool CanPerformTask(
    sgp_cpu_peripheral_t& cpu_state,
    const task_reqs_t& task_reqs
  ) {
    const size_t task_id = task_reqs.task_id;
    const size_t max_repeats = task_reqs.max_repeats;
    return cpu_state.GetTaskPerformanceCount(task_id) < max_repeats;
  }

  void ProcessHostOutputBuffer(sgp_host_t& host);
  void ProcessSymOutputBuffer(sgp_sym_t& sym);

public:
  SGPWorld(
    emp::Random& rnd,
    emp::Ptr<SymConfigSGP> _config
  ) :
    SymWorld(rnd, _config),
    scheduler(rnd),
    task_env(rnd),
    sgp_config(*_config)
  {
    // TODO - taskset

  }

  ~SGPWorld() {
    if(data_node_sym_donated) data_node_sym_donated.Delete();
    if(data_node_sym_stolen) data_node_sym_stolen.Delete();
    if(data_node_sym_earned) data_node_sym_earned.Delete();
  }

  /**
   * Input: None
   *
   * Output: The sgp configuration used for this world.
   *
   * Purpose: Allows accessing the world's sgp config.
   */
  // TODO - switch back to having a pointer to the config to be consistent with
  // base class? (lots of base classes assume we're hanging on to a pointer)
  const SymConfigSGP& GetConfig() const { return sgp_config; }
  emp::Ptr<SymConfigSGP> GetConfigPtr() { return &sgp_config; }


  size_t GetTaskCount() const { return task_env.GetTaskCount(); }

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
    if (sgp_config.PHYLOGENY()) {
      sym_sys->Update();
    }

    // Handle resource inflow
    // TODO - implement inflow configuration
    // fun_do_resource_inflow();

    // Update scheduler's evaluation order
    scheduler.UpdateSchedule();
    // Run scheduler to process organisms
    scheduler.Run(*this);

    // Process reproduction queue
    DoReproduction();

    // Process graveyard, deletes all dead organisms.
    ProcessGraveyard();
  }

  void Run() {
    emp_assert(sgp_config.UPDATES() >= 0);
    const size_t updates = sgp_config.UPDATES();
    for (size_t u = 0; u <= updates; ++u) {
      Update();
      if ((u % sgp_config.PRINT_INTERVAL()) == 0) {
        std::cout << "Update: " << u << std::endl;
      }
    }
  }

  // Process hosts at given position in world pop vector and free-living symbionts in world syms vector.
  void ProcessOrgsAt(size_t pop_id);

  // Process host at given position in world
  // NOTE - what functionality should be centralized vs in the host class vs functor/signal?
  void ProcessHostAt(const emp::WorldPosition& pos, sgp_host_t& host);

  // Process symbiont at given position in the world
  void ProcessFreeLivingSymAt(const emp::WorldPosition& pos, sgp_sym_t& sym);

  // Process all symbionts inside given host
  void ProcessEndosymbionts(sgp_host_t& host);

  // Process endosymbiont
  void ProcessEndosymbiont(const emp::WorldPosition& sym_pos, sgp_sym_t& sym, sgp_host_t& host);

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
  void SetupHosts(long unsigned int* POP_SIZE) override;
  void SetupSymbionts(long unsigned int* total_syms) override;

  // Prototypes for reproduction handling methods
  // SymDoBirth is for horizontal transmission and birthing free-living symbionts.
  // TODO - How to distinguish between the two?
  emp::WorldPosition SymDoBirth(
    emp::Ptr<Organism> sym_baby,
    emp::WorldPosition parent_pos
  ) override;

  void HostDoMutation(sgp_host_t& host);
  void SymDoMutation(sgp_sym_t& sym);

  // Returns neighboring host from given symbiont
  // NOTE - Opinions on name change? (originally GetNeighborHost)
  std::optional<emp::WorldPosition> FindHostForHorizontalTrans(
    size_t host_world_id,                 /* Parent's host location id in world (pops[0][id])*/
    emp::Ptr<sgp_sym_t> sym_parent_ptr    /* Pointer to symbiont parent (producing the sym offspring) */
  );

  /**
   * Input: An organism pointer to add to the graveyard
   *
   * Output: None
   *
   * Purpose: To add organisms to the graveyard
   */
  void SendToGraveyard(emp::Ptr<Organism> org) override;

  org_mode_t GetOrgType() const { return sgp_org_type; }
  stress_sym_mode_t GetStressSymType() const { return stress_sym_type; }

  ReproductionQueue& GetReproQueue() { return repro_queue; }

  // Data node methods
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