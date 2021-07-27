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

  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_hostintval; // New() reallocates this pointer
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_symintval;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_lysischance;
  emp::Ptr<emp::DataMonitor<int>> data_node_hostcount;
  emp::Ptr<emp::DataMonitor<int>> data_node_symcount;
  emp::Ptr<emp::DataMonitor<double>> data_node_burst_size;
  emp::Ptr<emp::DataMonitor<int>> data_node_burst_count;
  emp::Ptr<emp::DataMonitor<double>> data_node_efficiency;
  emp::Ptr<emp::DataMonitor<double>> data_node_hosted_syms;
  emp::Ptr<emp::DataMonitor<double>> data_node_free_syms;
  emp::Ptr<emp::DataMonitor<int>> data_node_cfu;


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
    if (data_node_lysischance) data_node_lysischance.Delete();
    if (data_node_hostcount) data_node_hostcount.Delete();
    if (data_node_symcount) data_node_symcount.Delete();
    if (data_node_burst_size) data_node_burst_size.Delete();
    if (data_node_burst_count) data_node_burst_count.Delete();
    if (data_node_cfu) data_node_cfu.Delete();
    if (data_node_hosted_syms) data_node_hosted_syms.Delete();
    if (data_node_free_syms) data_node_free_syms.Delete();
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
   * Output: The Empirical value pop_t that represents the world's population. 
   * 
   * Purpose: To get the world's population of organisms. 
   */  
  emp::World<Organism>::pop_t getPop() {return pop;}


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
   * Output: If there is unlimited resources, this will return resources_per_host_per_update. 
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

  //Overriding World's DoBirth to take a pointer instead of a reference
  //Because it takes a pointer, it doesn't support birthing multiple copies
  emp::WorldPosition DoBirth(emp::Ptr<Organism> new_org, size_t parent_pos) {
    before_repro_sig.Trigger(parent_pos);
    emp::WorldPosition pos;                                        // Position of each offspring placed.

    offspring_ready_sig.Trigger(*new_org, parent_pos);
    pos = fun_find_birth_pos(new_org, parent_pos);
    if (pos.IsValid() && pos.GetIndex() != parent_pos) {
      if(!IsOccupied(pos) || (pop[pos.GetIndex()]->IsHost())){ //if unoccupied or occupied by a host, add regularly
        AddOrgAt(new_org, pos, parent_pos);
      } else if(!pop[pos.GetIndex()]->IsHost() && new_org->IsHost()){ //If the new_org is a host and the occupying org is a sym, absorb it
        emp::Ptr<Organism> sym = pop[pos.GetIndex()];
        new_org->AddSymbiont(sym);
        pop[pos.GetIndex()] = new_org;
      } else { //this shouldn't happen
        new_org.Delete();
      }
    }
    else {
      new_org.Delete();
    } // Otherwise delete the organism.
    return pos;
  }

  int GetNeighborHost (size_t i) {
    const emp::vector<size_t> validNeighbors = GetValidNeighborOrgIDs(i);
    if (validNeighbors.empty()) return -1;
    else {
      int randI = GetRandom().GetUInt(0, validNeighbors.size());
      return validNeighbors[randI];
    }
  }


  /**
   * Input: The pointer to a organism that will be injected into a host.
   * 
   * Output: None
   * 
   * Purpose: To add a symbiont to a host's symbionts. 
   */  
  void InjectSymbiont(emp::Ptr<Organism> newSym){
    int newLoc = GetRandomOrgID();
    if(do_free_living_syms){ newLoc = GetRandomCellID(); }

    if(IsOccupied(newLoc) == true && pop[newLoc]->IsHost()) {
      newSym->SetHost(pop[newLoc]);
      pop[newLoc]->AddSymbiont(newSym);
    } else if (!IsOccupied(newLoc) && do_free_living_syms) {
      AddOrgAt(newSym, newLoc);
    } else {
      newSym.Delete();
    }
  }

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

  emp::DataFile & SetupEfficiencyFile(const std::string & filename) {
    auto & file = SetupFile(filename);
    auto & node = GetEfficiencyDataNode();
    file.AddVar(update, "update", "Update");
    file.AddMean(node, "mean_efficiency", "Average efficiency", true);
    file.PrintHeaderKeys();

    return file;
  }

  emp::DataFile & SetupSymIntValFile(const std::string & filename) {
    auto & file = SetupFile(filename);
    auto & node = GetSymIntValDataNode();
    auto & node1 = GetSymCountDataNode();
    auto & node2 = GetCountHostedSymsDataNode();
    auto & node3 = GetCountFreeSymsDataNode();
    node.SetupBins(-1.0, 1.1, 21); //Necessary because range exclusive
    file.AddVar(update, "update", "Update");
    file.AddMean(node, "mean_intval", "Average symbiont interaction value");
    file.AddTotal(node1, "count", "Total number of symbionts");
    file.AddTotal(node2, "hosted_syms", "Total number of syms in a host");
    file.AddTotal(node3, "free_syms", "Total number of free syms");
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

  emp::DataFile & SetupHostIntValFile(const std::string & filename) {
    auto & file = SetupFile(filename);
    auto & node = GetHostIntValDataNode();
    auto & node1 = GetHostCountDataNode();
    auto & cfu_node = GetCFUDataNode();
    node.SetupBins(-1.0, 1.1, 21);

    file.AddVar(update, "update", "Update");
    file.AddMean(node, "mean_intval", "Average host interaction value");
    file.AddTotal(node1, "count", "Total number of hosts");
    file.AddTotal(cfu_node, "cfu_count", "Total number of colony forming units"); //colony forming units are hosts that
    //either aren't infected at all or only with lysogenic phage if lysis is enabled
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

    emp::DataFile & SetupLysisChanceFile(const std::string & filename) {
    auto & file = SetupFile(filename);
    auto & node1 = GetSymCountDataNode();
    auto & node = GetLysisChanceDataNode();
    node.SetupBins(0.0, 1.1, 11); //Necessary because range exclusive
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
   * Input: None
   * 
   * Output:
   * 
   * Purpose: 
   */  
  emp::DataMonitor<int>& GetHostCountDataNode() {
    if(!data_node_hostcount) {
      data_node_hostcount.New();
      OnUpdate([this](size_t){
        data_node_hostcount -> Reset();
        for (size_t i = 0; i< pop.size(); i++)
          if(IsOccupied(i) && pop[i]->IsHost())
            data_node_hostcount->AddDatum(1);
      });
    }
    return *data_node_hostcount;
  }


  /**
   * Input: None
   * 
   * Output:
   * 
   * Purpose: 
   */  
  emp::DataMonitor<int>& GetSymCountDataNode() {
    if(!data_node_symcount) {
      data_node_symcount.New();
      OnUpdate([this](size_t){
        data_node_symcount -> Reset();
        for (size_t i = 0; i < pop.size(); i++)
          if(IsOccupied(i)){
            if(pop[i]->IsHost()){
              data_node_symcount->AddDatum((pop[i]->GetSymbionts()).size());
            } else data_node_symcount->AddDatum(1);
          }

      });
    }
    return *data_node_symcount;
  }


  /**
   * Input: None
   * 
   * Output:
   * 
   * Purpose: 
   */  
  emp::DataMonitor<int>& GetCFUDataNode() {
    //keep track of host organisms that are uninfected or infected by only lysogenic phage
    if(!data_node_cfu) {
      data_node_cfu.New();
      OnUpdate([this](size_t){
	  data_node_cfu -> Reset();

	  for (size_t i = 0; i < pop.size(); i++) {
	    if(IsOccupied(i) && pop[i]->IsHost()) {
	      if((pop[i]->GetSymbionts()).empty()) {
		      data_node_cfu->AddDatum(1);
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
   * Output:
   * 
   * Purpose: 
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
   * Output:
   * 
   * Purpose: 
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
   * Output:
   * 
   * Purpose: 
   */  
  emp::DataMonitor<double>& GetEfficiencyDataNode() {
    if (!data_node_efficiency) {
      data_node_efficiency.New();
      OnUpdate([this](size_t){
        data_node_efficiency->Reset();
        for (size_t i = 0; i< pop.size(); i++) {
          if (IsOccupied(i)) {
            if(pop[i]->IsHost()){
	    emp::vector<emp::Ptr<Organism>>& syms = pop[i]->GetSymbionts();
	    size_t sym_size = syms.size();
	    for(size_t j=0; j< sym_size; j++){
	      data_node_efficiency->AddDatum(syms[j]->GetEfficiency());
	    }//close for
    }
    else {
      data_node_efficiency->AddDatum(pop[i]->GetEfficiency());
    }
	  }//close if
	}//close for
      });
    }
    return *data_node_efficiency;
  }


  /**
   * Input: None
   * 
   * Output:
   * 
   * Purpose: 
   */  
  emp::DataMonitor<double, emp::data::Histogram>& GetHostIntValDataNode() {
    if (!data_node_hostintval) {
      data_node_hostintval.New();
      OnUpdate([this](size_t){
        data_node_hostintval->Reset();
        for (size_t i = 0; i< pop.size(); i++)
          if (IsOccupied(i) && pop[i]->IsHost())
            data_node_hostintval->AddDatum(pop[i]->GetIntVal());
      });
    }
    return *data_node_hostintval;
  }


  /**
   * Input: None
   * 
   * Output:
   * 
   * Purpose: 
   */  
  emp::DataMonitor<double>& GetCountHostedSymsDataNode(){
    if (!data_node_hosted_syms) {
      data_node_hosted_syms.New();
      OnUpdate([this](size_t){
        data_node_hosted_syms->Reset();
        for (size_t i = 0; i< pop.size(); i++)
          if (IsOccupied(i) && pop[i]->IsHost())
            data_node_hosted_syms->AddDatum(pop[i]->GetSymbionts().size());
      });
    }
    return *data_node_hosted_syms;
  }


  /**
   * Input: None
   * 
   * Output:
   * 
   * Purpose: 
   */  
    emp::DataMonitor<double>& GetCountFreeSymsDataNode(){
    if (!data_node_free_syms) {
      data_node_free_syms.New();
      OnUpdate([this](size_t){
        data_node_free_syms->Reset();
        for (size_t i = 0; i< pop.size(); i++)
          if (IsOccupied(i) && !pop[i]->IsHost())
            data_node_free_syms->AddDatum(1);
      });
    }
    return *data_node_free_syms;
  }


  /**
   * Input: None
   * 
   * Output:
   * 
   * Purpose: 
   */  
  emp::DataMonitor<double,emp::data::Histogram>& GetSymIntValDataNode() {
    if (!data_node_symintval) {
      data_node_symintval.New();
      OnUpdate([this](size_t){
        data_node_symintval->Reset();
        for (size_t i = 0; i< pop.size(); i++) {
          if (IsOccupied(i)) {
            if(pop[i]->IsHost()){
	    emp::vector<emp::Ptr<Organism>>& syms = pop[i]->GetSymbionts();
	    size_t sym_size = syms.size();
	    for(size_t j=0; j< sym_size; j++){
	      data_node_symintval->AddDatum(syms[j]->GetIntVal());
	    }//close for
    } else {
      data_node_symintval->AddDatum(pop[i]->GetIntVal());
    }

	  }//close if
	}//close for
      });
    }
    return *data_node_symintval;
  }


  /**
   * Input: None
   * 
   * Output:
   * 
   * Purpose: 
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
	      }//close for
      });
    }
    return *data_node_lysischance;
  }


  /**
   * Input: 
   * 
   * Output: None
   * 
   * Purpose: 
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
      MoveToFreeWorldPosition(sym_baby, i);
    }
  }


  /**
   * Input: 
   * 
   * Output: None
   * 
   * Purpose: 
   */  
  void MoveToFreeWorldPosition(emp::Ptr<Organism> sym, size_t i){
    emp::WorldPosition newLoc = GetRandomNeighborPos(i);
    int newLocIndex = newLoc.GetIndex();
    if(newLoc.IsValid()){
      if(!IsOccupied(newLoc) || (IsOccupied(newLoc) && !pop[newLocIndex]->IsHost())){
        //if the spot is empty or contains a sym, add normally
        sym->SetHost(NULL);
        AddOrgAt(sym, newLoc, i);
      } else if(pop[newLocIndex]->IsHost()){ //if it's occcupied by a host, be sucked up
        pop[newLocIndex]->AddSymbiont(sym);
        //AddSymbiont will set the baby's host appropriately
      } else sym.Delete(); //this shouldn't happen
    }
    else sym.Delete();
  }

  void MoveFreeSym(size_t i){
    emp::Ptr<Organism> sym = pop[i];
    if(!sym->IsHost() && sym->GetDead() == false){
      pop[i] = NULL;
      num_orgs--;
      MoveToFreeWorldPosition(sym, i);
    }
  }


  /**
   * Input: None
   * 
   * Output: None
   * 
   * Purpose: 
   */  
  void Update() {
    emp::World<Organism>::Update();

    //TODO: put in fancy scheduler at some point
    emp::vector<size_t> schedule = emp::GetPermutation(GetRandom(), GetSize());

    // divvy up and distribute resources to host and symbiont in each cell
    for (size_t i : schedule) {
      if (IsOccupied(i) == false){ continue;} // no organism at that cell

      //Would like to shove reproduction into Process, but it gets sticky with Symbiont reproduction
      //Could put repro in Host process and population calls Symbiont process and places offspring as necessary?
      if(pop[i]->IsHost()){//can't call GetDead on a deleted sym, so
        pop[i]->Process(i);
        if (pop[i]->GetDead()) { //Check if the host died
          DoDeath(i);
        }
      }
      else{
        pop[i]->Process(i);
      }

    } // for each cell in schedule
  } // Update()
};// SymWorld class
#endif