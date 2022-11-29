#ifndef LYSIS_DATA_NODES_C
#define LYSIS_DATA_NODES_C

#include "LysisWorld.h"
#include "Bacterium.h"
#include "Phage.h"

emp::DataMonitor<double, emp::data::Histogram> &
LysisWorld::GetInductionChanceDataNode() {
  if (!data_node_inductionchance) {
    data_node_inductionchance.New();
    OnUpdate([this](size_t) {
      data_node_inductionchance->Reset();
      for (size_t i = 0; i < pop.size(); i++) {
        if (IsOccupied(i)) {
          emp::vector<emp::Ptr<BaseSymbiont>> syms = pop[i]->GetSymbionts();
          long unsigned int sym_size = syms.size();
          for (size_t j = 0; j < sym_size; j++) {
            if (emp::Ptr<Phage> sym = syms[j].DynamicCast<Phage>()) {
              data_node_inductionchance->AddDatum(sym->GetInductionChance());
            }
          } // close for
        }   // close if
        if (sym_pop[i]) {
          if (emp::Ptr<Phage> sym = sym_pop[i].DynamicCast<Phage>()) {
            data_node_inductionchance->AddDatum(sym->GetInductionChance());
          }
        }
      } // close for
    });
  }
  data_node_inductionchance->SetupBins(0, 1.1, 11);
  return *data_node_inductionchance;
}
emp::DataMonitor<double, emp::data::Histogram> &
LysisWorld::GetIncorporationDifferenceDataNode() {
  if (!data_node_incorporation_difference) {
    data_node_incorporation_difference.New();
    OnUpdate([this](size_t) {
      data_node_incorporation_difference->Reset();
      for (size_t i = 0; i < pop.size(); i++) {
        if (IsOccupied(i)) {
          if (emp::Ptr<Bacterium> host = pop[i].DynamicCast<Bacterium>()) {
            double host_inc_val = host->GetIncVal();

            emp::vector<emp::Ptr<BaseSymbiont>> syms = host->GetSymbionts();
            long unsigned int sym_size = syms.size();
            for (size_t j = 0; j < sym_size; j++) {
              if (emp::Ptr<Phage> sym = syms[j].DynamicCast<Phage>()) {
                double inc_val_difference =
                    abs(host_inc_val - sym->GetIncVal());
                data_node_incorporation_difference->AddDatum(
                    inc_val_difference);
              }
            }
          }
        } // close if
      }   // close for
    });
  }
  data_node_incorporation_difference->SetupBins(0, 1.1, 11);
  return *data_node_incorporation_difference;
}
emp::DataMonitor<int> &LysisWorld::GetCFUDataNode() {
  // keep track of host organisms that are uninfected or infected with only
  // lysogenic phage
  if (!data_node_cfu) {
    data_node_cfu.New();
    OnUpdate([this](size_t) {
      data_node_cfu->Reset();

      for (size_t i = 0; i < pop.size(); i++) {
        if (IsOccupied(i)) {
          // uninfected hosts
          if ((pop[i]->GetSymbionts()).empty()) {
            data_node_cfu->AddDatum(1);
          }

          // infected hosts, check if all symbionts are lysogenic
          if (pop[i]->HasSym()) {
            emp::vector<emp::Ptr<BaseSymbiont>> syms = pop[i]->GetSymbionts();
            bool all_lysogenic = true;
            for (long unsigned int j = 0; j < syms.size(); j++) {
              if (emp::Ptr<Phage> sym = syms[j].DynamicCast<Phage>()) {
                if (sym->IsPhage() && sym->GetLysogeny() == false) {
                  all_lysogenic = false;
                }
              }
            }
            if (all_lysogenic) {
              data_node_cfu->AddDatum(1);
            }
          }
        } // endif
      }   // end for
    });   // end OnUpdate
  }       // end if
  return *data_node_cfu;
}

emp::DataMonitor<double, emp::data::Histogram> &
LysisWorld::GetLysisChanceDataNode() {
  if (!data_node_lysischance) {
    data_node_lysischance.New();
    OnUpdate([this](size_t) {
      data_node_lysischance->Reset();
      for (size_t i = 0; i < pop.size(); i++) {
        if (IsOccupied(i)) {
          emp::vector<emp::Ptr<BaseSymbiont>> syms = pop[i]->GetSymbionts();
          long unsigned int sym_size = syms.size();
          for (size_t j = 0; j < sym_size; j++) {
            if (emp::Ptr<Phage> sym = syms[j].DynamicCast<Phage>()) {
              data_node_lysischance->AddDatum(sym->GetLysisChance());
            }
          } // close for
        }   // close if
        if (sym_pop[i]) {
          if (emp::Ptr<Phage> sym = sym_pop[i].DynamicCast<Phage>()) {
            data_node_lysischance->AddDatum(sym->GetLysisChance());
          }
        }
      } // close for
    });
  }
  data_node_lysischance->SetupBins(0, 1.1, 11);
  return *data_node_lysischance;
}

#endif