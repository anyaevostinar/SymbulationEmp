#include "Symbiont.h"
#include "Host.h"

double Symbiont::ProcessResources(double host_donation,
                                  emp::Ptr<Organism> host) {
  if (host == nullptr) {
    host = my_host;
  }
  double sym_int_val = GetIntVal();
  double sym_portion = 0;
  double host_portion = 0;
  double synergy = my_config->SYNERGY();

  if (sym_int_val < 0) {
    double stolen = 0;
    if (auto host2 = host.DynamicCast<Host>()) {
      stolen = host2->StealResources(sym_int_val);
    }
    host_portion = 0;
    sym_portion = stolen + host_donation;
  } else if (sym_int_val >= 0) {
    host_portion = host_donation * sym_int_val;
    sym_portion = host_donation - host_portion;
  }
  AddPoints(sym_portion);
  return host_portion * synergy;
}
