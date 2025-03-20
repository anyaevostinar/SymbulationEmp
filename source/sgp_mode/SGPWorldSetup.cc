#ifndef SGP_WORLD_SETUP_C
#define SGP_WORLD_SETUP_C

#include "SGPWorld.h"
#include "org_type_info.h"
#include "utils.h"
#include "hardware/SGPHardware.h"
#include "sgpl/utility/ThreadLocalRandom.hpp"

#include "emp/datastructs/map_utils.hpp"
#include "emp/tools/string_utils.hpp"
// TODO - assert that sym / host has program
namespace sgpmode {
// TODO - implement "empty initialization" option
//        - Particularly useful for testing
void SGPWorld::Setup() {
  // Clear all world signals
  ClearWorldSignals();
  // Clear any config snapshot entries
  config_snapshot_entries.clear();

  // Reset the seed of the main sgp thread based on the config
  // TODO - should this be here? (used to be inside scheduler)
  sgpl::tlrand.Get().ResetSeed(sgp_config.SEED());

  // Configure start tag
  // TODO - clean up start tag management.
  START_TAG.SetUInt64(0, std::numeric_limits<uint64_t>::max());

  // TODO - configure program builder if necessary
  prog_builder.SetStartTag(START_TAG);

  // std::cout << opcode_rectifier.mapper << std::endl;
  // TODO - Delete this once confident in instruction removal
  std::cout << "Opcode rectifier mappings:";
  for (size_t i = 0; i < opcode_rectifier.mapper.size(); ++i) {
    std::cout << " " << (uint32_t)(opcode_rectifier.mapper[i]);
  }
  std::cout << std::endl;
  // TODO - print out mapper after all deletes, hand-check
  // TODO - add tests for rectifier inst removals

  // Configure SGP organism type
  SetupOrgMode();

  // Configure task environment
  SetupTaskEnvironment();

  // NOTE - Some of this code is repeated from base class.
  //  - Could do some reorganization to copy-paste. E.g., make functions for this,
  //     add hooks into the base setup to give more 1wnstream flexibility.
  double start_moi = sgp_config.START_MOI();
  // NOTE - should POP_SIZE be changed to INIT_POP_SIZE for clarity?
  long unsigned int POP_SIZE;
  // TODO - add pop mode?
  max_world_size = sgp_config.GRID_X() * sgp_config.GRID_Y();
  if (sgp_config.POP_SIZE() < 0) {
    POP_SIZE = max_world_size;
  } else {
    POP_SIZE = sgp_config.POP_SIZE();
  }

  // Setup mutation operator
  SetupMutator();

  // Setup population structure
  SetupPopStructure();

  // Setup scheduler
  SetupScheduler();

  // Setup host and symbiont reproduction
  SetupReproduction();

  // Setup any host-symbiont interactions
  SetupHostSymInteractions();

  SetupHosts(&POP_SIZE);
  Resize(max_world_size); // TODO - move this back to setup pop structure after fixing setup hosts
  // NOTE - any way to clean this up a little? Or, add some explanatory comments.
  long unsigned int total_syms = POP_SIZE * start_moi;
  SetupSymbionts(&total_syms);

  CreateDataFiles();
  SnapshotConfig();
  setup = true;
  // TODO - Delete this once confident in instruction removal
  std::cout << "Opcode rectifier mappings (post setup):";
  for (size_t i = 0; i < opcode_rectifier.mapper.size(); ++i) {
    std::cout << " " << (uint32_t)(opcode_rectifier.mapper[i]);
  }
  std::cout << std::endl;
}

void SGPWorld::SetupOrgMode() {
  // Convert cfg org type to lowercase
  std::string cfg_org_type(emp::to_lower(sgp_config.ORGANISM_TYPE()));
  // Get organism type (asserts validity)
  sgp_org_type = org_info::GetOrganismType(cfg_org_type);

  // Configure stress sym type
  // TODO - Implement SetupStressMode / etc
  std::string cfg_stress_sym_type(emp::to_lower(sgp_config.STRESS_TYPE()));
  // Get stress symbiont type (asserts validity)
  stress_sym_type = org_info::GetStressSymType(cfg_stress_sym_type);

  // TODO - configure other organism modes as appropriate

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

  // Configure stress
  if (sgp_config.ENABLE_STRESS()) {
    SetupStressInteractions();
  }
}

void SGPWorld::SetupStressInteractions() {
  emp_assert(sgp_config.ENABLE_STRESS());

  // Setup extinction variable
  // At beginning of update, determine whether an extinction event occurs
  begin_update_sig.AddAction(
    [this]() {
      const size_t u = GetUpdate();
      stress_extinction_update = (u > 0) && (u % sgp_config.EXTINCTION_FREQUENCY()) == 0;
    }
  );

  // Setup host interactions
  // NOTE - this can be simplified assuming no other desired differences in logic
  //        for parasite vs. mutualist (repeated code; only death chance is different)
  if (GetStressSymType() == stress_sym_mode_t::MUTUALIST) {
    before_host_process_sig.AddAction(
      [this](sgp_host_t& host) {
        if (!stress_extinction_update) return;
        // If host has a symbiont, death_chance = mutualist death chance
        // Otherwise, base death chance.
        const double death_chance = (host.HasSym()) ?
          sgp_config.MUTUALIST_DEATH_CHANCE() :
          sgp_config.BASE_DEATH_CHANCE();
        // Kill host with chosen probability
        if (random_ptr->P(death_chance)) {
          host.SetDead();
        }
      }
    );
  } else if (GetStressSymType() == stress_sym_mode_t::PARASITE) {
    before_host_process_sig.AddAction(
      [this](sgp_host_t& host) {
        if (!stress_extinction_update) return;
        // If host has a symbiont, death_chance = parasite death chance
        // Otherwise, base death chance.
        const double death_chance = (host.HasSym()) ?
          sgp_config.PARASITE_DEATH_CHANCE() :
          sgp_config.BASE_DEATH_CHANCE();
        // Kill host with chosen probability
        if (random_ptr->P(death_chance)) {
          host.SetDead();
        }
      }
    );
  }

  // TODO - Add instruction-mediated stress interaction mode

  // NOTE - What about free-living symbionts (if any)?
  //        Or endosymbionts?
}

void SGPWorld::SetupPopStructure() {
  // set world structure (either mixed or a grid with some dimensions)
  // and set synchronous generations to false
  if (!sgp_config.GRID()) {
    SetPopStruct_Mixed(false);
  } else {
    SetPopStruct_Grid(sgp_config.GRID_X(), sgp_config.GRID_Y(), false);
  }
  // Resize world capacity to max_world_size
  // Resize(max_world_size); // TODO - move ethis back here

  // Setup function that gets host neighbor (used for symbiont)
  // TODO - add different configuration options for this?
  fun_find_host_for_horizontal_trans = [this](
    size_t host_world_id,                 /* Parent's host location id in world (pops[0][id])*/
    emp::Ptr<sgp_sym_t> sym_parent_ptr    /* Pointer to symbiont parent (producing the sym offspring) */
  ) -> std::optional<emp::WorldPosition> {
    for (size_t attempt_i = 0; attempt_i < sgp_config.FIND_NEIGHBOR_HOST_ATTEMPTS(); ++attempt_i) {
      emp::WorldPosition candidate_pos(GetRandomNeighborPos(host_world_id));
      if (candidate_pos.IsValid() && IsOccupied(candidate_pos)) {
        emp::Ptr<Organism> neighbor_org_ptr = GetOrgPtr(candidate_pos.GetIndex());
        emp_assert(neighbor_org_ptr->IsHost());
        // Cast neighbor as sgp_host_t ptr.
        emp::Ptr<sgp_host_t> neighbor_host_ptr = static_cast<sgp_host_t*>(neighbor_org_ptr.Raw());
        const bool compatible = fun_host_sym_compatibility_check(
          *neighbor_host_ptr,
          *sym_parent_ptr
        );
        if (compatible) {
          return std::optional<emp::WorldPosition>{candidate_pos};
        }
      }
    }
    return std::nullopt;
  };
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
      SymDoBirth(child, repro_info.pos);
      // Mark parent as no longer reproducing
      // static_cast<sgp_sym_t*>(org.Raw())->GetHardware().GetCPUState().ResetReproState();
    }
  });

  SetupHostReproduction();
  SetupSymReproduction();
}

