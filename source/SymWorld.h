#ifndef SYM_WORLD_H
#define SYM_WORLD_H

#include "../../Empirical/include/emp/Evolve/World.hpp"
#include "../../Empirical/include/emp/Evolve/Systematics.hpp"
#include "../../Empirical/include/emp/Evolve/SystematicsAnalysis.hpp"
#include "../../Empirical/include/emp/data/DataFile.hpp"
#include "../../Empirical/include/emp/math/random_utils.hpp"
#include "../../Empirical/include/emp/math/Random.hpp"
#include "Organism.h"
#include <set>
#include <math.h>


class SymWorld : public emp::World<Organism>{
protected:
  // takes an organism (to classify), and returns an int (the org's taxon)
  using fun_calc_info_t = std::function<int(Organism &)>;


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

  /**
    *
    * Purpose: Represents the free living sym environment, parallel to "pop" for hosts
    *
  */
  pop_t sym_pop;

  /**
    *
    * Purpose: Represents a standard function object which determines which taxon an organism belongs to.
    *
  */
  fun_calc_info_t calc_info_fun;

  /**
    *
    * Purpose: Represents the systematics object tracking hosts.
    *
  */
  emp::Ptr<emp::Systematics<Organism, int>> host_sys = new emp::Systematics(GetCalcInfoFun());

  /**
    *
    * Purpose: Represents the systematics object tracking symbionts.
    *
  */
  emp::Ptr<emp::Systematics<Organism, int>> sym_sys = new emp::Systematics(GetCalcInfoFun());

  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_hostintval; // New() reallocates this pointer
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_symintval;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_freesymintval;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_hostedsymintval;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_syminfectchance;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_freesyminfectchance;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_hostedsyminfectchance;
  emp::Ptr<emp::DataMonitor<int>> data_node_hostcount;
  emp::Ptr<emp::DataMonitor<int>> data_node_symcount;
  emp::Ptr<emp::DataMonitor<int>> data_node_freesymcount;
  emp::Ptr<emp::DataMonitor<int>> data_node_hostedsymcount;
  emp::Ptr<emp::DataMonitor<int>> data_node_cfu;
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
    AddSystematics(host_sys);
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
    if (data_node_hostcount) data_node_hostcount.Delete();
    if (data_node_symcount) data_node_symcount.Delete();
    if (data_node_freesymcount) data_node_freesymcount.Delete();
    if (data_node_hostedsymcount) data_node_hostedsymcount.Delete();
    if (data_node_cfu) data_node_cfu.Delete();
    if (data_node_uninf_hosts) data_node_uninf_hosts.Delete();
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
   * Output: The systematic object tracking hosts
   *
   * Purpose: To retrieve the host systematic
   */
  emp::Ptr<emp::Systematics<Organism,int>> GetHostSys(){
    return host_sys;
  }


  /**
   * Input: None
   *
   * Output: The systematic object tracking hosts
   *
   * Purpose: To retrieve the symbiont systematic
   */
  emp::Ptr<emp::Systematics<Organism,int>> GetSymSys(){
    return sym_sys;
  }


  /**
   * Input: None
   *
   * Output: The standard function object that determines which bin organisms
   * should belong to depending on their interaction value
   *
   * Purpose: To classify organsims based on their interaction value.
   */
  fun_calc_info_t GetCalcInfoFun() {
    if (!calc_info_fun) {
      calc_info_fun = [ /* other variables inside local scope that you want to use here */ ](Organism & org){
        //classify orgs into bins base on interaction values,
        //same arrangement as histograms (bin 0 = ic -1 to -0.9)
        //inclusive of lower bound, exclusive of upper
        double int_val = org.GetIntVal();
        int bin = (int_val + 1)*10 + (0.0000000000001);
        if (bin > 19) bin = 19;
        return bin;
      };
    }
    return calc_info_fun;
  }


