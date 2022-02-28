#ifndef EFFWORLD_H
#define EFFWORLD_H

#include "../default_mode/SymWorld.h"
#include "../default_mode/DataNodes.h"

class EfficientWorld : public SymWorld {
private:
  /**
    *
    * Purpose: Data node tracking the average efficiency of efficient symbionts.
    *
  */
  emp::Ptr<emp::DataMonitor<double>> data_node_efficiency;
public:
  using SymWorld::SymWorld;

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To destruct the data nodes belonging to EfficientWorld to conserve memory.
   */
  ~EfficientWorld(){
      if (data_node_efficiency) data_node_efficiency.Delete();
  }
  /**
   * Input: The address of the string representing the file to be
   * created's name
   *
   * Output: The address of the DataFile that has been created.
   *
   * Purpose: To set up the file that will be used to track mean efficiency
   */
  emp::DataFile & SetupEfficiencyFile(const std::string & filename) {
    auto & file = SetupFile(filename);
    auto & node = GetEfficiencyDataNode();
    file.AddVar(update, "update", "Update");
    file.AddMean(node, "mean_efficiency", "Average efficiency", true);
    file.PrintHeaderKeys();

    return file;
  }

  /**
   * Input: None
   *
   * Output: The DataMonitor<double>& that has the information representing
   * the symbiont's efficiency.
   *
   * Purpose: To collect data on the lysis burst size to be saved to the
   * data file that is tracking lysis burst size.
   */
  emp::DataMonitor<double>& GetEfficiencyDataNode() {
    if (!data_node_efficiency) {
      data_node_efficiency.New();
      OnUpdate([this](size_t){
        data_node_efficiency->Reset();
        for (size_t i = 0; i< pop.size(); i++) {
          if (IsOccupied(i)) {
            emp::vector<emp::Ptr<Organism>>& syms = pop[i]->GetSymbionts();
            size_t sym_size = syms.size();
            for(size_t j=0; j< sym_size; j++){
              data_node_efficiency->AddDatum(syms[j]->GetEfficiency());
            }//close for
          }//close if
          if(sym_pop[i]) {
            data_node_efficiency->AddDatum(sym_pop[i]->GetEfficiency());
          }//close if
      }//close for
      });
    }
    return *data_node_efficiency;
  }

}; //end of EfficientWorld class
#endif
