#ifndef SYM_WORLD_H
#define SYM_WORLD_H
 
#include "../../Empirical/include/emp/Evolve/World.hpp"
#include "../../Empirical/include/emp/data/DataFile.hpp"
#include "../../Empirical/include/emp/math/random_utils.hpp"
#include "../../Empirical/include/emp/math/Random.hpp"
#include "Organism.h"
#include <set>
#include <math.h>
 
// #include <typeinfo>
// string s = typeid(p).name()
 
class SymWorld : public emp::World<Organism>{
private:
  /**
    * 
    * Purpose: Represents the vertical transmission rate. This can be set with SetVertTrans()
    * 
  */  
  double vertTrans = 0;
 
  /**
    * 
    * Purpose: Represents the total resources in the world. This can be set with SetTotalRes()
    * 
  */    
  int total_res = -1;
 
  /**
    * 
    * Purpose: Represents if resources are limited or not. This can be set with SetLimitedRes()
    * 
  */    
  bool limited_res = false;
 
  /**
    * 
    * Purpose: Represents if free living symbionts are allowed. This can be set with SetFreeLivingSyms()
    * 
  */    
  bool do_free_living_syms = false;
 
  /**
    * 
    * Purpose: Represents how many resources each host gets per update. This can be set with SetResPerUpdate()
    * 
  */  
  double resources_per_host_per_update = 0;
 
  /**
    * 
    * Purpose: Represents if free living symbionts are permitted to move around the world. 
    * 
  */  
  bool move_free_syms = false;
 
