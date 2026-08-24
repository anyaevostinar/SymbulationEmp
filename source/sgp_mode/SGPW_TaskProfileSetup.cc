#ifndef SGP_TASK_PROFILE_SETUP_CC
#define SGP_TASK_PROFILE_SETUP_CC

#include "SGPWorld.h"


namespace sgpmode {

void SGPWorld::SetupTaskProfileMode() {
  // Setup what we use for host/symbiont task profiles
  // PARENT-ALL
  // PARENT-FIRST
  // SELF-ALL
  // SELF-FIRST
  // TODO - Create an enum!
  if (sgp_config.TASK_PROFILE_MODE() == "parent-all") {
    fun_get_host_task_profile = [](const sgp_host_t& host) -> const emp::BitVector& {
      return host.GetHardware().GetCPUState().GetParentTasksPerformed();
    };
    fun_get_sym_task_profile = [](const sgp_sym_t& sym) -> const emp::BitVector& {
      return sym.GetHardware().GetCPUState().GetParentTasksPerformed();
    };
  } else if (sgp_config.TASK_PROFILE_MODE() == "parent-first") {
    fun_get_host_task_profile = [](const sgp_host_t& host) -> const emp::BitVector& {
      return host.GetHardware().GetCPUState().GetParentFirstTaskPerformed();
    };
    fun_get_sym_task_profile = [](const sgp_sym_t& sym) -> const emp::BitVector& {
      return sym.GetHardware().GetCPUState().GetParentFirstTaskPerformed();
    };
  } else if (sgp_config.TASK_PROFILE_MODE() == "self-all") {
    fun_get_host_task_profile = [](const sgp_host_t& host) -> const emp::BitVector& {
      return host.GetHardware().GetCPUState().GetTasksPerformed();
    };
    fun_get_sym_task_profile = [](const sgp_sym_t& sym) -> const emp::BitVector& {
      return sym.GetHardware().GetCPUState().GetTasksPerformed();
    };
  } else if (sgp_config.TASK_PROFILE_MODE() == "self-first") {
    fun_get_host_task_profile = [](const sgp_host_t& host) -> const emp::BitVector& {
      return host.GetHardware().GetCPUState().GetFirstTaskPerformed();
    };
    fun_get_sym_task_profile = [](const sgp_sym_t& sym) -> const emp::BitVector& {
      return sym.GetHardware().GetCPUState().GetFirstTaskPerformed();
    };
  } else {
    std::cout << "Unrecognized TASK_PROFILE_MODE: " << sgp_config.TASK_PROFILE_MODE() << std::endl;
    std::cout << "Exiting." << std::endl;
    exit(-1);
  }
}

void SGPWorld::SetupTaskProfileCompatibilityMode() {
  // Setup function that determines task profile compatibility
  // Task profile is determined by TASK_PROFILE_MODE
  if (sgp_config.TASK_PROFILE_COMPATIBILITY_MODE() == "always") {
    // Task profiles are always compatible no matter their makeup.
    fun_task_profile_compatibility_check = [this](
      const emp::BitVector& a,
      const emp::BitVector& b
    ) -> bool {
      return true;
    };
  } else if (sgp_config.TASK_PROFILE_COMPATIBILITY_MODE() == "task-any-match") {
    // Task profiles are compatible if they have at least one shared task between them.
    fun_task_profile_compatibility_check = [this](
      const emp::BitVector& a,
      const emp::BitVector& b
    ) -> bool {
      return utils::AnyMatchingOnes(a, b);
    };
  } else if (sgp_config.TASK_PROFILE_COMPATIBILITY_MODE() == "task-perfect-match") {
    fun_task_profile_compatibility_check = [this](
      const emp::BitVector& a,
      const emp::BitVector& b
    ) -> bool {
      return a == b;
    };
  } else {
    std::cout << "Unrecognized TASK_PROFILE_COMPATIBILITY_MODE: " << sgp_config.TASK_PROFILE_COMPATIBILITY_MODE() << std::endl;
    std::cout << "Exiting." << std::endl;
    exit(-1);
  }

}

void SGPWorld::SetupHorizontalTransmissionCompatibilityMode() {
  // Setup function that determines horizontal transmission compatibility based on task profiles
  if (sgp_config.HORIZONTAL_TRANSMISSION_COMPATIBILITY_MODE() == "always") {
    fun_horizontal_transmission_compatibility_check = [](
      sgp_host_t& host,
      sgp_sym_t& sym
    ) -> bool { return true; };
  } else if (sgp_config.HORIZONTAL_TRANSMISSION_COMPATIBILITY_MODE() == "task-profile-compatible") {
    fun_horizontal_transmission_compatibility_check = [this](
      sgp_host_t& host,
      sgp_sym_t& sym
    ) -> bool {
      const auto& host_profile = fun_get_host_task_profile(host);
      const auto& sym_profile = fun_get_sym_task_profile(sym);
      return fun_task_profile_compatibility_check(host_profile, sym_profile);
    };
  } else if (sgp_config.HORIZONTAL_TRANSMISSION_COMPATIBILITY_MODE() == "task-profile-strictly-stronger-match") {
    fun_horizontal_transmission_compatibility_check = [this](
      sgp_host_t& host,
      sgp_sym_t& sym
    ) -> bool {
      const auto& sym_profile = fun_get_sym_task_profile(sym);
      return NoBetterOrEquallyMatchingSymbionts(host, sym_profile);
    };
  } else if (sgp_config.HORIZONTAL_TRANSMISSION_COMPATIBILITY_MODE() == "task-profile-stronger-or-equal-match") {
    fun_horizontal_transmission_compatibility_check = [this](
      sgp_host_t& host,
      sgp_sym_t& sym
    ) -> bool {
      const auto& sym_profile = fun_get_sym_task_profile(sym);
      return NoBetterMatchingSymbionts(host, sym_profile);
    };
  } else {
    std::cout << "Unrecognized HORIZONTAL_TRANSMISSION_COMPATIBILITY_MODE: " << sgp_config.HORIZONTAL_TRANSMISSION_COMPATIBILITY_MODE() << std::endl;
    std::cout << "Exiting." << std::endl;
    exit(-1);
  }
}

void SGPWorld::SetupFindHostForHorizontalTransmission() {
  // Setup function that gets host neighbor (used for symbiont)
  // TODO - add different configuration options for this?
  fun_find_host_for_horizontal_trans = [this](
    emp::Ptr<sgp_sym_t> sym_parent_ptr,
    const emp::WorldPosition& parent_pos
  ) -> std::optional<emp::WorldPosition> {
    for (size_t attempt_i = 0; attempt_i < sgp_config.FIND_NEIGHBOR_HOST_ATTEMPTS(); ++attempt_i) {
      const size_t host_world_id = parent_pos.GetPopID();
      emp::WorldPosition candidate_pos(GetRandomNeighborPos(host_world_id));
      if (candidate_pos.IsValid() && IsOccupied(candidate_pos) && candidate_pos.GetIndex() != host_world_id) {
        emp::Ptr<Organism> prospective_org_ptr = GetOrgPtr(candidate_pos.GetIndex());
        emp_assert(prospective_org_ptr->IsHost());
        emp::Ptr<sgp_host_t> prospective_host_ptr = static_cast<sgp_host_t*>(prospective_org_ptr.Raw());

        const bool compatible = fun_horizontal_transmission_compatibility_check(
          *prospective_host_ptr,
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
}
#endif