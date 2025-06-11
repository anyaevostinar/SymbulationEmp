#ifndef SGPWORLD_H
#define SGPWORLD_H

#include "../default_mode/SymWorld.h"
#include "Scheduler.h"
#include "SGPConfigSetup.h"
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
#include <filesystem>

// TODO - Document how each base configuration works for SGPWorld
// TODO - Comments for every member variable / function / type alias

namespace sgpmode {

// TODO - do we want this to be configurable?
const size_t PROGRAM_LENGTH = 100;

// NOTE - SymWorld::GetPop returns pop by copy instead of by reference?
//        GetPop is expensive operation, avoid use

// TODO - add tests for stack limits on organisms
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
  using task_io_bank_t = typename task_env_t::io_bank_t;
  using task_io_t = typename task_io_bank_t::TaskIO;
  using mutator_t = SGPMutator<sgp_prog_t, Library>;
  using sgp_prog_rectifier_t = sgpl::OpCodeRectifier<Library>;

  using fun_sym_do_birth_t = std::function<emp::WorldPosition(
    emp::Ptr<sgp_sym_t>,          /* symbiont baby ptr */
    const emp::WorldPosition&     /* parent_position */
  )>;

  // NOTE - better name?
  using fun_can_attempt_vert_trans_t = std::function<bool(
    sgp_sym_t&,                /* symbiont_ptr */
    sgp_host_t&,               /* host_offspring_ptr (trans to) */
    sgp_host_t&,               /* host_parent_ptr (trans from) */
    const emp::WorldPosition&  /* parent_pos */
  )>;

  using fun_compatibility_check_t = std::function<bool(
    const sgp_host_t&,
    const sgp_sym_t&
  )>;

  using fun_do_resource_inflow_t = std::function<void(void)>;

  using fun_apply_nutrient_interaction_t = std::function<double(
    sgp_sym_t&, /* symbiont */
    double,     /* task value before nutrient interaction */
    size_t     /* task id */
  )>;

  // using fun_process_endosym_t = std::function<void(
  //   sgp_sym_t&,                /* endosymbiont */
  //   const emp::WorldPosition&, /* sym pos */
  //   sgp_host_t&                /* host */
  // )>;

  using org_mode_t = typename org_info::SGPOrganismType;
  using stress_sym_mode_t = typename org_info::StressSymbiontType;
  using health_sym_mode_t = typename org_info::HealthSymbiontType;
  using nutrient_sym_mode_t = typename org_info::NutrientSymbiontType;

  // Used for any snapshot info that should be added to the config snapshot file
  // in addition to values in sgp_config object.
  struct ConfigSnapshotEntry {

    std::string param;    ///< Parameter name
    std::string value;    ///< Parameter value

    ConfigSnapshotEntry(
      const std::string& p,
      const std::string& v
    ) :
      param(p),
      value(v)
    { }

  };

  // Tag used to trigger start module in signalgp programs during run
  tag_t START_TAG;

protected:
  Scheduler scheduler; // Manages order that world locations (organisms) are processed each update
  size_t max_world_size; // Maximum number of locations in the world
  ReproductionQueue repro_queue; // Stores which organisms are queued for reproduction
  ProgramBuilder<hw_spec_t> prog_builder; // Utility for building signalgp programs
  tasks::LogicTaskEnvironment task_env;   // Manages task set, task requirements, and task rewards
  mutator_t mutator;  // Handles mutating sgp programs
  // TODO - Consider having symbiont rectifier and host rectifier
  //        -> Symbiont-specific instructions wouldn't be in host's instruction set
  sgp_prog_rectifier_t opcode_rectifier; // Used to "disable" instructions at runtime based on run configuration

  // Flag for whether setup has been run.
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

  // What kind of SGP organism type to use?
  org_mode_t sgp_org_type = org_mode_t::DEFAULT;
  // If using stress organisms, what kind of stress?
  stress_sym_mode_t stress_sym_type = stress_sym_mode_t::MUTUALIST;
  bool stress_extinction_update = false;

  health_sym_mode_t health_sym_type = health_sym_mode_t::MUTUALIST;
  nutrient_sym_mode_t nutrient_sym_type = nutrient_sym_mode_t::MUTUALIST;

  fun_apply_nutrient_interaction_t fun_apply_nutrient_interaction;

  // NOTE - Don't love this being owned by the world.
  //        Not sure of better alterative. Need to know this in InitializeState
  //        (don't want to re-lookup every call using strings). Couldn't have it float
  //        inside of the GenomeLibrary file because the static map used by these
  //        GetOpCode functions isn't initialized at that point.
  std::unordered_set<uint8_t> sgp_jump_opcodes = {
    Library::GetOpCode("JumpIfNEq"),
    Library::GetOpCode("JumpIfEq"),
    Library::GetOpCode("JumpIfLess")
  };

