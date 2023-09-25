#ifndef LYSIS_WORLD_H
#define LYSIS_WORLD_H

#include "../default_mode/SymWorld.h"
#include "../default_mode/DataNodes.h"
#include "LysisConfigSetup.h"

class LysisWorld : public SymWorld {
private:

  /**
    *
    * Purpose: Holds all configuration settings and points to same configuration 
    * object as my_config from superclass, but with the correct subtype.
    *
  */
  emp::Ptr<SymConfigLysis> lysis_config = NULL;

  /**
    *
    * Purpose: Data nodes tracking lysis chance, induction chance, incorporation difference, lytic burst size, and lytic burst count.
    *
  */
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_lysischance;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_inductionchance;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_incorporation_difference;
  emp::Ptr<emp::DataMonitor<double>> data_node_burst_size;
  emp::Ptr<emp::DataMonitor<int>> data_node_burst_count;
  emp::Ptr<emp::DataMonitor<int>> data_node_cfu;

public:
  /**
   * Input: a reference to a random number generator and a pointer to the configuration object for this experiment.
   *
   * Purpose: The constructor for LysisWorld
   */
  LysisWorld(emp::Random& _random, emp::Ptr<SymConfigLysis> _config) : SymWorld(_random, _config) {
    lysis_config = _config;
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To destruct the data nodes belonging to LysisWorld to conserve memory.
   */
  ~LysisWorld(){
    if (data_node_lysischance) data_node_lysischance.Delete();
    if (data_node_inductionchance) data_node_inductionchance.Delete();
    if (data_node_incorporation_difference) data_node_incorporation_difference.Delete();
    if (data_node_burst_size) data_node_burst_size.Delete();
    if (data_node_burst_count) data_node_burst_count.Delete();
    if (data_node_cfu) data_node_cfu.Delete();
  }


  /**
   * Definitions of setup functions, expanded in LysisWorldSetup.cc
   */
  void SetupHosts(long unsigned int* POP_SIZE);
  void SetupSymbionts(long unsigned int* total_syms);


  /**
  * Input: None.
  *
  * Output: None.
  *
  * Purpose: To create and set up the data files (excluding for phylogeny) that contain data for the experiment.
  */
  void CreateDataFiles(){
    std::string file_ending = "_SEED"+std::to_string(lysis_config->SEED())+".data";
    SymWorld::CreateDataFiles();
    SetupLysisChanceFile(lysis_config->FILE_PATH()+"LysisChance"+lysis_config->FILE_NAME()+file_ending).SetTimingRepeat(lysis_config->DATA_INT());
    SetupInductionChanceFile(lysis_config->FILE_PATH()+"InductionChance"+lysis_config->FILE_NAME()+file_ending).SetTimingRepeat(lysis_config->DATA_INT());
    SetupIncorporationDifferenceFile(lysis_config->FILE_PATH()+"IncValDifferences"+lysis_config->FILE_NAME()+file_ending).SetTimingRepeat(lysis_config->DATA_INT());
  }

  /**
   * Input: The Empirical DataFile object tracking data nodes.
   *
   * Output: None.
   *
   * Purpose: To add bacterium data nodes to be tracked to the bacterium data file.
   */
  void SetupHostFileColumns(emp::DataFile & file){
    SymWorld::SetupHostFileColumns(file);
    auto & cfu_node = GetCFUDataNode();
    file.AddTotal(cfu_node, "cfu_count", "Total number of colony forming units"); //colony forming units are hosts that
  }

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
    data_node_lysischance->SetupBins(0, 1.1, 11);
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
   * the chance of induction for each symbiont.
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
    data_node_inductionchance->SetupBins(0, 1.1, 11);
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
    data_node_incorporation_difference->SetupBins(0, 1.1, 11);
    return *data_node_incorporation_difference;
  }

  /**
   * Input: None
   *
   * Output: The DataMonitor<int>& that has the information representing
   * the number of colony forming units.
   *
   * Purpose: To collect data on the CFU count to be saved to the
   * data file that is tracking CFU
   */
  emp::DataMonitor<int>& GetCFUDataNode() {
    //keep track of host organisms that are uninfected or infected with only lysogenic phage
    if(!data_node_cfu) {
      data_node_cfu.New();
      OnUpdate([this](size_t){
        data_node_cfu -> Reset();

        for (size_t i = 0; i < pop.size(); i++) {
          if(IsOccupied(i)) {
            //uninfected hosts
            if((pop[i]->GetSymbionts()).empty()) {
              data_node_cfu->AddDatum(1);
            }

            //infected hosts, check if all symbionts are lysogenic
            if(pop[i]->HasSym()) {
              emp::vector<emp::Ptr<Organism>>& syms = pop[i]->GetSymbionts();
              bool all_lysogenic = true;
              for(long unsigned int j = 0; j < syms.size(); j++){
                if(syms[j]->IsPhage() && syms[j]->GetLysogeny() == false){
                  all_lysogenic = false;
                }
              }
              if(all_lysogenic){
                data_node_cfu->AddDatum(1);
              }
            }
          } //endif
        } //end for
      }); //end OnUpdate
    } //end if
    return *data_node_cfu;
  }


}; //end of LysisWorld class
#endif
