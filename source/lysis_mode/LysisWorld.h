#ifndef LYSIS_WORLD_H
#define LYSIS_WORLD_H

#include "../../../Empirical/include/emp/Evolve/World.hpp"
#include "../../../Empirical/include/emp/data/DataFile.hpp"
#include "../../../Empirical/include/emp/math/random_utils.hpp"
#include "../../../Empirical/include/emp/math/Random.hpp"
#include "../Organism.h"
#include <set>
#include <math.h>
#include "../SymWorld.h"

class LysisWorld : public SymWorld {
public:
  using SymWorld::SymWorld;

  /**
   * Input: The address of the string representing the file to be
   * created's name
   *
   * Output: The address of the DataFile that has been created.
   *
   * Purpose: To set up the file that will be used to track mean
   * lysis chance, the number of symbionts, and the histogram of
   * the mean lysis chance.
   */
    emp::DataFile & SetupLysisChanceFile(const std::string & filename) {
    auto & file = SetupFile(filename);
    auto & node1 = GetSymCountDataNode();
    auto & node = GetLysisChanceDataNode();
    auto & node2 = GetBurstSizeDataNode();
    auto & node3 = GetBurstCountDataNode();
    file.AddVar(update, "update", "Update");
    file.AddTotal(node1, "count", "Total number of symbionts");
    file.AddMean(node2, "mean_burstsize", "Average burst size", true);
    file.AddTotal(node3, "burst_count", "Average burst count", true);
    node.SetupBins(0.0, 1.1, 10); //Necessary because range exclusive
    file.AddMean(node, "mean_lysischance", "Average chance of lysis");
    file.AddHistBin(node, 0, "Hist_0.0", "Count for histogram bin 0.0 to <0.1");
    file.AddHistBin(node, 1, "Hist_0.1", "Count for histogram bin 0.1 to <0.2");
    file.AddHistBin(node, 2, "Hist_0.2", "Count for histogram bin 0.2 to <0.3");
    file.AddHistBin(node, 3, "Hist_0.3", "Count for histogram bin 0.3 to <0.4");
    file.AddHistBin(node, 4, "Hist_0.4", "Count for histogram bin 0.4 to <0.5");
    file.AddHistBin(node, 5, "Hist_0.5", "Count for histogram bin 0.5 to <0.6");
    file.AddHistBin(node, 6, "Hist_0.6", "Count for histogram bin 0.6 to <0.7");
    file.AddHistBin(node, 7, "Hist_0.7", "Count for histogram bin 0.7 to <0.8");
    file.AddHistBin(node, 8, "Hist_0.8", "Count for histogram bin 0.8 to <0.9");
    file.AddHistBin(node, 9, "Hist_0.9", "Count for histogram bin 0.9 to 1.0");

    file.PrintHeaderKeys();

    return file;
  }

  /**
    * Input: The address of the string representing the file to be
    * created's name
    *
    * Output: The address of the DataFile that has been created.
    *
    * Purpose: To set up the file that will be used to track mean
    * induction chance, the number of symbionts, and the histogram of
    * the mean induction chance.
    */
     emp::DataFile & SetupInductionChanceFile(const std::string & filename) {
     auto & file = SetupFile(filename);
     auto & node1 = GetSymCountDataNode();
     auto & node = GetInductionChanceDataNode();
     node.SetupBins(0.0, 1.1, 10); //Necessary because range exclusive
     file.AddVar(update, "update", "Update");
     file.AddMean(node, "mean_inductionchance", "Average chance of induction");
     file.AddTotal(node1, "count", "Total number of symbionts");
     file.AddHistBin(node, 0, "Hist_0.0", "Count for histogram bin 0.0 to <0.1");
     file.AddHistBin(node, 1, "Hist_0.1", "Count for histogram bin 0.1 to <0.2");
     file.AddHistBin(node, 2, "Hist_0.2", "Count for histogram bin 0.2 to <0.3");
     file.AddHistBin(node, 3, "Hist_0.3", "Count for histogram bin 0.3 to <0.4");
     file.AddHistBin(node, 4, "Hist_0.4", "Count for histogram bin 0.4 to <0.5");
     file.AddHistBin(node, 5, "Hist_0.5", "Count for histogram bin 0.5 to <0.6");
     file.AddHistBin(node, 6, "Hist_0.6", "Count for histogram bin 0.6 to <0.7");
     file.AddHistBin(node, 7, "Hist_0.7", "Count for histogram bin 0.7 to <0.8");
     file.AddHistBin(node, 8, "Hist_0.8", "Count for histogram bin 0.8 to <0.9");
     file.AddHistBin(node, 9, "Hist_0.9", "Count for histogram bin 0.9 to 1.0");

     file.PrintHeaderKeys();

     return file;
   }

