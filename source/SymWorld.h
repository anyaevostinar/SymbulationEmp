#ifndef SYM_WORLD_H
#define SYM_WORLD_H

#include "../../Empirical/include/emp/Evolve/World.hpp"
#include "../../Empirical/include/emp/data/DataFile.hpp"
#include "../../Empirical/include/emp/math/random_utils.hpp"
#include "../../Empirical/include/emp/math/Random.hpp"
#include "Organism.h"
#include "Host.h"
#include <set>
#include <math.h>

class SymWorld : public emp::World<Host>{
private:
  double vertTrans = 0; 
  double mut_rate = 0;
  int sym_limit = -1;
  bool h_trans = 0;
  double host_repro = 0;

  double resources_per_host_per_update = 0;
  double synergy = 0;
  emp::Random &random;
  emp::Ptr<emp::Random> random_ptr;
  
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_hostintval; // New() reallocates this pointer
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_symintval;
  emp::Ptr<emp::DataMonitor<int>> data_node_hostcount;
  emp::Ptr<emp::DataMonitor<int>> data_node_symcount;
  emp::Ptr<emp::DataMonitor<double>> data_node_burst_size;
  emp::Ptr<emp::DataMonitor<int>> data_node_cfu;



public:
  //set fun_print_org to equal function that prints hosts/syms correctly
  SymWorld(emp::Random &random) : emp::World<Host>(random), random(random) {
    random_ptr.New(random);
    fun_print_org = [](Host & org, std::ostream & os) {
      //os << PrintHost(&org);
      os << "This doesn't work currently";
    };
  }

  ~SymWorld() {
    if (data_node_hostintval) data_node_hostintval.Delete();
    if (data_node_symintval) data_node_symintval.Delete();
    if (data_node_hostcount) data_node_hostcount.Delete();
    if (data_node_symcount) data_node_symcount.Delete();
    if (data_node_burst_size) data_node_burst_size.Delete();
    if (data_node_cfu) data_node_cfu.Delete();
  }
  
  void SetVertTrans(double vt) {vertTrans = vt;}
  void SetMutRate(double mut) {mut_rate = mut;}
  void SetSymLimit(int num) {sym_limit = num;}
  void SetHTransBool(bool val) {h_trans = val;}
  void SetHostRepro(double val) {host_repro = val;}
  void SetResPerUpdate(double val) {resources_per_host_per_update = val;}
  void SetSynergy(double val) {synergy = val;}

  emp::World<Host>::pop_t getPop() {return pop;}

  bool WillTransmit() {
    bool result = random.GetDouble(0.0, 1.0) <= vertTrans;
    return result;

  }

  int GetNeighborHost (size_t i) {
    const emp::vector<size_t> validNeighbors = GetValidNeighborOrgIDs(i);
    if (validNeighbors.empty()) return -1;
    else {
      int randI = random.GetUInt(0, validNeighbors.size());
      return validNeighbors[randI];
    }
  }

  void InjectSymbiont(emp::Ptr<Organism> newSym){
    int newLoc = GetRandomOrgID();
    if(IsOccupied(newLoc) == true) {
      newSym->SetHost(pop[newLoc]);
      pop[newLoc]->AddSymbiont(newSym, sym_limit);
    }
  }

  emp::DataFile & SetupLysisFile(const std::string & filename) {
    auto & file = SetupFile(filename);
    auto & node = GetBurstSizeDataNode();
    file.AddVar(update, "update", "Update");
    file.AddMean(node, "mean_burstsize", "Average burst size", true);
    file.PrintHeaderKeys();

    return file;
  }

