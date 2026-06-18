#ifndef SGP_WORLD_SETUP_C
#define SGP_WORLD_SETUP_C

#include "SGPWorld.h"
#include "org_type_info.h"
#include "utils.h"
#include "hardware/SGPHardware.h"
#include "sgpl/utility/ThreadLocalRandom.hpp"


#include "emp/datastructs/map_utils.hpp"
#include "emp/tools/string_utils.hpp"
#include "emp/math/math.hpp"


// TODO - assert that sym / host has program
namespace sgpmode {

void SGPWorld::Setup() {
  // Clear all world signals
  ClearWorldSignals();
  // Clear any config snapshot entries
  config_snapshot_entries.clear();

  // Configure population's spatial connectivity
  SetupSpatialStructure();
  // NOTE - Some of this code is repeated from base class.
  //  - Could do some reorganization to copy-paste. E.g., make functions for this,
  //     add hooks into the base setup to give more 1wnstream flexibility.
  const double start_moi = sgp_config.START_MOI();
  // NOTE - should POP_SIZE be changed to INIT_POP_SIZE for clarity?
  long unsigned int POP_SIZE;
  // TODO - add pop mode?
  // NOTE - SetupSpatialStructure should configure pop vector size to be carrying capacity.
  max_world_size = GetSize();
  if (sgp_config.INIT_POP_SIZE() < 0) {
    POP_SIZE = max_world_size;
  } else {
    POP_SIZE = sgp_config.INIT_POP_SIZE();
  }

  // Reset the seed of the main sgp thread based on the config
  // TODO - should this be here? (used to be inside scheduler)
  sgpl::tlrand.Get().ResetSeed(sgp_config.SEED());

  // Configure start tag
  // TODO - clean up start tag management.
  START_TAG.SetUInt64(0, std::numeric_limits<uint64_t>::max());

  // TODO - configure program builder if necessary
  prog_builder.SetStartTag(START_TAG);

  // TODO - print out mapper after all deletes, hand-check
  // TODO - add tests for rectifier inst removals

  // Configure SGP organism type variables based on config settings
  SetupOrgTypeVariables();

  // Remove and rectify instruction set as needed
  DisableConfigurableInstructions();

  // Configure task environment
  SetupTaskEnvironment();

  // Setup mutation operator
  SetupMutator();

  // Setup scheduler
  SetupScheduler();

  // Setup host and symbiont reproduction
  SetupReproduction();

  // Setup any host-symbiont interactions
  SetupHostSymInteractions();

  // CureHost signal
  // TODO: move to default? Figure out how to remove duplication
  if (sgp_config.CURE()) {
    begin_update_sig.AddAction(
      [this]() {
        if (GetUpdate() == sgp_config.CURE_UPDATES()) {
          CureHosts();
        }
      }
    );
  }

  if (sgp_config.ENABLE_TEMP_CHANGING_ENVIRONMENT()) {
    SetupChangingEnvironment();
  }

  SetupHosts(&POP_SIZE);
  // NOTE - any way to clean this up a little? Or, add some explanatory comments.
  long unsigned int total_syms = POP_SIZE * start_moi;
  SetupSymbionts(&total_syms);

  CreateDataFiles();
  SnapshotConfig();
  setup = true;
}

void SGPWorld::SetupChangingEnvironment() {
  // on setup, set NAND, AND-NOT, OR-NOT to be negative (at update zero)
  // then during each interval apply *-1 to the changing tasks

  size_t nand_task_id = task_env.GetTaskSet().GetSize();
  if (task_env.GetTaskSet().HasTask("NAND")) {
    nand_task_id = task_env.GetTaskSet().GetID("NAND");
  }
  else if (task_env.GetTaskSet().HasTask("nand")) {
    nand_task_id = task_env.GetTaskSet().GetID("nand");
  }

  size_t andn_task_id = task_env.GetTaskSet().GetSize();
  if (task_env.GetTaskSet().HasTask("AND_NOT")) {
    andn_task_id = task_env.GetTaskSet().GetID("AND_NOT");
  }
  else if (task_env.GetTaskSet().HasTask("and_not")) {
    andn_task_id = task_env.GetTaskSet().GetID("and_not");
  }


  size_t orn_task_id = task_env.GetTaskSet().GetSize();
  if (task_env.GetTaskSet().HasTask("OR_NOT")) {
    orn_task_id = task_env.GetTaskSet().GetID("OR_NOT");
  }
  else if (task_env.GetTaskSet().HasTask("or_not")) {
    orn_task_id = task_env.GetTaskSet().GetID("or_not");
  }


  // grab task ids for NOT, AND, OR
  size_t not_task_id = task_env.GetTaskSet().GetSize();
  if (task_env.GetTaskSet().HasTask("NOT")) {
    not_task_id = task_env.GetTaskSet().GetID("NOT");
  }
  else if (task_env.GetTaskSet().HasTask("not")) {
    not_task_id = task_env.GetTaskSet().GetID("not");
  }

  size_t and_task_id = task_env.GetTaskSet().GetSize();
  if (task_env.GetTaskSet().HasTask("AND")) {
    and_task_id = task_env.GetTaskSet().GetID("AND");
  }
  else if (task_env.GetTaskSet().HasTask("and")) {
    and_task_id = task_env.GetTaskSet().GetID("and");
  }

  size_t or_task_id = task_env.GetTaskSet().GetSize();
  if (task_env.GetTaskSet().HasTask("OR")) {
    or_task_id = task_env.GetTaskSet().GetID("OR");
  }
  else if (task_env.GetTaskSet().HasTask("or")) {
    or_task_id = task_env.GetTaskSet().GetID("or");
  }

  // update 0 will flip not-and-or to rewarded and nand-andn-orn to punished
  GetTaskEnv().GetHostTaskReq(not_task_id).task_value = -1 * GetTaskEnv().GetHostTaskReq(not_task_id).task_value;
  GetTaskEnv().GetSymTaskReq(not_task_id).task_value = -1 * GetTaskEnv().GetSymTaskReq(not_task_id).task_value;

  GetTaskEnv().GetHostTaskReq(and_task_id).task_value = -1 * GetTaskEnv().GetHostTaskReq(and_task_id).task_value;
  GetTaskEnv().GetSymTaskReq(and_task_id).task_value = -1 * GetTaskEnv().GetSymTaskReq(and_task_id).task_value;

  GetTaskEnv().GetHostTaskReq(or_task_id).task_value = -1 * GetTaskEnv().GetHostTaskReq(or_task_id).task_value;
  GetTaskEnv().GetSymTaskReq(or_task_id).task_value = -1 * GetTaskEnv().GetSymTaskReq(or_task_id).task_value;

  begin_update_sig.AddAction(
    [this, nand_task_id, andn_task_id, orn_task_id, not_task_id, and_task_id, or_task_id]() {
      if (GetUpdate() % sgp_config.TEMP_CHANGING_ENVIRONMENT_INTERVAL() == 0) {
        GetTaskEnv().GetHostTaskReq(nand_task_id).task_value = -1 * GetTaskEnv().GetHostTaskReq(nand_task_id).task_value;
        GetTaskEnv().GetHostTaskReq(andn_task_id).task_value = -1 * GetTaskEnv().GetHostTaskReq(andn_task_id).task_value;
        GetTaskEnv().GetHostTaskReq(orn_task_id).task_value = -1 * GetTaskEnv().GetHostTaskReq(orn_task_id).task_value;

        GetTaskEnv().GetHostTaskReq(not_task_id).task_value = -1 * GetTaskEnv().GetHostTaskReq(not_task_id).task_value;
        GetTaskEnv().GetHostTaskReq(and_task_id).task_value = -1 * GetTaskEnv().GetHostTaskReq(and_task_id).task_value;
        GetTaskEnv().GetHostTaskReq(or_task_id).task_value = -1 * GetTaskEnv().GetHostTaskReq(or_task_id).task_value;


        GetTaskEnv().GetSymTaskReq(nand_task_id).task_value = -1 * GetTaskEnv().GetSymTaskReq(nand_task_id).task_value;
        GetTaskEnv().GetSymTaskReq(andn_task_id).task_value = -1 * GetTaskEnv().GetSymTaskReq(andn_task_id).task_value;
        GetTaskEnv().GetSymTaskReq(orn_task_id).task_value = -1 * GetTaskEnv().GetSymTaskReq(orn_task_id).task_value;

        GetTaskEnv().GetSymTaskReq(not_task_id).task_value = -1 * GetTaskEnv().GetSymTaskReq(not_task_id).task_value;
        GetTaskEnv().GetSymTaskReq(and_task_id).task_value = -1 * GetTaskEnv().GetSymTaskReq(and_task_id).task_value;
        GetTaskEnv().GetSymTaskReq(or_task_id).task_value = -1 * GetTaskEnv().GetSymTaskReq(or_task_id).task_value;
      }
    }
  );
}

void SGPWorld::DisableConfigurableInstructions() {

  // Knock out any mode-related instructions that shouldn't be active for this run
  if (!sgp_config.DONATION_STEAL_INST()) {
    // Knockout donate instruction
    del_inst(
      opcode_rectifier.mapper.begin(),
      opcode_rectifier.mapper.end(),
      Library::GetOpCode("Donate"),
      Library::GetSize()
    );
    // Knockout steal instruction
    del_inst(
      opcode_rectifier.mapper.begin(),
      opcode_rectifier.mapper.end(),
      Library::GetOpCode("Steal"),
      Library::GetSize()
    );
  }

  // If free-living symbionts are disabled, disable the infect instruction
  if (!sgp_config.FREE_LIVING_SYMS()) {
    // Knockout the infect instruction
    del_inst(
      opcode_rectifier.mapper.begin(),
      opcode_rectifier.mapper.end(),
      Library::GetOpCode("Infect"),
      Library::GetSize()
    );
  }

  // if temporally changing environment are off, or if organisms aren't allowed to sense their environment,
  // disable the SenseTask instruction
  if (!sgp_config.ENABLE_TEMP_CHANGING_ENVIRONMENT() || sgp_config.TEMP_CHANGING_ENVIRONMENT_ORG_TYPE() == "static") {
    del_inst(
      opcode_rectifier.mapper.begin(),
      opcode_rectifier.mapper.end(),
      Library::GetOpCode("SenseTask"),
      Library::GetSize()
    );
  }
}

void SGPWorld::SetupScheduler() {
  // Configure scheduler w/max world size (updated in SGPWorld::Setup, and cfg thread count)
  scheduler.SetupScheduler(max_world_size);
  // Scheduler calls world's ProcessOrgAt function
}

void SGPWorld::SetupReproduction() {
  // Setup reproduction queue
  repro_queue.Clear();

  // NOTE - could configure reproduce function in repro queue for maximum
  //        runtime configurability
  repro_queue.SetReproduceOrgFun([this](ReproEvent& repro_info) {
    emp::Ptr<Organism> org = repro_info.org;
    emp::Ptr<Organism> child = org->Reproduce();
    if (child->IsHost()) {
      HostDoBirth(child, org, repro_info.pos);
      // Mark parent as no longer reproducing (world handles setting state, so should handle resetting)
      // NOTE - could move reset repro state in Reproduce functions
      // static_cast<sgp_host_t*>(org.Raw())->GetHardware().GetCPUState().ResetReproState();
    } else {
      const emp::WorldPosition sym_baby_pos = SymDoBirth(child, repro_info.pos);
      emp::Ptr<sgp_sym_t> sym_parent = static_cast<sgp_sym_t*>(org.Raw());
      // Trigger any post-birth actions
      after_sym_do_birth_sig.Trigger(sym_baby_pos, sym_parent);
      // Mark parent as no longer reproducing
      // static_cast<sgp_sym_t*>(org.Raw())->GetHardware().GetCPUState().ResetReproState();
    }
  });

  // OnBeforePlacement happens during emp::World's AddOrgAt
  // Set CPUState's location when organism is added to the world.
  OnBeforePlacement(
    [this](Organism& org, size_t loc) {
      if (org.IsHost()) {
        static_cast<sgp_host_t&>(org).GetHardware().GetCPUState().SetLocation({loc});
        this->AssignNewEnvIO(static_cast<sgp_host_t&>(org).GetHardware().GetCPUState()); // AEV Question: is there a better place for this?
      } else {
        static_cast<sgp_sym_t&>(org).GetHardware().GetCPUState().SetLocation({loc});
        this->AssignNewEnvIO(static_cast<sgp_sym_t&>(org).GetHardware().GetCPUState());
      }
    }
  );

  SetupHostReproduction();
  SetupSymReproduction();
}

// Configure HostDoBirth signals
void SGPWorld::SetupHostReproduction() {
  // TODO - anything else to configure here?
}

// Configure symbiont reproduction signals
void SGPWorld::SetupSymReproduction() {

  // Configure sym do birth function, if free-living, symbiont offspring go right into world
  if (sgp_config.FREE_LIVING_SYMS()) {
    // Configure sym birth in free-living symbiont mode
    fun_sym_do_birth = [this](
      emp::Ptr<sgp_sym_t> sym_baby_ptr,
      const emp::WorldPosition& parent_pos
    ) -> emp::WorldPosition {
      return FreeLivingSymDoBirth(sym_baby_ptr, parent_pos);
    };
  } else if (sgp_config.HORIZ_TRANS()){
    // Configure sym birth in non-free-living symbiont mode.
    fun_sym_do_birth = [this](
      emp::Ptr<sgp_sym_t> sym_baby_ptr,
      const emp::WorldPosition& parent_pos
    ) -> emp::WorldPosition {
      return SymAttemptHorizontalInfection(sym_baby_ptr, parent_pos);
    };
  } else {
    // Neither horizontal transmission nor free-living symbionts, so fun_sym_do_birth should just return invalid position and clean up the offspring
    fun_sym_do_birth = [this](
      emp::Ptr<sgp_sym_t> sym_baby_ptr,
      const emp::WorldPosition& parent_pos
    ) -> emp::WorldPosition {
      sym_baby_ptr.Delete();
      return emp::WorldPosition();
    };
  }

  // Configure after sym birth data tracking
  // NOTE - Should this be adjusted at all to account for configuration differences?
  // QUESTION - Should this fire for free-living symbionts?
  //            Can instead make this trigger after horizontal transmission
  after_sym_do_birth_sig.AddAction(
    [this](const emp::WorldPosition& sym_baby_pos, emp::Ptr<sgp_sym_t> sym_parent_ptr) {
      sym_parent_ptr->AfterIndependentReproduction(sym_baby_pos);
    }
  );

  // Configure vertical transmission
  // TODO - Probably will need to change VT_TASK_MATCH to a categorical variable
  //        to accomodate different mechanisms for determining whether vt is possible.
  if (sgp_config.VT_TASK_MATCH()) {
    // If task matching required, check.
    fun_vert_trans_compatible = [this](
      sgp_sym_t& sym,
      sgp_host_t& host_offspring,
      sgp_host_t& host_parent
    ) -> bool {
      // Check if host profile and sym profile have any overlap?
      auto& host_profile = fun_get_host_task_profile(host_parent);
      auto& sym_profile = fun_get_sym_task_profile(sym);
      return utils::AnyMatchingOnes(host_profile, sym_profile);
    };
  } else {
    // Otherwise, allow attempt in all cases.
    fun_vert_trans_compatible = [](
      sgp_sym_t& sym,
      sgp_host_t& host_offspring,
      sgp_host_t& host_parent
    ) -> bool {
      return true;
    };
  }

  // TODO - anything else to setup here?
}

// TODO - clear host process signals
void SGPWorld::SetupHosts(long unsigned int* POP_SIZE) {
  // TODO - add any signals for host/endosymbiont initialization?

  // TODO - discuss implications of timing for core launch
  // Launch core if none running.
  before_host_cpu_exec_sig.AddAction(
    [this](sgp_host_t& host) {
      // NOTE - currently, LaunchCPU will only launch if no cores currently running

      host.GetHardware().LaunchCPU(START_TAG);
    }
  );

  // TODO - inject initial population at fixed positions (unless configured otherwise)
  size_t not_task_id = task_env.GetTaskSet().GetSize();
  if (task_env.GetTaskSet().HasTask("NOT")) {
    not_task_id = task_env.GetTaskSet().GetID("NOT");
  } else if (task_env.GetTaskSet().HasTask("not")) {
    not_task_id = task_env.GetTaskSet().GetID("not");
  }


  const size_t init_pop_size = *POP_SIZE;
  for (size_t i = 0; i < init_pop_size; ++i) {
    emp::Ptr<sgp_host_t> new_host;
    sgp_prog_t init_prog(
      prog_builder.CreateNotProgram(PROGRAM_LENGTH)
    );
    switch (sgp_org_type) {
      case org_mode_t::DEFAULT:
        new_host = emp::NewPtr<sgp_host_t>(
          random_ptr,
          this,
          &sgp_config,
          init_prog,
          sgp_config.HOST_INT()
        );
        break;
        // TODO - add back more modes
      default:
        // org mode has already been verified, so something has gone very wrong
        // with that if we're here.
        std::cout << "Unrecognized SGP organism type: " << sgp_config.INTERACTION_MECHANISM() << std::endl;
        break;
    }

    // NOTE - what about other Start MOI values?
    // - these endosymbionts have empty programs?
    if (sgp_config.START_MOI() == 1) {
      sgp_prog_t sym_prog(
        prog_builder.CreateNotProgram(PROGRAM_LENGTH)
      );
      emp::Ptr<sgp_sym_t> new_sym = emp::NewPtr<sgp_sym_t>(
        random_ptr,
        this,
        &sgp_config,
        sym_prog,
        sgp_config.SYM_INT()
      );
      // TODO - add InjectSymIntoHost to wrap
      // NOTE - Move env io assignment to different signal that is triggered on inject?
      // AssignNewEnvIO(new_sym->GetHardware().GetCPUState()); // Add to AddSymbiont
      // Set sym's parent task
      if (task_env.IsSymTask(not_task_id)) {
        new_sym->GetHardware().GetCPUState().SetParentTaskPerformed(not_task_id, true);
        new_sym->GetHardware().GetCPUState().SetParentFirstTaskPerformed(not_task_id, true);
        new_sym->GetHardware().GetCPUState().MarkTaskPerformed(not_task_id);
      }
      // NOTE - Do we need to set location in cpu state here?
      new_host->AddSymbiont(new_sym);
    }
    // TODO - Add SGPWorld function to wrap inject host function
    // AssignNewEnvIO(new_host->GetHardware().GetCPUState()); // This is in OnPlacement now, so should be fine
    if (task_env.IsHostTask(not_task_id)) {
      new_host->GetHardware().GetCPUState().SetParentTaskPerformed(not_task_id, true);
      new_host->GetHardware().GetCPUState().SetParentFirstTaskPerformed(not_task_id, true);
    }
    InjectHost(new_host);
  }
}

void SGPWorld::SetupSymbionts(long unsigned int* total_syms) {
  // NOTE - this was empty in original implementation.

  before_endosym_process_sig.AddAction(
    [this](
      const emp::WorldPosition& sym_pos,
      sgp_sym_t& sym,
      sgp_host_t& host
    ) {
      // NOTE - currently, LaunchCPU will
      sym.GetHardware().LaunchCPU(START_TAG);
    }
  );

  before_freeliving_sym_process_sig.AddAction(
    [this](sgp_sym_t& sym) {
      // NOTE - currently, LaunchCPU will
      sym.GetHardware().LaunchCPU(START_TAG);
    }
  );

}

void SGPWorld::SetupTaskEnvironment() {
  // TODO - configure any world <--> environment interactions that need to be
  //        setup prior to run
  task_env.Setup(
    sgp_config.TASK_ENV_CFG_PATH(),
    sgp_config.TASK_IO_BANK_SIZE(),
    sgp_config.TASK_IO_UNIQUE_OUTPUT()
  );

  // Configure organism input buffers / environment id
  // NOTE - now that assigning new env io is in a function, could
  //        hardcode these calls in "ProcessOrg" functions.
  //        If this isn't something we want to configure at runtime, should do that.
  // TODO - Move assign new env to where host_do_birth_sig
  before_host_do_birth_sig.AddAction(
    [this](
      sgp_host_t& host_offspring,
      sgp_host_t& host_parent,
      const emp::WorldPosition&  parent_pos
    ) {
      // auto& offspring_cpu_state = host_offspring.GetHardware().GetCPUState();
      // auto& parent_cpu_state = host_parent.GetHardware().GetCPUState();
      // AssignNewEnvIO(offspring_cpu_state); // This is in OnPlacement now, so should be fine
    }
  );

  before_sym_do_birth_sig.AddAction(
    [this](
      emp::Ptr<sgp_sym_t> sym_baby_ptr,
      const emp::WorldPosition& parent_pos
    ) {
      // AssignNewEnvIO(sym_baby_ptr->GetHardware().GetCPUState()); // This is in AddSymbiont and OnPlacement now, so should be fine
    }
  );

  // Inconsistent between host do birth, sym do birth, vert trans
  // because vert trans doesn't know sym offspring until after
  after_sym_vert_transmission_sig.AddAction(
    [this](
      emp::Ptr<sgp_sym_t> sym_offspring_ptr,
      emp::Ptr<sgp_sym_t> sym_parent_ptr,
      emp::Ptr<sgp_host_t> host_offspring_ptr,
      emp::Ptr<sgp_host_t> host_parent_ptr,
      bool success                        /* vertical transmission success */
    ) {
      if (!success) return;
      emp_assert(sym_offspring_ptr != nullptr);
    }
  );

  // --- Setup task completion/output buffer checks ---
  // NOTE - discuss timing of this check. Currently happens after executing cpu
  //        fully for this update
  // NOTE - discuss whether we want ability to configure this differently for different
  //        kinds of organisms
  // TODO - Move this into Process functions
  after_host_cpu_exec_sig.AddAction(
    [this](sgp_host_t& host) {
      host.ProcessOutputBuffer();
    }
  );

  // E.g., fine for freeliving and endo syms to have same output processing?
  after_freeliving_sym_cpu_exec_sig.AddAction(
    [this](sgp_sym_t& sym) {
      ProcessSymOutputBuffer(sym);
    }
  );

  after_endosym_cpu_exec_sig.AddAction(
    [this](
      const emp::WorldPosition& sym_pos,
      sgp_sym_t& sym,
      sgp_host_t& host
    ) {
      ProcessSymOutputBuffer(sym);
    }
  );
}

void SGPWorld::SetupMutator() {
  // NOTE - can add more flexibility to mutator
  mutator.SetPerBitMutationRate(sgp_config.SGP_MUT_PER_BIT_RATE());
  // NOTE - could make host mutator a functor that could be configured here
  //        same with endosymbionts / etc
}

}

#endif