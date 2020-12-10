#ifndef ORGANISM_H
#define ORGANISM_H

#include <set>
#include <iomanip> // setprecision
#include <sstream> // stringstream

class Organism {

  public:

  Organism() = default;
  Organism(const Organism &) = default;
  Organism(Organism &&) = default;
  virtual ~Organism() {}
  Organism & operator=(const Organism &) = default;
  Organism & operator=(Organism &&) = default;
  bool operator==(const Organism &other) const {return (this == &other);}
  bool operator!=(const Organism &other) const {return !(*this == other);}

  virtual double GetIntVal() const {
    std::cout << "This shouldn't happen either" << std::endl;
    return 0.0;}
  virtual double GetPoints() {return 0.0;}
  virtual void SetIntVal(double _in) {}
  virtual void SetPoints(double _in) {}
  virtual void AddPoints(double _in) {}
  virtual void SetHost(emp::Ptr<Organism> _in) {}

  //Symbiont functions

  virtual void mutate() {}
  virtual void process(size_t location) {}
  virtual emp::Ptr<Organism> reproduce() {return nullptr;}

  //Host functions

  virtual emp::vector<emp::Ptr<Organism>>& GetSymbionts() {
    std::cout << "This shouldn't happen" << std::endl;
    return *(new emp::vector<emp::Ptr<Organism>>());}
  virtual emp::vector<emp::Ptr<Organism>>& GetReproSymbionts() {
    return *(new emp::vector<emp::Ptr<Organism>>());}
  virtual std::set<int> GetResTypes() const {return {};}
  virtual void SetSymbionts(emp::vector<emp::Ptr<Organism>> _in) {}
  virtual void SetResTypes(std::set<int> _in) {}
  virtual void AddSymbiont(emp::Ptr<Organism> _in, int sym_limit) {}
  virtual void AddReproSym(emp::Ptr<Organism> _in) {}
  virtual bool HasSym() {return false;}
  virtual void DistribResources(double resources, double synergy) {}
  virtual void Process(double resources_per_host_per_update, double synergy)  {}

  //Phage functions
  virtual double GetBurstTimer() {return 0.0;}
  virtual void IncBurstTimer() {}
  virtual void SetBurstTimer(int _in) {}

};

#endif