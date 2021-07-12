#ifndef SYMBIONT_H
#define SYMBIONT_H

#include "../../Empirical/include/emp/math/Random.hpp"
#include "../../Empirical/include/emp/tools/string_utils.hpp"
#include "SymWorld.h"
#include <set>
#include <iomanip> // setprecision
#include <sstream> // stringstream


class Symbiont: public Organism {
protected:
  double interaction_val = 0;
  double points = 0;
  double sym_h_res = 100;
  bool h_trans = true;
  double mut_size = 0.002;
  double ht_mut_size = 0.002;
  double mut_rate = 0;
  double ht_mut_rate = 0;
  bool dead = false;

  emp::Ptr<emp::Random> random = NULL;
  emp::Ptr<SymWorld> my_world = NULL;
  emp::Ptr<Organism> my_host = NULL;
  emp::Ptr<SymConfigBase> my_config = NULL;

public:
  Symbiont(emp::Ptr<emp::Random> _random, emp::Ptr<SymWorld> _world, emp::Ptr<SymConfigBase> _config, double _intval=0.0, double _points = 0.0) :  interaction_val(_intval), points(_points), random(_random), my_world(_world), my_config(_config) {
    sym_h_res = my_config->SYM_HORIZ_TRANS_RES();
    h_trans = my_config->HORIZ_TRANS();
    mut_rate = my_config->MUTATION_RATE();
    if(my_config->HORIZ_MUTATION_RATE() < 0){
      ht_mut_rate = mut_rate;
    } else {
      ht_mut_rate = my_config->HORIZ_MUTATION_RATE();
    }
    mut_size = my_config->MUTATION_SIZE();
    if(my_config->HORIZ_MUTATION_SIZE() < 0) {
      ht_mut_size = mut_size;
    } else {
      ht_mut_size = my_config->HORIZ_MUTATION_SIZE();
    }
    if ( _intval > 1 || _intval < -1) {
       throw "Invalid interaction value. Must be between -1 and 1";   // Exception for invalid interaction value
    };
  }
  Symbiont(const Symbiont &) = default;
  Symbiont(Symbiont &&) = default;
  Symbiont() = default;


  Symbiont & operator=(const Symbiont &) = default;
  Symbiont & operator=(Symbiont &&) = default;

  double GetIntVal() const {
    return interaction_val;}
  double GetPoints() {return points;}
  bool IsPhage() {return false;}
  bool IsHost() {return false;}
  emp::Ptr<Organism> GetHost() {return my_host;}
  //  std::set<int> GetResTypes() const {return res_types;}

  void SetDead() { dead = true; }
  bool GetDead() { return dead; }

  void SetIntVal(double _in) {
    if ( _in > 1 || _in < -1) {
       throw "Invalid interaction value. Must be between -1 and 1";   // Exception for invalid interaction value
     }
     else {
        interaction_val = _in;
     }
  }
  void SetPoints(double _in) {points = _in;}
  void AddPoints(double _in) { points += _in;}
  void SetHost(emp::Ptr<Organism> _in) {my_host = _in;}


  //void SetResTypes(std::set<int> _in) {res_types = _in;}

  void uponInjection(){
    //does nothing for now, added for backwards compatibility from phage to symbiont
  } 

