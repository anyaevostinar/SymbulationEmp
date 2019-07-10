#include "../../Empirical/source/tools/Random.h"
#include "../../Empirical/source/tools/string_utils.h"
#include <set>
#include <iomanip> // setprecision
#include <sstream> // stringstream


class Symbiont {
private:  
  double interaction_val;
  double points;
  std::set<int> res_types;


public:

  Symbiont(double _intval=0.0, double _points = 0.0, std::set<int> _set = std::set<int>())
    : interaction_val(_intval), points(_points), res_types(_set) { ; }
  Symbiont(const Symbiont &) = default;
  Symbiont(Symbiont &&) = default;
  

  Symbiont & operator=(const Symbiont &) = default;
  Symbiont & operator=(Symbiont &&) = default;

  double GetIntVal() const {return interaction_val;}
  double GetPoints() {return points;}
  //  std::set<int> GetResTypes() const {return res_types;}

  void SetIntVal(double _in) { interaction_val = _in;}
  void SetPoints(double _in) { points = _in;}
  void AddPoints(double _in) { points += _in;}
  //void SetResTypes(std::set<int> _in) {res_types = _in;}

  //TODO: change everything to camel case
  void mutate(emp::Random &random, double mut_rate){
    interaction_val += random.GetRandNormal(0.0, mut_rate);
    if(interaction_val < -1) interaction_val = -1;
    else if (interaction_val > 1) interaction_val = 1;
  }

};

std::string PrintSym(Symbiont  org){
  if (org.GetPoints() < 0) return "-";
  double out_val = org.GetIntVal();  
  
  // this prints the symbiont with two decimal places for easier reading
  std::stringstream temp;
  temp << std::fixed << std::setprecision(2) << out_val;
  std::string formattedstring = temp.str();
  return formattedstring;
  
  // return emp::to_string(out_val);  // creates a string without specifying format

}

class Host {
private:
  double interaction_val;
  emp::vector<Symbiont> syms;
  std::set<int> res_types;
  double points;

public:
 Host(double _intval =0.0, emp::vector<Symbiont> _syms = {}, std::set<int> _set = std::set<int>(), double _points = 0.0) : interaction_val(_intval), syms(_syms), res_types(_set), points(_points) { ; }
  Host(const Host &) = default;
  Host(Host &&) = default;
  // Host() : interaction_val(0), sym(*(new Symbiont(0, -1))), res_types(std::set<int>()), points(0) { ; }


  Host & operator=(const Host &) = default;
  Host & operator=(Host &&) = default;
  bool operator==(const Host &other) const { return (this == &other);}
  bool operator!=(const Host &other) const {return !(*this == other);}


  double GetIntVal() const { return interaction_val;}
  emp::vector<Symbiont>* GetSymbionts() { return &syms;}
  std::set<int> GetResTypes() const { return res_types;}
  double GetPoints() { return points;}


  void SetIntVal(double _in) {interaction_val = _in;}
  void SetSymbionts(emp::vector<Symbiont> _in) {syms = _in;}
  void SetResTypes(std::set<int> _in) {res_types = _in;}
  void SetPoints(double _in) {points = _in;}

  void AddSymbionts(Symbiont _in) {syms.push_back(_in);}
  void AddPoints(double _in) {points += _in;}
  
  
  bool HasSym() {
    if (syms.size() <= 0) { 
      return false;
    } else {
      return true;
    }
  	
  }

  void mutate(emp::Random &random, double mut_rate){
    interaction_val += random.GetRandNormal(0.0, mut_rate);
    if(interaction_val < -1) interaction_val = -1;
    else if (interaction_val > 1) interaction_val = 1;
  }
  
