#ifndef EFFSYM_H
#define EFFSYM_H

#include "Symbiont.h"
#include "SymWorld.h"

class EfficientSymbiont: public Symbiont {
protected:

  /**
    * 
    * Purpose: Represents the efficeny of a symbiont. This has a multiplicable imapact on a 
    * symbiont's resource collection. 
    * 
  */  
  double efficiency;


public:
  /**
   * The constructor for efficent symbiont
   */
  EfficientSymbiont(emp::Ptr<emp::Random> _random, emp::Ptr<SymWorld> _world, emp::Ptr<SymConfigBase> _config, double _intval=0.0, double _points = 0.0, double _efficient = 0.1) : Symbiont(_random, _world, _config, _intval, _points) {
    efficiency = _efficient;
  }


  /**
   * Input:
   * 
   * Output:
   * 
   * Purpose:
   */  
  EfficientSymbiont(const EfficientSymbiont &) = default;


  /**
   * Input:
   * 
   * Output:
   * 
   * Purpose:
   */  
  EfficientSymbiont(EfficientSymbiont &&) = default;


  /**
   * Input:
   * 
   * Output:
   * 
   * Purpose:
   */  
  EfficientSymbiont() = default;


  /**
   * Input: None
   * 
   * Output: A double representing the symbiont's efficency. 
   * 
   * Purpose: Getting an efficent symbiont's efficency value. 
   */
  double GetEfficiency() {return efficiency;}


  /**
   * Input: A double representing the amount to be incremented to a symbiont's points.
   * 
   * Output: None 
   * 
   * Purpose: Incrementing an efficent symbiont's points. 
   */
  void AddPoints(double _in) {points += (_in * efficiency);}


  /**
   * Input: None
   * 
   * Output: None 
   * 
   * Purpose: Mutating the efficency of an efficent symbiont based upon the config 
   * setting for mutation size.  
   */
  void mutate(){
    Symbiont::mutate();
    if (random->GetDouble(0.0, 1.0) <= mut_rate) {
      efficiency += random->GetRandNormal(0.0, mut_size);
      if(efficiency < 0) efficiency = 0;
      else if (efficiency > 1) efficiency = 1;
    }
  }


  /**
   * Input: None
   * 
   * Output: None
   * 
   * Purpose: Mutating the efficency of an efficent symbiont based upon the config 
   * setting for horizontal mutation size.
   */
  void HorizMutate() {
      Symbiont::HorizMutate();
      if (random->GetDouble(0.0, 1.0) <= my_config->EFFICIENCY_MUT_RATE()) {
        efficiency += random->GetRandNormal(0.0, ht_mut_size);
        if(efficiency < 0) efficiency = 0;
        else if (efficiency > 1) efficiency = 1;
      }
    }


  /** 
   * Input: None
   * 
   * Output: An Empirical pointer to a newly created organism 
   * 
   * Purpose: Producing a new efficent organism.
   */
  emp::Ptr<Organism> reproduce() {
    //TODO: pull out creation of new offspring into separate method so the repeat code between here and Symbiont.h can be removed
    emp::Ptr<EfficientSymbiont> sym_baby = emp::NewPtr<EfficientSymbiont>(*this); //constructor that takes parent values                                             
    sym_baby->SetPoints(0);
    sym_baby->mutate();
    //mutate(); //mutate parent symbiont
    return sym_baby;
  }


  /**
   * Input: The world location that corresponds to the organism being processed. 
   * 
   * Output: None
   * 
   * Purpose: Mutating the efficency of an efficent symbiont based upon the config setting 
   * for horizontal mutation size.
   */
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