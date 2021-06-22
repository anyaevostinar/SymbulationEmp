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
  double _points = 0.0) : random(_random), my_world(_world), my_config(_config),
  interaction_val(_intval), syms(_syms), repro_syms(_repro_syms),
  res_types(_set), points(_points) {
    if ( _intval > 1 || _intval < -1) {
       throw "Invalid interaction value. Must be between -1 and 1";  // Exception for invalid interaction value
     };
   }

  ~Host(){
    for(int i=0; i<syms.size(); i++){
      syms[i].Delete();
    }
    for(int j=0; j<repro_syms.size(); j++){
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


  double GetIntVal() const { return interaction_val;}
  emp::vector<emp::Ptr<Organism>>& GetSymbionts() {return syms;}
  emp::vector<emp::Ptr<Organism>>& GetReproSymbionts() {return repro_syms;}
  std::set<int> GetResTypes() const { return res_types;}
  double GetPoints() { return points;}
  bool IsHost() { return true; }


  void SetIntVal(double _in) {
    if ( _in > 1 || _in < -1) {
       throw "Invalid interaction value. Must be between -1 and 1";  // Exception for invalid interaction value
     }
     else {
       interaction_val = _in;
     }
  }
  void SetSymbionts(emp::vector<emp::Ptr<Organism>> _in) {syms = _in;}
  void SetResTypes(std::set<int> _in) {res_types = _in;}
  void SetPoints(double _in) {points = _in;}
  void ClearReproSyms() {
    repro_syms.resize(0);
  }
  void SetDead() { dead = true;}
  bool GetDead() {return dead;}


  void AddPoints(double _in) {points += _in;}


  void AddSymbiont(emp::Ptr<Organism> _in) {
    if(syms.size() < my_config->SYM_LIMIT()){
      syms.push_back(_in);
      _in->SetHost(this);
    } else {
      _in.Delete();
    }
  }
  void AddReproSym(emp::Ptr<Organism> _in) {repro_syms.push_back(_in);}

  bool HasSym() {
    return syms.size() != 0;
  }



  void mutate(){
    if(random->GetDouble(0.0, 1.0) <= my_config->MUTATION_RATE()){
      interaction_val += random->GetRandNormal(0.0, my_config->MUTATION_SIZE());
      if(interaction_val < -1) interaction_val = -1;
      else if (interaction_val > 1) interaction_val = 1;
    }
  }

  void DistribResources(double resources) {
    double hostIntVal = interaction_val; //using private variable because we can
    double synergy = my_config->SYNERGY();
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
      return; //This concludes resource distribution.
    }

    //Otherwise, split resources into equal chunks for each symbiont
    int num_sym = syms.size();
    double sym_piece = (double) resources / num_sym;

    for(size_t i=0; i < syms.size(); i++){
      double symIntVal = syms[i]->GetIntVal();

      double hostPortion = 0.0;
      double hostDonation = 0.0;
      double symPortion = 0.0;
      double symReturn = 0.0;
      double bonus = synergy;


      if (hostIntVal >= 0 && symIntVal >= 0)  {
        hostDonation = sym_piece * hostIntVal;
        hostPortion = sym_piece - hostDonation;

        symReturn = (hostDonation * symIntVal) * bonus;
        symPortion = hostDonation - (hostDonation * symIntVal);

        hostPortion += symReturn;

        syms[i]->AddPoints(symPortion);
        this->AddPoints(hostPortion);

      } else if (hostIntVal <= 0 && symIntVal < 0) {
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

        syms[i]->AddPoints(symPortion);
        this->AddPoints(hostPortion);

      } else if (hostIntVal > 0 && symIntVal < 0) {
        hostDonation = hostIntVal * sym_piece;
        hostPortion = sym_piece - hostDonation;

        double symSteals = -1.0 * (hostPortion * symIntVal);
        hostPortion = hostPortion - symSteals;
        symPortion = hostDonation + symSteals;

        syms[i]->AddPoints(symPortion);
        this->AddPoints(hostPortion);


      } else if (hostIntVal < 0 && symIntVal >= 0) {
        double hostDefense = -1.0 * (hostIntVal * sym_piece);
        hostPortion = sym_piece - hostDefense;

        // symbiont gets nothing from antagonistic host
        symPortion = 0.0;

        syms[i]->AddPoints(symPortion);
        this->AddPoints(hostPortion);
      } else {

        //TODO: add error here
        std::cout << "This should never happen." << std::endl;

      }

    } //end syms[i] for loop

  } //end DistribResources

  void Process(double resources, int location) {
    //Currently just wrapping to use the existing function
    DistribResources(resources);
    // Check reproduction

    if (GetPoints() >= my_config->HOST_REPRO_RES() ) {  // if host has more points than required for repro
        // will replicate & mutate a random offset from parent values
        // while resetting resource points for host and symbiont to zero
        emp::Ptr<Host> host_baby = emp::NewPtr<Host>(random, my_world, my_config, GetIntVal());
        host_baby->mutate();
        //mutate(); //parent mutates and loses current resources, ie new organism but same symbiont
        SetPoints(0);

        //Now check if symbionts get to vertically transmit
        for(size_t j = 0; j< (GetSymbionts()).size(); j++){
          emp::Ptr<Organism> parent = GetSymbionts()[j];
           if (my_world->WillTransmit()) { //Vertical transmission!

            emp::Ptr<Organism> sym_baby = parent->reproduce();
            host_baby->AddSymbiont(sym_baby);

          } //end will transmit
        } //end for loop for each symbiont
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
          if (GetDead()){
            return; //If previous symbiont killed host, we're done
          }
          syms[j]->process(location);


        } //for each sym in syms
      } //if org has syms
  }

};//Host

#endif
