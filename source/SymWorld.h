#include "../../Empirical/source/Evolve/World.h"
#include "../../Empirical/source/tools/Random.h"
#include <set>
#include "SymOrg.h"
#include "../../Empirical/source/tools/random_utils.h"
#include "../../Empirical/source/data/DataFile.h"
#include <math.h>

class SymWorld : public emp::World<Host>{
 private:
  double vertTrans = 0; 
  double mut_rate = 0;
  int sym_limit = -1;
  emp::Random random;
  
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_hostintval;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_symintval;
  emp::Ptr<emp::DataMonitor<int>> data_node_hostcount;
  emp::Ptr<emp::DataMonitor<int>> data_node_symcount;


 public:
  

    //set fun_print_org to equal function that prints hosts/syms correctly
 SymWorld(emp::Random &random) : emp::World<Host>(random) {
    fun_print_org = [](Host & org, std::ostream & os) {
      //os << PrintHost(&org);
      os << "This doesn't work currently";
    };
    }
  
  void SetVertTrans(double vt) {
    vertTrans = vt;
  }
  void SetMutRate(double mut) {
    mut_rate = mut;
  }

  void SetSymLimit(int num) {
    sym_limit = num;
  }

  bool WillTransmit() {
    if (random.GetDouble(0.0, 1.0) <= vertTrans) {
      return true;
    }  else {
      return false;
    }
    

  }

  void InjectSymbiont(Symbiont newSym){
    int newLoc = GetRandomCellID();
    if(IsOccupied(newLoc) == true){
      pop[newLoc]->AddSymbionts(newSym, sym_limit);
    }
  }

  //TODO: Can I put the counts into the int val file??
  emp::DataFile & SetupSymCountFile(const std::string & filename) {
    auto & file = SetupFile(filename);
    auto & node = GetSymCountDataNode();
    file.AddVar(update, "update", "Update");
    file.AddTotal(node, "count", "Total number of symbionts");

    file.PrintHeaderKeys();

    return file;
  }

  emp::DataFile & SetupHostCountFile(const std::string & filename) {
    auto & file = SetupFile(filename);
    auto & node = GetHostCountDataNode();
    file.AddVar(update, "update", "Update");
    file.AddTotal(node, "count", "Total number of hosts");

    file.PrintHeaderKeys();
    return file;
  }

