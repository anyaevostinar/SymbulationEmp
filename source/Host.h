#ifndef HOST_H
#define HOST_H

#include "../../Empirical/include/emp/math/Random.hpp"
#include "../../Empirical/include/emp/tools/string_utils.hpp"
#include <set>
#include <iomanip> // setprecision
#include <sstream> // stringstream
#include "Organism.h"
#include "SymWorld.h"


class Host: public Organism {
private:
  double interaction_val = 0;
  emp::vector<emp::Ptr<Organism>> syms = {};
  emp::vector<emp::Ptr<Organism>> repro_syms = {};
  std::set<int> res_types = {};
  double points = 0;
  emp::Ptr<emp::Random> random = NULL;
  emp::Ptr<SymWorld> my_world = NULL;
  emp::Ptr<SymConfigBase> my_config = NULL;
  bool dead = false;

public:
  Host(emp::Ptr<emp::Random> _random, emp::Ptr<SymWorld> _world, emp::Ptr<SymConfigBase> _config,
  double _intval =0.0, emp::vector<emp::Ptr<Organism>> _syms = {},
  emp::vector<emp::Ptr<Organism>> _repro_syms = {},
  std::set<int> _set = std::set<int>(),
  double _points = 0.0) : interaction_val(_intval), syms(_syms), repro_syms(_repro_syms),
  res_types(_set), points(_points), random(_random), my_world(_world), my_config(_config) { 
    if ( _intval > 1 || _intval < -1) {
       throw "Invalid interaction value. Must be between -1 and 1";  // Exception for invalid interaction value
     };
   }

  ~Host(){
    for(size_t i=0; i<syms.size(); i++){
      syms[i].Delete();
    }
    for(size_t j=0; j<repro_syms.size(); j++){
      repro_syms[j].Delete();
    }
  }

  Host(const Host &) = default;
  Host(Host &&) = default;
  Host() = default;

  Host & operator=(const Host &) = default;
  Host & operator=(Host &&) = default;
  bool operator==(const Host &other) const { return (this == &other);}
  bool operator!=(const Host &other) const {return !(*this == other);}

  ///Input: None
  ///Output: The double representing host's interaction value. 
  ///Purpose: To get the double representing host's interaction value. 
  double GetIntVal() const { return interaction_val;}
  ///Input: None
  ///Output: A vector of pointers to the organisms that are the host's syms.  
  ///Purpose: To get the vector containing pointers to the host's symbionts.
  emp::vector<emp::Ptr<Organism>>& GetSymbionts() {return syms;}
  ///Input: None
  ///Output: A vector of pointers to the organisms that are the host's repro syms.  
  ///Purpose: To get the vector containing pointers to the host's repro syms. 
  emp::vector<emp::Ptr<Organism>>& GetReproSymbionts() {return repro_syms;}
  ///Input: None
  ///Output: The set of ints representing a host's res type.
  ///Purpose: To get the set of ints representing host's res type.
  std::set<int> GetResTypes() const { return res_types;}
  ///Input: None
  ///Output: The double representing a host's points.
  ///Purpose: To get the host's res type.
  double GetPoints() { return points;}
  ///Input: None
  ///Output: The bool representing if an organism is a host. 
  ///Purpose: To determine if an organism is a host. 
  bool IsHost() { return true; }


  ///Input: A double representing the host's new interaction value. 
  ///Output: None. 
  ///Purpose: To set a host's interaction value. 
  void SetIntVal(double _in) {
    if ( _in > 1 || _in < -1) {
       throw "Invalid interaction value. Must be between -1 and 1";  // Exception for invalid interaction value
     }
     else {
       interaction_val = _in;
     }
  }

