#include "source/Evo/World.h"
#include "source/tools/Random.h"
#include <set>
#include "SymOrg.h"
#include "source/tools/random_utils.h"
#include "source/Evo/World_file.h"

class SymWorld : public emp::World<Host>{
 private:
  const double synergy = 5;
  double vertTrans = 0; 
  double mut_rate = 0;
  emp::Random random;
  
  Ptr<DataMonitor<double, emp::data::Histogram>> data_node_vt;

 public:
  //set fun_print_org to equal function that prints hosts/syms correctly
 SymWorld(emp::Random &random) : emp::World<Host>(random) {
    fun_print_org = [](Host & org, std::ostream & os) {
      os << PrintHost(&org);
    };
  }
  
  void SetVertTrans(double vt) {
    vertTrans = vt;
  }
  void SetMutRate(double mut) {
    mut_rate = mut;
  }


  bool WillTransmit() {
    if (random.GetDouble(0.0, 1.0) <= vertTrans) {
      return true;
    }  else {
      return false;
    }
    

  }

  World_file & SetupVTFile(const std::string & filename) {
    auto & file = SetupFile(filename);
    auto & node = GetVTDataNode(); 
    node.SetupBins(-1.0, 1.0, 20);
    file.AddVar(update, "update", "Update");
    file.AddMean(node, "mean_vt", "Average organism vertical transmission");
    file.AddHistBin(node, 0, "Hist_-1", "Count for histogram bin -1 to <-0.9");
    file.AddHistBin(node, 0, "Hist_-0.9", "Count for histogram bin -0.9 to <-0.8");
    file.AddHistBin(node, 0, "Hist_-0.8", "Count for histogram bin -0.8 to <-0.7");
    file.AddHistBin(node, 0, "Hist_-0.7", "Count for histogram bin -0.7 to <-0.6");
    file.AddHistBin(node, 0, "Hist_-0.6", "Count for histogram bin -0.6 to <-0.5");
    file.AddHistBin(node, 0, "Hist_-0.5", "Count for histogram bin -0.5 to <-0.4");
    file.AddHistBin(node, 0, "Hist_-0.4", "Count for histogram bin -0.4 to <-0.3");
    file.AddHistBin(node, 0, "Hist_-0.3", "Count for histogram bin -0.3 to <-0.2");
    file.AddHistBin(node, 0, "Hist_-0.2", "Count for histogram bin -0.2 to <-0.1");
    file.AddHistBin(node, 0, "Hist_-0.1", "Count for histogram bin -0.1 to <0.0");
    file.AddHistBin(node, 0, "Hist_0.0", "Count for histogram bin 0.0 to <0.1");
    file.AddHistBin(node, 0, "Hist_0.1", "Count for histogram bin 0.1 to <0.2");
    file.AddHistBin(node, 0, "Hist_0.2", "Count for histogram bin 0.2 to <0.3");
    file.AddHistBin(node, 0, "Hist_0.3", "Count for histogram bin 0.3 to <0.4");
    file.AddHistBin(node, 0, "Hist_0.4", "Count for histogram bin 0.4 to <0.5");
   
    //TODO:Add the rest of the bins
    //TODO: histogram for hosts and symbionts both, more data in data node?

    file.PrintHeaderKeys();
  }

  double CalcVT(size_t i) {
    return pop[i]->GetVertTrans(); //TODO: write GetVertTrans
  }

  DataMonitor<double> GetVTDataNode() {
    if (!data_node_vt) {
      data_node_vt.New();
      OnUpdate(
	       [this](size_t){
		 data_node_vt->Reset();
		 for (size_t i = 0; i< pop.size(); i++) {
		   if (IsOccupied(i)) data_node_vt->AddDatum(CalcVT(i));
		 }
	       }
	       );
    }
    return *data_node_vt;
  }
  
  void Update(size_t new_resources=10) {
    emp::World<Host>::Update();
    double symIntVal;

    //TODO: put in fancy scheduler at some point
    
    vector<size_t> schedule = emp::GetPermutation(random, GetSize());
    
    // divvy up and distribute resources to host and symbiont in each cell 
    for (size_t i : schedule) {
      if (IsOccupied(i) == false) continue;  // no organism at that cell
  	   

      //combine for loops and put everyhting into Host.Process
      ProcessID(i, random); //whatever it passes to process
      //TODO: feature request process shuffle
      //TODO: write process
      //TODO: async repro and mutation feature request
      //TODO: write host equality override using this to access pointer value
      

      
      if (pop[i]->HasSym()) {
	symIntVal = pop[i]->GetSymbiont().GetIntVal();
      } else {
	symIntVal = 0.0;
      }
  	  	
      pop[i]->DistribResources(new_resources, synergy); // --- USING NEW FUNCTION!!
    }
 
    // host reproduces if it can
    for (size_t i = 0; i < GetSize(); i++) {
      if (IsOccupied(i) == false) continue;  // nothing to replicate!
      
      if (pop[i]->GetPoints() >= 100 ) {  // host replication
	// will replicate & mutate a random offset from parent values
	// while resetting resource points for host and symbiont to zero
	Symbiont *sym_baby;
	if (pop[i]->HasSym() && WillTransmit()) { //Vertican transmission!
	  sym_baby = new Symbiont(pop[i]->GetSymbiont().GetIntVal(), 0.0); //constructor that takes parent values
	  sym_baby->mutate(random, mut_rate);
	  pop[i]->GetSymbiont().mutate(random, mut_rate); //mutate parent symbiont
	}else{
	  sym_baby = new Symbiont(0.0, -1.0); 
	}

	//move mutations to within host and symbiont	
	Host *host_baby = new Host(pop[i]->GetIntVal(),*sym_baby,std::set<int>(), 0.0);
	host_baby->mutate(random, mut_rate);
	pop[i]->mutate(random, mut_rate); //parent mutates and loses current resources, ie new organism but same symbiont
	pop[i]->SetPoints(0);
	//TODO: is this how I did it for the dissertation? reset parent completely?
	InjectAt(*host_baby, GetRandomCellID());


      }
      if (pop[i]->HasSym() && pop[i]->GetSymbiont().GetPoints() >= 100) {  
	//TODO: check symbiont reproduction value
	// symbiont reproduces independently (horizontal transmission) if it has >= 100 resources
	// new symbiont in this host with mutated value
	pop[i]->ResetSymPoints();
	Symbiont *sym_baby = new Symbiont(pop[i]->GetSymbiont());
	sym_baby->mutate(random, mut_rate);
	pop[i]->GetSymbiont().mutate(random, mut_rate);

  	 	 
	// pick new host to infect, if one exists at the new location and does NOT already have a symbiont
	int newLoc = GetRandomCellID();
	if (IsOccupied(newLoc) == true) {
 		   
	  if (!pop[newLoc]->HasSym()) {
	    pop[newLoc]->SetSymbiont(*sym_baby);

	  }
	}
      }	 
    }
  	 	
  }
};