  pop_t sym_pop; //free living sym pop
 
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_hostintval; // New() reallocates this pointer
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_symintval;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_freesymintval;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_hostedsymintval;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_syminfectchance;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_freesyminfectchance;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_hostedsyminfectchance;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_lysischance;
  emp::Ptr<emp::DataMonitor<int>> data_node_hostcount;
  emp::Ptr<emp::DataMonitor<int>> data_node_symcount;
  emp::Ptr<emp::DataMonitor<int>> data_node_freesymcount;
  emp::Ptr<emp::DataMonitor<int>> data_node_hostedsymcount;
  emp::Ptr<emp::DataMonitor<double>> data_node_burst_size;
  emp::Ptr<emp::DataMonitor<int>> data_node_burst_count;
  emp::Ptr<emp::DataMonitor<double>> data_node_efficiency;
  emp::Ptr<emp::DataMonitor<int>> data_node_cfu;
  emp::Ptr<emp::DataMonitor<double,emp::data::Histogram>> data_node_Pgg;
  emp::Ptr<emp::DataMonitor<int>> data_node_uninf_hosts;
 
 
public:
  /**
   * Input: The world's random seed
   * 
   * Output: None
   * 
   * Purpose: To construct an instance of SymWorld
   */
  SymWorld(emp::Random & _random) : emp::World<Organism>(_random) {
    fun_print_org = [](Organism & org, std::ostream & os) {
      //os << PrintHost(&org);
      os << "This doesn't work currently";
    };
  }
 
  
  /**
   * Input: None
   * 
   * Output: None
   * 
   * Purpose: To destruct the data nodes belonging to SymWorld to conserve memory. 
   */
  ~SymWorld() {
    if (data_node_hostintval) data_node_hostintval.Delete();
    if (data_node_symintval) data_node_symintval.Delete();
    if (data_node_freesymintval) data_node_freesymintval.Delete();
    if (data_node_hostedsymintval) data_node_hostedsymintval.Delete();
    if (data_node_syminfectchance) data_node_syminfectchance.Delete();
    if (data_node_freesyminfectchance) data_node_freesyminfectchance.Delete();
    if (data_node_hostedsyminfectchance) data_node_hostedsyminfectchance.Delete();
    if (data_node_lysischance) data_node_lysischance.Delete();
    if (data_node_hostcount) data_node_hostcount.Delete();
    if (data_node_symcount) data_node_symcount.Delete();
    if (data_node_freesymcount) data_node_freesymcount.Delete();
    if (data_node_hostedsymcount) data_node_hostedsymcount.Delete();
    if (data_node_burst_size) data_node_burst_size.Delete();
    if (data_node_burst_count) data_node_burst_count.Delete();
    if (data_node_cfu) data_node_cfu.Delete();
    if (data_node_uninf_hosts) data_node_uninf_hosts.Delete();
    if (data_node_Pgg) data_node_Pgg.Delete();
  }
 
 
  /**
   * Input: The double representing the vertical transmission rate
   * 
   * Output: None
   * 
   * Purpose: To set the vertical transmission rate
   */
  void SetVertTrans(double vt) {vertTrans = vt;}
 
 
  /**
   * Input: The double representing the number of resources each host gets in each update. 
   * 
   * Output: None
   * 
   * Purpose: To set the resources that each host gets per update.
   */  
  void SetResPerUpdate(double val) {resources_per_host_per_update = val;}
 
 
  /**
   * Input: To boolean representing if resources are limited or not. 
   * 
   * Output: None
   * 
   * Purpose: To allow for resources to be limited or unlimited. 
   */   
  void SetLimitedRes(bool val) {limited_res = val;}
 
 
  /**
   * Input: The boolean representing if symbionts are allowed to be free living. 
   * 
   * Output: None
   * 
   * Purpose: To allow for free-living symbionts
   */  
  void SetFreeLivingSyms(bool flp) {do_free_living_syms = flp; }
 
 
  /**
   * Input: The bool representing if free living symbionts are 
   * permitted to move around in the world. 
   * 
   * Output: None
   * 
   * Purpose: To set the value representing if FLS are 
   * permitted to move around. 
   */    
  void SetMoveFreeSyms(bool mfs) {move_free_syms = mfs;}
 
 
  /**
   * Input: The int representing the total number of resources for the world. 
   * 
   * Output: None
   * 
   * Purpose: To set the total number of resources in the world. If limited resources 
   * is off, then the total resource value is of no consequence. 
   */  
  void SetTotalRes(int val) {
    if(val<0){
      SetLimitedRes(false);
    } else {
      SetLimitedRes(true);
      total_res = val;
    }
  }
 
 
  /**
   * Input: None
   * 
   * Output: The pop_t value that represents the world's population. 
   * 
   * Purpose: To get the world's population of organisms. 
   */  
  emp::World<Organism>::pop_t GetPop() {return pop;}
 
 
  /**
   * Input: None
   * 
   * Output: The pop_t value that represent the world's symbiont
   * population. 
   * 
   * Purpose: To get the world's symbiont population. 
   */    
  emp::World<Organism>::pop_t GetSymPop() {return sym_pop;}
 
 
  /**
   * Input: None
   * 
   * Output: The boolean representing if vertical transmission will occur 
   * 
   * Purpose: To determine if vertical transmission will occur
   */  
  bool WillTransmit() {
    bool result = GetRandom().GetDouble(0.0, 1.0) < vertTrans;
    return result;
  }
 
 
  /**
   * Input: None
   * 
   * Output: If there are unlimited resources, this will return resources_per_host_per_update. 
   * Else, if the total resources is greater than the resources per host, the resources_per_host_per_update
   * will be returned. If total_res is less than resources_per_host_per_update, but greater than 0, 
   * then total_res will be returned. If none of these are true, then 0 will be returned. 
   * 
   * Purpose: To determine how many resources to distribute to each host.
   */  
  int PullResources() {
    if(!limited_res) {
      return resources_per_host_per_update;
    } else {
      if (total_res>=resources_per_host_per_update) {
        total_res = total_res - resources_per_host_per_update;
        return resources_per_host_per_update;
      } else if (total_res>0) {
        int resources_to_return = total_res;
        total_res = 0;
        return resources_to_return;
      } else {
        return 0;
      }
    }
  }
 
 
  /**
   * Input: The size_t representing the world's new width;
   * the size_t representing the world's new height. 
   * 
   * Output: None
   * 
   * Purpose: To overwrite the Empirical resize so that sym_pop is also resized
   */    
  void Resize(size_t new_width, size_t new_height) {
    size_t new_size = new_width * new_height;
    pop.resize(new_size);
    sym_pop.resize(new_size);
    pop_sizes.resize(2);
    pop_sizes[0] = new_width; pop_sizes[1] = new_height;
  }
 
 
  /**
   * Input: The size_t representing the new size of the world
   * 
   * Output: None
   * 
   * Purpose: To override the Empirical Resize function with 
   * a single-arg method that can be used for AddOrgAt vector
   * expansions
   */    
  void Resize(size_t new_size){
    pop.resize(new_size);
    sym_pop.resize(new_size);
    pop_sizes.resize(2);
  }
 
 
  /**
   * Input: The pointer to the new organism; 
   * the world position of the location to add
   * the new organism to. 
   * 
   * Output: None
   * 
   * Purpose: To overwrite the empirical AddOrgAt function to permit syms to 
   * be added into sym_pop
   */
  void AddOrgAt(emp::Ptr<Organism> new_org, emp::WorldPosition pos, emp::WorldPosition p_pos=emp::WorldPosition()) {
    emp_assert(new_org);         // The new organism must exist.
    emp_assert(pos.IsValid());   // Position must be legal.
 
    //if the pos it out of bounds, expand the worlds so that they can fit it.
    if(pos.GetIndex() >= sym_pop.size() || pos.GetIndex() >= pop.size()){
      Resize(pos.GetIndex() + 1);
    }
 
    if(new_org->IsHost()){ //if the org is a host, use the empirical addorgat function
      emp::World<Organism>::AddOrgAt(new_org, pos,p_pos);
    } else { //if it is not a host, then add it to the sym population
      size_t pos_index = pos.GetIndex();
 
      //if it is adding a sym to the pop, add to the num_org count
      //otherwise, delete the sym currently occupying the spot
      if(!sym_pop[pos_index]) ++num_orgs;
      else sym_pop[pos_index].Delete();
 
      //set the pointer to NULL
      sym_pop[pos_index] = nullptr;
      sym_pop[pos_index] = new_org;
    }
  }
 
 
  //Overriding World's DoBirth to take a pointer instead of a reference
  //Because it takes a pointer, it doesn't support birthing multiple copies
  /**
   * Input: (1) The pointer to the organism that is being birthed;
   * (2) The size_t location of the parent organism. 
   * 
   * Output: The WorldPosition of the position of the new organism. 
   * 
   * Purpose: To introduce new organisms to the world. If the new organism is a host, 
   * and the position generated for its location is occupied by a symbiont, then the 
   * symbiont will be added to the host's symbionts. If the new organism is a host, 
   * and the position generated for its location is unoccupied or occupied by a host, 
   * then the new organism will be added regularly. 
   */  
  emp::WorldPosition DoBirth(emp::Ptr<Organism> new_org, size_t parent_pos) {
    before_repro_sig.Trigger(parent_pos);
    emp::WorldPosition pos; // Position of each offspring placed.
 
    offspring_ready_sig.Trigger(*new_org, parent_pos);
    pos = fun_find_birth_pos(new_org, parent_pos);
    if (pos.IsValid() && pos.GetIndex() != parent_pos) {
      //Add to the specified position, overwriting what may exist there
      AddOrgAt(new_org, pos, parent_pos);
    }
    else {
      new_org.Delete();
    } // Otherwise delete the organism.
    return pos;
  }
 
 
  /**
   * Input: The size_t value representing the location whose neighbors 
   * are being searched. 
   * 
   * Output: If there are no occupied neighboring positions, -1 will be returned. 
   * If there are occupied neighboring positions, then the location of one 
   * occupied position will be returned. 
   * 
   * Purpose: To determine the location of a valid occupied neighboring position.
   */
  int GetNeighborHost (size_t i) {
    const emp::vector<size_t> validNeighbors = GetValidNeighborOrgIDs(i);
    if (validNeighbors.empty()) return -1;
    else {
      int randI = GetRandom().GetUInt(0, validNeighbors.size());
      return validNeighbors[randI];
    }
  }
 
 
  /**
   * Input: The pointer to an organism that will be injected into a host.
   * 
   * Output: None
   * 
   * Purpose: To add a symbiont to a host's symbionts. 
   */  
  void InjectSymbiont(emp::Ptr<Organism> new_sym){
    size_t new_loc;
    if(!do_free_living_syms){
      new_loc = GetRandomOrgID();
      //if the position is acceptable, add the sym to the host in that position
      if(IsOccupied(new_loc)) pop[new_loc]->AddSymbiont(new_sym);
    } else {
      new_loc = GetRandomCellID();
      //if the position is within bounds, add the sym to it
      if(new_loc < sym_pop.size()) AddOrgAt(new_sym, new_loc);
    }
  }
 
 
  /**
   * Input: The address of the string representing the file to be 
   * created's name 
   * 
   * Output: The address of the DataFile that has been created. 
   * 
   * Purpose: To set up the file that will be used to track mean_burstsize,
   * and burst_count. 
   */
  emp::DataFile & SetupLysisFile(const std::string & filename) {
    auto & file = SetupFile(filename);
    auto & node1 = GetBurstSizeDataNode();
    auto & node = GetBurstCountDataNode();
    file.AddVar(update, "update", "Update");
    file.AddMean(node1, "mean_burstsize", "Average burst size", true);
    file.AddTotal(node, "burst_count", "Average burst count", true);
    file.PrintHeaderKeys();
 
    return file;
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
   * Input: The address of the string representing the file to be 
   * created's name 
   * 
   * Output: The address of the DataFile that has been created. 
   * 
   * Purpose: To set up the file that will be used to track the average symbiont
   * interaction value, the total number of symbionts, the total number of symbionts 
   * in a host, the total number of free syms and set up a histogram of the
   * symbiont's interaction values. 
   */
  emp::DataFile & SetupSymIntValFile(const std::string & filename) {
    auto & file = SetupFile(filename);
    auto & node = GetSymIntValDataNode();
    auto & node1 = GetSymCountDataNode();
 
    node.SetupBins(-1.0, 1.1, 21); //Necessary because range exclusive
    file.AddVar(update, "update", "Update");
    file.AddMean(node, "mean_intval", "Average symbiont interaction value");
    file.AddTotal(node1, "count", "Total number of symbionts");
 
    //interaction val histogram
    file.AddHistBin(node, 0, "Hist_-1", "Count for histogram bin -1 to <-0.9");
    file.AddHistBin(node, 1, "Hist_-0.9", "Count for histogram bin -0.9 to <-0.8");
    file.AddHistBin(node, 2, "Hist_-0.8", "Count for histogram bin -0.8 to <-0.7");
    file.AddHistBin(node, 3, "Hist_-0.7", "Count for histogram bin -0.7 to <-0.6");
    file.AddHistBin(node, 4, "Hist_-0.6", "Count for histogram bin -0.6 to <-0.5");
    file.AddHistBin(node, 5, "Hist_-0.5", "Count for histogram bin -0.5 to <-0.4");
    file.AddHistBin(node, 6, "Hist_-0.4", "Count for histogram bin -0.4 to <-0.3");
    file.AddHistBin(node, 7, "Hist_-0.3", "Count for histogram bin -0.3 to <-0.2");
    file.AddHistBin(node, 8, "Hist_-0.2", "Count for histogram bin -0.2 to <-0.1");
    file.AddHistBin(node, 9, "Hist_-0.1", "Count for histogram bin -0.1 to <0.0");
    file.AddHistBin(node, 10, "Hist_0.0", "Count for histogram bin 0.0 to <0.1");
    file.AddHistBin(node, 11, "Hist_0.1", "Count for histogram bin 0.1 to <0.2");
    file.AddHistBin(node, 12, "Hist_0.2", "Count for histogram bin 0.2 to <0.3");
    file.AddHistBin(node, 13, "Hist_0.3", "Count for histogram bin 0.3 to <0.4");
    file.AddHistBin(node, 14, "Hist_0.4", "Count for histogram bin 0.4 to <0.5");
    file.AddHistBin(node, 15, "Hist_0.5", "Count for histogram bin 0.5 to <0.6");
    file.AddHistBin(node, 16, "Hist_0.6", "Count for histogram bin 0.6 to <0.7");
    file.AddHistBin(node, 17, "Hist_0.7", "Count for histogram bin 0.7 to <0.8");
    file.AddHistBin(node, 18, "Hist_0.8", "Count for histogram bin 0.8 to <0.9");
    file.AddHistBin(node, 19, "Hist_0.9", "Count for histogram bin 0.9 to 1.0");
 
    file.PrintHeaderKeys();
 
    return file;
  }
 
 
  /**
   * Input:#
   * 
   * Output: 
   * 
   * Purpose: 
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
   * Input: The address of the string representing the file to be 
   * created's name 
   * 
   * Output: The address of the DataFile that has been created. 
   * 
   * Purpose: To set up the file that will be used to track host's
   * interaction values, the total number of hosts, the total 
   * number of colony forming units, and the histogram of the
   * host's interaction values
   */
  emp::DataFile & SetupHostIntValFile(const std::string & filename) {
    auto & file = SetupFile(filename);
    auto & node = GetHostIntValDataNode();
    auto & node1 = GetHostCountDataNode();
    auto & cfu_node = GetCFUDataNode();
    auto & uninf_hosts_node = GetUninfectedHostsDataNode();
    node.SetupBins(-1.0, 1.1, 21);
 
    file.AddVar(update, "update", "Update");
    file.AddMean(node, "mean_intval", "Average host interaction value");
    file.AddTotal(node1, "count", "Total number of hosts");
    file.AddTotal(cfu_node, "cfu_count", "Total number of colony forming units"); //colony forming units are hosts that
    //either aren't infected at all or only with lysogenic phage if lysis is enabled
    file.AddTotal(uninf_hosts_node, "uninfected_host_count", "Total number of hosts that are uninfected");
    file.AddHistBin(node, 0, "Hist_-1", "Count for histogram bin -1 to <-0.9");
    file.AddHistBin(node, 1, "Hist_-0.9", "Count for histogram bin -0.9 to <-0.8");
    file.AddHistBin(node, 2, "Hist_-0.8", "Count for histogram bin -0.8 to <-0.7");
    file.AddHistBin(node, 3, "Hist_-0.7", "Count for histogram bin -0.7 to <-0.6");
    file.AddHistBin(node, 4, "Hist_-0.6", "Count for histogram bin -0.6 to <-0.5");
    file.AddHistBin(node, 5, "Hist_-0.5", "Count for histogram bin -0.5 to <-0.4");
    file.AddHistBin(node, 6, "Hist_-0.4", "Count for histogram bin -0.4 to <-0.3");
    file.AddHistBin(node, 7, "Hist_-0.3", "Count for histogram bin -0.3 to <-0.2");
    file.AddHistBin(node, 8, "Hist_-0.2", "Count for histogram bin -0.2 to <-0.1");
    file.AddHistBin(node, 9, "Hist_-0.1", "Count for histogram bin -0.1 to <0.0");
    file.AddHistBin(node, 10, "Hist_0.0", "Count for histogram bin 0.0 to <0.1");
    file.AddHistBin(node, 11, "Hist_0.1", "Count for histogram bin 0.1 to <0.2");
    file.AddHistBin(node, 12, "Hist_0.2", "Count for histogram bin 0.2 to <0.3");
    file.AddHistBin(node, 13, "Hist_0.3", "Count for histogram bin 0.3 to <0.4");
    file.AddHistBin(node, 14, "Hist_0.4", "Count for histogram bin 0.4 to <0.5");
    file.AddHistBin(node, 15, "Hist_0.5", "Count for histogram bin 0.5 to <0.6");
    file.AddHistBin(node, 16, "Hist_0.6", "Count for histogram bin 0.6 to <0.7");
    file.AddHistBin(node, 17, "Hist_0.7", "Count for histogram bin 0.7 to <0.8");
    file.AddHistBin(node, 18, "Hist_0.8", "Count for histogram bin 0.8 to <0.9");
    file.AddHistBin(node, 19, "Hist_0.9", "Count for histogram bin 0.9 to 1.0");
 
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
   * lysis chance, the number of symbionts, and the histogram of
   * the mean lysis chance.
   */
    emp::DataFile & SetupLysisChanceFile(const std::string & filename) {
    auto & file = SetupFile(filename);
    auto & node1 = GetSymCountDataNode();
    auto & node = GetLysisChanceDataNode();
    node.SetupBins(0.0, 1.1, 10); //Necessary because range exclusive
    file.AddVar(update, "update", "Update");
    file.AddMean(node, "mean_lysischance", "Average chance of lysis");
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
   * Input:#
   * 
   * Output: 
   * 
   * Purpose: 
   */  
  emp::DataFile & SetUpFreeLivingSymFile(const std::string & filename){
    auto & file = SetupFile(filename);
    auto & node1 = GetSymCountDataNode(); //count
    auto & node2 = GetCountFreeSymsDataNode();
    auto & node3 = GetCountHostedSymsDataNode();
    auto & node4 = GetSymIntValDataNode(); //interaction_val
    auto & node5 = GetFreeSymIntValDataNode();
    auto & node6 = GetHostedSymIntValDataNode();
    auto & node7 = GetSymInfectChanceDataNode(); //infect chance
    auto & node8 = GetFreeSymInfectChanceDataNode();
    auto & node9 = GetHostedSymInfectChanceDataNode();
 
    file.AddVar(update, "update", "Update");
 
    //count
    file.AddTotal(node1, "count", "Total number of symbionts");
    file.AddTotal(node2, "free_syms", "Total number of free syms");
    file.AddTotal(node3, "hosted_syms", "Total number of syms in a host");
 
 
    //interaction val
    file.AddMean(node4, "mean_intval", "Average symbiont interaction value");
    file.AddMean(node5, "mean_freeintval", "Average free symbiont interaction value");
    file.AddMean(node6, "mean_hostedintval", "Average hosted symbiont interaction value");
 
    //infection chance
    file.AddMean(node7, "mean_infectchance", "Average symbiont infection chance");
    file.AddMean(node8, "mean_freeinfectchance", "Average free symbiont infection chance");
    file.AddMean(node9, "mean_hostedinfectchance", "Average hosted symbiont infection chance");
 
    file.PrintHeaderKeys();
 
    return file;
  }
 
 
  /**
   * Input: None
   * 
   * Output: The DataMonitor<int>& that has the information representing
   * the host count. 
   * 
   * Purpose: To collect data on the host count to be saved to the 
   * data file that is tracking host count
   */  
  emp::DataMonitor<int>& GetHostCountDataNode() {
    if(!data_node_hostcount) {
      data_node_hostcount.New();
      OnUpdate([this](size_t){
        data_node_hostcount -> Reset();
        for (size_t i = 0; i< pop.size(); i++)
          if(IsOccupied(i))
            data_node_hostcount->AddDatum(1);
      });
    }
    return *data_node_hostcount;
  }
 
 
  /**
   * Input: None
   * 
   * Output: The DataMonitor<int>& that has the information representing
   * the symbiont count. 
   * 
   * Purpose: To collect data on the symbiont count to be saved to the 
   * data file that is tracking symbiont count
   */   
  emp::DataMonitor<int>& GetSymCountDataNode() {
    if(!data_node_symcount) {
      data_node_symcount.New();
      OnUpdate([this](size_t){
        data_node_symcount -> Reset();
        for (size_t i = 0; i < pop.size(); i++){
          if(IsOccupied(i)){
            data_node_symcount->AddDatum((pop[i]->GetSymbionts()).size());
          }
          if(sym_pop[i]){
            data_node_symcount->AddDatum(1);
          }
        }
      });
    }
    return *data_node_symcount;
  }
 
 
  /**
   * Input: None
   * 
   * Output: The DataMonitor<double>& that has the information representing
   * the count of the hosted symbionts. 
   * 
   * Purpose: To collect data on the count of the hosted symbionts to be saved to the 
   * data file that is tracking the count of the hosted symbionts. 
   */  
  emp::DataMonitor<int>& GetCountHostedSymsDataNode(){
    if (!data_node_hostedsymcount) {
      data_node_hostedsymcount.New();
      OnUpdate([this](size_t){
        data_node_hostedsymcount->Reset();
        for (size_t i = 0; i< pop.size(); i++)
          if (IsOccupied(i))
            data_node_hostedsymcount->AddDatum(pop[i]->GetSymbionts().size());
      });
    }
    return *data_node_hostedsymcount;
  }
 
 
  /**
   * Input: None
   * 
   * Output: The DataMonitor<double>& that has the information representing
   * the count of the free symbionts. 
   * 
   * Purpose: To collect data on the count of the free symbionts to be saved to the 
   * data file that is tracking the count of the free symbionts. 
   */   
    emp::DataMonitor<int>& GetCountFreeSymsDataNode(){
    if (!data_node_freesymcount) {
      data_node_freesymcount.New();
      OnUpdate([this](size_t){
        data_node_freesymcount->Reset();
        for (size_t i = 0; i< pop.size(); i++)
          if (sym_pop[i])
            data_node_freesymcount->AddDatum(1);
      });
    }
    return *data_node_freesymcount;
  }
 
 
  /**
   * Input:#
   * 
   * Output: 
   * 
   * Purpose: 
   */    
  emp::DataMonitor<int>& GetUninfectedHostsDataNode() {
    //keep track of host organisms that are uninfected
    if(!data_node_uninf_hosts) {
      data_node_uninf_hosts.New();
      OnUpdate([this](size_t){
    data_node_uninf_hosts -> Reset();
 
    for (size_t i = 0; i < pop.size(); i++) {
      if(IsOccupied(i)) {
        if((pop[i]->GetSymbionts()).empty()) {
          data_node_uninf_hosts->AddDatum(1);
        }
      } //endif
    } //end for
  }); //end OnUpdate
    } //end if
    return *data_node_uninf_hosts;
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
                if(syms[j]->IsPhage()){
                  if(syms[j]->GetLysogeny() == false){
                    all_lysogenic = false;
                  }
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
   * Input:#
   * 
   * Output: 
   * 
   * Purpose: 
   */    
  void MoveIntoNewFreeWorldPos(emp::Ptr<Organism> sym, size_t i){
    emp::WorldPosition newLoc = GetRandomNeighborPos(i);
    if(newLoc.IsValid()){
      sym->SetHost(nullptr);
      AddOrgAt(sym, newLoc, i);
    } else sym.Delete();
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
 
 
  /**
   * Input: None
   * 
   * Output: The DataMonitor<double, emp::data::Histogram>& that has the information representing
   * the host interaction value. 
   * 
   * Purpose: To collect data on the host interaction value to be saved to the 
   * data file that is tracking host interaction value. 
   */  
  emp::DataMonitor<double, emp::data::Histogram>& GetHostIntValDataNode() {
    if (!data_node_hostintval) {
      data_node_hostintval.New();
      OnUpdate([this](size_t){
        data_node_hostintval->Reset();
        for (size_t i = 0; i< pop.size(); i++)
          if (IsOccupied(i))
            data_node_hostintval->AddDatum(pop[i]->GetIntVal());
      });
    }
    return *data_node_hostintval;
  }
 
 
  /**
   * Input: None
   * 
   * Output: The DataMonitor<double, emp::data::Histogram>& that has the information representing
   * the symbiont interaction value. 
   * 
   * Purpose: To collect data on the symbiont interaction value to be saved to the 
   * data file that is tracking symbionts interaction value. 
   */
  emp::DataMonitor<double,emp::data::Histogram>& GetSymIntValDataNode() {
    if (!data_node_symintval) {
      data_node_symintval.New();
      OnUpdate([this](size_t){
        data_node_symintval->Reset();
        for (size_t i = 0; i< pop.size(); i++) {
          if (IsOccupied(i)) {
            emp::vector<emp::Ptr<Organism>>& syms = pop[i]->GetSymbionts();
            size_t sym_size = syms.size();
            for(size_t j=0; j< sym_size; j++){
              data_node_symintval->AddDatum(syms[j]->GetIntVal());
            }//close for
          }
          if (sym_pop[i]) {
            data_node_symintval->AddDatum(sym_pop[i]->GetIntVal());
          } //close if
        }//close for
      });
    }
    return *data_node_symintval;
  }
 
 
  /**
   * Input:#
   * 
   * Output: 
   * 
   * Purpose:
   */ 
  emp::DataMonitor<double,emp::data::Histogram>& GetFreeSymIntValDataNode() {
    if (!data_node_freesymintval) {
      data_node_freesymintval.New();
      OnUpdate([this](size_t){
        data_node_freesymintval->Reset();
        for (size_t i = 0; i< pop.size(); i++) {
          if (sym_pop[i]) {
            data_node_freesymintval->AddDatum(sym_pop[i]->GetIntVal());
          } //close if
        }//close for
      });
    }
    return *data_node_freesymintval;
  }
 
 
  /**
   * Input:#
   * 
   * Output: 
   * 
   * Purpose:
   */ 
  emp::DataMonitor<double,emp::data::Histogram>& GetHostedSymIntValDataNode() {
    if (!data_node_hostedsymintval) {
      data_node_hostedsymintval.New();
      OnUpdate([this](size_t){
        data_node_hostedsymintval->Reset();
        for (size_t i = 0; i< pop.size(); i++) {
          if (IsOccupied(i)) {
            emp::vector<emp::Ptr<Organism>>& syms = pop[i]->GetSymbionts();
            size_t sym_size = syms.size();
            for(size_t j=0; j< sym_size; j++){
              data_node_hostedsymintval->AddDatum(syms[j]->GetIntVal());
            }//close for
          }//close if
        }//close for
      });
    }
    return *data_node_hostedsymintval;
  }
 
 
  /**
   * Input:#
   * 
   * Output: 
   * 
   * Purpose:
   */  
  emp::DataMonitor<double,emp::data::Histogram>& GetSymInfectChanceDataNode() {
    if (!data_node_syminfectchance) {
      data_node_syminfectchance.New();
      OnUpdate([this](size_t){
        data_node_syminfectchance->Reset();
        for (size_t i = 0; i< pop.size(); i++) {
          if (IsOccupied(i)) {
            emp::vector<emp::Ptr<Organism>>& syms = pop[i]->GetSymbionts();
            size_t sym_size = syms.size();
            for(size_t j=0; j< sym_size; j++){
              data_node_syminfectchance->AddDatum(syms[j]->GetInfectionChance());
            }//close for
          }
          if (sym_pop[i]) {
            data_node_syminfectchance->AddDatum(sym_pop[i]->GetInfectionChance());
          } //close if
        }//close for
      });
    }
    return *data_node_syminfectchance;
  }
 
 
  /**
   * Input:#
   * 
   * Output: 
   * 
   * Purpose:
   */  
  emp::DataMonitor<double,emp::data::Histogram>& GetFreeSymInfectChanceDataNode() {
    if (!data_node_freesyminfectchance) {
      data_node_freesyminfectchance.New();
      OnUpdate([this](size_t){
        data_node_freesyminfectchance->Reset();
        for (size_t i = 0; i< pop.size(); i++) {
          if (sym_pop[i]) {
            data_node_freesyminfectchance->AddDatum(sym_pop[i]->GetInfectionChance());
          } //close if
        }//close for
      });
    }
    return *data_node_freesyminfectchance;
  }
 
 
  /**
   * Input:#
   * 
   * Output: 
   * 
   * Purpose:
   */  
  emp::DataMonitor<double,emp::data::Histogram>& GetHostedSymInfectChanceDataNode() {
    if (!data_node_hostedsyminfectchance) {
      data_node_hostedsyminfectchance.New();
      OnUpdate([this](size_t){
        data_node_hostedsyminfectchance->Reset();
        for (size_t i = 0; i< pop.size(); i++) {
          if (IsOccupied(i)) {
            emp::vector<emp::Ptr<Organism>>& syms = pop[i]->GetSymbionts();
            size_t sym_size = syms.size();
            for(size_t j=0; j< sym_size; j++){
              data_node_hostedsyminfectchance->AddDatum(syms[j]->GetInfectionChance());
            }//close for
          }
        }//close for
      });
    }
    return *data_node_hostedsyminfectchance;
  }
 
 
  /**
   * Input: None
   * 
   * Output: The DataMonitor<double, emp::data::Histogram>& that has the information representing
   * the chance of lysis for each symbionts. 
   * 
   * Purpose: To collect data on the chance of lysis for each symbiont to be saved to the 
   * data file that is tracking chance of lysis for each symbiont. 
   */
  emp::DataMonitor<double,emp::data::Histogram>& GetLysisChanceDataNode() {
    if (!data_node_lysischance) {
      data_node_lysischance.New();
      OnUpdate([this](size_t){
        data_node_lysischance->Reset();
        for (size_t i = 0; i< pop.size(); i++) {
          if (IsOccupied(i)) {
            emp::vector<emp::Ptr<Organism>>& syms = pop[i]->GetSymbionts();
            int sym_size = syms.size();
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
 
 
  /**
   * Input: The pointer to the organism that is being birthed, and the size_t location 
   * of the parent symbiont. 
   * 
   * Output: None
   * 
   * Purpose: To birth a new symbiont. If free living symbionts is on, the new symbiont 
   * can be put into an unoccupied place in the world. If not, then it will be placed 
   * in a host near its parent's location, or deleted if the parent's location has
   * no eligible near-by hosts. 
   */  
  void SymDoBirth(emp::Ptr<Organism> sym_baby, size_t i) {
    if(!do_free_living_syms){
      int newLoc = GetNeighborHost(i);
      if (newLoc > -1) { //-1 means no living neighbors
        pop[newLoc]->AddSymbiont(sym_baby);
      } else {
        sym_baby.Delete();
      }
    } else {
      MoveIntoNewFreeWorldPos(sym_baby, i);
    }
  }
 
 
  /**
   * Input: The pointer to the symbiont that is moving, the size_t to its
   * location. 
   * 
   * Output: None
   * 
   * Purpose:#
   */  
  void MoveToFreeWorldPosition(emp::Ptr<Organism> sym, size_t i){
    emp::WorldPosition newLoc = GetRandomNeighborPos(i);
    if(newLoc.IsValid()){
      sym->SetHost(nullptr);
      AddOrgAt(sym, newLoc, i);
    } else sym.Delete();
  }
 
 
  /**
   * Input: The size_t location of the symbiont to be moved. 
   * 
   * Output: None
   * 
   * Purpose: #
   */  
  void MoveFreeSym(size_t i){
    //the sym can either move into a parallel sym or to some random position
    if(IsOccupied(i) && sym_pop[i]->WantsToInfect()) {
      pop[i]->AddSymbiont(ExtractSym(i));
    }
    else if(move_free_syms) {
      MoveIntoNewFreeWorldPos(ExtractSym(i), i);
    }
  }
 
 
  /**
   * Input:#
   * 
   * Output: 
   * 
   * Purpose:
   */  
  emp::Ptr<Organism> ExtractSym(size_t i){
    if(sym_pop[i]){
      emp::Ptr<Organism> sym = sym_pop[i];
      num_orgs--;
      sym_pop[i] = nullptr;
      return sym;
    }
  }
 
 
  /**
   * Input: None
   * 
   * Output: None
   * 
   * Purpose: To call the process functions for hosts and symbionts. 
   */  
  void Update() {
    emp::World<Organism>::Update();
 
    //TODO: put in fancy scheduler at some point
    emp::vector<size_t> schedule = emp::GetPermutation(GetRandom(), GetSize());
 
    // divvy up and distribute resources to host and symbiont in each cell
    for (size_t i : schedule) {
      if (IsOccupied(i) == false && !sym_pop[i]){ continue;} // no organism at that cell
 
      //Would like to shove reproduction into Process, but it gets sticky with Symbiont reproduction
      //Could put repro in Host process and population calls Symbiont process and place offspring as necessary?
      if(IsOccupied(i)){//can't call GetDead on a deleted sym, so
        pop[i]->Process(i);
        if (pop[i]->GetDead()) { //Check if the host died
          DoDeath(i);
        }
      }
      if(sym_pop[i]){
        sym_pop[i]->Process(i);
      }
    } // for each cell in schedule
  } // Update()
};// SymWorld class
#endif