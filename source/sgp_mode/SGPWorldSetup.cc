#ifndef SGP_WORLD_SETUP_C
#define SGP_WORLD_SETUP_C

#include "SGPWorld.h"
#include "org_type_info.h"
#include "utils.h"
#include "sgpl/utility/ThreadLocalRandom.hpp"

#include "emp/datastructs/map_utils.hpp"
#include "emp/tools/string_utils.hpp"

namespace sgpmode {
// TODO - implement "empty initialization" option
//        - Particularly useful for testing
void SGPWorld::Setup() {
  // Reset the seed of the main sgp thread based on the config
  // TODO - should this be here? (used to be inside scheduler)
  sgpl::tlrand.Get().ResetSeed(sgp_config.SEED());

  // TODO - move these into appropriate setup functions
  before_host_process_sig.Clear();
  after_host_process_sig.Clear();
  after_host_cpu_step_sig.Clear();
  before_freeliving_sym_process_sig.Clear();
  after_freeliving_sym_process_sig.Clear();
  after_freeliving_sym_cpu_step_sig.Clear();
  before_endosym_process_sig.Clear();
  after_endosym_process_sig.Clear();
  after_endosym_cpu_step_sig.Clear();


  // Configure SGP organism type
  SetupOrgMode();

  // NOTE - Some of this code is repeated from base class.
  //  - Could do some reorganization to copy-paste. E.g., make functions for this,
  //     add hooks into the base setup to give more downstream flexibility.
  double start_moi = sgp_config.START_MOI();

  // Setup population structure
  SetupPopStructure();

  // Setup scheduler
  SetupScheduler();

  // Setup host reproduction
  repro_queue.Clear(); // Clear reproduction queue
  SetupHostReproduction();
  SetupSymReproduction();
  // Setup any host-symbiont interactions
  SetupHostSymInteractions();

  // -- bookmark --

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
}

void SGPWorld::SetupPopStructure() {
  long unsigned int POP_SIZE;
  // TODO - add pop mode?
  if (sgp_config.POP_SIZE() == -1) {
    POP_SIZE = sgp_config.GRID_X() * sgp_config.GRID_Y();
  } else {
    POP_SIZE = sgp_config.POP_SIZE();
  }
  max_world_size = (size_t)POP_SIZE;

  // set world structure (either mixed or a grid with some dimensions)
  // and set synchronous generations to false
  if (!sgp_config.GRID()) {
    SetPopStruct_Mixed(false);
  } else {
    SetPopStruct_Grid(sgp_config.GRID_X(), sgp_config.GRID_Y(), false);
  }
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
  scheduler.SetupScheduler(max_world_size, sgp_config.THREAD_COUNT());
  // Scheduler calls world's ProcessOrgAt function
}

// Configure HostDoBirth signals
void SGPWorld::SetupHostReproduction() {
  // Reset host birth signals
  before_host_do_birth_sig.Clear();
  after_host_do_birth_sig.Clear();
  // TODO - anything else to configure here?
}

// Configure symbiont reproduction signals
void SGPWorld::SetupSymReproduction() {
  // Reset sym do birth signals
  after_sym_do_birth_sig.Clear();

  // Reset sym vertical transmission signals
  before_sym_vert_transmission_sig.Clear();
  after_sym_vert_transmission_sig.Clear();

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
void SGPWorld::SetupHosts(long unsigned int* POP_SIZE) {/*TODO*/}
// TODO - clear symbiont process signals
void SGPWorld::SetupSymbionts(long unsigned int* total_syms) {/*TODO*/}


}

#endif