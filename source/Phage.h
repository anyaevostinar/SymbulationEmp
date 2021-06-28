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

  Phage(emp::Ptr<emp::Random> _random, emp::Ptr<SymWorld> _world, emp::Ptr<SymConfigBase> _config, double _intval=0.0, double _points = 0.0) : Symbiont(_random, _world, _config, _intval, _points) {
    burst_time = my_config->BURST_TIME();
    sym_lysis_res = my_config->SYM_LYSIS_RES();
    lysis = my_config->LYSIS();
  }
  Phage(const Phage &) = default;
  Phage(Phage &&) = default;
  Phage() = default;

  double GetBurstTimer() {return burst_timer;}
  void IncBurstTimer() {

    burst_timer += random->GetRandNormal(1.0, 1.0);

  }
  void SetBurstTimer(int _in) {burst_timer = _in;}

  bool IsPhage() {return true;}


  double GetIntVal() const {
    return -1; //non-lysogenized lytic phage shuts down host reproduction if possible

  }

  emp::Ptr<Organism> reproduce() {
    emp::Ptr<Phage> sym_baby = emp::NewPtr<Phage>(*this); //constructor that takes parent values
    sym_baby->SetPoints(0);
    sym_baby->SetBurstTimer(0);
    sym_baby->mutate();
    mutate(); //mutate parent symbiont
    return sym_baby;
  }

  void process(double resources, size_t location) {
    if(lysis && GetHost() != NULL) { //lysis enabled, checking for lysis
      if(GetBurstTimer() >= burst_time) { //time to lyse!
        emp::vector<emp::Ptr<Organism>>& repro_syms = my_host->GetReproSymbionts();
        //Record the burst size
	      // update this for my_world: data_node_burst_size -> AddDatum(repro_syms.size());
        for(size_t r=0; r<repro_syms.size(); r++) {
          my_world->SymDoBirth(repro_syms[r], location);
        }
        my_host->ClearReproSyms();
        my_host->SetDead();
        return;

      } else {
        IncBurstTimer();
        if(sym_lysis_res == 0) {
          std::cout << "Lysis with a sym_lysis_res of 0 leads to an \
           infinite loop, please change" << std::endl;
          std::exit(1);
        }
        while(GetPoints() >= sym_lysis_res) {
          emp::Ptr<Organism> sym_baby = reproduce();
          my_host->AddReproSym(sym_baby);
          SetPoints(GetPoints() - sym_lysis_res);
        }
      }
    } else {
      my_world->MoveFreeSym(location);
    }

  }

};

#endif
