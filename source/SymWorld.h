#include "../../Empirical/source/Evolve/World.h"
#include "../../Empirical/source/data/DataFile.h"
#include "../../Empirical/source/tools/random_utils.h"
#include "../../Empirical/source/tools/Random.h"
#include "SymOrg.h"
#include <set>
#include <math.h>

class SymWorld : public emp::World<Host>{
private:
  double vertTrans = 0; 
  double mut_rate = 0;
  int sym_limit = -1;
  bool lysis = 0;
  bool h_trans = 0;
  int burst_size = 0;
  int burst_time = 0;
  double host_repro = 0;
  double sym_h_res = 0;
  double sym_lysis_res = 0;
  emp::Random random;
  
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_hostintval;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_symintval;
  emp::Ptr<emp::DataMonitor<int>> data_node_hostcount;
  emp::Ptr<emp::DataMonitor<int>> data_node_symcount;


public:
  //set fun_print_org to equal function that prints hosts/syms correctly
  SymWorld(emp::Random &random) : emp::World<Host>(random), random(random) {
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
  }
  
  void SetVertTrans(double vt) {vertTrans = vt;}
  void SetMutRate(double mut) {mut_rate = mut;}
  void SetSymLimit(int num) {sym_limit = num;}
  void SetLysisBool(bool val) {lysis = val;}
  void SetHTransBool(bool val) {h_trans = val;}
  void SetBurstSize(int val) {burst_size = val;}
  void SetBurstTime(int val) {burst_time = val;}
  void SetHostRepro(double val) {host_repro = val;}
  void SetSymHRes(double val) {sym_h_res = val;}
  void SetSymLysisRes(double val) {sym_lysis_res = val;}


  bool WillTransmit() {
    return random.GetDouble(0.0, 1.0) <= vertTrans;
  }

  size_t GetNeighborHost (size_t i) {
    size_t newLoc = GetRandomNeighborPos(i).GetIndex();
    while (newLoc == i) {
      newLoc = GetRandomNeighborPos(i).GetIndex();
    }
    return newLoc;
  }

  void InjectSymbiont(Symbiont newSym){
    int newLoc = GetRandomCellID();
    if(IsOccupied(newLoc) == true) {
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
    node.SetupBins(-1.0, 1.1, 21);

    file.AddVar(update, "update", "Update");
    file.AddMean(node, "mean_intval", "Average host interaction value");
    file.AddTotal(node1, "count", "Total number of hosts");
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
        for (size_t i = 0; i< pop.size(); i++){
          if (IsOccupied(i)){
	    emp::vector<Symbiont>& syms = pop[i]->GetSymbionts();
	    int sym_size = syms.size();
	    for(size_t j=0; j< sym_size; j++){
	      data_node_symintval->AddDatum(syms[j].GetIntVal());
	    }//close for
	  }//close if
	}//close for
      });
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
      if (pop[i]->GetPoints() >= host_repro ) {  // if host has more points than required for repro                                                                                                   
        // will replicate & mutate a random offset from parent values
        // while resetting resource points for host and symbiont to zero                                              

        Host *host_baby = new Host(pop[i]->GetIntVal());
        host_baby->mutate(random, mut_rate);
        pop[i]->mutate(random, mut_rate); //parent mutates and loses current resources, ie new organism but same symbiont  
        pop[i]->SetPoints(0);
	pop[i]->SetSymbionts({});

        //Now check if symbionts get to vertically transmit
        for(size_t j = 0; j< (pop[i]->GetSymbionts()).size(); j++){
          Symbiont parent = ((pop[i]->GetSymbionts()))[j];
          
          if (WillTransmit()) { //Vertical transmission!  
            
            Symbiont * sym_baby = new Symbiont(parent.GetIntVal(), 0.0); //constructor that takes parent values                                             
            sym_baby->mutate(random, mut_rate);
            parent.mutate(random, mut_rate); //mutate parent symbiont                                   
            host_baby->AddSymbionts(*sym_baby, sym_limit);
          } //end will transmit
        } //end for loop for each symbiont
	if(host_baby->GetReproSymbionts().size() != 0){
	  std::cout << "sneaky syms " << host_baby->GetReproSymbionts().size() << std::endl;
	}
        DoBirth(*host_baby, i); //Automatically deals with grid
      }

      if (pop[i]->HasSym()) { //check each sym for horizontal transmission and lysis
        emp::vector<Symbiont>& syms = pop[i]->GetSymbionts();
        for(size_t j = 0; j < syms.size(); j++){
          
          if(lysis) { //lysis enabled, checking for lysis
            if (syms[j].GetBurstTimer() >= burst_time) { //time to lyse!
              //        std::cout << "Lysis time!" << std::endl;
              //distribute all the offspring in the repro offspring list 
              //TODO: SymDoBirth should replace the below
              emp::vector<Symbiont>& repro_syms = (pop[i] ->GetReproSymbionts());
              for(size_t r = 0; r < repro_syms.size(); r++){
                size_t newLoc = GetNeighborHost(i);
                if (IsOccupied(newLoc) == true)
                  pop[newLoc]->AddSymbionts(repro_syms[r], sym_limit);
              }
              DoDeath(i); //kill organism
              break;  //continue to next organism

            } else {
              syms[j].IncBurstTimer();
              //std::cout << "Should have incremented " << syms[j].GetBurstTimer() << std::endl;
              int offspring_per_tick = burst_size/burst_time;
              for(size_t o=0; o<= offspring_per_tick; o++) {
                if(syms[j].GetPoints() >= sym_lysis_res) { //check if sym has resources to produce offspring
                  //if so, make a new symbiont and add it to Repro sym list
                  Symbiont *sym_baby = new Symbiont(syms[j].GetIntVal());
                  sym_baby->mutate(random, mut_rate);
                  syms[j].mutate(random, mut_rate);
                  pop[i]->AddReproSym(*sym_baby);
                  syms[j].SetPoints(syms[j].GetPoints() - sym_lysis_res);
                }
                else
                  break;
              }
            } 
          }

          if(h_trans) { //non-lytic horizontal transmission enabled
            if (syms[j].GetPoints() >= sym_h_res) {
              // symbiont reproduces independently (horizontal transmission) if it has >= 100 resources (by default)
              // new symbiont in this host with mutated value
              // TODO: Make SymDoBirth instead of injecting
              syms[j].SetPoints(0);
              //TODO: test just subtracting points instead of setting to 0
              Symbiont *sym_baby = new Symbiont(syms[j].GetIntVal());
              sym_baby->mutate(random, mut_rate);
              syms[j].mutate(random, mut_rate);

              
              // pick new host to infect, if one exists at the new location and isn't at the limit

              int newLoc = GetNeighborHost(i);
              if (IsOccupied(newLoc) == true)
                pop[newLoc]->AddSymbionts(*sym_baby, sym_limit);

            } // if syms[j]
          } // non-lytic horizontal transmission enabled
        } //for each sym in syms
      } //if org has syms
    } // for each cell in schedule
  } // Update()
};// SymWorld class
