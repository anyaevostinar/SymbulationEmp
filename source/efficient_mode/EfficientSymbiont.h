#ifndef EFFSYM_H
#define EFFSYM_H

#include "../default_mode/Symbiont.h"
#include "EfficientWorld.h"
#include "EfficientHost.h"

class EfficientSymbiont: public Symbiont {
protected:

  /**
    *
    * Purpose: Represents the efficiency of a symbiont. This has a multiplicable impact on a
    * symbiont's resource collection.
    *
  */
  double efficiency;

  /**
    *
    * Purpose: Represents the standard deviation of the values
    * chosen as mutations of a symbiont's interaction value when
    * horizontal transmission is occuring. .
    *
  */
  double ht_mut_size = 0.002;

  /**
    *
    * Purpose: Represents the probability (0-1) of mutation occurring
    * during horizontal transmission.
    *
  */
  double ht_mut_rate = 0;

  /**
    *
    * Purpose: Represents the probability (0-1) of mutation occurring
    * during horizontal transmission for the efficiency trait.
    *
  */
  double eff_mut_rate = 0;

  /**
    *
    * Purpose: Represents the world that the efficient symbionts are living in.
    *
  */
  emp::Ptr<EfficientWorld> my_world = NULL;
public:
  /**
   * The constructor for efficient symbiont
   */
  EfficientSymbiont(emp::Ptr<emp::Random> _random, emp::Ptr<EfficientWorld> _world, emp::Ptr<SymConfigBase> _config, double _intval=0.0, double _points = 0.0, double _efficient = 0.1) : Symbiont(_random, _world, _config, _intval, _points) {
    efficiency = _efficient;
    my_world = _world;
    if(my_config->HORIZ_MUTATION_RATE() < 0){
      ht_mut_rate = mut_rate;
    } else {
      ht_mut_rate = my_config->HORIZ_MUTATION_RATE();
    }
    if(my_config->HORIZ_MUTATION_SIZE() < 0) {
      ht_mut_size = mut_size;
    } else {
      ht_mut_size = my_config->HORIZ_MUTATION_SIZE();
    }
    if(my_config->EFFICIENCY_MUT_RATE() < 0) {
      eff_mut_rate = ht_mut_rate;
    } else {
      eff_mut_rate = my_config->EFFICIENCY_MUT_RATE();
    }
  }


  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To force a copy constructor to be generated by the compiler.
   */
  EfficientSymbiont(const EfficientSymbiont &) = default;


  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To force a move constructor to be generated by the compiler
   */
  EfficientSymbiont(EfficientSymbiont &&) = default;


  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To tell the compiler to use its default generated variants of the constructor
   */
  EfficientSymbiont() = default;


  /**
   * Input: None
   *
   * Output: A double representing the symbiont's efficiency.
   *
   * Purpose: Getting an efficient symbiont's efficiency value.
   */
  double GetEfficiency() {return efficiency;}


  /**
   * Input: A double representing the amount to be incremented to a symbiont's points.
   *
   * Output: None
   *
   * Purpose: Incrementing an efficient symbiont's points.
   * The points are adjusted by the efficiency of the symbiont.
   */
  void AddPoints(double _in) {points += (_in * efficiency);}


  /**
   * Input: String indicating mode, either "vertical" or "horizontal"
   *
   * Output: None
   *
   * Purpose: Mutating the efficiency of an efficient symbiont based upon the config
   * setting for mutation size.
   */
  void mutate(std::string mode){
    double local_size;
    double local_rate;
    if(mode == "vertical"){
      local_rate = mut_rate;
      local_size = mut_size;
    } else if(mode == "horizontal") {
      local_rate = ht_mut_rate;
      local_size = ht_mut_size;
    } else {
      throw "Illegal argument passed to mutate in EfficientSymbiont";
    }
    if (random->GetDouble(0.0, 1.0) <= local_rate) {
      interaction_val += random->GetRandNormal(0.0, local_size);
      if(interaction_val < -1) interaction_val = -1;
      else if (interaction_val > 1) interaction_val = 1;

      //also modify infection chance, which is between 0 and 1
      if(my_config->FREE_LIVING_SYMS()){
        infection_chance += random->GetRandNormal(0.0, local_size);
        if (infection_chance < 0) infection_chance = 0;
        else if (infection_chance > 1) infection_chance = 1;
      }
    }
    if (random->GetDouble(0.0, 1.0) <= eff_mut_rate) {
      efficiency += random->GetRandNormal(0.0, local_size);
      if(efficiency < 0) efficiency = 0;
      else if (efficiency > 1) efficiency = 1;
    }
  }



  /**
   * Input: None.
   *
   * Output: A new bacterium with same properties as this bacterium.
   *
   * Purpose: To avoid creating an organism via constructor in other methods.
   */
  emp::Ptr<Organism> makeNew(){
    return emp::NewPtr<EfficientSymbiont>(*this); //constructor that takes parent values
  }

  /**
   * Input: String to indicate the mode of transmission, either vertical or horizontal
   *
   * Output: The pointer to the newly created organism
   *
   * Purpose: To produce a new symbiont
   */
  emp::Ptr<Organism> reproduce(std::string mode) {
    emp::Ptr<Organism> sym_baby = makeNew();
    sym_baby->SetPoints(0);
    sym_baby->mutate(mode);
    return sym_baby;
  }

  /**
   * Input: The pointer to the organism that is the new host baby
   *
   * Output: None
   *
   * Purpose: To allow for vertical transmission to occur
   */
  void VerticalTransmission(emp::Ptr<Organism> host_baby) {
    if((my_world->WillTransmit()) && GetPoints() >= my_config->SYM_VERT_TRANS_RES()){ //if the world permits vertical tranmission and the sym has enough resources, transmit!
      emp::Ptr<Organism> sym_baby = reproduce("vertical");
      host_baby->AddSymbiont(sym_baby);
    }
  }

  /**
   * Input: The location of the organism (and it's Host) as a size_t
   *
   * Output: None
   *
   * Purpose: To check and allow for horizontal transmission to occur
   */
  void HorizontalTransmission(size_t location) {
    if (h_trans) { //non-lytic horizontal transmission enabled
      if(GetPoints() >= sym_h_res) {
        // symbiont reproduces independently (horizontal transmission) if it has enough resources
        // new symbiont in this host with mutated value
        SetPoints(0); //TODO: test just subtracting points instead of setting to 0
        emp::Ptr<Organism> sym_baby = reproduce("horizontal");
        my_world->SymDoBirth(sym_baby, location);
      }
    }
  }
};
#endif