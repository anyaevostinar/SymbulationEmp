#ifndef PGG_WORLD_H
#define PGG_WORLD_H

#include "../default_mode/SymWorld.h"

class PggWorld : public SymWorld {
private:
  /**
    *
    * Purpose: Data node tracking the pgg donation rate.
    *
  */
  emp::Ptr<emp::DataMonitor<double,emp::data::Histogram>> data_node_Pgg;
public:
  using SymWorld::SymWorld;

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To destruct the data nodes belonging to PggWorld to conserve memory.
   */
  ~PggWorld(){
      if (data_node_Pgg) data_node_Pgg.Delete();
  }

  /**
   * Input: The address of the string representing the file to be
   * created's name
   *
   * Output: The address of the DataFile that has been created.
   *
   * Purpose: To set up the file that will be used to track
   * information about the PGG symbiont's interaction values
   */
  emp::DataFile & SetupPGGSymIntValFile(const std::string & filename) {
    auto & file = SetupFile(filename);
    auto & node1 = GetSymCountDataNode();
    auto & node2 = GetCountFreeSymsDataNode();
    auto & node3 = GetCountHostedSymsDataNode();
    auto & node4 = GetPGGDataNode();
    node4.SetupBins(0, 1.1, 11); //Necessary because range exclusive
    file.AddVar(update, "update", "Update");
    file.AddTotal(node1, "count", "Total number of symbionts");
    file.AddTotal(node2, "free_syms", "Total number of free syms");
    file.AddTotal(node3, "hosted_syms", "Total number of syms in a host");
    file.AddMean(node4, "Pgg_donationrate","Average donation rate");
    file.AddHistBin(node4, 0, "Hist_0", "Count for histogram bin -1 to <-0.9");
    file.AddHistBin(node4, 1, "Hist_0.1", "Count for histogram bin 0.0 to <0.1");
    file.AddHistBin(node4, 2, "Hist_0.2", "Count for histogram bin 0.1 to <0.2");
    file.AddHistBin(node4, 3, "Hist_0.3", "Count for histogram bin 0.2 to <0.3");
    file.AddHistBin(node4, 4, "Hist_0.4", "Count for histogram bin 0.3 to <0.4");
    file.AddHistBin(node4, 5, "Hist_0.5", "Count for histogram bin 0.4 to <0.5");
    file.AddHistBin(node4, 6, "Hist_0.6", "Count for histogram bin 0.5 to <0.6");
    file.AddHistBin(node4, 7, "Hist_0.7", "Count for histogram bin 0.6 to <0.7");
    file.AddHistBin(node4, 8, "Hist_0.8", "Count for histogram bin 0.7 to <0.8");
    file.AddHistBin(node4, 9, "Hist_0.9", "Count for histogram bin 0.8 to <0.9");
    file.AddHistBin(node4, 10, "Hist_1.0", "Count for histogram bin 0.9 to 1.0");


    file.PrintHeaderKeys();

    return file;
  }

  /**
   * Input: The double representing the number of resources each host gets in each update.
   *
   * Output: None
   *
   * Purpose: To set the resources that each host gets per update.
   */
  emp::DataMonitor<double, emp::data::Histogram>& GetPGGDataNode() {
    if (!data_node_Pgg) {
      data_node_Pgg.New();
      OnUpdate([this](size_t){
        data_node_Pgg->Reset();
        for (size_t i = 0; i< pop.size(); i++) {
          if (IsOccupied(i)) { //track hosted syms
            emp::vector<emp::Ptr<Organism>>& syms = pop[i]->GetSymbionts();
            size_t sym_size = syms.size();
            for(size_t j=0; j< sym_size; j++){
              data_node_Pgg->AddDatum(syms[j]->GetDonation());
            }//close for
          }//close if
          if(sym_pop[i]){ //track free-living syms
            data_node_Pgg->AddDatum(sym_pop[i]->GetDonation());
          }//close if
        }//close for
      });
    }
    return *data_node_Pgg;
  }

}; //end of PggWorld class
#endif
