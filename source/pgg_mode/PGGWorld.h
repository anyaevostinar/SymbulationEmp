#ifndef PGG_WORLD_H
#define PGG_WORLD_H

#include "../default_mode/SymWorld.h"
#include "../default_mode/DataNodes.h"

class PGGWorld : public SymWorld {
private:
  /**
    *
    * Purpose: Data node tracking the pgg donation rate.
    *
  */
  emp::Ptr<emp::DataMonitor<double,emp::data::Histogram>> data_node_PGG;
public:
  using SymWorld::SymWorld;

  PGGWorld(emp::Random & _random, emp::Ptr<SymConfigBase> _config) : 
    SymWorld(_random, _config) {

      if (my_config->PHYLOGENY()) {
        emp_assert(my_config->NUM_PHYLO_BINS() <= 100 &&
          "PGG taxon calculation assumes you're using <= phylo bins");
        // Set calc info function for symbiont taxa to include donation
        // rate (as specified by 100's and 1000's places) and interaction
        // value (as specified by 1's and 10's places)  
        calc_sym_info_fun = [&](Organism & org){
            size_t num_phylo_bins = my_config->NUM_PHYLO_BINS();
            //classify orgs into bins base on interaction values,
            //inclusive of lower bound, exclusive of upper
            float size_of_bin = 2.0 / num_phylo_bins;
            double int_val = org.GetIntVal();
            float prog = (int_val + 1);
            prog = (prog/size_of_bin) + (0.0000000000001);
            size_t int_bin = (size_t) prog;
            if (int_bin >= num_phylo_bins) int_bin = num_phylo_bins - 1;

            double don_val = org.GetDonation();
            size_of_bin = 1.0 / num_phylo_bins;
            prog = (don_val);
            prog = (prog/size_of_bin) + (0.0000000000001);
            size_t don_bin = (size_t) prog;
            if (don_bin >= num_phylo_bins) don_bin = num_phylo_bins - 1;

            return int_bin + don_bin*100;
          };
        sym_sys->SetCalcInfoFun(calc_sym_info_fun);
      }
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To destruct the data nodes belonging to PGGWorld to conserve memory.
   */
  ~PGGWorld(){
      if (data_node_PGG) data_node_PGG.Delete();
  }

  /**
  * Input: None.
  *
  * Output: None.
  *
  * Purpose: To create and set up the data files (excluding for phylogeny) that contain data for the experiment.
  */
  void CreateDataFiles(){
    std::string file_ending = "_SEED"+std::to_string(my_config->SEED())+".data";
    SymWorld::CreateDataFiles();
    SetupPGGSymIntValFile(my_config->FILE_PATH()+"PGGSymVals"+my_config->FILE_NAME()+file_ending).SetTimingRepeat(my_config->DATA_INT());
  }


   /**
    * Input: The address of the string representing the file to be
    * created's name
    *
    * Output: The address of the DataFile that has been created.
    *
    * Purpose: To set up the file that will be used to track mean
    * information about the PGG symbionts in the world.
    * This includes: (1) their total count, (2) the counts
    * of the free and hosted symbionts, (3) the average donation values
    * for all symbionts, and (4) the histogram data for the distribution of
    * symbionts at various donation values.
    */
  emp::DataFile & SetupPGGSymIntValFile(const std::string & filename) {
    auto & file = SetupFile(filename);
    auto & node1 = GetSymCountDataNode();
    auto & node2 = GetCountFreeSymsDataNode();
    auto & node3 = GetCountHostedSymsDataNode();
    auto & node4 = GetPGGDataNode();

    file.AddVar(update, "update", "Update");
    file.AddTotal(node1, "count", "Total number of symbionts");
    file.AddTotal(node2, "free_syms", "Total number of free syms");
    file.AddTotal(node3, "hosted_syms", "Total number of syms in a host");
    file.AddMean(node4, "PGG_donationrate","Average donation rate");

    file.AddHistBin(node4, 0, "Hist_0.0", "Count for histogram bin 0.0 to <0.1");
    file.AddHistBin(node4, 1, "Hist_0.1", "Count for histogram bin 0.1 to <0.2");
    file.AddHistBin(node4, 2, "Hist_0.2", "Count for histogram bin 0.2 to <0.3");
    file.AddHistBin(node4, 3, "Hist_0.3", "Count for histogram bin 0.3 to <0.4");
    file.AddHistBin(node4, 4, "Hist_0.4", "Count for histogram bin 0.4 to <0.5");
    file.AddHistBin(node4, 5, "Hist_0.5", "Count for histogram bin 0.5 to <0.6");
    file.AddHistBin(node4, 6, "Hist_0.6", "Count for histogram bin 0.6 to <0.7");
    file.AddHistBin(node4, 7, "Hist_0.7", "Count for histogram bin 0.7 to <0.8");
    file.AddHistBin(node4, 8, "Hist_0.8", "Count for histogram bin 0.8 to <0.9");
    file.AddHistBin(node4, 9, "Hist_0.9", "Count for histogram bin 0.9 to 1.0");


    file.PrintHeaderKeys();

    return file;
  }

  /**
   * Input: None
   *
   * Output: The DataMonitor<double, emp::data::Histogram>& that has
   * information representing the average donation value for all symbionts
   * in the system.
   *
   * Purpose: To collect data on the average donation value to be saved to the
   * data file that is tracking PGG data.
   */
  emp::DataMonitor<double, emp::data::Histogram>& GetPGGDataNode() {
    if (!data_node_PGG) {
      data_node_PGG.New();
      OnUpdate([this](size_t){
        data_node_PGG->Reset();
        for (size_t i = 0; i< pop.size(); i++) {
          if (IsOccupied(i)) { //track hosted syms
            emp::vector<emp::Ptr<Organism>>& syms = pop[i]->GetSymbionts();
            size_t sym_size = syms.size();
            for(size_t j=0; j< sym_size; j++){
              data_node_PGG->AddDatum(syms[j]->GetDonation());
            }//close for
          }//close if
          if(sym_pop[i]){ //track free-living syms
            data_node_PGG->AddDatum(sym_pop[i]->GetDonation());
          }//close if
        }//close for
      });
    }
    data_node_PGG->SetupBins(0, 1.1, 11);
    return *data_node_PGG;
  }

}; //end of PGGWorld class
#endif
