#ifndef SGP_W_INT_MECH_C
#define SGP_W_INT_MECH_C

#include "SGPWorld.h"


namespace sgpmode {

void SGPWorld::SetupHostSymInteractions() {

  // These are defined in SGPW_TaskProfileSetup
  SetupTaskProfileMode();
  SetupTaskProfileCompatibilityMode();
  SetupHorizontalTransmissionCompatibilityMode();

  SetupFindHostForHorizontalTransmission();

  // Configure stress
  if (sgp_config.ENABLE_STRESS()) {
    SetupStressInteractions();
  }
  // Configure health interactions
  if (sgp_config.ENABLE_HEALTH()) {
    SetupHealthInteractions();
  }

  // Configure nutrient interactions
  if (sgp_config.ENABLE_NUTRIENT()) {
    SetupNutrientInteractions();
  } // default is no nutrient interactions, which is set in SGPWorld constructor

  SetupHostTaskRewards();

}

void SGPWorld::SetupOrgTypeVariables(){

  // Convert cfg org type to lowercase
  std::string cfg_org_type(emp::to_lower(sgp_config.INTERACTION_MECHANISM()));
  // Get organism type (asserts validity)
  sgp_org_type = org_info::GetOrganismType(cfg_org_type);
  // Configure stress sym type
  std::string cfg_stress_sym_type(emp::to_lower(sgp_config.STRESS_TYPE()));
  // Get stress symbiont type (asserts validity)
  stress_sym_type = org_info::GetStressSymType(cfg_stress_sym_type);
  // Configure heatlh sym type
  std::string cfg_health_sym_type(emp::to_lower(sgp_config.HEALTH_TYPE()));
  health_sym_type = org_info::GetHealthSymType(cfg_health_sym_type);
  // Configure nutrient sym type
  std::string cfg_nutrient_sym_type(emp::to_lower(sgp_config.NUTRIENT_TYPE()));
  nutrient_sym_type = org_info::GetNutrientSymType(cfg_nutrient_sym_type);


}



/************************** Health ********************************* */
// TODO - use compatibility check to determine interaction
void SGPWorld::SetupHealthInteractions() {
  emp_assert(sgp_config.ENABLE_HEALTH());
  // std::cout << "Setting up health host-endosymbiont interactions" << std::endl;
  // NOTE - currently this does not necessarily make sense for multiple symbiotns
  //        (host gains/loses once and all syms gain/lose same amount; i.e., no splitting)
  // NOTE - currently set up as donate/steal interaction. There's no penalty/multiplier
  //         which we probably want?
  // Hosts lose/gain extra CPU cycles
  if (GetHealthSymType() == health_sym_mode_t::MUTUALIST) {
    // Mutualist endosymbionts may donate a proportion of their CPU cycles to their
    // host.
    before_endosym_host_process_sig.AddAction(
      [this](
        const emp::WorldPosition& sym_pos,
        sgp_sym_t& sym,
        sgp_host_t& host
      ) {
        auto& sym_state = sym.GetHardware().GetCPUState();
        // If symbiont is dead or doesn't have a host, skip.
        if (sym.GetDead()) { return; }
        // Will sym donate?
        bool interact = random_ptr->P(sgp_config.HEALTH_INTERACTION_CHANCE());

        const auto& host_task_profile = fun_get_host_task_profile(host);
        const auto& sym_task_profile = fun_get_sym_task_profile(sym);
        interact = interact && fun_task_profile_compatibility_check(host_task_profile, sym_task_profile);

        const double donate_prop = sgp_config.MUTUALIST_CYCLE_GAIN_PROP();
        emp_assert(donate_prop <= 1.0 && donate_prop >= 0.0);
        const double sym_cycles = (double)sym_state.GetCPUCyclesToExec();
        // How much will sym donate?

        size_t sym_donate = (size_t)(((double)interact) * (donate_prop * sym_cycles));
        if(host.GetCountofMatchingSymsToInteractWith() > 0){
          sym_donate = (size_t)(((double)interact) * (donate_prop * sym_cycles)/host.GetCountofMatchingSymsToInteractWith());

        }
        emp_assert(sym_donate >= 0);
        sym_state.LoseCPUCycles(emp::Min(sym_donate, sym_state.GetCPUCyclesToExec()));
        // Adjust host's cpu cycles
        host.GetHardware().GetCPUState().GainCPUCycles(
          sgp_config.MUTUALIST_CYCLE_DONATE_MULTIPLIER() * sym_donate
        );
      }
    );
  } else if (GetHealthSymType() == health_sym_mode_t::PARASITE) {
    // Symbionts are hardcoded as health parasites.
    // Parasitic health endosymbionts may steal a proportion of the host's CPU cycles

    // --
    // From Anya's current version:
    //  if interaction, parasite's cycles come from stealing on match;
    //  if no interaction, parasite's cycles on from world, no stealing

    before_endosym_host_process_sig.AddAction(
      [this](
        const emp::WorldPosition& sym_pos,
        sgp_sym_t& sym,
        sgp_host_t& host
      ) {
        auto& sym_state = sym.GetHardware().GetCPUState();
        // If symbiont is dead or doesn't have a host, skip.
        if (sym.GetDead()) { return; }
        auto& host_state = host.GetHardware().GetCPUState();
        // Will sym steal?
        bool interact = random_ptr->P(sgp_config.HEALTH_INTERACTION_CHANCE());
        const auto& host_task_profile = fun_get_host_task_profile(host);
        const auto& sym_task_profile = fun_get_sym_task_profile(sym);
        interact = interact && fun_task_profile_compatibility_check(host_task_profile, sym_task_profile);

        const double steal_prop = sgp_config.PARASITE_CYCLE_LOSS_PROP();
        emp_assert(steal_prop <= 1.0 && steal_prop >= 0.0);
        // How much?
        const double host_cycles = (double)host_state.GetCPUCyclesToExec();

        size_t sym_steal_start = (size_t)(((double)interact) * (steal_prop * host_cycles));
        if(host.GetCountofMatchingSymsToInteractWith() > 0){
          sym_steal_start = (size_t)(((double)interact) * (steal_prop * host_cycles)/host.GetCountofMatchingSymsToInteractWith());
          if(interact){
            host.SetCountofMatchingSymsToInteractWith(host.GetCountofMatchingSymsToInteractWith() - 1);
          }
        }
        const size_t sym_steal = sym_steal_start;
        // Set parasite CPU cycles
        // - Open question to how we want to do this
        sym_state.SetCPUCyclesToExec((size_t)(sgp_config.PARASITE_BASE_CYCLE_PROP() * sgp_config.CYCLES_PER_UPDATE()));
        // Adjust sym and host states
        sym_state.GainCPUCycles(sgp_config.PARASITE_CYCLE_STEAL_MULTIPLIER() * sym_steal);
        host_state.LoseCPUCycles(sym_steal);
      }
    );
  } else if (GetHealthSymType() == health_sym_mode_t::INTERACTION_VALUE_BASED) {
    // Symbiont interaction value used to determine whether symbiont is a mutualist
    //   or parasite. Interaction intensity scales according to interaction value.
    before_endosym_host_process_sig.AddAction(
      [this](
        const emp::WorldPosition& sym_pos,
        sgp_sym_t& sym,
        sgp_host_t& host
      ) {
        auto& sym_state = sym.GetHardware().GetCPUState();
        // If symbiont is dead or doesn't have a host, skip.
        if (sym.GetDead()) { return; }
        auto& host_state = host.GetHardware().GetCPUState();
        // Will host and symbiont interact?
        bool interact = random_ptr->P(sgp_config.HEALTH_INTERACTION_CHANCE());
        const auto& host_task_profile = fun_get_host_task_profile(host);
        const auto& sym_task_profile = fun_get_sym_task_profile(sym);
        interact = interact && fun_task_profile_compatibility_check(host_task_profile, sym_task_profile);
        const double sym_interaction_value = sym.GetIntVal();
        emp_assert(sym_interaction_value >= -1.0);
        emp_assert(sym_interaction_value <= 1.0 );
        if (interact && (sym_interaction_value < 0.0)) {
          // Parasitic interaction
          // Steal proportion bounded: [0:cycle_loss_prop]
          emp_assert(sgp_config.PARASITE_CYCLE_LOSS_PROP() <= 1.0);
          emp_assert(sgp_config.PARASITE_CYCLE_LOSS_PROP() >= 0.0);
          const double steal_prop = (-1 * sym_interaction_value) * sgp_config.PARASITE_CYCLE_LOSS_PROP();
          emp_assert(steal_prop <= 1.0 && steal_prop >= 0.0);
          // How much?
          const double host_cycles = (double)host_state.GetCPUCyclesToExec();
          const size_t sym_steal = (size_t)(steal_prop * host_cycles);
          // Set parasite CPU cycles
          // - Open question to how we want to do this
          sym_state.SetCPUCyclesToExec(
            (size_t)(sgp_config.PARASITE_BASE_CYCLE_PROP() * sgp_config.CYCLES_PER_UPDATE())
          );
          // Adjust sym and host states
          sym_state.GainCPUCycles(sgp_config.PARASITE_CYCLE_STEAL_MULTIPLIER() * sym_steal);
          host_state.LoseCPUCycles(sym_steal);
        } else if (interact && (sym_interaction_value > 0.0)) {
          // Mutualistic interaction
          // Donate bounded: [0:cycle_gain_prop]
          emp_assert(sgp_config.MUTUALIST_CYCLE_GAIN_PROP() <= 1.0);
          emp_assert(sgp_config.MUTUALIST_CYCLE_GAIN_PROP() >= 0.0);
          const double donate_prop = sym_interaction_value * sgp_config.MUTUALIST_CYCLE_GAIN_PROP();
          emp_assert(donate_prop <= 1.0 && donate_prop >= 0.0);
          const double sym_cycles = (double)sym_state.GetCPUCyclesToExec();
          // How much will sym donate?
          size_t sym_donate = (size_t)(donate_prop * sym_cycles);
          emp_assert(sym_donate >= 0);
          sym_state.LoseCPUCycles(emp::Min(sym_donate, sym_state.GetCPUCyclesToExec()));
          // Adjust host's cpu cycles
          host.GetHardware().GetCPUState().GainCPUCycles(
            sgp_config.MUTUALIST_CYCLE_DONATE_MULTIPLIER() * sym_donate
          );
        } // Otherwise, no/neutral interaction.
      }
    );
  } else if (GetHealthSymType() == health_sym_mode_t::NEUTRAL) {
    // Symbionts are hardcoded as health neutralists.
    // Not health interaction here?
  } else {
    std::cout << "Unimplemented health symbiont type (" << sgp_config.HEALTH_TYPE() << "). Exiting." << std::endl;
    exit(-1);
  }
  // TODO - add instruction-mediated interaction
}


/************************** Stress ********************************* */
void SGPWorld::SetupStressInteractions() {
  emp_assert(sgp_config.ENABLE_STRESS());
  // Setup extinction variable
  // At beginning of update, determine whether an extinction event occurs
  begin_update_sig.AddAction(
    [this]() {
      const size_t u = GetUpdate();
      // Note: not applying stress on the final update; when we record data for the final update, don't want stress
      //       skewing the numbers.
      stress_extinction_update = (u > 0) && ((u % sgp_config.STRESS_FREQUENCY()) == 0) && !(u >= (sgp_config.UPDATES() - 1));
    }
  );

  // Setup host interactions
  // NOTE - this can be simplified assuming no other desired differences in logic
  //        for parasite vs. mutualist (repeated code; only death chance is different)
  if (GetStressSymType() == stress_sym_mode_t::MUTUALIST) {
    // Use mutualist death chance
    before_host_cpu_exec_sig.AddAction(
      [this](sgp_host_t& host) {
        if (!stress_extinction_update) return;
        // If host has a mutualist symbiont with a matching task profile, death_chance = mutualist death chance
        // Otherwise, base death chance.
        const emp::BitVector& host_task_profile = fun_get_host_task_profile(host);
        bool interact = false;
        auto& endosymbionts = host.GetSymbionts();
        for (size_t sym_i = 0; sym_i < endosymbionts.size(); ++sym_i) {
            // Check if symbiont matches task profile
            emp::Ptr<sgp_sym_t> endosym_ptr = static_cast<sgp_sym_t*>(endosymbionts[sym_i].Raw());
            // interact = utils::AnyMatchingOnes(
            //   host_task_profile,
            //   fun_get_sym_task_profile(*endosym_ptr)
            // );
            interact = fun_task_profile_compatibility_check(host_task_profile, fun_get_sym_task_profile(*endosym_ptr));
            if (interact) {
              break;
            }
        }
        const double death_chance = (interact) ?
          sgp_config.MUTUALIST_DEATH_CHANCE() :
          sgp_config.BASE_DEATH_CHANCE();
        // Kill host with chosen probability
        if (random_ptr->P(death_chance)) {
          host.SetDead();
        }
      }
    );
  } else if (GetStressSymType() == stress_sym_mode_t::PARASITE) {
    if (sgp_config.PARASITE_ESCAPEE_TIMING() == "on-match") {
      // Parasites that match with their host get to produce escapees regardless
      // of whether host dies
      before_host_cpu_exec_sig.AddAction(
        [this](sgp_host_t& host) {
          if (!stress_extinction_update) return;
          // If host has a symbiont, death_chance = parasite death chance
          // Otherwise, base death chance.
          double death_chance = sgp_config.BASE_DEATH_CHANCE();
          auto& endosymbionts = host.GetSymbionts();
          const emp::BitVector& host_task_profile = fun_get_host_task_profile(host);
          for (size_t sym_i = 0; sym_i < endosymbionts.size(); ++sym_i) {
            // Check if symbiont matches task profile
            emp::Ptr<sgp_sym_t> endosym_ptr = static_cast<sgp_sym_t*>(endosymbionts[sym_i].Raw());
            const emp::BitVector& endosym_task_profile = fun_get_sym_task_profile(*endosym_ptr);
            const bool can_escape = fun_task_profile_compatibility_check(host_task_profile, fun_get_sym_task_profile(*endosym_ptr));
            if (can_escape) {
              death_chance = sgp_config.PARASITE_DEATH_CHANCE();
              // Endosymbiont gets opportunity to horizontally transmit
              // By using this queue, offspring of parasites avoid getting into hosts that will die to the
              // current stress event.
              for (size_t i = 0; i < sgp_config.PARASITE_NUM_OFFSPRING_ON_STRESS_INTERACTION(); ++i) {
                emp::Ptr<Organism> sym_offspring = endosym_ptr->Reproduce();
                symbiont_stress_escapees.emplace_back(
                  static_cast<sgp_sym_t*>(sym_offspring.Raw()),
                  endosym_task_profile,
                  endosym_ptr->GetHardware().GetCPUState().GetLocation().GetPopID()
                );
              }
              // Once we leave this signal, the host (and this symbiont) will
              // potentially be deleted.
              // So, we need to handle the reproduction here (versus putting it into the queue).
            }
          }
          // Kill host with chosen probability
          if (random_ptr->P(death_chance)) {
            host.SetDead();
          }
        }
      );
    } else if (sgp_config.PARASITE_ESCAPEE_TIMING() == "on-match-host-death") {
      // Parasites that match with their host get to produce escapees only when
      // their host dies.
      before_host_cpu_exec_sig.AddAction(
        [this](sgp_host_t& host) {
          if (!stress_extinction_update) return;
          // If host has a symbiont, death_chance = parasite death chance
          // Otherwise, base death chance.
          double death_chance = sgp_config.BASE_DEATH_CHANCE();
          auto& endosymbionts = host.GetSymbionts();
          const emp::BitVector& host_task_profile = fun_get_host_task_profile(host);
          emp::vector<size_t> escapee_ids;
          for (size_t sym_i = 0; sym_i < endosymbionts.size(); ++sym_i) {
            // Check if symbiont matches task profile
            emp::Ptr<sgp_sym_t> endosym_ptr = static_cast<sgp_sym_t*>(endosymbionts[sym_i].Raw());
            const bool can_escape = fun_task_profile_compatibility_check(host_task_profile, fun_get_sym_task_profile(*endosym_ptr));
            if (can_escape) {
              death_chance = sgp_config.PARASITE_DEATH_CHANCE();
              escapee_ids.emplace_back(sym_i);
            }
          }
          // Kill host with chosen probability + allow escapees.
          if (random_ptr->P(death_chance)) {
            // ------
            // Give any escapees a chance to escape!
            // Once we leave this signal, the host (and this symbiont) will
            // potentially be deleted.
            // So, we need to handle the reproduction here (versus putting it into the queue) .
            for (size_t escapee_id : escapee_ids) {
              emp::Ptr<sgp_sym_t> endosym_ptr = static_cast<sgp_sym_t*>(endosymbionts[escapee_id].Raw());
              const emp::BitVector& endosym_task_profile = fun_get_sym_task_profile(*endosym_ptr);
              for (size_t i = 0; i < sgp_config.PARASITE_NUM_OFFSPRING_ON_STRESS_INTERACTION(); ++i) {
                emp::Ptr<Organism> sym_offspring = endosym_ptr->Reproduce();
                symbiont_stress_escapees.emplace_back(
                  static_cast<sgp_sym_t*>(sym_offspring.Raw()),
                  endosym_task_profile,
                  endosym_ptr->GetHardware().GetCPUState().GetLocation().GetPopID()
                );
              }
            }
            // ------
            // Mark host as dead
            host.SetDead();
          }
        }
      );
    } else {
      std::cout << "Unknown PARASITE_ESCAPEE_TIMING option: " << sgp_config.PARASITE_ESCAPEE_TIMING() << std::endl;
      exit(-1);
    }
  } else if (GetStressSymType() == stress_sym_mode_t::INTERACTION_VALUE_BASED) {
    // This mode assumes: [mutualist death chance <= base death chance <= parasite death chance]
    // We use symbiont's interaciton value to scale death chance between mutualist:base or base:parasite
    emp_assert(sgp_config.BASE_DEATH_CHANCE() <= sgp_config.PARASITE_DEATH_CHANCE());
    emp_assert(sgp_config.BASE_DEATH_CHANCE() >= sgp_config.MUTUALIST_DEATH_CHANCE());
    // NOTE - this is implementing assuming 1 host / 1 parasite
    before_host_cpu_exec_sig.AddAction(
      [this](sgp_host_t& host) {
        if (!stress_extinction_update) return;
        const emp::BitVector& host_task_profile = fun_get_host_task_profile(host);
        emp::vector<size_t> escapee_ids; // Any parasite escapees?
        bool interact = false;
        double endosym_interaction_value = 0.0;
        auto& endosymbionts = host.GetSymbionts();
        for (size_t sym_i = 0; sym_i < endosymbionts.size(); ++sym_i) {
          // Check if symbiont matches task profile
          emp::Ptr<sgp_sym_t> endosym_ptr = static_cast<sgp_sym_t*>(endosymbionts[sym_i].Raw());
          interact = fun_task_profile_compatibility_check(
            host_task_profile,
            fun_get_sym_task_profile(*endosym_ptr)
          );
          if (interact) {
            endosym_interaction_value = endosym_ptr->GetIntVal();
            if (endosym_interaction_value < 0.0) {
              escapee_ids.emplace_back(sym_i);
            }
            break;
          }
        }
        // Calculate death chance based on endosymbiont interaction value
        const double base_death_chance = sgp_config.BASE_DEATH_CHANCE();
        const double mutualist_death_chance = sgp_config.MUTUALIST_DEATH_CHANCE();
        const double parasite_death_chance = sgp_config.PARASITE_DEATH_CHANCE();
        double death_chance = base_death_chance;
        emp_assert(endosym_interaction_value >= -1.0);
        emp_assert(endosym_interaction_value <= 1.0);
        if (interact && (endosym_interaction_value < 0.0)) {
          // Parasitic interaction
          // Lots of asserts to ensure death chance working as expected.
          emp_assert(base_death_chance <= parasite_death_chance);
          const double chance_range = parasite_death_chance - base_death_chance;
          // Death chance is already base death chance, adjust up by value scaled
          //   by interaction value.
          death_chance += (chance_range * (-1 * endosym_interaction_value));
          emp_assert(death_chance <= parasite_death_chance);
          emp_assert(death_chance >= base_death_chance);
        } else if (interact && (endosym_interaction_value > 0.0)) {
          // Mutualistic interaction
          emp_assert(base_death_chance >= mutualist_death_chance);
          const double chance_range = base_death_chance - mutualist_death_chance;
          death_chance -= (chance_range * endosym_interaction_value);
          emp_assert(death_chance <= base_death_chance);
          emp_assert(death_chance >= mutualist_death_chance);
        } // Otherwise, interaction value == 0.0, no interaction (neutral).

        // Kill host with chosen probability + allow any parasite escapees out
        if (random_ptr->P(death_chance)) {
          // ------
          // Give any escapees a chance to escape!
          // Once we leave this signal, the host (and this symbiont) will
          // potentially be deleted.
          // So, we need to handle the reproduction here (versus putting it into the queue) .
          for (size_t escapee_id : escapee_ids) {
            emp::Ptr<sgp_sym_t> endosym_ptr = static_cast<sgp_sym_t*>(endosymbionts[escapee_id].Raw());
            const emp::BitVector& endosym_task_profile = fun_get_sym_task_profile(*endosym_ptr);
            for (size_t i = 0; i < sgp_config.PARASITE_NUM_OFFSPRING_ON_STRESS_INTERACTION(); ++i) {
              emp::Ptr<Organism> sym_offspring = endosym_ptr->Reproduce();
              symbiont_stress_escapees.emplace_back(
                static_cast<sgp_sym_t*>(sym_offspring.Raw()),
                endosym_task_profile,
                endosym_ptr->GetHardware().GetCPUState().GetLocation().GetPopID()
              );
            }
          }
          host.SetDead();
        }
      }
    );
  } else if (GetStressSymType() == stress_sym_mode_t::NEUTRAL) {
    // Symbionts have no effect on hosts with respect to stress event.
    before_host_cpu_exec_sig.AddAction(
      [this](sgp_host_t& host) {
        if (!stress_extinction_update) return;
        // If host has a symbiont, death_chance = mutualist death chance
        // Otherwise, base death chance.
        const double death_chance = sgp_config.BASE_DEATH_CHANCE();
        // Kill host with chosen probability
        if (random_ptr->P(death_chance)) {
          host.SetDead();
        }
      }
    );
  } else {
    std::cout << "Unimplemented stress symbiont type (" << sgp_config.STRESS_TYPE() << "). Exiting." << std::endl;
    exit(-1);
  }

  // TODO - Add instruction-mediated stress interaction mode

  // NOTE - What about free-living symbionts (if any)?
  //        Or endosymbionts?
}



/************************** Nutrient ********************************* */
void SGPWorld::SetupNutrientInteractions() {
  emp_assert(sgp_config.ENABLE_NUTRIENT());
  // std::cout << "Setting up nutrient host-endosymbiont interactions." << std::endl;

  // NOTE - should nutrient interaction be based on host's tasks or host's parent tasks
  if (GetNutrientSymType() == nutrient_sym_mode_t::MUTUALIST) {
    // Nutrient mutualist - if mutualist task matches a host,
    //  mutualist donates some resources to the host.
    // functor calculates how many points the sym will give
    fun_calc_sym_nutrient_interaction = [this](
      sgp_host_t& host,
      sgp_sym_t& sym,
      double task_points,
      size_t task_id,
      size_t task_matching_sym_count

    ) -> double {
        //Assumes that symbiont has performed task_id

        // Task match, donate proportion of task points to host.
        // Can't donate more than task value or less than 0.0
       double to_donate = std::clamp(
          sgp_config.NUTRIENT_DONATE_PROP() * task_points,
          0.0,
          task_points
        );
        to_donate = std::min(to_donate, sym.GetPoints());

        // Returns amount of points the symbiont will lose from giving to the host
        return (-to_donate)/task_matching_sym_count;

    };
  } else if (GetNutrientSymType() == nutrient_sym_mode_t::PARASITE) {
    // Nutrient parasite - if parasite performs task that host also performs,
    //  parasite steals some proportion of earned points from host
    // functor calculates how many points the sym will gain
    fun_calc_sym_nutrient_interaction = [this](
      sgp_host_t& host,
      sgp_sym_t& sym,
      double task_points,
      size_t task_id,
      size_t task_matching_sym_count
    ) -> double {
        //Assumes that symbiont has performed task_id

        // Task match, steal proportion of earned task points from host.
        // Can't try to steal less than 0 or more than task was worth
        const double to_steal = std::clamp(
          sgp_config.NUTRIENT_STEAL_PROP() * task_points,
          0.0,
          task_points
        );

        // Returns amount of points this parasite will get from stealing from the host
        return (to_steal * sgp_config.NUTRIENT_INTERACTION_MULTIPLIER())/task_matching_sym_count;

    };
  } else if (GetNutrientSymType() == nutrient_sym_mode_t::INTERACTION_VALUE_BASED) {
    // Symbiont interaction value determines whether interaction is parasitic (negative)
    //   or mutualistic (positive). Interacition intensity is scaled by symbiont's
    //   interaction value.
    // functor calculates how many points the sym will gain/give
    fun_calc_sym_nutrient_interaction = [this](
      sgp_host_t& host,
      sgp_sym_t& sym,
      double task_points,
      size_t task_id,
      size_t task_matching_sym_count
    ) -> double {
      //Assumes that symbiont has performed task_id

      const double sym_interaction_value = sym.GetIntVal();
      emp_assert(-1.0 <= sym_interaction_value && sym_interaction_value <= 1.0);
      if (sym_interaction_value > 0.0) {
        // Task match from mutualist: mutualistic interaction
        //   Donate proportion of earned task points to host.
        // Can't donate more than task value or less than 0.0
        const double donate_prop = sgp_config.NUTRIENT_DONATE_PROP() * sym_interaction_value;
        emp_assert(donate_prop >= 0.0 && donate_prop <= sgp_config.NUTRIENT_DONATE_PROP());
       double to_donate = std::clamp(
          donate_prop * task_points,
          0.0,
          task_points
        );
        to_donate = std::min(to_donate, sym.GetPoints());

        // Returns amount of points the symbiont will lose from giving to the host
        return (-to_donate)/task_matching_sym_count;
      } else if (sym_interaction_value < 0.0) {
        // Task match from parasite: parasitic interaction
        const double steal_prop = sgp_config.NUTRIENT_STEAL_PROP() * (-1 * sym_interaction_value);
        emp_assert(steal_prop >= 0.0 && steal_prop <= sgp_config.NUTRIENT_STEAL_PROP());
        double to_steal = std::clamp(
          steal_prop * task_points,
          0.0,
          task_points
        );


        // Returns amount of points this parasite will get from stealing from the host
        return (to_steal * sgp_config.NUTRIENT_INTERACTION_MULTIPLIER())/task_matching_sym_count;
      }
      // Otherwise, no interaction. Return task points.
      return 0.0;
    };
  } else if (GetNutrientSymType() == nutrient_sym_mode_t::NEUTRAL) {
    // Keep default behavior
  } else {
    std::cout << "Unimplemented nutrient symbiont type (" << sgp_config.NUTRIENT_TYPE() << "). Exiting." << std::endl;
    exit(-1);
  }

  if (GetNutrientSymType() == nutrient_sym_mode_t::MUTUALIST) {
    // Nutrient mutualist - if mutualist task matches a host,
    // mutualist donates some resources to the host.
    // functor calculates how many points the host will get
    fun_calc_host_nutrient_interaction = [this](
      sgp_host_t& host,
      sgp_sym_t& sym,
      double task_points,
      size_t task_id,
      size_t task_matching_sym_count
    ) -> double {
      //Assumes that symbiont has performed task_id

      // Task match, donate proportion of earned task points to host.
        // Can't donate more than task value or less than 0.0
        double to_donate = std::clamp(
          sgp_config.NUTRIENT_DONATE_PROP() * task_points,
          0.0,
          task_points
        );
        to_donate = std::min(to_donate, sym.GetPoints());

        // Return amount of additional points host will receieve from this symbiont
        return (to_donate * sgp_config.NUTRIENT_INTERACTION_MULTIPLIER())/task_matching_sym_count;

    };
  } else if (GetNutrientSymType() == nutrient_sym_mode_t::PARASITE) {
    // Nutrient parasite - if parasite performs task that host also performs,
    //  parasite steals some proportion of earned points from host
    // functor calculates how many points the host will lose
    fun_calc_host_nutrient_interaction = [this](
      sgp_host_t& host,
      sgp_sym_t& sym,
      double task_points,
      size_t task_id,
      size_t task_matching_sym_count

    ) -> double {

        // Task match, steal proportion of earned task points from host.
        // Can't try to steal less than 0 or more than task was worth
        const double to_steal = std::clamp(
          sgp_config.NUTRIENT_STEAL_PROP() * task_points,
          0.0,
          task_points
        );

        // Return amount of points host will lose from this symbiont
        return (-to_steal)/task_matching_sym_count;

    };
  } else if (GetNutrientSymType() == nutrient_sym_mode_t::INTERACTION_VALUE_BASED) {
    // Symbiont interaction value determines whether interaction is parasitic (negative)
    //   or mutualistic (positive). Interacition intensity is scaled by symbiont's
    //   interaction value.
    // functor calculates how many points the host will gain/lose
    fun_calc_host_nutrient_interaction = [this](
      sgp_host_t& host,
      sgp_sym_t& sym,
      double task_points,
      size_t task_id,
      size_t task_matching_sym_count
    ) -> double {
      //Assumes that symbiont has performed task_id

      const double sym_interaction_value = sym.GetIntVal();
      emp_assert(-1.0 <= sym_interaction_value && sym_interaction_value <= 1.0);
      if (sym_interaction_value > 0.0) {
        // Task match from mutualist: mutualistic interaction
        //   Donate proportion of earned task points to host.
        // Can't donate more than task value or less than 0.0
        const double donate_prop = sgp_config.NUTRIENT_DONATE_PROP() * sym_interaction_value;
        emp_assert(donate_prop >= 0.0 && donate_prop <= sgp_config.NUTRIENT_DONATE_PROP());
        double to_donate = std::clamp(
          donate_prop * task_points,
          0.0,
          task_points
        );
        to_donate = std::min(to_donate, sym.GetPoints());

        // Return amount of additional points host will receieve from this symbiont
         return (to_donate * sgp_config.NUTRIENT_INTERACTION_MULTIPLIER())/task_matching_sym_count;
      } else if (sym_interaction_value < 0.0) {
        // Task match from parasite: parasitic interaction
        const double steal_prop = sgp_config.NUTRIENT_STEAL_PROP() * (-1 * sym_interaction_value);
        emp_assert(steal_prop >= 0.0 && steal_prop <= sgp_config.NUTRIENT_STEAL_PROP());
        const double to_steal = std::clamp(
          steal_prop * task_points,
          0.0,
          task_points
        );


        // Return amount of points host will lose from this symbiont
        return (-to_steal)/task_matching_sym_count;
      }
      // Otherwise, no interaction. Return task points.
      return 0;
    };
  } else if (GetNutrientSymType() == nutrient_sym_mode_t::NEUTRAL) {
    // Keep default behavior
  } else {
    std::cout << "Unimplemented nutrient symbiont type (" << sgp_config.NUTRIENT_TYPE() << "). Exiting." << std::endl;
    exit(-1);
  }
}

/*
 * Input: None
 * Outpt: None
 * Purpose: Sets up functor to decide what happens when a host tries to receive point reward for completing a task
*/
void SGPWorld::SetupHostTaskRewards() {

  if(sgp_config.ENABLE_NUTRIENT() == false){
    fun_apply_host_points = [this](
      sgp_host_t& host,
      double task_value_before,
      size_t task_id
    ) {
        host.AddPoints(task_value_before);
    };
  }
  else {
    fun_apply_host_points = [this](
      sgp_host_t& host,
      double task_value_before,
      size_t task_id
    ) {
        int task_matching_sym_count = 0;
        emp::vector<emp::Ptr<Organism>>& syms = host.GetSymbionts();
        for (size_t endosym_i = 0; endosym_i < syms.size(); ++endosym_i) {

          emp::Ptr<sgp_sym_t> cur_symbiont = static_cast<sgp_sym_t*>(syms[endosym_i].Raw());
          bool dead = cur_symbiont->GetDead();
          // Skip if dead
          if (dead) {
              continue;
          }

          const emp::BitVector& endosym_task_profile = fun_get_sym_task_profile(*cur_symbiont);
          bool sym_performed = endosym_task_profile.Get(task_id);
          task_matching_sym_count += sym_performed;

        }

        double point_difference_from_syms = 0;
        if(task_matching_sym_count > 0){
          for (size_t endosym_i = 0; endosym_i < syms.size(); ++endosym_i) {

            emp::Ptr<sgp_sym_t> cur_symbiont = static_cast<sgp_sym_t*>(syms[endosym_i].Raw());
            bool dead = cur_symbiont->GetDead();
            // Skip if dead
            if (dead) {
                continue;
            }

           const emp::BitVector& endosym_task_profile = fun_get_sym_task_profile(*cur_symbiont);
           bool sym_performed = endosym_task_profile.Get(task_id);
           if(sym_performed){
              double sym_task_point = CalcSymNutrientInteraction(host,*cur_symbiont, task_value_before, task_id,task_matching_sym_count);
              point_difference_from_syms += CalcHostNutrientInteraction(host, *cur_symbiont, task_value_before, task_id,task_matching_sym_count);
              cur_symbiont->AddPoints(sym_task_point);

            }

        }
        }
        host.AddPoints(task_value_before+point_difference_from_syms);
    };

  }
}





}

#endif