  emp::DataFile & SetupSymIntValFile(const std::string & filename) {
    auto & file = SetupFile(filename);
    auto & node1 = GetSymCountDataNode();
    auto & node = GetSymIntValDataNode();
    node.SetupBins(-1.0, 1.1, 21); //Necessary because range exclusive
    file.AddVar(update, "update", "Update");
    file.AddMean(node, "mean_intval", "Average symbiont interaction value");
    file.AddTotal(node1, "count", "Total number of symbionts");
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

  emp::DataMonitor<int>& GetSymCountDataNode() {
    if(!data_node_symcount) {
      data_node_symcount.New();
      OnUpdate([this](size_t){
        data_node_symcount -> Reset();
        for (size_t i = 0; i < pop.size(); i++)
          if(IsOccupied(i))
            data_node_symcount->AddDatum((pop[i]->GetSymbionts()).size());
      });
    }
    return *data_node_symcount;
  }
 
  emp::DataMonitor<int>& GetCFUDataNode() {
    if(!data_node_cfu) {
      data_node_cfu.New();
      OnUpdate([this](size_t){
	  data_node_cfu -> Reset();
	  for (size_t i = 0; i < pop.size(); i++) {
	    if(IsOccupied(i)) {
	      if((pop[i]->GetSymbionts()).empty()) {
		data_node_cfu->AddDatum(1);
	      }
	    } //endif
	  } //end for
	}); //end OnUpdate
    } //end if
    return *data_node_cfu;
  }

  emp::DataMonitor<double>& GetBurstSizeDataNode() {
    if (!data_node_burst_size) {
      data_node_burst_size.New();
    }
    return *data_node_burst_size;

  }

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

  emp::DataMonitor<double,emp::data::Histogram>& GetSymIntValDataNode() {
    if (!data_node_symintval) {
      data_node_symintval.New();
      OnUpdate([this](size_t){
        data_node_symintval->Reset();
        for (size_t i = 0; i< pop.size(); i++) {
          if (IsOccupied(i)) {
	    emp::vector<emp::Ptr<Organism>>& syms = pop[i]->GetSymbionts();
	    int sym_size = syms.size();
	    for(size_t j=0; j< sym_size; j++){
	      data_node_symintval->AddDatum(syms[j]->GetIntVal());
	    }//close for
	  }//close if
	}//close for
      });
    }
    return *data_node_symintval;
  }

  void SymDoBirth(emp::Ptr<Organism> sym_baby, size_t i) {
    // pick new host to infect, if one exists at the new location and isn't at the limit
    int newLoc = GetNeighborHost(i);
    if (newLoc > -1) { //-1 means no living neighbors
      pop[newLoc]->AddSymbiont(sym_baby, sym_limit);
    }
  }
  

  void Update() {
    emp::World<Host>::Update();

    //TODO: put in fancy scheduler at some point
    
    emp::vector<size_t> schedule = emp::GetPermutation(random, GetSize());
    
    // divvy up and distribute resources to host and symbiont in each cell 
    for (size_t i : schedule) {
      if (IsOccupied(i) == false) continue;  // no organism at that cell

       
      //Would like to shove reproduction into Process, but it gets sticky with Symbiont reproduction
      //Could put repro in Host process and population calls Symbiont process and places offspring as necessary?
      pop[i]->Process(resources_per_host_per_update, synergy);
      //      std::cout << pop[i]->GetReproSymbionts().size() << std::endl;
  
      //Check reproduction                                                                                                                         
      if (pop[i]->GetPoints() >= host_repro ) {  // if host has more points than required for repro                                                                                                   
        // will replicate & mutate a random offset from parent values
        // while resetting resource points for host and symbiont to zero                                             
        emp::Ptr<Host> host_baby = new Host(random_ptr, pop[i]->GetIntVal());
        host_baby->mutate();
        pop[i]->mutate(); //parent mutates and loses current resources, ie new organism but same symbiont  
        pop[i]->SetPoints(0);
	

        //Now check if symbionts get to vertically transmit
        for(size_t j = 0; j< (pop[i]->GetSymbionts()).size(); j++){
          emp::Ptr<Organism> parent = ((pop[i]->GetSymbionts()))[j];
           if (WillTransmit()) { //Vertical transmission!  
            
            emp::Ptr<Organism> sym_baby = parent->reproduce();                                  
            host_baby->AddSymbiont(sym_baby, sym_limit);

          } //end will transmit
        } //end for loop for each symbiont
        //Will need to change this to AddOrgAt and write my own position grabber 
        //when I want ecto-symbionts
        DoBirth(*host_baby, i); //Automatically deals with grid
      }

      if (pop[i]->HasSym()) { //let each sym do whatever they need to do
        emp::vector<emp::Ptr<Organism>>& syms = pop[i]->GetSymbionts();
        for(size_t j = 0; j < syms.size(); j++){
          syms[j]->process(i);

        } //for each sym in syms
      } //if org has syms
    } // for each cell in schedule
  } // Update()
};// SymWorld class

#endif
