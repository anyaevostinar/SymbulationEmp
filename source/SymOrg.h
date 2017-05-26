#include "tools/Random.h"
#include "tools/string_utils.h"
#include <set>


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

};

std::string PrintSym(Symbiont  org){
  if (org.GetPoints() < 0) return "-";
  int out_val = org.GetIntVal();
  return emp::to_string(out_val);

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

};

std::string PrintHost(Host * org) {
  if (!org) return "-/-";
  std::string out_val = emp::to_string(org->GetIntVal(),"/", PrintSym(org->GetSymbiont()));
  return out_val;
}

std::string PrintOrg(Host * org) {return PrintHost(org);}
