#include "Organism.h"
#include "default_mode/SymWorld.h"

size_t BaseHost::AddSymbiont(emp::Ptr<BaseSymbiont> _in) {
  if ((int)syms.size() < my_config->SYM_LIMIT() && SymAllowedIn()) {
    syms.push_back(_in);
    _in->SetHost(this);
    _in->UponInjection();
    return syms.size();
  } else {
    _in.Delete();
    return 0;
  }
}

bool BaseHost::SymAllowedIn() const {
  bool do_phage_exclusion = my_config->PHAGE_EXCLUDE();
  if (!do_phage_exclusion) {
    return true;
  } else {
    int num_syms = syms.size();
    // essentially imitaties a 1/ 2^n chance, with n = number of symbionts
    int enter_chance = random->GetUInt((int)pow(2.0, num_syms));
    if (enter_chance == 0) {
      return true;
    }
    return false;
  }
}

void BaseSymbiont::VerticalTransmission(emp::Ptr<Organism> host_baby) {
  if (my_world->WillTransmit()) {
    // Vertical transmission data nodes
    // Attempt vs success for vertical transmission is just whether it has
    // enough resources
    my_world->GetVerticalTransmissionAttemptCount().AddDatum(1);

    // If the world permits vertical transmission and the sym has enough
    // resources, transmit!
    if (GetPoints() >= my_config->SYM_VERT_TRANS_RES()) {
      emp::Ptr<BaseSymbiont> sym_baby = ReproduceSym();
      points -= my_config->SYM_VERT_TRANS_RES();
      host_baby->AddSymbiont(sym_baby);

      my_world->GetVerticalTransmissionSuccessCount().AddDatum(1);
    }
  }
}