  // Directory to dump output files into.
  std::filesystem::path output_dir;
  // Contains config information (from outside sgp_config) to include in configuration snapshot
  emp::vector<ConfigSnapshotEntry> config_snapshot_entries;

  // Function to check compatibility between host and symbiont
  // - Used to check eligibility for vertical / horizontal transmission, etc.
  fun_compatibility_check_t fun_host_sym_compatibility_check;

  // begin_update_sig - Triggers at the beginning of an Update call.
  //  Triggers before schedule update, before processing any organisms.
  //  E.g., used for resetting any per-update data tracking.
  emp::Signal<void(void)> begin_update_sig;

  // ---- Symbiont birth signals / functors ----
  // before_sym_do_birth_sig - Triggers during SymDoBirth function.
  //  Triggers after sym offspring is created but before fun_sym_do_birth() is called.
  emp::Signal<void(
    emp::Ptr<sgp_sym_t>,          /* sym_baby_ptr */
    const emp::WorldPosition&     /* parent_pos */
  )> before_sym_do_birth_sig;

  // after_sym_do_birth_sig - Triggers during SymDoBirth function.
  //  Triggers after fun_sym_do_birth() is called.
  emp::Signal<void(
    const emp::WorldPosition& /* sym_baby_pos */
  )> after_sym_do_birth_sig;

  // fun_sym_do_birth - Configurable functor that handles calling appropriate
  //  "DoBirth" function depending on whether free-living symbionts are turned on.
  fun_sym_do_birth_t fun_sym_do_birth;

  // ---- Symbiont vertical transmission signals / functors ----
  // before_sym_vert_transmission_sig - Triggers in EndosymAttemptVertTransmission function.
  //  Triggers before VerticalTransmission is called on endosymbiont attempting
  //  vertical transmission. I.e., before attempt is made.
  emp::Signal<void(
    emp::Ptr<sgp_sym_t>,       /* sym_ptr - symbiont producing offspring */
    emp::Ptr<sgp_host_t>,      /* host_offspring_ptr - transmission to */
    emp::Ptr<sgp_host_t>,      /* host_parent_ptr - transmission from */
    const emp::WorldPosition&  /* host_parent_pos */
  )> before_sym_vert_transmission_sig;

  // after_sym_vert_transmission_sig - Triggers in EndosymAttemptVertTransmission function.
  //  Triggers ater vertical transmission attempt has been made.
  //  If attempt was successful, sym_offspring_ptr will point to the new symbiont
  //  offspring (that was vertically transmitted). If unsuccessful, sym_offspring_ptr
  //  will be a nullptr.
  emp::Signal<void(
    emp::Ptr<sgp_sym_t>,         /* sym_offspring_ptr */
    emp::Ptr<sgp_sym_t>,         /* sym_parent_ptr */
    emp::Ptr<sgp_host_t>,        /* host_offspring_ptr */
    emp::Ptr<sgp_host_t>,        /* host_parent_ptr */
    const emp::WorldPosition&,   /* host_parent_pos */
    bool                         /* vertical transmission success */
  )> after_sym_vert_transmission_sig;

  // fun_can_attempt_vert_trans - Called during HostDoBirth to determine if
  //  a given symbiont can attempt vertical transmission into host offspring.
  fun_can_attempt_vert_trans_t fun_can_attempt_vert_trans;

  // ---- Host birth signals / functors ----
  // before_host_do_birth_sig - Triggers during HostDoBirth().
  //  When this triggers, host offspring has been created (when reproduction queue)
  //  is processed. Triggers before endosymbionts attempt vertical transmission and
  //  before DoBirth is called.
  emp::Signal<void(
    sgp_host_t&,               /* host_offspring_ptr */
    sgp_host_t&,               /* host_parent_ptr */
    const emp::WorldPosition&  /* parent_pos */
  )> before_host_do_birth_sig;

  // after_host_do_birth_sig - Triggers during HostDoBirth().
  //  Triggers after endosymbionts attempt vertical transmission and after DoBirth
  //  is called on the host offspring.
  // NOTE - add more parameters to this? We know the parent / offsspring where this is called.
  emp::Signal<void(
    const emp::WorldPosition& /* host_offspring_pos */
  )> after_host_do_birth_sig;

  // ---- Host process signals / functors ----
  // before_host_process_sig - Triggers in ProcessHostAt()
  //  Triggers before running the host's CPU / after updating host location.
  //  Host is not guaranteed to still be alive if prior actions attached to this signal
  //  kill the host.
  emp::Signal<void(
    sgp_host_t&
  )> before_host_process_sig;