  emp::DataFile & SetupSymIntValFile(const std::string & filename) {
    auto & file = SetupFile(filename);
    auto & node = GetSymIntValDataNode();
    node.SetupBins(-1.0, 1.0, 20);
    file.AddVar(update, "update", "Update");
    file.AddMean(node, "mean_intval", "Average symbiont interaction value");
    file.AddTotal(node, "count", "Total number of symbionts");
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
    node.SetupBins(-1.0, 1.0, 20);

    file.AddVar(update, "update", "Update");
    file.AddMean(node, "mean_intval", "Average host interaction value");
    file.AddTotal(node, "count", "Total number of hosts");
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

  double CalcIntVal(size_t i) {
    return pop[i]->GetIntVal(); 
  }

  double CalcSymIntVal(size_t i) {
    emp::vector<Symbiont> syms = *(pop[i]->GetSymbionts());
    int sym_size = syms.size();
    double intValSum = 0.0;
    for (i =0; i < sym_size; i++){
      intValSum += syms[i].GetIntVal();
    }
    if (sym_size) return (intValSum/sym_size);
    else return 0;
  }

  emp::DataMonitor<int>& GetHostCountDataNode() {
    if(!data_node_hostcount) {
      data_node_hostcount.New();
      OnUpdate(
	       [this](size_t){
		 data_node_hostcount -> Reset();
		 for (size_t i = 0; i< pop.size(); i++){
		   if(IsOccupied(i)) data_node_hostcount->AddDatum(1);
		 }
	       }
	       );
    }
    return *data_node_hostcount;

  }

  emp::DataMonitor<int>& GetSymCountDataNode() {
    if(!data_node_symcount) {
      data_node_symcount.New();
      OnUpdate(
	       [this](size_t){
		 data_node_symcount -> Reset();
		 for (size_t i = 0; i < pop.size(); i++){
		   if(IsOccupied(i)) data_node_symcount->AddDatum((pop[i]->GetSymbionts())->size());
		 }
	       }
	       );
    }
    return *data_node_symcount;
  }

  emp::DataMonitor<double, emp::data::Histogram>& GetHostIntValDataNode() {
    if (!data_node_hostintval) {
      data_node_hostintval.New();
      OnUpdate(
	       [this](size_t){
		 data_node_hostintval->Reset();
		 for (size_t i = 0; i< pop.size(); i++) {
		   if (IsOccupied(i)) data_node_hostintval->AddDatum(CalcIntVal(i));
		 }
	       }
	       );
    }
    return *data_node_hostintval;
  }



  emp::DataMonitor<double,emp::data::Histogram>& GetSymIntValDataNode() {
    if (!data_node_symintval) {
      data_node_symintval.New();
      OnUpdate(
	       [this](size_t){
		 data_node_symintval->Reset();
		 for (size_t i = 0; i< pop.size(); i++) {
		   if (IsOccupied(i)) data_node_symintval->AddDatum(CalcSymIntVal(i));
		 }
	       }
	       );
    }
    return *data_node_symintval;
  }
  

  void Update(size_t new_resources=10) {
    emp::World<Host>::Update();

    //TODO: put in fancy scheduler at some point
    
    emp::vector<size_t> schedule = emp::GetPermutation(random, GetSize());
    
    // divvy up and distribute resources to host and symbiont in each cell 
    for (size_t i : schedule) {
      if (IsOccupied(i) == false) continue;  // no organism at that cell

  	   
      //Would like to shove reproduction into Process, but it gets sticky with Symbiont reproduction
      //Could put repro in Host process and population calls Symbiont process and places offspring as necessary?
      pop[i]->Process(random);
  
      //Check reproduction                                                                                                                              
      if (pop[i]->GetPoints() >= 1000 ) {  // host replication                                                                                                   
	// will replicate & mutate a random offset from parent values
	// while resetting resource points for host and symbiont to zero                                              

	Host *host_baby = new Host(pop[i]->GetIntVal());
	host_baby->mutate(random, mut_rate);
	pop[i]->mutate(random, mut_rate); //parent mutates and loses current resources, ie new organism but same symbiont  
	pop[i]->SetPoints(0);

	//Now check if symbionts get to vertically transmit
	for(size_t j = 0; j< (pop[i]->GetSymbionts())->size(); j++){
	  Symbiont parent = (*(pop[i]->GetSymbionts()))[j];
	  
	  if (WillTransmit()) { //Vertical transmission!  
	    
	    Symbiont * sym_baby = new Symbiont(parent.GetIntVal(), 0.0); //constructor that takes parent values                                             
	    sym_baby->mutate(random, mut_rate);
	    parent.mutate(random, mut_rate); //mutate parent symbiont                                   
	    host_baby->AddSymbionts(*sym_baby, sym_limit);
	  } //end will transmit
	} //end for loop for each symbiont
	DoBirth(*host_baby, i); //Automatically deals with grid
      }
      if (pop[i]->HasSym()) { //check each sym for horizontal transmission
	if(true){
	  //Original evolution method
	  emp::vector<Symbiont> syms = *(pop[i]->GetSymbionts());
	  for(size_t j = 0; j < syms.size(); j++){
	    if (syms[j].GetPoints() >= 100) {
	      // symbiont reproduces independently (horizontal transmission) if it has >= 100 resources
	      // new symbiont in this host with mutated value
	      // TODO: Make SymDoBirth instead of injecting
	      syms[j].SetPoints(0);
	      Symbiont *sym_baby = new Symbiont(syms[j].GetIntVal());
	      sym_baby->mutate(random, mut_rate);
	      syms[j].mutate(random, mut_rate);

  	 	 
	      // pick new host to infect, if one exists at the new location and does NOT already have a symbiont
	      //TODO: Make this work based on neighbors so that it changes correctly in the future

	      int newLoc = GetRandomCellID();
	      if (IsOccupied(newLoc) == true) {
		pop[newLoc]->AddSymbionts(*sym_baby, sym_limit);

	      }
	    } // if syms[j]
	  } // for each sym in syms
	} //else (ie not using moi)
      } //if org has syms
    } // for each in schedule
  } // Update()

};


