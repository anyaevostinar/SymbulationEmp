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
  // std::cout << "Opcode rectifier mappings:";
  // for (size_t i = 0; i < opcode_rectifier.mapper.size(); ++i) {
  //   std::cout << " " << (uint32_t)(opcode_rectifier.mapper[i]);
  // }
  // std::cout << std::endl;
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

  // CureHost signal
  if (sgp_config.CURE()) {
    begin_update_sig.AddAction(
      [this]() {
        if(GetUpdate() == sgp_config.CURE_UPDATES()) {
          CureHosts();
        }
      }
    );
  }

  SetupHosts(&POP_SIZE);
  Resize(max_world_size); // TODO - move this back to setup pop structure after fixing setup hosts
  // NOTE - any way to clean this up a little? Or, add some explanatory comments.
  long unsigned int total_syms = POP_SIZE * start_moi;
  SetupSymbionts(&total_syms);

  CreateDataFiles();
  SnapshotConfig();
  setup = true;
  // TODO - Delete this once confident in instruction removal
  // std::cout << "Opcode rectifier mappings (post setup):";
  // for (size_t i = 0; i < opcode_rectifier.mapper.size(); ++i) {
  //   std::cout << " " << (uint32_t)(opcode_rectifier.mapper[i]);
  // }
  // std::cout << std::endl;
}

