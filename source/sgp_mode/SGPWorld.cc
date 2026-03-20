#ifndef SGP_WORLD_C
#define SGP_WORLD_C

#include "SGPWorld.h"
//#include "SGPHost.h"
//#include "SGPSymbiont.h"
#include "utils.h"

namespace sgpmode {

// TODO - Make clear that this will process host and free-living symbiont
//        ProcessOrgsAt?
void SGPWorld::ProcessOrgsAt(size_t pop_id) {
  // std::cout << "ProcessOrgsAt " << pop_id << "; " << IsOccupied(pop_id) << std::endl;
  // Process host at this location (if any)
  if (IsOccupied(pop_id)) {
    auto& org = GetOrg(pop_id);;
    emp_assert(org.IsHost());
    ProcessHostAt(
      {pop_id},
      static_cast<sgp_host_t&>(org)
    );
  }
  // TODO - double check that my interpretation is correct here
  // TODO - can we condiitonally tack this onto processing only
  //        when free-living syms are turned on
  // Process free-living symbiont at this location (if any)
//   if (IsSymPopOccupied(pop_id)) {
//     emp_assert(!GetSymAt(pop_id)->IsHost());
//     ProcessFreeLivingSymAt(
//       emp::WorldPosition(0, pop_id),
//       static_cast<sgp_sym_t&>(*(GetSymAt(pop_id)))
//     );
//   }
}

// TODO - discuss timing
// NOTE - DoDeath repeated several times here. Maybe move that check out to ProcessOrgsAt?
void SGPWorld::ProcessHostAt(const emp::WorldPosition& pos, sgp_host_t& host) {
  // std::cout << "ProcessHostAt" << std::endl;
  // If host is dead, don't process.
  if (host.GetDead()) {
    DoDeath(pos);
    return;
  }
  // Update host location
  host.GetHardware().GetCPUState().SetLocation(pos); // TODO - is this necessary here?
  // NOTE - Will symbionts be able to modify host's cycles during *their* executation?
  //        How do we want to handle that? (modify host's execution on next update?)
  // NOTE - Will need to update/revist this if we have instruction-mediated interactions

  // Hosts gain baseline number of CPU cycles
  host.GetHardware().GetCPUState().GainCPUCycles(
    sgp_config.CYCLES_PER_UPDATE()
  );

  // NOTE - Discuss timing of endosym pre-process signal and host preprocess signal
  //        Currently endosyms go first and then hosts. This is to model endosyms
  //        having opportunity to steal / donate cpu cycles and then host responding
  //        to endosym behavior (but could argue it should be the other way around).
  // Give endosymbionts their baseline CPU cycles
  // Trigger signal to all endosymbionts that host is about to process
  //   Gives endosymbionts chance to interact with host before it processes.
  //   E.g., symbiont could steal / donate cpu cycles, resources, etc.
//   emp::vector<emp::Ptr<Organism>>& syms = host.GetSymbionts();
//   for (size_t endosym_i = 0; endosym_i < syms.size(); ++endosym_i) {
//     emp_assert(!(syms[endosym_i]->IsHost()));
//     emp::Ptr<sgp_sym_t> cur_symbiont = static_cast<sgp_sym_t*>(syms[endosym_i].Raw());
//     const bool dead = cur_symbiont->GetDead();
//     // Skip if dead
//     if (dead) {
//       continue;
//     }
//     // Endosymbiont gains baseline number of CPU cycles
//     cur_symbiont->GetHardware().GetCPUState().GainCPUCycles(
//       sgp_config.CYCLES_PER_UPDATE()
//     );
//     before_endosym_host_process_sig.Trigger(
//       {endosym_i + 1, host.GetLocation().GetIndex()},
//       *cur_symbiont,
//       host
//     );
//   }

  before_host_process_sig.Trigger(host);

  // Host may have died as a result of this signal.
  // NOTE - Do we want to return early here + do death?
  //        Anywhere else we want to check for death?
  if (host.GetDead()) {
    DoDeath(pos);
    return;
  }

  // NOTE - Do we want to drain cpu cycles here (i.e., get cashed in for execution?)
  const size_t cycles_to_exec = host.GetHardware().GetCPUState().ExtractCPUCycles();
  // host.GetHardware().GetCPUState().LoseCPUCycles(cycles_to_execute);
  // Execute organism hardware according to cycles_to_exec
  // NOTE - Discuss possibility of host dying because of instruction executions.
  //        As-is, still run hardware forward full amount regardless
  for (size_t i = 0; i < cycles_to_exec; ++i) {
    // TODO - do we need to update org location every update? (this was being done in RunCPUStep every cpu step)
    // Execute 1 CPU cycle
    host.GetHardware().RunCPUStep(1);

    // Did host attempt to reproduce?
    // NOTE - could move into a signal response
    // NOTE - want to handle this after every clock cycle?
    if (host.GetHardware().GetCPUState().ReproAttempt()) {
      // upside to handling this here: we have direct access to organism
      HostAttemptRepro(pos, host);
    }

    after_host_cpu_step_sig.Trigger(host);
    // NOTE - Check death here?
  }
  after_host_cpu_exec_sig.Trigger(host);
  // Handle any endosymbionts (configurable at setup-time)
  // NOTE - is there any reason that this might need to be a functor?
  //ProcessEndosymbionts(host);
  // Endosymbionts might kill host.
  if (host.GetDead()) {
    DoDeath(pos);
    return;
  }
  // Run host's process function (post cpu steps, post endosymbiont processing)
  // TODO - when do we actually want to run this?
  host.Process(pos);
  after_host_process_sig.Trigger(host);
  // If process resulted in death, DoDeath.
  if (host.GetDead()) {
    DoDeath(pos);
  }
}

}
#endif