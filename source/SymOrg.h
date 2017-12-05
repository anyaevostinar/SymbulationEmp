#include "tools/Random.h"
#include "tools/string_utils.h"
#include <set>
#include <iomanip> // setprecision
#include <sstream> // stringstream


class Symbiont {
private:  
  double interaction_val;
  double points;
  std::set<int> res_types;


public:
  // neutral interaction value 0.0 default to start - should this be configurable from file/UI?
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

};

std::string PrintSym(Symbiont  org){
  if (org.GetPoints() < 0) return "-";
  double out_val = org.GetIntVal();   // fixed the printing 0 for 0.5 issue by declaring it a double rather than int
  
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
// neutral interaction value 0.0 default to start - should this be configurable from file/UI?
 Host(double _intval =0.0, Symbiont _sym = Symbiont(), std::set<int> _set = std::set<int>(), double _points = 0.0) : interaction_val(_intval), sym(_sym), res_types(_set), points(_points) { ; }
  Host(const Host &) = default;
  Host(Host &&) = default;

  Host & operator=(const Host &) = default;
  Host & operator=(Host &&) = default;

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
  	
  /*
  void GetBackPoints(double _in, double synergy)  {   // Obsolete testing function
  	double sym_portion = _in;  // current amount we are redistributing 
  	double bonus = synergy; // the multiplier when resources are returned to host
  	
  	// what the host will get back 
//  	std::cout << "Symbiont's interaction value is: " << sym.GetIntVal() << " "; 	
  	double sym_returns = sym_portion * sym.GetIntVal();
  	double host_gets = sym_returns + (bonus * sym_returns);  
  	
  	// symbiont loses what it gives back
  	sym.AddPoints(-1 * sym_returns);
  	points += host_gets;
  
  }
  */
  
  void SetSymIntVal (double _in) {
  	sym.SetIntVal(_in);
  
  }
  
  bool HasSym() {
  	if (sym.GetPoints() < 0) { 
  		return false;
  	} else {
  	    return true;
  	}
  	
  }
  
  void DistribResources(int resources, double hostIntVal, double symIntVal, double synergy) { 
  // might want to declare a remainingResources variable just to make this easier to maintain
	
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
//	     std::cout << "Host invests " << hostDefense << " in defense (which is lost), ";
	     resources = resources - hostDefense;
//	     std::cout << "leaving " << resources << " available for reproduction. " << std::endl;
	     
	     double symSteals = (hostIntVal - symIntVal) * resources;
//	     std::cout << "Symbiont steals " << symSteals << " resources." << std::endl;
	     symPortion = symSteals;
	     
	     hostPortion = resources - symSteals;
//	     std::cout << "Leaving host receiving " << hostPortion << " resources." << std::endl;
	     
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
		std::cout << "Missed a logical case in distributing resources." << std::endl;
	}

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


