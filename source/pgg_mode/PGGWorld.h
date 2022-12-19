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
  * Definitions of setup functions, expanded in PGGWorldSetup.cc
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
  void CreateDataFiles();

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
  emp::DataFile &SetupPGGSymIntValFile(const std::string &filename);

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
  emp::DataMonitor<double, emp::data::Histogram> &GetPGGDataNode();

}; //end of PGGWorld class
#endif