   /**
    * Input: The address of the string representing the file to be
    * created's name
    *
    * Output: The address of the DataFile that has been created.
    *
    * Purpose: To set up the file that will be used to track the difference between
    * bacterium and phage incorporation values and the histogram of the difference between
    * the incorporation vals.
    */
     emp::DataFile & SetupIncorporationDifferenceFile(const std::string & filename) {
     auto & file = SetupFile(filename);
     auto & node = GetIncorporationDifferenceDataNode();
     node.SetupBins(0.0, 1.1, 10); //Necessary because range exclusive
     file.AddVar(update, "update", "Update");
     file.AddMean(node, "mean_incval_difference", "Average difference in incorporation value between bacteria and their phage");
     file.AddHistBin(node, 0, "Hist_0.0", "Count for histogram bin 0.0 to <0.1");
     file.AddHistBin(node, 1, "Hist_0.1", "Count for histogram bin 0.1 to <0.2");
     file.AddHistBin(node, 2, "Hist_0.2", "Count for histogram bin 0.2 to <0.3");
     file.AddHistBin(node, 3, "Hist_0.3", "Count for histogram bin 0.3 to <0.4");
     file.AddHistBin(node, 4, "Hist_0.4", "Count for histogram bin 0.4 to <0.5");
     file.AddHistBin(node, 5, "Hist_0.5", "Count for histogram bin 0.5 to <0.6");
     file.AddHistBin(node, 6, "Hist_0.6", "Count for histogram bin 0.6 to <0.7");
     file.AddHistBin(node, 7, "Hist_0.7", "Count for histogram bin 0.7 to <0.8");
     file.AddHistBin(node, 8, "Hist_0.8", "Count for histogram bin 0.8 to <0.9");
     file.AddHistBin(node, 9, "Hist_0.9", "Count for histogram bin 0.9 to 1.0");

     file.PrintHeaderKeys();

     return file;
   }


  /**
   * Input: None
   *
   * Output: The DataMonitor<double, emp::data::Histogram>& that has the information representing
   * the chance of lysis for each symbiont.
   *
   * Purpose: To collect data on the chance of lysis for each symbiont to be saved to the
   * data file that is tracking the chance of lysis for each symbiont.
   */
  emp::DataMonitor<double,emp::data::Histogram>& GetLysisChanceDataNode() {
    if (!data_node_lysischance) {
      data_node_lysischance.New();
      OnUpdate([this](size_t){
        data_node_lysischance->Reset();
        for (size_t i = 0; i< pop.size(); i++) {
          if (IsOccupied(i)) {
            emp::vector<emp::Ptr<Organism>>& syms = pop[i]->GetSymbionts();
            long unsigned int sym_size = syms.size();
            for(size_t j=0; j< sym_size; j++){
              data_node_lysischance->AddDatum(syms[j]->GetLysisChance());
            }//close for
          }//close if
          if (sym_pop[i]){
            data_node_lysischance->AddDatum(sym_pop[i]->GetLysisChance());
          }
        }//close for
      });
    }
    return *data_node_lysischance;
  }

  /**
   * Input: None
   *
   * Output: The DataMonitor<double>& that has the information representing
   * the lysis burst size.
   *
   * Purpose: To collect data on the lysis burst size to be saved to the
   * data file that is tracking lysis burst size.
   */
  emp::DataMonitor<double>& GetBurstSizeDataNode() {
    if (!data_node_burst_size) {
      data_node_burst_size.New();
    }
    return *data_node_burst_size;
  }


  /**
   * Input: None
   *
   * Output: The DataMonitor<int>& that has the information representing
   * the lysis burst count.
   *
   * Purpose: To collect data on the lysis burst count to be saved to the
   * data file that is tracking lysis burst count.
   */
  emp::DataMonitor<int>& GetBurstCountDataNode() {
    if (!data_node_burst_count) {
      data_node_burst_count.New();
    }
    return *data_node_burst_count;
  }

  /**
   * Input: None
   *
   * Output: The DataMonitor<double, emp::data::Histogram>& that has the information representing
   * the chance of induction for each symbionts.
   *
   * Purpose: To collect data on the chance of induction for each symbiont to be saved to the
   * data file that is tracking chance of induction for each symbiont.
   */
  emp::DataMonitor<double,emp::data::Histogram>& GetInductionChanceDataNode() {
    if (!data_node_inductionchance) {
      data_node_inductionchance.New();
      OnUpdate([this](size_t){
        data_node_inductionchance->Reset();
        for (size_t i = 0; i< pop.size(); i++) {
          if (IsOccupied(i)) {
            emp::vector<emp::Ptr<Organism>>& syms = pop[i]->GetSymbionts();
            long unsigned int sym_size = syms.size();
            for(size_t j=0; j< sym_size; j++){
              data_node_inductionchance->AddDatum(syms[j]->GetInductionChance());
            }//close for
          }//close if
          if (sym_pop[i]){
            data_node_inductionchance->AddDatum(sym_pop[i]->GetInductionChance());
          }
        }//close for
      });
    }
    return *data_node_inductionchance;
  }

  /**
   * Input: None
   *
   * Output: The DataMonitor<double, emp::data::Histogram>& that has the information representing
   * the difference between incorporation vals for bacteriums and their phage
   *
   * Purpose: To collect data on the difference between incorporation vals for each bacteria and their phage
   * to be saved to the data file that is tracking incorporation val differences.
   */
  emp::DataMonitor<double,emp::data::Histogram>& GetIncorporationDifferenceDataNode() {
    if (!data_node_incorporation_difference) {
      data_node_incorporation_difference.New();
      OnUpdate([this](size_t){
        data_node_incorporation_difference->Reset();
        for (size_t i = 0; i< pop.size(); i++) {
          if (IsOccupied(i)) {
            double host_inc_val = pop[i]->GetIncVal();

            emp::vector<emp::Ptr<Organism>>& syms = pop[i]->GetSymbionts();
            long unsigned int sym_size = syms.size();
            for(size_t j=0; j< sym_size; j++){
              double inc_val_difference = abs(host_inc_val - syms[j]->GetIncVal());
              data_node_incorporation_difference->AddDatum(inc_val_difference);
            }
          }//close if
        }//close for
      });
    }
    return *data_node_incorporation_difference;
  }
  
}; //end of LysisWorld class
#endif
