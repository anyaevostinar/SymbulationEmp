#ifndef PHAGE_H
#define PHAGE_H

#include "Symbiont.h"
#include "SymWorld.h"

class Phage: public Symbiont {
protected:
  double burst_timer = 0;
  bool lysis = true;
  double burst_time = 60;
  double sym_lysis_res = 15;


public:

  Phage(emp::Ptr<emp::Random> _random, emp::Ptr<SymWorld> _world, double _intval=0.0,
    double _points = 0.0, double _h_res = 100.0, bool _h_trans = true, 
    double _mut_rate = 0.002, double _bt = 60, bool _lysis = true, double _l_res = 15) 
    : Symbiont(_random, _world, _intval, _points, _h_res, _h_trans, 
    _mut_rate), burst_time(_bt), sym_lysis_res(_l_res) {}
  Phage(const Phage &) = default;
  Phage(Phage &&) = default;

  double GetBurstTimer() {return burst_timer;}
  void IncBurstTimer() {

    burst_timer += random->GetRandNormal(1.0, 0.5);
    
  }
  void SetBurstTimer(int _in) {burst_timer = _in;}


  double GetIntVal() const {
    return -1; //non-lysogenized lytic phage shuts down host reproduction if possible
    
  }

  void process(size_t location) {
    if(lysis) { //lysis enabled, checking for lysis
      if(GetBurstTimer() >= burst_time) { //time to lyse!
        emp::vector<emp::Ptr<Organism>>& repro_syms = my_host->GetReproSymbionts();
        //Record the burst size
	      // update this for my_world: data_node_burst_size -> AddDatum(repro_syms.size());
        for(size_t r=0; r<repro_syms.size(); r++) {
          my_world->SymDoBirth(repro_syms[r], location);
        }
        my_world->DoDeath(location);
        
      } else {
        IncBurstTimer();
        if(sym_lysis_res == 0) {
          std::cout << "Lysis with a sym_lysis_res of 0 leads to an \
           infinite loop, please change" << std::endl;
          std::exit(1);
        }
        while(Symbiont::GetPoints() >= sym_lysis_res) {
          emp::Ptr<Organism> sym_baby = new Phage(*this);
          sym_baby->SetPoints(0);
          sym_baby->mutate();
          Symbiont::mutate(); // TODO: test removing this since it's weird
          my_host->AddReproSym(sym_baby);
          Symbiont::SetPoints(GetPoints() - sym_lysis_res);
        }
      }
    }
  }

};

#endif