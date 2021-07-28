#ifndef PGGHOST_H
#define PGGHOST_H

// #include "../../Empirical/include/emp/math/Random.hpp"
// #include "../../Empirical/include/emp/tools/string_utils.hpp"
// #include <set>
// #include <iomanip> // setprecision
// #include <sstream> // stringstream
#include "Host.h"
#include "SymWorld.h"


class PggHost: public Host {
protected:
  double sourcepool=0;


public:
  PggHost(emp::Ptr<emp::Random> _random, emp::Ptr<SymWorld> _world, emp::Ptr<SymConfigBase> _config,
  double _intval =0.0, emp::vector<emp::Ptr<Organism>> _syms = {},
  emp::vector<emp::Ptr<Organism>> _repro_syms = {},
  std::set<int> _set = std::set<int>(),
  double _points = 0.0) : Host(_random, _world, _config, _intval,_syms, _repro_syms, _set, _points) {}



  PggHost(const PggHost &) = default;
  PggHost(PggHost &&) = default;
  PggHost() = default;
 
  double GetPool() {return sourcepool;}
  void SetPool(double _in) {sourcepool= _in;}
  void AddPool(double _in) {sourcepool += _in;}


  void DistribResources(double resources) {
    Host::DistribResources(resources); 

    for(size_t i=0; i < syms.size(); i++){
      double hostPool = syms[i]->ProcessPool();
      this->AddPool(hostPool);
    }
    if(syms.size()>0){this->DistribPool();}
    

  } //end DistribResources

  void DistribPool(){
    //to do: marginal return
    int num_sym = syms.size();
    double bonus = my_config->PGG_SYNERGY();
    double sym_piece = (double) sourcepool / num_sym;
    for(size_t i=0; i < syms.size(); i++){
        syms[i]->AddPoints(sym_piece);
    }
    this->SetPool(0);
  }

  void Process(size_t location) {
    //Currently just wrapping to use the existing function
    double resources = my_world->PullResources();
    DistribResources(resources);
    // Check reproduction
    if (GetPoints() >= my_config->HOST_REPRO_RES() && repro_syms.size() == 0) {  // if host has more points than required for repro
        // will replicate & mutate a random offset from parent values
        // while resetting resource points for host and symbiont to zero
        emp::Ptr<PggHost> host_baby = emp::NewPtr<PggHost>(random, my_world, my_config, GetIntVal());
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