// Configure HostDoBirth signals
void SGPWorld::SetupHostReproduction() {
  // TODO - anything else to configure here?
}

// Configure symbiont reproduction signals
void SGPWorld::SetupSymReproduction() {
  // TODO - clean this up
  // stress hard-coded transmission modes
  // TODO - allow "layering on" of stress/nutrient/etc functionality
  if (sgp_org_type == org_mode_t::STRESS) {
    if (stress_sym_type == stress_sym_mode_t::MUTUALIST) {
      // mutualists
      sgp_config.VERTICAL_TRANSMISSION(1.0);
      sgp_config.HORIZ_TRANS(false);
    } else if (stress_sym_type == stress_sym_mode_t::PARASITE) {
      // parasites
      sgp_config.VERTICAL_TRANSMISSION(0);
      sgp_config.HORIZ_TRANS(true);
    }
  }

  // Configure sym do birth function
  // QUESTION - Is this setup function appropriate for this? Different setup function more appropriate?
  // QUESTION - Do we want to support free-living and horizontal transmission simultaneously?
  //            (current/original verison of code does not)
  if (sgp_config.FREE_LIVING_SYMS()) {
    // Configure sym birth in free-living symbiont mode
    fun_sym_do_birth = [this](
      emp::Ptr<sgp_sym_t> sym_baby_ptr,
      const emp::WorldPosition& parent_pos
    ) -> emp::WorldPosition {
      return FreeLivingSymDoBirth(sym_baby_ptr, parent_pos);
    };
  } else {
    // Configure sym birth in non-free-living symbiont mode.
    fun_sym_do_birth = [this](
      emp::Ptr<sgp_sym_t> sym_baby_ptr,
      const emp::WorldPosition& parent_pos
    ) -> emp::WorldPosition {
      return SymAttemptHorizontalTrans(sym_baby_ptr, parent_pos);
    };
  }

  // Configure after sym birth data tracking
  // NOTE - Should this be adjusted at all to account for configuration differences?
  // QUESTION - Should this fire for free-living symbionts?
  //            Can instead make this trigger after horizontal transmission
  after_sym_do_birth_sig.AddAction(
    [this](const emp::WorldPosition& sym_baby_pos) {
      // Because we're not calling HorizontalTransmission, we need to adjust
      // these data nodes here
      GetHorizontalTransmissionAttemptCount().AddDatum(1);
      if (sym_baby_pos.IsValid()) {
        GetHorizontalTransmissionSuccessCount().AddDatum(1);
      }
    }
  );

  // Configure vertical transmission
  // TODO - Probably will need to change VT_TASK_MATCH to a categorical variable
  //        to accomodate different mechanisms for determining whether vt is possible.
  if (sgp_config.VT_TASK_MATCH()) {
    // If task matching required, check.
    fun_can_attempt_vert_trans = [this](
      sgp_sym_t& sym,
      sgp_host_t& host_offspring,
      sgp_host_t& host_parent,
      const emp::WorldPosition& parent_pos
    ) -> bool {
      return fun_host_sym_compatibility_check(
        host_offspring,
        sym
      );
    };
  } else {
    // Otherwise, allow attempt in all cases.
    fun_can_attempt_vert_trans = [](
      sgp_sym_t& sym,
      sgp_host_t& host_offspring,
      sgp_host_t& host_parent,
      const emp::WorldPosition& parent_pos
    ) -> bool {
      return true;
    };
  }

  // TODO - anything else to setup here?
}