  //TODO: change everything to camel case
  void mutate(){
    // double pre_value = interaction_val;
    if (random->GetDouble(0.0, 1.0) <= mut_rate) {
      interaction_val += random->GetRandNormal(0.0, mut_size);
      if(interaction_val < -1) interaction_val = -1;
      else if (interaction_val > 1) interaction_val = 1;
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
  }

  double ProcessResources(double sym_piece){
    //TODO - not what it should be right now, it is supposed to be only calculating
    //symPortion, but it's also doing hostPortion. Need to figure out how to 
    //calculate and separate out so I'm not just copying and pasting code
    double symIntVal = GetIntVal();
    double hostIntVal = my_host->GetIntVal();
    double hostPortion = 0.0;
    double hostDonation = 0.0;
    double symPortion = 0.0;
    double symReturn = 0.0;
    double bonus = my_config->SYNERGY();

    if (hostIntVal >= 0 && symIntVal >= 0){ //mutualistic relationship
      hostDonation = sym_piece * hostIntVal;
      hostPortion = sym_piece - hostDonation;

      symReturn = (hostDonation * symIntVal) * bonus;
      symPortion = hostDonation - (hostDonation * symIntVal);

      hostPortion += symReturn; //hostPortion is positive
    }
    else if (hostIntVal <= 0 && symIntVal < 0){ //antagonistic from both sides
      double hostDefense = -1.0 * (hostIntVal * sym_piece);
      double remainingResources = 0.0;
      remainingResources = sym_piece - hostDefense;

      // if both are hostile, then the symbiont must be more hostile than in order to gain any resources
      if (symIntVal < hostIntVal) { //symbiont overcomes host's defenses
        double symSteals = (hostIntVal - symIntVal) * remainingResources;

        symPortion = symSteals;
        hostPortion = remainingResources - symSteals;

      } else { // symbiont cannot overcome host's defenses
        symPortion = 0.0;
        hostPortion = remainingResources;
      }
    }
    else if (hostIntVal > 0 && symIntVal < 0){
      hostDonation = hostIntVal * sym_piece;
      hostPortion = sym_piece - hostDonation;

      double symSteals = -1.0 * (hostPortion * symIntVal);
      hostPortion = hostPortion - symSteals;
      symPortion = hostDonation + symSteals;
    }
    else if (hostIntVal < 0 && symIntVal >= 0){
      double hostDefense = -1.0 * (hostIntVal * sym_piece);
      hostPortion = sym_piece - hostDefense;

      // symbiont gets nothing from antagonistic host
      symPortion = 0.0;
    }
    else {
      std::cout << "This should never happen." << std::endl;
    }
    this->AddPoints(symPortion);
    return hostPortion;
  }

  void Process(size_t location) {
    if (my_host.IsNull() && my_config->FREE_LIVING_SYMS()) { //free living symbiont
      double resources = my_world->PullResources();
      AddPoints(resources);
    }
    if (h_trans) { //non-lytic horizontal transmission enabled
      if(GetPoints() >= sym_h_res) {
        // symbiont reproduces independently (horizontal transmission) if it has enough resources
        // new symbiont in this host with mutated value
        SetPoints(0); //TODO: test just subtracting points instead of setting to 0
        emp::Ptr<Symbiont> sym_baby = emp::NewPtr<Symbiont>(*this);
        sym_baby->SetPoints(0);
        sym_baby->HorizMutate();
        //HorizMutate();
        my_world->SymDoBirth(sym_baby, location);
      }
    }
    if (my_host.IsNull() && my_config->FREE_LIVING_SYMS() && my_config->MOVE_FREE_SYMS()) {
      my_world->MoveFreeSym(location);
    }
  }

  emp::Ptr<Organism> reproduce() {
    emp::Ptr<Symbiont> sym_baby = emp::NewPtr<Symbiont>(*this); //constructor that takes parent values
    sym_baby->SetPoints(0);
    sym_baby->mutate();
    //mutate(); //mutate parent symbiont
    return sym_baby;
  }

  void VerticalTransmission(emp::Ptr<Organism> host_baby) {
    if(my_world->WillTransmit()){
      emp::Ptr<Organism> sym_baby = reproduce();
      host_baby->AddSymbiont(sym_baby);
    }
  }

};

std::string PrintSym(emp::Ptr<Symbiont>  org){
  if (org->GetPoints() < 0) return "-";
  double out_val = org->GetIntVal();

  // this prints the symbiont with two decimal places for easier reading
  std::stringstream temp;
  temp << std::fixed << std::setprecision(2) << out_val;
  std::string formattedstring = temp.str();
  return formattedstring;

  // return emp::to_string(out_val);  // creates a string without specifying format
}//Symbiont
#endif
