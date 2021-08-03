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
  bool mutate_chance_of_induction = false;
  bool induction_enabled = true;
  double induction_chance = 1;


public:

  Phage(emp::Ptr<emp::Random> _random, emp::Ptr<SymWorld> _world, emp::Ptr<SymConfigBase> _config, double _intval=0.0, double _points = 0.0) : Symbiont(_random, _world, _config, _intval, _points) {
    burst_time = my_config->BURST_TIME();
    sym_lysis_res = my_config->SYM_LYSIS_RES();
    lysis_enabled = my_config->LYSIS();
    mutate_chance_of_induction = my_config->MUTATE_INDUCTION_CHANCE();
    mutate_chance_of_lysis = my_config->MUTATE_LYSIS_CHANCE();
    chance_of_lysis = my_config->LYSIS_CHANCE();
    induction_chance = my_config->CHANCE_OF_INDUCTION();
    if(chance_of_lysis == -1){
      chance_of_lysis = random->GetDouble(0.0, 1.0);
    }
    if(induction_chance == -1){
      induction_chance = random->GetDouble(0.0, 1.0);
    }
  }
  Phage(const Phage &) = default;
  Phage(Phage &&) = default;
  Phage() = default;

  double GetBurstTimer() {return burst_timer;}
  void IncBurstTimer() {

    burst_timer += random->GetRandNormal(1.0, 1.0);

  }
  void SetBurstTimer(int _in) {burst_timer = _in;}

  double GetLysisChance() {return chance_of_lysis;}
  void SetLysisChance(double _in) {chance_of_lysis = _in;}

  double GetInductionChance() {return induction_chance;}
  void SetInductionChance(double _in) {induction_chance = _in;}

  bool GetLysogeny() {return lysogeny;}
  bool IsPhage(){return true;}
  void uponInjection() {
    //decide if the phage will choose lysis or lysogeny
    double rand_chance = random->GetDouble(0.0, 1.0);
    if (rand_chance <= chance_of_lysis){
      lysogeny = false;
      SetIntVal(-1); //lytic phage are antagonistic
    } else {
      lysogeny = true;
      SetIntVal(0); //lysogenic phage are neutral
    }
  }

  void mutate() {
   Symbiont::mutate();
    if (random->GetDouble(0.0, 1.0) <= mut_rate) {
      //mutate chance of lysis/lysogeny, if enabled
      if(mutate_chance_of_lysis){
        chance_of_lysis += random->GetRandNormal(0.0, mut_size);
        if(chance_of_lysis < 0) chance_of_lysis = 0;
        else if (chance_of_lysis > 1) chance_of_lysis = 1;
      }
      if(mutate_chance_of_induction){
        induction_chance += random->GetRandNormal(0.0, mut_size);
        if(induction_chance < 0) induction_chance = 0;
        else if (induction_chance > 1) induction_chance = 1;
    }
    }
  }

  emp::Ptr<Organism> reproduce() {
    emp::Ptr<Phage> sym_baby = emp::NewPtr<Phage>(*this); //constructor that takes parent values
    sym_baby->SetPoints(0);
    sym_baby->SetBurstTimer(0);
    sym_baby->mutate();
    return sym_baby;
  }
 void HorizontalTransmission(size_t location) { //Horizontal transmission process after lysis or induction are called
    if(GetBurstTimer() >= burst_time ) { //time to lyse!
          emp::vector<emp::Ptr<Organism>>& repro_syms = my_host->GetReproSymbionts();
          //Record the burst size and count
          emp::DataMonitor<double>& data_node_burst_size = my_world->GetBurstSizeDataNode();
          data_node_burst_size.AddDatum(repro_syms.size());
          emp::DataMonitor<int>& data_node_burst_count = my_world->GetBurstCountDataNode();
          data_node_burst_count.AddDatum(1);

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

  void VerticalTransmission(emp::Ptr<Organism> host_baby){
    //lysogenic phage have 100% chance of vertical transmission, lytic phage have 0% chance
    if(lysogeny){
      emp::Ptr<Organism> phage_baby = reproduce();
      host_baby->AddSymbiont(phage_baby);
    }
  }

  double ProcessResources(double hostDonation){
    if(lysogeny){
      return 0;
    }
    else{
      return Symbiont::ProcessResources(hostDonation); //lytic phage do steal resources
    }
  }

  void Process(size_t location) {
    if(lysis_enabled && !GetHost().IsNull()) { //lysis enabled and phage is in a host
      if(!lysogeny){ //phage has chosen lysis
        HorizontalTransmission(location);
        }
      }

    else if(lysogeny){ //phage has chosen lysogeny
      if (induction_enabled && !GetHost().IsNull()){ //induction enabled and phage is in host
        double rand_chance = random->GetDouble(0.0, 1.0);
        if (rand_chance <= induction_chance){//phage has chosen to induce and perform lysis
          lysogeny = false;
          HorizontalTransmission(location);
        } 
      }
      else if(random->GetDouble(0.0, 1.0) <= my_config->PROPHAGE_LOSS_RATE()){ //check if the phage's host should become susceptible again
        SetDead();
      }
    else if (GetHost().IsNull() && my_config->FREE_LIVING_SYMS()) { //phage is free living
      my_world->MoveFreeSym(location);
    }
  }
  }

};

#endif
