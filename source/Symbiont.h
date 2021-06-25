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

  emp::Ptr<emp::Random> random = NULL;
  emp::Ptr<SymWorld> my_world = NULL;
  emp::Ptr<Organism> my_host = NULL;
  emp::Ptr<SymConfigBase> my_config = NULL;

public:
  Symbiont(emp::Ptr<emp::Random> _random, emp::Ptr<SymWorld> _world, emp::Ptr<SymConfigBase> _config, double _intval=0.0, double _points = 0.0) : random(_random), my_world(_world), my_config(_config), interaction_val(_intval), points(_points) {
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
  emp::Ptr<Organism> GetHost() {return my_host;}
  //  std::set<int> GetResTypes() const {return res_types;}



  void SetIntVal(double _in) {
    if ( _in > 1 || _in < -1) {
       throw "Invalid interaction value. Must be between -1 and 1";   // Exception for invalid interaction value
     }
     else {
        interaction_val = _in;
     }

  }

  void SetPoints(double _in) { points = _in;}
  void AddPoints(double _in) { points += _in;}
  void SetHost(emp::Ptr<Organism> _in) {my_host = _in;}
  bool IsHost() { return false; }
  //void SetResTypes(std::set<int> _in) {res_types = _in;}


  //TODO: change everything to camel case
  void mutate(){
    double pre_value = interaction_val;
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
    double pre_value = interaction_val;
    if (random->GetDouble(0.0, 1.0) <= ht_mut_rate) {
      interaction_val += random->GetRandNormal(0.0, ht_mut_size);
      if(interaction_val < -1) interaction_val = -1;
      else if (interaction_val > 1) interaction_val = 1;
    }
    //if((pre_value*interaction_val) < 0) {
    //  std::cout << "switched!" << std::endl;
    //}
  }

  void process(double resources, size_t location) {
    double rss = GetPoints();
    if (h_trans) { //non-lytic horizontal transmission enabled
      if (my_host == NULL){
        rss = resources + GetPoints();
      }
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
  }

  emp::Ptr<Organism> reproduce() {
    emp::Ptr<Symbiont> sym_baby = emp::NewPtr<Symbiont>(*this); //constructor that takes parent values
    sym_baby->SetPoints(0);
    sym_baby->mutate();
    //mutate(); //mutate parent symbiont
    return sym_baby;
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
