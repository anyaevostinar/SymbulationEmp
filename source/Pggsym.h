#ifndef PGGSYM_H
#define PGGSYM_H

#include "Symbiont.h"
#include "SymWorld.h"

class PGGSymbiont: public Symbiont {
protected:

  /**
    *
    * Purpose: #
    *
  */
  double Pgg_donate = 0;


public:
  PGGSymbiont(emp::Ptr<emp::Random> _random, emp::Ptr<SymWorld> _world, emp::Ptr<SymConfigBase> _config, double _intval=0.0, double _donation = 0.0, double _points = 0.0 ) : Symbiont(_random, _world, _config, _intval, _points),Pgg_donate(_donation)
  {}

    /**
   * Input: #
   *
   * Output:
   *
   * Purpose:
   */
  PGGSymbiont(const PGGSymbiont &) = default;


  /**
   * Input: #
   *
   * Output:
   *
   * Purpose:
   */
  PGGSymbiont(PGGSymbiont &&) = default;


  /**
   * Input: #
   *
   * Output:
   *
   * Purpose:
   */
  PGGSymbiont() = default;


  /**
   * Input: #
   *
   * Output:
   *
   * Purpose:
   */
  PGGSymbiont & operator=(const PGGSymbiont &) = default;


  /**
   * Input: #
   *
   * Output:
   *
   * Purpose:
   */
  PGGSymbiont & operator=(PGGSymbiont &&) = default;


  /**
   * Input: #
   *
   * Output:
   *
   * Purpose:
   */
  double GetDonation() {return Pgg_donate;}


  /**
   * Input: #
   *
   * Output:
   *
   * Purpose:
   */
  void Setdonation(double _in) {Pgg_donate = _in;}


  /**
   * Input: #
   *
   * Output:
   *
   * Purpose:
   */
  void mutate(){
    // double pre_value = interaction_val;
    Symbiont::mutate();
    if (random->GetDouble(0.0, 1.0) <= mut_rate) {
      Pgg_donate += random->GetRandNormal(0.0, mut_size);
      if(Pgg_donate < 0) Pgg_donate = 0;
      else if (Pgg_donate > 1) Pgg_donate = 1;
    }
    //if((pre_value*interaction_val) < 0) {
    //  std::cout << "switched2!" << std::endl;
    //}
  }


  /**
   * Input: #
   *
   * Output:
   *
   * Purpose:
   */
  void HorizMutate(){
    // double pre_value = interaction_val;
    Symbiont::HorizMutate();
    if (random->GetDouble(0.0, 1.0) <= ht_mut_rate) {
      Pgg_donate += random->GetRandNormal(0.0, ht_mut_size);
      if(Pgg_donate < 0) Pgg_donate = 0;
      else if (Pgg_donate > 1) Pgg_donate = 1;
    }
  }


  /**
   * Input: #
   *
   * Output:
   *
   * Purpose:
   */
  double ProcessPool(){
    double symdonation = GetDonation();
    double symPortion = GetPoints();
    double hostreturn = symdonation*symPortion;
    SetPoints(symPortion-hostreturn);
    return hostreturn;
  }


  /**
   * Input: #
   *
   * Output:
   *
   * Purpose:
   */
  void Process(size_t location) {
    if (my_host.IsNull() && my_config->FREE_LIVING_SYMS()) {
      double resources = my_world->PullResources(my_config->FREE_SYM_RES_DISTRIBUTE());
      AddPoints(resources);
    }
    if (h_trans) { //non-lytic horizontal transmission enabled
      if(GetPoints() >= sym_h_res) {
        // symbiont reproduces independently (horizontal transmission) if it has enough resources
        // new symbiont in this host with mutated value
        SetPoints(0); //TODO: test just subtracting points instead of setting to 0
        emp::Ptr<PGGSymbiont> sym_baby = emp::NewPtr<PGGSymbiont>(*this);
        sym_baby->SetPoints(0);
        sym_baby->HorizMutate();
        //HorizMutate();
        my_world->SymDoBirth(sym_baby, location);
      }
    }
    if (my_host.IsNull() && my_config->FREE_LIVING_SYMS()) {my_world->MoveFreeSym(location);}
  }


  /**
   * Input: #
   *
   * Output:
   *
   * Purpose:
   */
  emp::Ptr<Organism> reproduce() {
    emp::Ptr<PGGSymbiont> sym_baby = emp::NewPtr<PGGSymbiont>(*this); //constructor that takes parent values
    sym_baby->SetPoints(0);
    sym_baby->mutate();
    //mutate(); //mutate parent symbiont
    return sym_baby;
  }


  /**
   * Input: #
   *
   * Output:
   *
   * Purpose:
   */
  std::string PrintSym(emp::Ptr<PGGSymbiont>  org){
    if (org->GetPoints() < 0) return "-";
    double out_val = org->GetIntVal();
    double donate_val = org->GetDonation();
    // this prints the symbiont with two decimal places for easier reading
    std::stringstream temp;
    temp << std::fixed << std::setprecision(2) << "Interaction value: " << out_val << " Donation value: " << donate_val;
    std::string formattedstring = temp.str();
    return formattedstring;
  }//Pggsym
};
#endif
