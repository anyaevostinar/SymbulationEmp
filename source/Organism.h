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
  bool dead = false;
  double points;
  /**
   * Purpose: Represents the number of updates the host
   * has lived through; at birth is set to 0.
   */
  size_t age = 0;

public:
  Organism(emp::Ptr<SymConfigBase> my_config, emp::Ptr<SymWorld> my_world,
           emp::Ptr<emp::Random> random, double points = 0.0)
      : my_config(my_config), my_world(my_world), random(random),
        points(points) {}

  virtual ~Organism() {}
  virtual bool operator<(const Organism &other) const {
    return (this < &other);
  };
  virtual bool operator==(const Organism &other) const {
    return (this == &other);
  }
  bool operator!=(const Organism &other) const { return !(*this == other); }

  virtual std::string const GetName() const { return "Organism"; }
  virtual bool IsHost() const = 0;
  virtual bool HasSym() const { return false; }
  virtual size_t AddSymbiont(emp::Ptr<BaseSymbiont> sym) = 0;
  virtual emp::vector<emp::Ptr<BaseSymbiont>> GetSymbionts() const = 0;
  virtual void Process(emp::WorldPosition location) = 0;
  virtual bool GetDead() const { return dead; }
  virtual void SetDead() { dead = true; }
  virtual void Mutate() = 0;
  virtual double GetPoints() const { return points; }
  virtual void AddPoints(double add) { points += add; }
  virtual void SetPoints(double set) { points = set; }
  virtual emp::Ptr<Organism> Reproduce() = 0;
  /**
   * Input: None
   *
   * Output: an int representing the current age of the Host
   *
   * Purpose: To get the Host's age.
   */
  virtual size_t GetAge() { return age; }
  virtual void SetAge(size_t _age) { age = _age; }

  /**
   * Input: None
   *
   * Output: Which of the NUM_PHYLO_BINS this organism should belong to.
   *
   * Purpose: Provides a mode-neutral way to classify organisms for phylogeny.
   * The default implementation puts every organism in bin 0, but it can be
   * overridden to add phylogeny support to an Organism subclass.
   */
  virtual int GetPhyloBin() const { return 0; }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Increments age by one and kills it if too old.
   */
  virtual void GrowOlder() {
    age = age + 1;
    size_t max =
        IsHost() ? my_config->HOST_AGE_MAX() : my_config->SYM_AGE_MAX();
    if (age > max && max > 0) {
      SetDead();
    }
  }
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
   * Output: None
   *
   * Purpose: To clear a host's symbionts.
   */
  void ClearSyms() { syms.resize(0); }

  /**
   * Input: None
   *
   * Output: A bool representing if a host has any symbionts.
   *
   * Purpose: To determine if a host has any symbionts, though they might be
   * corpses that haven't been removed yet.
   */
  bool HasSym() const override { return syms.size() != 0; }

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
protected:
  emp::Ptr<emp::Taxon<int>> my_taxon;
  emp::Ptr<BaseHost> my_host;
  double infection_chance;

public:
  BaseSymbiont() {
    infection_chance = my_config->SYM_INFECTION_CHANCE();
    // randomized starting infection chance
    if (infection_chance == -2)
      infection_chance = random->GetDouble(0, 1);
    // exception for invalid infection chance
    if (infection_chance > 1 || infection_chance < 0)
      throw "Invalid infection chance. Must be between 0 and 1";
  }

  bool IsHost() const override { return false; }
  double GetInfectionChance() const { return infection_chance; }

  virtual void SetHost(emp::Ptr<BaseHost> _in) { my_host = _in; }
  virtual emp::Ptr<BaseHost> GetHost() const { return my_host; }
  virtual emp::Ptr<emp::Taxon<int>> GetTaxon() { return my_taxon; }
  virtual void SetTaxon(emp::Ptr<emp::Taxon<int>> _in) { my_taxon = _in; }
  virtual double ProcessResources(double sym_piece, emp::Ptr<Organism> host) {
    AddPoints(sym_piece);
    return 0;
  };
  virtual void VerticalTransmission(emp::Ptr<Organism> host_baby);
  virtual emp::Ptr<BaseSymbiont> ReproduceSym() = 0;
  emp::Ptr<Organism> Reproduce() override { return ReproduceSym(); }
  virtual size_t AddSymbiont(emp::Ptr<BaseSymbiont> sym) override {
    // Symbionts can't have their own symbionts (but a subclass might)
    sym.Delete();
    return 0;
  }
  emp::vector<emp::Ptr<BaseSymbiont>> GetSymbionts() const override {
    return {};
  }

  virtual void Mutate() override {
    // modify infection chance, which is between 0 and 1
    double local_size = my_config->MUTATION_SIZE();
    if (my_config->FREE_LIVING_SYMS()) {
      infection_chance += random->GetRandNormal(0.0, local_size);
      if (infection_chance < 0)
        infection_chance = 0;
      else if (infection_chance > 1)
        infection_chance = 1;
    }
  }

  /**
   * Input: The double that will be the symbiont's infection chance
   *
   * Output: None
   *
   * Purpose: To set a symbiont's infection host and check that the proposed
   * value is valid.
   */
  void SetInfectionChance(double _in) {
    if (_in > 1 || _in < 0)
      throw "Invalid infection chance. Must be between 0 and 1 (inclusive)";
    else
      infection_chance = _in;
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