  void DistribResources(int resources, double synergy) { 
    //split resources into equal chunks for each symbiont
    int num_sym = syms.size();
    double sym_piece = (double) resources / num_sym;

    double hostIntVal = interaction_val; //using private variable because we can

    for(size_t i=0; i < syms.size(); i++){
      
      double symIntVal = syms[i].GetIntVal();
    
      double hostPortion = 0.0;
      double hostDonation = 0.0;
      double symPortion = 0.0;
      double symReturn = 0.0;
      double bonus = synergy; 

	
//	std::cout << "Dividing resources: " << resources << std::endl;
      if (hostIntVal >= 0 && symIntVal >= 0)  {  
	hostDonation = sym_piece * hostIntVal;
	hostPortion = sym_piece - hostDonation;  
	    
//	    std::cout << "Host keeps " << hostPortion << " and gives " << hostDonation << " to symbiont." << std::endl;
	    
	symReturn = (hostDonation * symIntVal) * bonus;  
	symPortion = hostDonation - (hostDonation * symIntVal);
	    
//	    std::cout << "Symbiont keeps " << symPortion << " and returns " << symReturn << " to host (multiplied by) " << bonus << std::endl;

	hostPortion += symReturn;
	    
//	    std::cout << "In the end, host gets " << hostPortion << std::endl;
	    
	syms[i].AddPoints(symPortion);
	this->AddPoints(hostPortion);
	    
      } else if (hostIntVal <= 0 && symIntVal < 0) {  // NEED TO CHECK THAT THIS IS CORRECT - see dissertation
	double hostDefense = -1.0 * (hostIntVal * sym_piece);
	double remainingResources = 0.0;
//	     std::cout << "Host: " << hostIntVal << " symbiont: " << symIntVal;
//	     std::cout << " fight over " << resources << std::endl;
// 	     std::cout << "Host invests " << hostDefense << " in defense (which is lost), ";
	remainingResources = sym_piece - hostDefense;
// 	     std::cout << "leaving " << remainingResources << " available for reproduction. " << std::endl;
	     
	     // if both are hostile, then the symbiont must be more hostile than in order to gain any resources 
	if (symIntVal < hostIntVal) { //symbiont overcomes host's defenses
	  double symSteals = (hostIntVal - symIntVal) * remainingResources;
//	     	std::cout << "Symbiont steals " << symSteals << " resources." << std::endl;
	  symPortion = symSteals;
	  hostPortion = remainingResources - symSteals;
//	     	std::cout << "Leaving host receiving " << hostPortion << " resources." << std::endl;
	} else { // symbiont cannot overcome host's defenses
//	     	std::cout << "Symbiont cannot overcome host's defenses, and host keeps " << remainingResources << std::endl;
	     	
	  symPortion = 0.0;
	  hostPortion = remainingResources;
	     	
	}

	
	syms[i].AddPoints(symPortion);
	this->AddPoints(hostPortion);
	     
	
      } else if (hostIntVal > 0 && symIntVal < 0) {
	hostDonation = hostIntVal * sym_piece;
	hostPortion = sym_piece - hostDonation;
//		std::cout << "Host donates " << hostDonation << " to symbiont." << std::endl;
	sym_piece = sym_piece - hostDonation;
		
	double symSteals = -1.0 * (sym_piece * symIntVal);
	hostPortion = hostPortion - symSteals;
	symPortion = hostDonation + symSteals;
//		std::cout << "Symbiont steals an additional " << symSteals << " resources, ";
//		std::cout << "leaving host with " << hostPortion << " resources.  ";
//		std::cout << "Symbiont has " << symPortion << " at end." << std::endl;
		
	syms[i].AddPoints(symPortion);
	this->AddPoints(hostPortion);
		
		
      } else if (hostIntVal < 0 && symIntVal >= 0) {
	double hostDefense = -1.0 * (hostIntVal * sym_piece);
	hostPortion = sym_piece - hostDefense;
		
//		std::cout << "Host invests " << hostDefense << " in defense against a friendly symbiont." << std::endl;
//		std::cout << "Host keeps " << hostPortion << " and symbiont gets nothing." << std::endl;
		// symbiont gets nothing from antagonistic host
	symPortion = 0.0;
		
	syms[i].AddPoints(symPortion);
	this->AddPoints(hostPortion);
      } else {
//		std::cout << "Missed a logical case in distributing resources." << std::endl;
	//TODO: add error here
      }
    } //end syms[i] for loop

  } //end DistribResources

  void Process(emp::Random &random) {
    //Currently just wrapping to use the existing function
    //TODO: make the below config options
    DistribResources(100, 5); 

  }
  

};

/*std::string PrintHost(Host * org) {
  if (!org) return "-/-";
  
  std::stringstream temp;
  temp << std::fixed << std::setprecision(2) << org->GetIntVal();
  std::string formattedstring = temp.str();
  
  std::string out_val = formattedstring + "/" + PrintSym(org->GetSymbiont());
  
 // std::string out_val = emp::to_string(org->GetIntVal(),"/", PrintSym(org->GetSymbiont()));  // not completely formatted
  return out_val;
}

std::string PrintOrg(Host * org) {return PrintHost(org);}
*/

