#ifndef PHAGE_H
#define PHAGE_H

#include "Symbiont.h"
#include "SymWorld.h"

class Phage: public Symbiont {
protected:
  double burst_timer = 0;
  bool lysis_enabled = true;
  bool lysogeny = false;
  double burst_time = 60;
  double sym_lysis_res = 15;
  double chance_of_lysis = 1;
  bool mutate_chance_of_lysis = false;


public:

  Phage(emp::Ptr<emp::Random> _random, emp::Ptr<SymWorld> _world, emp::Ptr<SymConfigBase> _config, double _intval=0.0, double _points = 0.0) : Symbiont(_random, _world, _config, _intval, _points) {
    burst_time = my_config->BURST_TIME();
    sym_lysis_res = my_config->SYM_LYSIS_RES();
    lysis_enabled = my_config->LYSIS();
    mutate_chance_of_lysis = my_config->MUTATE_LYSIS_CHANCE();
    chance_of_lysis = my_config->LYSIS_CHANCE();
    if(chance_of_lysis == -1){
      chance_of_lysis = random->GetDouble(0.0, 1.0);
    }
  }
  Phage(const Phage &) = default;
  Phage(Phage &&) = default;
  Phage() = default;
  ///Input: None
  ///Output: The double representing the phage's burst timer. 
  ///Purpose: To get a phage's burst timer. 
  double GetBurstTimer() {return burst_timer;}

  ///Input: None
  ///Output: None
  ///Purpose: To increment a phage's burst timer. 
  void IncBurstTimer() {burst_timer += random->GetRandNormal(1.0, 1.0);}

  ///Input: The int to be set as the phage's burst timer 
  ///Output: None
  ///Purpose: To set a phage's burst timer.  
  void SetBurstTimer(int _in) {burst_timer = _in;}

  ///Input: None
  ///Output: The double representing a phage's change of lysis.
  ///Purpose: To determine a phage's chance of lysis.
  double GetLysisChance() {return chance_of_lysis;}

  ///Input: The double to be set as the phage's chance of lysis. 
  ///Output: None
  ///Purpose: To set a phage's chance of lysis 
  void SetLysisChance(double _in) {chance_of_lysis = _in;}

  ///Input: None
  ///Output: The bool representing if a phage will do lysogeny.
  ///Purpose: To determine if a phage will do lysogeny 
  bool GetLysogeny() {return lysogeny;}

  ///Input: None
  ///Output: The bool representing if an organism is a phage. 
  ///Purpose: To determine if an organism is a phage.
  bool IsPhage(){return true;}

  ///Input: None
  ///Output: None
  ///Purpose: To determine if a phage will choose lysis or lysogeny.
  void uponInjection() {
    double rand_chance = random->GetDouble(0.0, 1.0);
    if (rand_chance <= chance_of_lysis){
      lysogeny = false;
      SetIntVal(-1); //lytic phage are antagonistic
    } else {
      lysogeny = true;
      SetIntVal(0); //lysogenic phage are neutral
    }
  }

  ///Input: None
  ///Output: None
  ///Purpose: To mutate a phage's chance of lysis. 
  void mutate() {
   Symbiont::mutate();
    if (random->GetDouble(0.0, 1.0) <= mut_rate) {
      //mutate chance of lysis/lysogeny, if enabled
      if(mutate_chance_of_lysis){
        chance_of_lysis += random->GetRandNormal(0.0, mut_size);
        if(chance_of_lysis < 0) chance_of_lysis = 0;
        else if (chance_of_lysis > 1) chance_of_lysis = 1;
      }
    }
  }

  ///Input: None
  ///Output: The pointer to the new phage that has been produced.
  ///Purpose: To reproduce phage. 
  emp::Ptr<Organism> reproduce() {
    emp::Ptr<Phage> sym_baby = emp::NewPtr<Phage>(*this); //constructor that takes parent values                                             
    sym_baby->SetPoints(0);
    sym_baby->SetBurstTimer(0);
    sym_baby->mutate();
    return sym_baby;
  }

  ///Input: A pointer to the baby host to have symbiont's added. 
  ///Output: None
  ///Purpose: To allow for vertical transmisison to occur. 
  void VerticalTransmission(emp::Ptr<Organism> host_baby){
    //lysogenic phage have 100% chance of vertical transmission, lytic phage have 0% chance
    if(lysogeny){
      emp::Ptr<Organism> phage_baby = reproduce();
      host_baby->AddSymbiont(phage_baby);
    }
  }

  ///Input: The double representing the resources that will be given to a phage. 
  ///Output: The double reoresenting the resources that have been distirbuted to a phage. 
  ///Purpose: To mutate a phage's chance of lysis. 
  double ProcessResources(double sym_piece){
    if(lysogeny){
      return sym_piece; //lysogenic phage don't steal any resources from their host
    } else {
      return Symbiont::ProcessResources(sym_piece); //lytic phage do steal resources
    }
  }

  ///Input: The size_t representing the location of the phage being processed. 
  ///Output: None
  ///Purpose: To process a phage, meaning check for reproduciton, check for lysis, and move the phage. 
  void Process(size_t location) {
    if(lysis_enabled && GetHost() != NULL) { //lysis enabled, checking for lysis
      if(!lysogeny){ //phage has chosen lysis
        if(GetBurstTimer() >= burst_time ) { //time to lyse!
          emp::vector<emp::Ptr<Organism>>& repro_syms = my_host->GetReproSymbionts();
          //Record the burst size
          // update this for my_world: data_node_burst_size -> AddDatum(repro_syms.size());
          for(size_t r=0; r<repro_syms.size(); r++) {
            my_world->SymDoBirth(repro_syms[r], location);
          }
          my_host->ClearReproSyms();
          my_host->SetDead();
          return;
          
        } else { //not time to lyse
          IncBurstTimer();
          if(sym_lysis_res == 0) {
            std::cout << "Lysis with a sym_lysis_res of 0 leads to an \
            infinite loop, please change" << std::endl;
            std::exit(1);
          }
          while(GetPoints() >= sym_lysis_res) {
            emp::Ptr<Organism> sym_baby = reproduce();
            my_host->AddReproSym(sym_baby);
            SetPoints(GetPoints() - sym_lysis_res);
          }
        }
      }
      else if(lysogeny){ //phage has chosen lysogeny
        //check if the phage's host should become susceptible again
        if(random->GetDouble(0.0, 1.0) <= my_config->PROPHAGE_LOSS_RATE()){
          SetDead();
        }
      }
    }else{
      my_world->MoveFreeSym(location);
    }
  }

};

#endif
