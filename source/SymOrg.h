#include "source/tools/Random.h"
#include "source/tools/string_utils.h"
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
  std::set<int> GetResTypes() const {return res_types;}

  void SetIntVal(double _in) { interaction_val = _in;}
  void SetPoints(double _in) { points = _in;}
  void AddPoints(double _in) { points += _in;}
  void SetResTypes(std::set<int> _in) {res_types = _in;}

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
  Symbiont sym;
  std::set<int> res_types;
  double points;

public:
 Host(double _intval =0.0, Symbiont _sym = *(new Symbiont(0, -1)), std::set<int> _set = std::set<int>(), double _points = 0.0) : interaction_val(_intval), sym(_sym), res_types(_set), points(_points) { ; }
  Host(const Host &) = default;
  Host(Host &&) = default;
  // Host() : interaction_val(0), sym(*(new Symbiont(0, -1))), res_types(std::set<int>()), points(0) { ; }


  Host & operator=(const Host &) = default;
  Host & operator=(Host &&) = default;
  bool operator==(const Host &other) const { return (this == &other);}
  bool operator!=(const Host &other) const {return !(*this == other);}


  double GetIntVal() const { return interaction_val;}
  Symbiont GetSymbiont() { return sym;}
  std::set<int> GetResTypes() const { return res_types;}
  double GetPoints() { return points;}


  void SetIntVal(double _in) {interaction_val = _in;}
  void SetSymbiont(Symbiont _in) {sym = _in;}
  void SetResTypes(std::set<int> _in) {res_types = _in;}
  void SetPoints(double _in) {points = _in;}
  void AddPoints(double _in) {points += _in;}
  
  void GiveSymPoints(double _in) {
    double distrib = _in;
    sym.AddPoints(distrib);
    
  }
  
  void ResetSymPoints() {
    sym.SetPoints(0.0);
  }
  	

  
  void SetSymIntVal (double _in) {
    sym.SetIntVal(_in);
  
  }
  
  void DeleteSym() {
    sym.SetPoints(-1.0);
  }
  
  bool HasSym() {
    if (sym.GetPoints() < 0) { 
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
  // might want to declare a remainingResources variable just to make this easier to maintain
    double hostIntVal = interaction_val; //using private variable because we can
    double symIntVal = sym.GetIntVal();
    
    double hostPortion = 0.0;
    double hostDonation = 0.0;
    double symPortion = 0.0;
    double symReturn = 0.0;
    double bonus = synergy; 

	
//	std::cout << "Dividing resources: " << resources << std::endl;
	if (hostIntVal >= 0 && symIntVal >= 0)  {  
	    hostDonation = resources * hostIntVal;
	    hostPortion = resources - hostDonation;  
	    
//	    std::cout << "Host keeps " << hostPortion << " and gives " << hostDonation << " to symbiont." << std::endl;
	    
	    symReturn = (hostDonation * symIntVal) * bonus;  
	    symPortion = hostDonation - (hostDonation * symIntVal);
	    
//	    std::cout << "Symbiont keeps " << symPortion << " and returns " << symReturn << " to host (multiplied by) " << bonus << std::endl;

	    hostPortion += symReturn;
	    
//	    std::cout << "In the end, host gets " << hostPortion << std::endl;
	    
	    this->GiveSymPoints(symPortion);
	    this->AddPoints(hostPortion);
	    
	} else if (hostIntVal <= 0 && symIntVal < 0) {  // NEED TO CHECK THAT THIS IS CORRECT - see dissertation
	     double hostDefense = -1.0 * (hostIntVal * resources);
	     double remainingResources = 0.0;
//	     std::cout << "Host: " << hostIntVal << " symbiont: " << symIntVal;
//	     std::cout << " fight over " << resources << std::endl;
// 	     std::cout << "Host invests " << hostDefense << " in defense (which is lost), ";
	     remainingResources = resources - hostDefense;
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

	     
	    this->GiveSymPoints(symPortion);
	    this->AddPoints(hostPortion);
	     
	
	} else if (hostIntVal > 0 && symIntVal < 0) {
		hostDonation = hostIntVal * resources;
		hostPortion = resources - hostDonation;
//		std::cout << "Host donates " << hostDonation << " to symbiont." << std::endl;
		resources = resources - hostDonation;
		
		double symSteals = -1.0 * (resources * symIntVal);
		hostPortion = hostPortion - symSteals;
		symPortion = hostDonation + symSteals;
//		std::cout << "Symbiont steals an additional " << symSteals << " resources, ";
//		std::cout << "leaving host with " << hostPortion << " resources.  ";
//		std::cout << "Symbiont has " << symPortion << " at end." << std::endl;
		
		this->GiveSymPoints(symPortion);
	    this->AddPoints(hostPortion);
		
		
	} else if (hostIntVal < 0 && symIntVal >= 0) {
		double hostDefense = -1.0 * (hostIntVal * resources);
		hostPortion = resources - hostDefense;
		
//		std::cout << "Host invests " << hostDefense << " in defense against a friendly symbiont." << std::endl;
//		std::cout << "Host keeps " << hostPortion << " and symbiont gets nothing." << std::endl;
		// symbiont gets nothing from antagonistic host
		symPortion = 0.0;
		
		this->GiveSymPoints(symPortion);
	    this->AddPoints(hostPortion);
	} else {
//		std::cout << "Missed a logical case in distributing resources." << std::endl;
	}

  }

  void Process(emp::Random &random) {
    //Currently just wrapping to use the existing function
    //TODO: make the below config options
    DistribResources(100, 5); 

  }
  

};

std::string PrintHost(Host * org) {
  if (!org) return "-/-";
  
  std::stringstream temp;
  temp << std::fixed << std::setprecision(2) << org->GetIntVal();
  std::string formattedstring = temp.str();
  
  std::string out_val = formattedstring + "/" + PrintSym(org->GetSymbiont());
  
 // std::string out_val = emp::to_string(org->GetIntVal(),"/", PrintSym(org->GetSymbiont()));  // not completely formatted
  return out_val;
}

std::string PrintOrg(Host * org) {return PrintHost(org);}