  /**
   * Input: The amount of resourcces an organism wants from the world.
   *
   * Output: If there are unlimited resources or the total resources are greater than those requested,
   * returns the amount of desired resources.
   * If total_res is less than the desired resources, but greater than 0,
   * then total_res will be returned. If none of these are true, then 0 will be returned.
   *
   * Purpose: To determine how many resources to distribute to each organism.
   */
  int PullResources(int desired_resources) {
    if(!limited_res) {
      return desired_resources;
    } else {
      if (total_res>=desired_resources) {
        total_res = total_res - desired_resources;
        return desired_resources;
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
    Resize(new_size);
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
   * the new organism.
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
      emp::World<Organism>::AddOrgAt(new_org, pos, p_pos);

    } else { //if it is not a host, then add it to the sym population
      //for symbionts, their place in their host's world is indicated by their ID,
      //and their position in the host indicated by index
      size_t pos_index = pos.GetIndex();

      if(!sym_pop[pos_index]) ++num_orgs;
      else {
        sym_pop[pos_index].Delete();
        sym_sys->RemoveOrgAfterRepro(pos);
      }

      //add sym to systematic
      //if the parent pos is free living and empty, this is a swap, not a birth
      //so don't add the sym to sym_sys, instead swap in

      sym_sys->AddOrg(*new_org, pos);
      sym_sys->SetNextParent(p_pos);

  /*    size_t p_pos_index = p_pos.GetIndex();
      if(p_pos_index < sym_pop.size() && sym_pop[p_pos_index] != nullptr){
        sym_sys->SwapPositions(pos, p_pos);
      } else {
        sym_sys->AddOrg(*new_org, pos);
        sym_sys->SetNextParent(p_pos);
      }*/
      //set the cell to point to the new sym
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
   * Purpose: To introduce new organisms to the world.
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
      if(IsOccupied(new_loc)) {
        pop[new_loc]->AddSymbiont(new_sym);
        //std::cout << "adding sym to host" << std::endl;
      }
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
   * information about the free living symbionts in the world.
   *  This includes: (1) their total count, (2) the counts
   * of the free and hosted symbionts, (3) the interaction
   * values for the free and hosted symbionts, and (4) the
   * infection chances from the total population, free symbionts,
   * and hosted symbionts.
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
   * Input: None
   *
   * Output: The DataMonitor<int>& that has the information representing
   * the count of the uninfected hosts
   *
   * Purpose: To collect data on the count of the uninfected hosts to be saved to the
   * data file that is tracking the count of the uninfected hosts.
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
   * Input: The pointer to the symbiont that is moving, the size_t to its
   * location.
   *
   * Output: None
   *
   * Purpose: To move a symbiont into a new world position.
   */
  void MoveIntoNewFreeWorldPos(emp::Ptr<Organism> sym, emp::WorldPosition parent_pos){
    size_t i = parent_pos.GetIndex();
    emp::WorldPosition new_pos = GetRandomNeighborPos(i);
    if(new_pos.IsValid()){
      sym->SetHost(nullptr);
      AddOrgAt(sym, new_pos, parent_pos);
    } else sym.Delete();
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
   * Input: None
   *
   * Output: The DataMonitor<double>& that has the information representing
   *  the free symbiont's interaction value.
   *
   * Purpose: To collect data on the interaction value of the free symbionts to be saved to the
   * data file that is tracking the interaction value of the free symbionts.
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
   * Input:None
   *
   * Output:
   *
   * Purpose: To access the data node that is tracking
   * the hosted symbiont interaction value
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
   * Input: None
   *
   * Output: The DataMonitor<double, emp::data::Histogram>& that has the information representing
   * the infection chance for each symbionts.
   *
   * Purpose: To access the data node that is tracking the
   * symbiont infection chance
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
   * Input: None
   *
   * Output: The DataMonitor<double, emp::data::Histogram>& that has the information representing
   * the free symbionts' chance of infection
   *
   *
   * Purpose: To access the data node that is tracking the
   * infection chance within the free symbionts.
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
   * Input: None
   *
   * Output: The DataMonitor<double, emp::data::Histogram>& that has the information representing
   * the infection chance for the hosted symbionts
   *
   * Purpose: To retrieve the data nodes that is tracking the
   * infection chance within the hosted symbionts.
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
  void SymDoBirth(emp::Ptr<Organism> sym_baby, emp::WorldPosition parent_pos) {
    size_t i = parent_pos.GetIndex();
    if(!do_free_living_syms){
      int new_pos = GetNeighborHost(i);
      if (new_pos > -1) { //-1 means no living neighbors
        pop[new_pos]->AddSymbiont(sym_baby);
        //std::cout << "sym added to host" << std::endl;
      } else {
        sym_baby.Delete();
        //std::cout << "sym baby deleted" << std::endl;
      }
    } else {
      MoveIntoNewFreeWorldPos(sym_baby, parent_pos);
    }
  }


  /**
   * Input: The size_t location of the symbiont to be moved.
   *
   * Output: None
   *
   * Purpose: To move a symbiont, either into a host, or into a free world position
   */
  void MoveFreeSym(size_t i){
    //the sym can either move into a parallel sym or to some random position
    if(IsOccupied(i) && sym_pop[i]->WantsToInfect()) {
      emp::Ptr<Organism> sym = ExtractSym(i);
      if(sym->InfectionFails()) sym.Delete(); //if the sym tries to infect and fails it dies
      else pop[i]->AddSymbiont(sym);
    }
    else if(move_free_syms) {
      MoveIntoNewFreeWorldPos(ExtractSym(i), i);
    }
  }

  /*
  * Input: The size_t location of the sym to be pointed to.
  *
  * Output: A pointer to the sym.
  *
  * Purpose: To allow access to syms at a specified location in the sym_pop.
  */
  emp::Ptr<Organism> GetSymAt(size_t location){
    if (location >= 0 && location < sym_pop.size()){
      return sym_pop[location];
    } else {
      throw "Attempted to get out of bounds sym.";
    }
  }

  /**
   * Input: The size_t representing the location of the symbiont to be
   * extracted from the world.
   *
   * Output: The pointer to the organism that was removed from the world
   *
   * Purpose: To remove a symbiont from the world
   */
  emp::Ptr<Organism> ExtractSym(size_t i){
    emp::Ptr<Organism> sym;
    if(sym_pop[i]){
      sym = sym_pop[i];
      num_orgs--;
      sym_pop[i] = nullptr;
    }
    return sym;
  }

  /**
   * Input: The size_t representing the location of the symbiont to be
   * deleted from the world.
   *
   * Output: None
   *
   * Purpose: To delete a symbiont from the world.
   */
  void DoSymDeath(size_t i){
    if(sym_pop[i]){
      sym_pop[i].Delete();
      sym_pop[i] = nullptr;
      num_orgs--;
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
    sym_sys->Update(); //sym_sys is not part of the systematics vector, handle it independently

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
      if(sym_pop[i]){ //for sym movement reasons, syms are deleted the update after they are set to dead
        if (sym_pop[i]->GetDead()) DoSymDeath(i); //Might have died since their last time being processed
        else sym_pop[i]->Process(i);
        //if (sym_pop[i]->GetDead()) DoSymDeath(i); //Checking if they died during their process and cleaning up the corpse
        //TODO: fix the reason why the corpse can't be immediately cleaned up
      }
    } // for each cell in schedule
  } // Update()
};// SymWorld class
#endif