void SGPWorld::SetupHostSymInteractions() {
  // Setup function that determines host-symbiont compatibility
  // TODO - Will probably need to shift this bool config to a categorical config
  //        if we want to accomodate anything like tag matching for compatibility
  //        checking.
  if (sgp_config.TRACK_PARENT_TASKS()) {
    // Host-symbiont compatibility determined by their parent task profiles
    fun_host_sym_compatibility_check = [this](
      const sgp_host_t& host,
      const sgp_sym_t& sym
    ) -> bool {
      // Get tasks from host/sym
      const emp::BitVector& host_parent_tasks = host.GetHardware().GetCPUState().GetParentTasksPerformed();
      const emp::BitVector& sym_parent_tasks = sym.GetHardware().GetCPUState().GetParentTasksPerformed();
      return utils::AnyMatch(host_parent_tasks, sym_parent_tasks);
    };
  } else {
    // Host-symbiont compatibility determined by their task profiles
    fun_host_sym_compatibility_check = [this](
      const sgp_host_t& host,
      const sgp_sym_t& sym
    ) -> bool {
      // Get tasks from host/sym
      const emp::BitVector& host_parent_tasks = host.GetHardware().GetCPUState().GetTasksPerformed();
      const emp::BitVector& sym_parent_tasks = sym.GetHardware().GetCPUState().GetTasksPerformed();
      return utils::AnyMatch(host_parent_tasks, sym_parent_tasks);
    };
  }

}