void SGPWorld::SetupOrgMode() {
  // Convert cfg org type to lowercase
  std::string cfg_org_type(emp::to_lower(sgp_config.INTERACTION_MECHANISM())); // AEV TODO: Change all these other references to org type to interaction mechanism
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

}
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
        const size_t sym_steal = (size_t)(((double)interact) * (steal_prop * host_cycles));
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
    before_host_process_sig.AddAction(
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
      before_host_process_sig.AddAction(
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
      before_host_process_sig.AddAction(
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
            const emp::BitVector& endosym_task_profile = fun_get_sym_task_profile(*endosym_ptr);
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
    before_host_process_sig.AddAction(
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
    before_host_process_sig.AddAction(
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

void SGPWorld::SetupNutrientInteractions() {
  emp_assert(sgp_config.ENABLE_NUTRIENT());
  // std::cout << "Setting up nutrient host-endosymbiont interactions." << std::endl;

  // NOTE - should nutrient interaction be based on host's tasks or host's parent tasks
  if (GetNutrientSymType() == nutrient_sym_mode_t::MUTUALIST) {
    // Nutrient mutualist - if mutualist task matches a host,
    //  mutualist donates some resources to the host.
    fun_apply_nutrient_interaction = [this](
      sgp_sym_t& sym,
      double task_points,
      size_t task_id
    ) -> double {
      auto& sym_state = sym.GetHardware().GetCPUState();
      // If symbiont has no host, no interaction to modify points.
      if (!sym_state.HasHost()) { return task_points; }
      // Symbiont must have a host, so they interact.
      auto& host = *static_cast<sgp_host_t*>(sym.GetHost().Raw());

      // auto& host_state = host.GetHardware().GetCPUState();
      const emp::BitVector& host_task_profile = fun_get_host_task_profile(host);
      // const bool host_performed = host_state.GetParentTaskPerformed(task_id);
      bool host_performed = host_task_profile.Get(task_id);

      if (!host_performed) {
        // Task mismatch, no interaction between host and mutualist.
        return task_points;
      } else {
        // Task match, donate proportion of earned task points to host.
        // Can't donate more than task value or less than 0.0
        const double to_donate = std::clamp(
          sgp_config.NUTRIENT_DONATE_PROP() * task_points,
          0.0,
          task_points
        );
        // Donate points to host
        host.AddPoints(to_donate * sgp_config.NUTRIENT_INTERACTION_MULTIPLIER());
        // Return task points minus donated value for symbiont to earn
        return task_points - to_donate;
      }
    };
  } else if (GetNutrientSymType() == nutrient_sym_mode_t::PARASITE) {
    // Nutrient parasite - if parasite performs task that host also performs,
    //  parasite steals some proportion of earned points from host
    fun_apply_nutrient_interaction = [this](
      sgp_sym_t& sym,
      double task_points,
      size_t task_id
    ) -> double {
      auto& sym_state = sym.GetHardware().GetCPUState();
      // If symbiont has no host, no interaction to modify points.
      if (!sym_state.HasHost()) { return task_points; }
      // Symbiont must have a host, so they interact.
      auto& host = *static_cast<sgp_host_t*>(sym.GetHost().Raw());
      const emp::BitVector& host_task_profile = fun_get_host_task_profile(host);
      bool host_performed = host_task_profile.Get(task_id);
      if (!host_performed) {
        // Task mismatch, no interaction between host and parasite.
        // NOTE - do we want earning full amount or not?
        //  - Probably not? Otherwise, no incentive for parasitism?
        // return 0.0;
        return sgp_config.PARASITE_BASE_TASK_VALUE_PROP() * task_points;
      } else {
        // Task match, steal proportion of earned task points from host.
        // Can't try to steal less than 0 or more than task was worth
        const double to_steal = std::clamp(
          sgp_config.NUTRIENT_STEAL_PROP() * task_points,
          0.0,
          task_points
        );
        // Can't take more than host has
        const double from_host = emp::Min(
          host.GetPoints(),
          to_steal
        );
        host.DecPoints(from_host);
        // NOTE - subtract to_steal or from_host?
        // const double from_world = task_points - to_steal;
        // Take points from host
        return task_points * sgp_config.NUTRIENT_INTERACTION_MULTIPLIER();
      }
    };
  } else if (GetNutrientSymType() == nutrient_sym_mode_t::INTERACTION_VALUE_BASED) {
    // Symbiont interaction value determines whether interaction is parasitic (negative)
    //   or mutualistic (positive). Interacition intensity is scaled by symbiont's
    //   interaction value.
    fun_apply_nutrient_interaction = [this](
      sgp_sym_t& sym,
      double task_points,
      size_t task_id
    ) -> double {
      auto& sym_state = sym.GetHardware().GetCPUState();
      // If symbiont has no host, no interaction to modify points.
      if (!sym_state.HasHost()) { return task_points; }
      // Symbiont must have a host, so they interact.
      auto& host = *static_cast<sgp_host_t*>(sym.GetHost().Raw());
      const emp::BitVector& host_task_profile = fun_get_host_task_profile(host);
      bool host_performed = host_task_profile.Get(task_id);
      const double sym_interaction_value = sym.GetIntVal();
      emp_assert(-1.0 <= sym_interaction_value && sym_interaction_value <= 1.0);
      if (!host_performed && (sym_interaction_value > 0.0)) {
        // Task mismatch from mutualist
        return task_points;
      } else if (!host_performed && (sym_interaction_value < 0.0)) {
        // Task mismatch from parasite
        return sgp_config.PARASITE_BASE_TASK_VALUE_PROP() * task_points;
      } else if (host_performed && (sym_interaction_value > 0.0)) {
        // Task match from mutualist: mutualistic interaction
        //   Donate proportion of earned task points to host.
        // Can't donate more than task value or less than 0.0
        const double donate_prop = sgp_config.NUTRIENT_DONATE_PROP() * sym_interaction_value;
        emp_assert(donate_prop >= 0.0 && donate_prop <= sgp_config.NUTRIENT_DONATE_PROP());
        const double to_donate = std::clamp(
          donate_prop * task_points,
          0.0,
          task_points
        );
        // Donate points to host
        host.AddPoints(to_donate * sgp_config.NUTRIENT_INTERACTION_MULTIPLIER());
        // Return task points minus donated value for symbiont to earn
        return task_points - to_donate;
      } else if (host_performed && (sym_interaction_value < 0.0)) {
        // Task match from parasite: parasitic interaction
        const double steal_prop = sgp_config.NUTRIENT_STEAL_PROP() * (-1 * sym_interaction_value);
        emp_assert(steal_prop >= 0.0 && steal_prop <= sgp_config.NUTRIENT_STEAL_PROP());
        const double to_steal = std::clamp(
          steal_prop * task_points,
          0.0,
          task_points
        );
        // Can't take more than host has
        const double from_host = emp::Min(
          host.GetPoints(),
          to_steal
        );
        host.DecPoints(from_host);
        // NOTE - subtract to_steal or from_host?
        // const double from_world = task_points - to_steal;
        // Take points from host
        return task_points * sgp_config.NUTRIENT_INTERACTION_MULTIPLIER();
      }
      // Otherwise, no interaction. Return task points.
      return task_points;
    };
  } else if (GetNutrientSymType() == nutrient_sym_mode_t::NEUTRAL) {
    // Keep default behavior
  } else {
    std::cout << "Unimplemented nutrient symbiont type (" << sgp_config.NUTRIENT_TYPE() << "). Exiting." << std::endl;
    exit(-1);
  }
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

  // Configure sym do birth function
  // Don't need to simultaneously support free-living *and* horizontal transmission
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
      // Check if host profile and sym profile have any overlap?
      auto& host_profile = fun_get_host_task_profile(host_parent);
      auto& sym_profile = fun_get_sym_task_profile(sym);
      return utils::AnyMatchingOnes(host_profile, sym_profile);
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
  // std::cout << "Setup Host-symbiont interactions" << std::endl;

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

  if (sgp_config.HORIZONTAL_TRANSMISSION_COMPATIBILITY_MODE() == "always") {
    fun_host_sym_horizontal_trans_compatibility_check = [](
      sgp_host_t& host,
      sgp_sym_t& sym
    ) -> bool { return true; };
    fun_host_sym_stress_trans_compatibility_check = [](
      sgp_host_t& host,
      const emp::BitVector& profile
    ) -> bool { return true; };
  } else if (sgp_config.HORIZONTAL_TRANSMISSION_COMPATIBILITY_MODE() == "task-profile-compatible") {
    fun_host_sym_horizontal_trans_compatibility_check = [this](
      sgp_host_t& host,
      sgp_sym_t& sym
    ) -> bool {
      const auto& host_profile = fun_get_host_task_profile(host);
      const auto& sym_profile = fun_get_sym_task_profile(sym);
      return fun_task_profile_compatibility_check(host_profile, sym_profile);
    };
    fun_host_sym_stress_trans_compatibility_check = [this](
      sgp_host_t& host,
      const emp::BitVector& profile
    ) -> bool {
      const auto& host_profile = fun_get_host_task_profile(host);
      return fun_task_profile_compatibility_check(host_profile, profile);
    };
  } else if (sgp_config.HORIZONTAL_TRANSMISSION_COMPATIBILITY_MODE() == "task-profile-strictly-stronger-match") {
    fun_host_sym_horizontal_trans_compatibility_check = [this](
      sgp_host_t& host,
      sgp_sym_t& sym
    ) -> bool {
      const emp::BitVector& incoming_sym_task_profile = fun_get_sym_task_profile(sym);
      return NoBetterOrEquallyMatchingSymbionts(host, incoming_sym_task_profile);
    };
    fun_host_sym_stress_trans_compatibility_check = [this](
      sgp_host_t& host,
      const emp::BitVector& profile
    ) -> bool {
      return NoBetterOrEquallyMatchingSymbionts(host, profile);
    };
  } else if (sgp_config.HORIZONTAL_TRANSMISSION_COMPATIBILITY_MODE() == "task-profile-stronger-or-equal-match") {
    fun_host_sym_horizontal_trans_compatibility_check = [this](
      sgp_host_t& host,
      sgp_sym_t& sym
    ) -> bool {
      const emp::BitVector& incoming_sym_task_profile = fun_get_sym_task_profile(sym);
      return NoBetterMatchingSymbionts(host, incoming_sym_task_profile);
    };
    fun_host_sym_stress_trans_compatibility_check = [this](
      sgp_host_t& host,
      const emp::BitVector& profile
    ) -> bool {
      return  NoBetterMatchingSymbionts(host, profile);
    };
  } else {
    std::cout << "Unrecognized HORIZONTAL_TRANSMISSION_COMPATIBILITY_MODE: " << sgp_config.HORIZONTAL_TRANSMISSION_COMPATIBILITY_MODE() << std::endl;
    std::cout << "Exiting." << std::endl;
    exit(-1);
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
        const bool compatible = fun_host_sym_horizontal_trans_compatibility_check(
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

  // Configure stress
  if (sgp_config.ENABLE_STRESS()) {
    SetupStressInteractions();
  }
  // Configure health interactions
  if (sgp_config.ENABLE_HEALTH()) {
    SetupHealthInteractions();
  }

  // Configure nutrient interactions
  // Configure default (no) nutrient interaction (IMPORTANT!)
  // - Nutrient interaction setup will override this behavior if enabled.
  fun_apply_nutrient_interaction = [](
    sgp_sym_t& sym,
    double task_points,
    size_t task_id
  ) {
    return task_points;
  };
  if (sgp_config.ENABLE_NUTRIENT()) {
    SetupNutrientInteractions();
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

  // Configure oganism input buffers / environment id
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
      auto& offspring_cpu_state = host_offspring.GetHardware().GetCPUState();
      // auto& parent_cpu_state = host_parent.GetHardware().GetCPUState();
      //AssignNewEnvIO(offspring_cpu_state); // This is in OnPlacement now, so should be fine
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
      const emp::WorldPosition& host_parent_pos,
      bool success                        /* vertical transmission success */
    ) {
      if (!success) return;
      emp_assert(sym_offspring_ptr != nullptr);
      auto& sym_offspring_cpu_state = sym_offspring_ptr->GetHardware().GetCPUState();
      // auto& sym_parent_cpu_state = sym_parent_ptr->GetHardware().GetCPUState();
      //AssignNewEnvIO(sym_offspring_cpu_state); // This is in AddSymbiont now, so should be fine
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