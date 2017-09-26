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
  // interaction value 0.5 default to start
  Symbiont(double _intval=0.5, double _points = 0.0, std::set<int> _set = std::set<int>())
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
  
  // this prints the symbiont with two decimal places for ease in reading
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
  	
  void GetBackPoints(double _in)  {
  	double sym_portion = _in;  // current amount we are redistributing 
  	
  	// what the host will get back 
//  	std::cout << "Symbiont's interaction value is: " << sym.GetIntVal() << " "; 	
  	double sym_returns = sym_portion * sym.GetIntVal();
  	double host_gets = sym_returns + (5 * sym_returns);  // BUMP THIS BONUS WAY UP FOR REALZ
  	
  	// symbiont loses what it gives back
  	sym.AddPoints(-1 * sym_returns);
  	points += host_gets;
  
  }
  
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