  // after_host_process_sig - Triggers in ProcessHostAt()
  //  Triggers at end of ProcessHostAt. There is one final check for death after
  //  after this triggers in case an attached action kills the host.
  emp::Signal<void(
    sgp_host_t&
  )> after_host_process_sig;

  // after_host_cpu_step_sig - Triggers in ProcessHostAt()
  //  Triggers after each CPU cycle (potentially multiple times per update) and after
  //  handling a repro attempt by the host for that CPU cycle.
  emp::Signal<void(
    sgp_host_t&
  )> after_host_cpu_step_sig;

  // after_host_cpu_exec_sig - Triggers in ProcessHostAt()
  //  Triggers after executing all CPU cycles allotted to host being processed and
  //  before processing the host's endosymbionts.
  emp::Signal<void(
    sgp_host_t&
  )> after_host_cpu_exec_sig;
  // fun_process_endosym_t fun_process_endosym; // NOTE - not used at the moment

  // ---- Free-living symbiont signals / functors ----
  // before_freeliving_sym_process_sig - Triggers in ProcessFreeLivingSymAt()
  //  Triggers if sym is alive before executing sym's CPU.
  emp::Signal<void(
    sgp_sym_t&  /* sym */
  )> before_freeliving_sym_process_sig;

  // after_freeliving_sym_process_sig - Triggers in ProcessFreeLivingSymAt()
  //  Triggers at end of ProcessFreeLivingSymAt, but before a final check/potential
  //  DoSymDeath call
  emp::Signal<void(
    sgp_sym_t&  /* sym */
  )> after_freeliving_sym_process_sig;

  // after_freeliving_sym_cpu_step_sig - Triggers in ProcessFreeLivingSymAt()
  //  Triggers after each CPU cycle after handling an instruction-triggered repro attempt.
  emp::Signal<void(
    sgp_sym_t&  /* sym */
  )> after_freeliving_sym_cpu_step_sig;

  // after_freeliving_sym_cpu_exec_sig - Triggers in ProcessFreeLivingSymAt()
  //  Triggers after executing all CPU cycles allotted to sym being processed and
  //  before SGPSymbiont::Process is called.
  emp::Signal<void(
    sgp_sym_t&  /* sym */
  )> after_freeliving_sym_cpu_exec_sig;

  // ---- Endosymbiont process signals / functors ----
  emp::Signal<void(
    const emp::WorldPosition&, /* sym_pos */
    sgp_sym_t&,                /* sym */
    sgp_host_t&                /* host */
  )> before_endosym_host_process_sig;

  // before_endosym_process_sig - Triggers during ProcessEndoSymbiont()
  emp::Signal<void(
    const emp::WorldPosition&, /* sym_pos */
    sgp_sym_t&,                /* sym */
    sgp_host_t&                /* host */
  )> before_endosym_process_sig;

  // after_endosym_process_sig - Triggers during ProcessEndoSymbiont()
  emp::Signal<void(
    const emp::WorldPosition&, /* sym_pos */
    sgp_sym_t&,                /* sym */
    sgp_host_t&                /* host */
  )> after_endosym_process_sig;

  // after_endosym_cpu_step_sig - Triggers during ProcessEndoSymbiont()
  emp::Signal<void(
    const emp::WorldPosition&, /* sym_pos */
    sgp_sym_t&,                /* sym */
    sgp_host_t&                /* host */
  )> after_endosym_cpu_step_sig;

  // after_endosym_cpu_exec_sig - Triggers during ProcessEndoSymbiont()
  emp::Signal<void(
    const emp::WorldPosition&, /* sym_pos */
    sgp_sym_t&,                /* sym */
    sgp_host_t&                /* host */
  )> after_endosym_cpu_exec_sig;


  // ---- Environment signals/functors ----
  // fun_do_resource_inflow_t fun_do_resource_inflow;

  // Called in FindHostForHorizontalTrans(), configured in SetupPopStructure().
  // Returns a target position for symbiont to horizontally transmit into.
  // Returns std::nullopt if failed to find suitable target position.
  std::function<std::optional<emp::WorldPosition>(
    size_t,                 /* Parent's host location id in world (pops[0][id])*/
    emp::Ptr<sgp_sym_t>     /* Pointer to symbiont parent (producing the sym offspring) */
  )> fun_find_host_for_horizontal_trans;

  // ----- Internal helper functions -----
  // Called by Update()
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
  void SetupScheduler();
  void SetupReproduction();
  void SetupSymReproduction();
  void SetupHostReproduction();
  void SetupHostSymInteractions();
  void SetupTaskEnvironment();
  void SetupMutator();
  void SetupStressInteractions();
  void SetupHealthInteractions();
  void SetupNutrientInteractions();

