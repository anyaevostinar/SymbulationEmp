#ifndef PGGSYM_H
#define PGGSYM_H

#include "../default_mode/Symbiont.h"
#include "PggWorld.h"

class PGGSymbiont: public Symbiont {
protected:

  /**
    *
    * Purpose: #
    *
  */
  double Pgg_donate = 0;

  /**
    *
    * Purpose: Represents the world that the pgg symbionts are living in.
    *
  */
  emp::Ptr<PggWorld> my_world = NULL;

public:
  PGGSymbiont(emp::Ptr<emp::Random> _random, emp::Ptr<PggWorld> _world, emp::Ptr<SymConfigBase> _config, double _intval=0.0, double _donation = 0.0, double _points = 0.0 ) : Symbiont(_random, _world, _config, _intval, _points),Pgg_donate(_donation)
  {my_world = _world;}

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
    Symbiont::mutate();
    if (random->GetDouble(0.0, 1.0) <= my_config->MUTATION_RATE()) {
      Pgg_donate += random->GetRandNormal(0.0, my_config->MUTATION_SIZE());
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
   * Input: None
   *
   * Output: The pointer to the newly created organism
   *
   * Purpose: To produce a new phage, identical to the original
   */
  emp::Ptr<Organism> makeNew() {
    emp::Ptr<PGGSymbiont> sym_baby = emp::NewPtr<PGGSymbiont>(random, my_world, my_config, GetIntVal());
    sym_baby->SetInfectionChance(GetInfectionChance());
    sym_baby->Setdonation(GetDonation());
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
