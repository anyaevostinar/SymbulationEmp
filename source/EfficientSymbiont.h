#ifndef EFFSYM_H
#define EFFSYM_H

#include "Symbiont.h"
#include "SymWorld.h"

class EfficientSymbiont: public Symbiont {
protected:
  double efficiency;


public:

  EfficientSymbiont(emp::Ptr<emp::Random> _random, emp::Ptr<SymWorld> _world, emp::Ptr<SymConfigBase> _config, double _intval=0.0, double _points = 0.0, double _efficient = 0.1) : Symbiont(_random, _world, _config, _intval, _points) {
    efficiency = _efficient;
  }
  EfficientSymbiont(const EfficientSymbiont &) = default;
  EfficientSymbiont(EfficientSymbiont &&) = default;
  EfficientSymbiont() = default;

  double GetEfficiency() {
      return efficiency;
  }

  void AddPoints(double _in) {
      points += (_in * efficiency);
  }

  void mutate(){
    Symbiont::mutate();
    if (random->GetDouble(0.0, 1.0) <= mut_rate) {
      efficiency += random->GetRandNormal(0.0, mut_size);
      if(efficiency < 0) efficiency = 0;
      else if (efficiency > 1) efficiency = 1;
    }
  }

  void HorizMutate() {
      Symbiont::HorizMutate();
      //
      if (random->GetDouble(0.0, 1.0) <= my_config->EFFICIENCY_MUT_RATE()) {
        efficiency += random->GetRandNormal(0.0, ht_mut_size);
        if(efficiency < 0) efficiency = 0;
        else if (efficiency > 1) efficiency = 1;
      }
    }

  emp::Ptr<Organism> reproduce() {
    //TODO: pull out creation of new offspring into separate method so the repeat code between here and Symbiont.h can be removed
    emp::Ptr<EfficientSymbiont> sym_baby = emp::NewPtr<EfficientSymbiont>(*this); //constructor that takes parent values                                             
    sym_baby->SetPoints(0);
    sym_baby->mutate();
    //mutate(); //mutate parent symbiont
    return sym_baby;
  }

  void process(size_t location) {
    if (h_trans) { //non-lytic horizontal transmission enabled
      if(GetPoints() >= sym_h_res) {
        // symbiont reproduces independently (horizontal transmission) if it has enough resources
        // new symbiont in this host with mutated value
        SetPoints(0); //TODO: test just subtracting points instead of setting to 0
        emp::Ptr<EfficientSymbiont> sym_baby = emp::NewPtr<EfficientSymbiont>(*this);
        sym_baby->SetPoints(0);
        sym_baby->HorizMutate();
        //HorizMutate();
        
        my_world->SymDoBirth(sym_baby, location);

      }
    }
  }


};

#endif