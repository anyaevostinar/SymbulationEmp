#include "Instructions.h"
#include "SGPOrganism.h"

template <typename F> void inst::TryEctoLink(CPUState &state, F &&closure) {
  if (emp::Ptr<BaseSymbiont> sym = state.organism.DynamicCast<BaseSymbiont>()) {
    // exit if org is a host OR
    // if it's a free living sym AND (ecto is off OR there is no parallel host)
    bool ecto_conditions = !state.world->GetConfig()->ECTOSYMBIOSIS() ||
                           !state.world->IsOccupied(state.location.GetPopID());
    if (sym->GetHost() == nullptr && ecto_conditions) {
      return;
    }

    // get the sym's host
    emp::Ptr<Organism> host = sym->GetHost();

    // ecto link
    emp::Ptr<emp::BitSet<64>> orig_used_resources;
    emp::Ptr<emp::vector<size_t>> orig_shared_available_dependencies;
    emp::Ptr<emp::vector<uint32_t>> orig_internal_environment;
    bool linked = false;
    int pop_index = state.location.GetPopID();
    if (host == nullptr) {
      // set the working host to be the parallel host
      host = state.world->GetPop()[pop_index];
      if (host->HasSym() && state.world->GetConfig()->ECTOSYMBIOTIC_IMMUNITY())
        return;

      // store old sym env variables
      orig_used_resources = state.used_resources;
      orig_shared_available_dependencies = state.shared_available_dependencies;
      orig_internal_environment = state.internalEnvironment;

      // set environment variables to that of the host's

      state.used_resources =
          host.DynamicCast<SGPOrganism>()->GetCPU().state.used_resources;
      state.shared_available_dependencies =
          host.DynamicCast<SGPOrganism>()
              ->GetCPU()
              .state.shared_available_dependencies;
      state.internalEnvironment =
          host.DynamicCast<SGPOrganism>()->GetCPU().state.internalEnvironment;
      linked = true;
    }

    if (host) {
      closure(host);
      // Donate 20% of the total points of the symbiont-host system
      // This way, a sym can donate e.g. 40 or 60 percent of their points in a
      // couple of instructions
      double to_donate =
          fmin(sym->GetPoints(), (sym->GetPoints() + host->GetPoints()) * 0.20);
      if (sym->GetHost() != nullptr) {
        state.world->GetHostedSymDonatedDataNode().WithMonitor(
            [=](auto &m) { m.AddDatum(to_donate); });
      } else {
        state.world->GetFreeSymDonatedDataNode().WithMonitor(
            [=](auto &m) { m.AddDatum(to_donate); });
      }

      host->AddPoints(to_donate *
                      (1.0 - state.world->GetConfig()->DONATE_PENALTY()));
      sym->AddPoints(-to_donate);
    }

    // ecto unlink
    if (linked) {
      // state.org->Host is still nullptr; it's just the variable "host" that
      // was relinked to the parallel host
      state.used_resources = orig_used_resources;
      state.shared_available_dependencies = orig_shared_available_dependencies;
      state.internalEnvironment = orig_internal_environment;
    }
  }
}