// TODO - clear host process signals
void SGPWorld::SetupHosts(long unsigned int* POP_SIZE) {
  // TODO - add any signals for host/endosymbiont initialization?

  // TODO - discuss implications of timing for core launch
  // Launch core if none running.
  before_host_process_sig.AddAction(
    [this](sgp_host_t& host) {
      // NOTE - currently, LaunchCPU will only launch if no cores currently running
      host.GetHardware().LaunchCPU(START_TAG);
    }
  );

  // TODO - inject initial population at fixed positions (unless configured otherwise)
  const size_t init_pop_size = *POP_SIZE;
  for (size_t i = 0; i < init_pop_size; ++i) {
    emp::Ptr<sgp_host_t> new_host;
    sgp_prog_t init_prog(
      prog_builder.CreateNotProgram(PROGRAM_LENGTH)
    );
    // std::cout << "Creating host " << i << std::endl;
    switch (sgp_org_type) {
      case org_mode_t::DEFAULT:
        new_host = emp::NewPtr<sgp_host_t>(
          random_ptr,
          this,
          &sgp_config,
          init_prog,
          sgp_config.HOST_INT()
        );
        // new_host->GetHardware().PrintCode();
        break;
        // TODO - add back more modes
      default:
        // org mode has already been verified, so something has gone very wrong
        // with that if we're here.
        std::cout << "Unrecognized SGP organism type: " << sgp_config.ORGANISM_TYPE() << std::endl;
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
      // std::cout << "  Injecting symbiont into host" << std::endl;
      AssignNewEnvIO(new_sym->GetHardware().GetCPUState());
      new_host->AddSymbiont(new_sym);
    }
    // TODO - Add SGPWorld function to wrap inject host function
    AssignNewEnvIO(new_host->GetHardware().GetCPUState());
    InjectHost(new_host);
  }
}

void SGPWorld::SetupSymbionts(long unsigned int* total_syms) {
  // NOTE - this was empty in original implementation.

  before_endosym_process_sig.AddAction(
    [this](
      const emp::WorldPosition& sym_pos ,
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

  // TODO - configure offspring to have parent task profiles in cpustate

  // NOTE - discuss whether we want to reset parent after reproduction
  //        Currently, inconsistent between host do birth, sym do birth, vert trans
  //        because vert trans doesn't know sym offspring until after

  // Configure oganism input buffers / environment id
  // NOTE - now that assigning new env io is in a function, could
  //        hardcode these calls in "ProcessOrg" functions.
  //        If this isn't something we want to configure at runtime, should do that.
  before_host_do_birth_sig.AddAction(
    [this](
      sgp_host_t& host_offspring,
      sgp_host_t& host_parent,
      const emp::WorldPosition&  parent_pos
    ) {
      auto& offspring_cpu_state = host_offspring.GetHardware().GetCPUState();
      // auto& parent_cpu_state = host_parent.GetHardware().GetCPUState();
      AssignNewEnvIO(offspring_cpu_state);
    }
  );

  before_sym_do_birth_sig.AddAction(
    [this](
      emp::Ptr<sgp_sym_t> sym_baby_ptr,
      const emp::WorldPosition& parent_pos
    ) {
      AssignNewEnvIO(sym_baby_ptr->GetHardware().GetCPUState());
    }
  );

  after_sym_vert_transmission_sig.AddAction(
    [this](
      emp::Ptr<sgp_sym_t> sym_offspring_ptr,
      emp::Ptr<sgp_sym_t> sym_parent_ptr,
      emp::Ptr<sgp_host_t> host_offspring_ptr,
      emp::Ptr<sgp_host_t> host_parent_ptr,
      const emp::WorldPosition& host_parent_pos,
      bool success                        /* vertical transmission success */
    ) {
      if (!success) return;
      emp_assert(sym_offspring_ptr != nullptr);
      auto& sym_offspring_cpu_state = sym_offspring_ptr->GetHardware().GetCPUState();
      // auto& sym_parent_cpu_state = sym_parent_ptr->GetHardware().GetCPUState();
      AssignNewEnvIO(sym_offspring_cpu_state);
    }
  );

  // --- Setup task completion/output buffer checks ---
  // NOTE - discuss timing of this check. Currently happens after executing cpu
  //        fully for this update
  // NOTE - discuss whether we want ability to configure this differently for different
  //        kinds of organisms
  after_host_cpu_exec_sig.AddAction(
    [this](sgp_host_t& host) {
      ProcessHostOutputBuffer(host);
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