  // Clear all world signals
  void ClearWorldSignals() {
    begin_update_sig.Clear();
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
    before_endosym_host_process_sig.Clear();
    before_endosym_process_sig.Clear();
    after_endosym_process_sig.Clear();
    after_endosym_cpu_step_sig.Clear();
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

  // Utility function to get cpu state from an org pointer
  sgp_cpu_peripheral_t& GetCPUState(emp::Ptr<Organism> org_ptr) {
    return (org_ptr->IsHost()) ?
      (static_cast<sgp_host_t*>(org_ptr.Raw()))->GetHardware().GetCPUState() :
      (static_cast<sgp_sym_t*>(org_ptr.Raw()))->GetHardware().GetCPUState();
  }

public:
  SGPWorld(
    emp::Random& rnd,
    emp::Ptr<SymConfigSGP> _config
  ) :
    SymWorld(rnd, _config),
    scheduler(rnd),
    prog_builder(opcode_rectifier),
    task_env(rnd),
    mutator(opcode_rectifier),
    sgp_config(*_config)
  { }

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
  // NOTE - Thoughts on holding a reference vs a pointer. Reference is a little
  //   cleaner in my opinion, but will always need to know at construction
  //   (which is how things were already setup with the pointer).
  const SymConfigSGP& GetConfig() const { return sgp_config; }
  emp::Ptr<SymConfigSGP> GetConfigPtr() { return &sgp_config; }

  task_env_t& GetTaskEnv() { return task_env; }
  const task_env_t& GetTaskEnv() const { return task_env; }

  size_t GetTaskCount() const { return task_env.GetTaskCount(); }

  const std::unordered_set<uint8_t>& GetJumpInstOpcodes() const { return sgp_jump_opcodes; }

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
    scheduler.UpdateSchedule();
    // Run scheduler to process organisms
    scheduler.Run(*this);

    // Process reproduction queue
    DoReproduction();

    // Process graveyard, deletes all dead organisms.
    ProcessGraveyard();

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

  void ProcessHostOutputBuffer(sgp_host_t& host);
  void ProcessSymOutputBuffer(sgp_sym_t& sym);

  // NOTE - moved to be public for testing
  void AssignNewEnvIO(sgp_cpu_peripheral_t& cpu_state) {
    const size_t env_id = GetRandom().GetUInt(task_env.GetIOBank().GetSize());
    const auto& task_io = task_env.GetIOBank().GetIO(env_id);
    cpu_state.SetTaskEnvID(env_id);
    cpu_state.SetInputs(task_io.input_buffer);
    cpu_state.ResetCreditedOutputs();
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
  void SetupHosts(long unsigned int* POP_SIZE) override;
  void SetupSymbionts(long unsigned int* total_syms) override;

  // Prototypes for reproduction handling methods
  // SymDoBirth is for horizontal transmission and birthing free-living symbionts.
  emp::WorldPosition SymDoBirth(
    emp::Ptr<Organism> sym_baby,
    emp::WorldPosition parent_pos
  ) override;

  void HostDoMutation(sgp_host_t& host);
  void SymDoMutation(sgp_sym_t& sym);

  void SymDonateToHost(Organism& from_sym, Organism& to_host);
  void SymStealFromHost(Organism& to_sym, Organism& from_host);
  void FreeLivingSymDoInfect(Organism& sym);

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
  health_sym_mode_t GetHealthSymType() const { return health_sym_type; }
  nutrient_sym_mode_t GetNutrientSymType() const { return nutrient_sym_type; }

  ReproductionQueue& GetReproQueue() { return repro_queue; }

  // Data node methods
  emp::DataMonitor<double>& GetSymDonatedDataNode() {
    emp_assert(data_node_sym_donated != nullptr);
    return *data_node_sym_donated;
  }
  emp::DataMonitor<double>& GetSymStolenDataNode() {
    emp_assert(data_node_sym_stolen != nullptr);
    return *data_node_sym_stolen;
  }
  emp::DataMonitor<double>& GetSymEarnedDataNode() {
    emp_assert(data_node_sym_earned != nullptr);
    return *data_node_sym_earned;
  }
  void SetupTasksNodes();

  ProgramBuilder<hw_spec_t>& GetProgramBuilder() { return prog_builder; }
  const ProgramBuilder<hw_spec_t>& GetProgramBuilder() const { return prog_builder; }

  emp::DataFile& SetupOrgCountFile(const std::string& filepath);
  emp::DataFile& SetupSymDonatedFile(const std::string& filepath);
  emp::DataFile& SetupTasksFile(const std::string& filepath);
  void WriteTaskCombinationsFile(const std::string& filepath);
  void WriteOrgReproHistFile(const std::string& filepath);

  void CreateDataFiles() override;

  void SnapshotConfig(const std::string& filename="run_config.csv");
};

}

#endif