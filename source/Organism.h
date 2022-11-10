#ifndef ORGANISM_H
#define ORGANISM_H

#include "ConfigSetup.h"
#include "emp/Evolve/Systematics.hpp"
#include "emp/Evolve/World_structure.hpp"
#include "emp/math/Random.hpp"
#include <string>

class BaseHost;
class BaseSymbiont;
class SymWorld;

class Organism {
protected:
  emp::Ptr<SymConfigBase> my_config;
  emp::Ptr<SymWorld> my_world;
  emp::Ptr<emp::Random> random;
  bool dead;
  double points;

public:
  Organism(emp::Ptr<SymConfigBase> my_config, emp::Ptr<SymWorld> my_world,
           emp::Ptr<emp::Random> random)
      : my_config(my_config), my_world(my_world), random(random) {}

  virtual ~Organism() {}
  Organism &operator=(const Organism &) = default;
  Organism &operator=(Organism &&) = default;
  virtual bool operator<(const Organism &other) const {
    std::cout << "operator< called from Organism" << std::endl;
    throw "Organism method called!";
  };
  virtual bool operator==(const Organism &other) const {
    return (this == &other);
  }
  bool operator!=(const Organism &other) const { return !(*this == other); }

  // virtual std::string const GetName() const = 0;
  virtual bool IsHost() const = 0;
  virtual size_t AddSymbiont(emp::Ptr<BaseSymbiont> sym) = 0;
  virtual emp::vector<emp::Ptr<BaseSymbiont>> GetSymbionts() const = 0;
  virtual void Process(emp::WorldPosition location) = 0;
  virtual bool GetDead() const { return dead; };
  virtual void Mutate() = 0;
  virtual double GetPoints() const { return points; }
  virtual void AddPoints(double add) { points += add; }
};

class BaseHost : public virtual Organism {
protected:
  emp::vector<emp::Ptr<BaseSymbiont>> syms;

public:
  bool IsHost() const override { return true; }

  /**
   * Input: The pointer to the organism that is to be added to the host's
   * symbionts.
   *
   * Output: The int describing the symbiont's position ID, or 0 if it did not
   * successfully get added to the host's list of symbionts.
   *
   * Purpose: To add a symbionts to a host's symbionts
   */
  size_t AddSymbiont(emp::Ptr<BaseSymbiont> _in) override;

  emp::vector<emp::Ptr<BaseSymbiont>> GetSymbionts() const override {
    return syms;
  }

  /**
   * Input: None
   *
   * Output: A bool representing if a symbiont will be allowed to enter a host.
   *
   * Purpose: To determine if a symbiont will be allowed into a host. If phage
   * exclusion is off, this function will always return true. If phage exclusion
   * is on, then there is a 1/2^n chance of a new phage being allowed in, where
   * n is the number of existing phage.
   */
  bool SymAllowedIn() const;
};

class BaseSymbiont : public virtual Organism {
private:
  emp::Ptr<emp::Taxon<int>> taxon;
  emp::Ptr<BaseHost> host;
  double infection_chance;

public:
  bool IsHost() const override { return false; }
  double GetInfectionChance() const { return infection_chance; }

  virtual void SetHost(emp::Ptr<BaseHost> _in) { host = _in; }
  virtual emp::Ptr<emp::Taxon<int>> GetTaxon() { return taxon; }
  virtual void SetTaxon(emp::Ptr<emp::Taxon<int>> _in) { taxon = _in; }
  virtual double ProcessResources(double sym_piece, emp::Ptr<Organism> host) {
    AddPoints(sym_piece);
    return 0;
  };
  virtual void VerticalTransmission(emp::Ptr<Organism> host_baby);
  virtual emp::Ptr<BaseSymbiont> Reproduce() = 0;
  virtual size_t AddSymbiont(emp::Ptr<BaseSymbiont> sym) override {
    // Symbionts can't have their own symbionts (but a subclass might)
    sym.Delete();
    return 0;
  }
  emp::vector<emp::Ptr<BaseSymbiont>> GetSymbionts() const override {
    return {};
  }

  /**
   * Input: None
   *
   * Output: The boolean representing if a symbiont
   * will seek out to infect a host.
   *
   * Purpose: To determine if a symbiont wants to
   * infect a host based upon its infection chance
   */
  virtual bool WantsToInfect() const {
    bool result = random->GetDouble(0.0, 1.0) < infection_chance;
    return result;
  }

  /**
   * Input: None
   *
   * Output: The boolean representing if a symbiont
   * will survive crossing over into the host world.
   *
   * Purpose: To determine if a symbiont will survive
   * crossing over into the host world based on infection risk.
   */
  virtual bool InfectionFails() {
    // note: this can be returned true, and an infecting sym can then be killed
    // by a host that is already infected.
    bool sym_dies =
        random->GetDouble(0.0, 1.0) < my_config->SYM_INFECTION_FAILURE_RATE();
    return sym_dies;
  }
};

#endif
