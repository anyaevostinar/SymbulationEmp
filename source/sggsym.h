#ifndef SGGSYM_H
#define SGGSYM_H

#include "Symbiont.h"
#include "SymWorld.h"

class SggSymbiont: public Symbiont {
protected:
  double sgg_donate = 0;


public:
  SggSymbiont(emp::Ptr<emp::Random> _random, emp::Ptr<SymWorld> _world, emp::Ptr<SymConfigBase> _config, double _intval=0.0, double _donation = 0.0, double _points = 0.0 ) : Symbiont(_random, _world, _config, _intval, _points),sgg_donate(_donation)
  {}
  SggSymbiont(const SggSymbiont &) = default;
  SggSymbiont(SggSymbiont &&) = default;
  SggSymbiont() = default;


  SggSymbiont & operator=(const SggSymbiont &) = default;
  SggSymbiont & operator=(SggSymbiont &&) = default;


  double Getdonation() {return sgg_donate;}
  void Setdonation(double _in) {sgg_donate = _in;}

  void mutate(){
    // double pre_value = interaction_val;
    if (random->GetDouble(0.0, 1.0) <= mut_rate) {
      interaction_val += random->GetRandNormal(0.0, mut_size);
      if(interaction_val < -1) interaction_val = -1;
      else if (interaction_val > 1) interaction_val = 1;
    }
    if (random->GetDouble(0.0, 1.0) <= mut_rate) {
      sgg_donate += random->GetRandNormal(0.0, mut_size);
      if(sgg_donate < 0) sgg_donate = 0;
      else if (sgg_donate > 1) sgg_donate = 1;
    }
    //if((pre_value*interaction_val) < 0) {
    //  std::cout << "switched2!" << std::endl;
    //}
  }

  void HorizMutate(){
    // double pre_value = interaction_val;
    if (random->GetDouble(0.0, 1.0) <= ht_mut_rate) {
      interaction_val += random->GetRandNormal(0.0, ht_mut_size);
      if(interaction_val < -1) interaction_val = -1;
      else if (interaction_val > 1) interaction_val = 1;
    }
    if (random->GetDouble(0.0, 1.0) <=ht_mut_rate) {
      sgg_donate += random->GetRandNormal(0.0, ht_mut_size);
      if(sgg_donate < 0) sgg_donate = 0;
      else if (sgg_donate > 1) sgg_donate = 1;
    }
  }

  double ProcessPool(){
    double symdonation = Getdonation();
    double symPortion = GetPoints();
    double hostreturn = symdonation*symPortion;
    SetPoints(symPortion-hostreturn);
    return hostreturn;
  }

  void Process(size_t location) {
    if (my_host.IsNull() && my_config->FREE_LIVING_SYMS()) {
      double resources = my_world->PullResources();
      AddPoints(resources);
    }
    if (h_trans) { //non-lytic horizontal transmission enabled
      if(GetPoints() >= sym_h_res) {
        // symbiont reproduces independently (horizontal transmission) if it has enough resources
        // new symbiont in this host with mutated value
        SetPoints(0); //TODO: test just subtracting points instead of setting to 0
        emp::Ptr<SggSymbiont> sym_baby = emp::NewPtr<SggSymbiont>(*this);
        sym_baby->SetPoints(0);
        sym_baby->HorizMutate();
        //HorizMutate();
        my_world->SymDoBirth(sym_baby, location);
      }
    }
    if (my_host.IsNull() && my_config->FREE_LIVING_SYMS()) {my_world->MoveFreeSym(location);}
  }

  emp::Ptr<Organism> reproduce() {
    emp::Ptr<SggSymbiont> sym_baby = emp::NewPtr<SggSymbiont>(*this); //constructor that takes parent values
    sym_baby->SetPoints(0);
    sym_baby->mutate();
    //mutate(); //mutate parent symbiont
    return sym_baby;
  }


std::string PrintSym(emp::Ptr<SggSymbiont>  org){
  if (org->GetPoints() < 0) return "-";
  double out_val = org->GetIntVal();

  // this prints the symbiont with two decimal places for easier reading
  std::stringstream temp;
  temp << std::fixed << std::setprecision(2) << out_val;
  std::string formattedstring = temp.str();
  return formattedstring;

  // return emp::to_string(out_val);  // creates a string without specifying format
}//Symbiont
};
#endif