  ///Input: A vector of pointers to organisms that will become a host's symbionts.
  ///Output: None.
  ///Purpose: To set a host's symbionts to the input vector of organisms. 
  void SetSymbionts(emp::vector<emp::Ptr<Organism>> _in) {
    ClearSyms();
    for(size_t i = 0; i < _in.size(); i++){
      AddSymbiont(_in[i]);
    }
  }
  ///Input: A set of ints representing a host's resource type.
  ///Output: None
  ///Purpose: To set a host's resource types to the input. 
  void SetResTypes(std::set<int> _in) {res_types = _in;}
  ///Input: A double representing a host's new point value.
  ///Output: None
  ///Purpose: To set a host's points.
  void SetPoints(double _in) {points = _in;}
  ///Input: None
  ///Output: None
  ///Purpose: To clear a host's symbionts. 
  void ClearSyms() {syms.resize(0);}
  ///Input: None
  ///Output: None
  ///Purpose: To clear a host's repro symbionts. 
  void ClearReproSyms() {repro_syms.resize(0);}
  ///Input: None
  ///Output: None
  ///Purpose: To kill a host.
  void SetDead() { dead = true;}
  ///Input: None
  ///Output: None
  ///Purpose: To determine if a host is dead. 
  bool GetDead() {return dead;}
  ///Input: The double representing the number of points to be incremented onto a host's points.
  ///Output: None
  ///Purpose: To increment a host's points by the input value.  
  void AddPoints(double _in) {points += _in;}
  ///Input: The pointer to the organism that is to be added to the host's symbionts.
  ///Output: None
  ///Purpose: To add a host's symbionts 
  void AddSymbiont(emp::Ptr<Organism> _in) {
    if((int)syms.size() < my_config->SYM_LIMIT() && SymAllowedIn()){
      syms.push_back(_in);
      _in->SetHost(this);
      _in->uponInjection();
    } else {
      _in.Delete();
    }
  }
  ///Input: None
  ///Output: A bool representing if a symbiont will be allowed to enter a host.
  ///Purpose: To determine if a symbiont will be allowed into a host. 
  bool SymAllowedIn(){
    bool do_phage_exclusion = my_config->PHAGE_EXCLUDE();
    if(!do_phage_exclusion){
     return true;
    }
    else{
     int num_syms = syms.size();
     //essentially imitaties a 1/ 2^n chance, with n = number of symbionts
     int enter_chance = random->GetUInt((int) pow(2.0, num_syms));
     if(enter_chance == 0) { return true; }
     return false;
    }
  }
  ///Input: A pointer to the organism to be added to the host's symbionts. 
  ///Output: None
  ///Purpose: To add a repro sym to the host's symbionts. 
  void AddReproSym(emp::Ptr<Organism> _in) {repro_syms.push_back(_in);}
  ///Input: None
  ///Output: A bool representing if a host has any symbionts.
  ///Purpose: To determine if a host has any symbionts. 
  bool HasSym() {
    return syms.size() != 0;
  }
  ///Input: None
  ///Output: None
  ///Purpose: To mutate a host's interaction value. 
  void mutate(){
    if(random->GetDouble(0.0, 1.0) <= my_config->MUTATION_RATE()){
      interaction_val += random->GetRandNormal(0.0, my_config->MUTATION_SIZE());
      if(interaction_val < -1) interaction_val = -1;
      else if (interaction_val > 1) interaction_val = 1;
    }
  }
  ///Input: The double representing the number of resources to be distrubuted to the host and its symbionts.
  ///Output: None
  ///Purpose: To distribute resources to a host and its symbions.
  void DistribResources(double resources) {
    double hostIntVal = interaction_val; //using private variable because we can

    //In the event that the host has no symbionts, the host gets all resources not allocated to defense or given to absent partner.
    if(syms.empty()) {

      if(hostIntVal >= 0){
	      double spent = resources * hostIntVal;
        this->AddPoints(resources - spent);
      }
      else {
        double hostDefense = -1.0 * hostIntVal * resources;
        this->AddPoints(resources - hostDefense);
      }
      return; //This concludes resource distribution for a host without symbionts
    }

    //Otherwise, split resources into equal chunks for each symbiont
    int num_sym = syms.size();
    double sym_piece = (double) resources / num_sym;

    for(size_t i=0; i < syms.size(); i++){
      double hostPortion = syms[i]->ProcessResources(sym_piece);
      this->AddPoints(hostPortion);
    }

  } //end DistribResources
  ///Input: The size_t value representing the location of the host. 
  ///Output: None
  ///Purpose: To process the host, meaning determing eligability for reproduction, etc. 
  void Process(size_t location) {
    //Currently just wrapping to use the existing function
    double resources = my_world->PullResources();
    DistribResources(resources);
    // Check reproduction
    if (GetPoints() >= my_config->HOST_REPRO_RES() && repro_syms.size() == 0) {  // if host has more points than required for repro
        // will replicate & mutate a random offset from parent values
        // while resetting resource points for host and symbiont to zero
        emp::Ptr<Host> host_baby = emp::NewPtr<Host>(random, my_world, my_config, GetIntVal());
        host_baby->mutate();
        //mutate(); //parent mutates and loses current resources, ie new organism but same symbiont
        SetPoints(0);

        //Now check if symbionts get to vertically transmit
        for(size_t j = 0; j< (GetSymbionts()).size(); j++){
          emp::Ptr<Organism> parent = GetSymbionts()[j];
          parent->VerticalTransmission(host_baby);
        }

        //Will need to change this to AddOrgAt and write my own position grabber 
        //when I want ecto-symbionts
        my_world->DoBirth(host_baby, location); //Automatically deals with grid
      }
    if (GetDead()){
        return; //If host is dead, return
      }
    if (HasSym()) { //let each sym do whatever they need to do
        emp::vector<emp::Ptr<Organism>>& syms = GetSymbionts();
        for(size_t j = 0; j < syms.size(); j++){
          emp::Ptr<Organism> curSym = syms[j];
          if (GetDead()){ 
            return; //If previous symbiont killed host, we're done
          }
          if(!curSym->GetDead()){
            curSym->Process(location);
          }
          if(curSym->GetDead()){
            syms.erase(syms.begin() + j); //if the symbiont dies during their process, remove from syms list
            curSym.Delete();
          }
        } //for each sym in syms
      } //if org has syms
  }

};//Host

#endif
