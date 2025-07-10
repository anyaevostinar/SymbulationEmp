#ifndef SGPSYMBIONT_H
#define SGPSYMBIONT_H

#include "../default_mode/Symbiont.h"
#include "CPU.h"
#include "SGPHost.h"
#include "SGPWorld.h"
#include "emp/Evolve/World_structure.hpp"

class SGPSymbiont : public Symbiont {
private:
  CPU cpu;
  const emp::Ptr<SGPWorld> my_world;

  /**
   *
   * Purpose: Tracks the number of reproductive events in this symbiont's lineage.
   *
   */
  unsigned int reproductions = 0;

protected:
  /**
   * 
   * Purpose: Holds all configuration settings and points to same configuration
   * object as my_config from superclass, but with the correct subtype.
   * 
   */
  emp::Ptr<SymConfigSGP> sgp_config = NULL;
public:
  /**
   * Constructs a new SGPSymbiont as an ancestor organism, with either a random
   * genome or a blank genome that knows how to do a simple task depending on
   * the config setting RANDOM_ANCESTOR.
   */
  SGPSymbiont(emp::Ptr<emp::Random> _random, emp::Ptr<SGPWorld> _world,
              emp::Ptr<SymConfigSGP> _config, double _intval = 0.0,
              double _points = 0.0)
      : Symbiont(_random, _world, _config, _intval, _points), cpu(this, _world),
        my_world(_world) {
    sgp_config = _config;
  }

  /**
   * Constructs an SGPSymbiont with a copy of the provided genome.
   */
  SGPSymbiont(emp::Ptr<emp::Random> _random, emp::Ptr<SGPWorld> _world,
              emp::Ptr<SymConfigSGP> _config,
              const sgpl::Program<Spec> &genome, double _intval = 0.0,
              double _points = 0.0)
      : Symbiont(_random, _world, _config, _intval, _points),
        cpu(this, _world, genome), my_world(_world) {
    sgp_config = _config;
  }

  SGPSymbiont(const SGPSymbiont &symbiont)
      : Symbiont(symbiont),
        cpu(this, symbiont.my_world, symbiont.cpu.GetProgram()),
        my_world(symbiont.my_world) {}

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Perform necessary cleanup when a symbiont dies, freeing
   * heap-allocated state and canceling any in-progress reproduction.
   */
  ~SGPSymbiont() {
    // Invalidate any in-progress reproduction
    if (cpu.state.in_progress_repro != -1 && my_world->to_reproduce.size() > cpu.state.in_progress_repro) {
      //TODO: figure out why the second part of the line above is necessary and also write a test
      my_world->to_reproduce[cpu.state.in_progress_repro].second =
          emp::WorldPosition::invalid_id;
    }
  }

  bool operator<(const Organism &other) const {
    if (const SGPSymbiont *sgp = dynamic_cast<const SGPSymbiont *>(&other)) {
      return cpu.GetProgram() < sgp->cpu.GetProgram();
    } else {
      return false;
    }
  }

  bool operator==(const Organism &other) const {
    if (const SGPSymbiont *sgp = dynamic_cast<const SGPSymbiont *>(&other)) {
      return cpu.GetProgram() == sgp->cpu.GetProgram();
    } else {
      return false;
    }
  }

  /**
   * Input: Set the reproduction counter
   *
   * Output: None
   *
   * Purpose: To set the count of reproductions in this lineage.
   */
  void SetReproCount(int _in) { reproductions = _in; }
 
  /**
   * Input: None.
   *
   * Output: The reproduction count
   *
   * Purpose: To get the count of reproductions in this lineage.
   */
  unsigned int GetReproCount() { return reproductions; }

  void AddPoints(double _in) {
    if(my_host && _in == 5.0) {
      //Would need to check 5.0 check to check if sym actually did same task as host somehow
      //my_host->AddPoints(-_in * 0.5);
    }
    points += _in;
  }
  /**
   * Input: None
   *
   * Output: The CPU associated with this symbiont.
   *
   * Purpose: Allows accessing the symbiont's CPU.
   */
  CPU &GetCPU() { return cpu; }

  /**
   * Input: The location of the symbiont, which includes the symbiont's position
   * in the host (default -1 if it doesn't have a host)
   *
   * Output: None
   *
   * Purpose: To process a symbiont, meaning running its program code, which
   * can include reproduction and acquisition of resources; and to allow for
   * movement
   */
  void Process(emp::WorldPosition pos) {
    // Instead of calling Host::Process, do the important stuff here
    // Our instruction handles reproduction
    if (GetDead()) {
      return;
    }
    
    cpu.RunCPUStep(pos, sgp_config->CYCLES_PER_UPDATE());
    // The parts of Symbiont::Process that don't use resources or reproduction

    // Age the organism
    GrowOlder();
  }

  /**
   * Input: The pointer to the organism that is the new host baby
   *
   * Output: None
   *
   * Purpose: To allow for vertical transmission to occur. This performs extra
   * bookkeeping on top of `Symbiont::VerticalTransmission()` to avoid messing
   * with the reproduction queue which is used for horizontal transmission.
   */
  void VerticalTransmission(emp::Ptr<Organism> host_baby) {
    // Save and restore the in-progress reproduction, since Reproduce() will be
    // called but it will still be on the queue for horizontal transmission
    size_t old = cpu.state.in_progress_repro;
    Symbiont::VerticalTransmission(host_baby);
    cpu.state.in_progress_repro = old;
  }


  /**
   * Input: None
   *
   * Output: The pointer to the newly created organism
   *
   * Purpose: To produce a new SGPSymbiont
   */
  emp::Ptr<Organism> Reproduce() {
    emp::Ptr<SGPSymbiont> sym_baby = Symbiont::Reproduce().DynamicCast<SGPSymbiont>();
    sym_baby->SetReproCount(reproductions + 1);
    // This organism is reproducing, so it must have gotten off the queue
    cpu.state.in_progress_repro = -1;
    if (sgp_config->TRACK_PARENT_TASKS()) {
      sym_baby->GetCPU().state.parent_tasks_performed->Import(*GetCPU().state.tasks_performed);
      //inherit towards-from tracking
      for (int i = 0; i < CPU_BITSET_LENGTH; i++) {

        // lineage task gain / loss
        sym_baby->GetCPU().state.task_change_lose[i] = cpu.state.task_change_lose[i];
        sym_baby->GetCPU().state.task_change_gain[i] = cpu.state.task_change_gain[i];
        if (cpu.state.tasks_performed->Get(i) && !cpu.state.parent_tasks_performed->Get(i)) {
          // child gains the ability to infect hosts whose parents have done this task
          sym_baby->GetCPU().state.task_change_gain[i] = cpu.state.task_change_gain[i] + 1;
        }
        else if (!cpu.state.tasks_performed->Get(i) && cpu.state.parent_tasks_performed->Get(i)) {
          // child loses the ability to infect hosts with whom this parent had only this task in common 
          sym_baby->GetCPU().state.task_change_lose[i] = cpu.state.task_change_lose[i] + 1;
        }

        if (my_host) {
          // divergence from/convergence towards parent's partner
          emp::Ptr<emp::BitSet<CPU_BITSET_LENGTH>> host_tasks = my_host.DynamicCast<SGPHost>()->GetCPU().state.parent_tasks_performed;
          sym_baby->GetCPU().state.task_from_partner[i] = cpu.state.task_from_partner[i];
          sym_baby->GetCPU().state.task_toward_partner[i] = cpu.state.task_toward_partner[i];
          if (cpu.state.parent_tasks_performed->Get(i) != host_tasks->Get(i) &&
            cpu.state.tasks_performed->Get(i) == host_tasks->Get(i)) {
            // parent != partner and child == partner
            sym_baby->GetCPU().state.task_toward_partner[i] = cpu.state.task_toward_partner[i] + 1;
          }
          else if (cpu.state.parent_tasks_performed->Get(i) == host_tasks->Get(i) &&
            cpu.state.tasks_performed->Get(i) != host_tasks->Get(i)) {
            // parent == partner and child != partner
            sym_baby->GetCPU().state.task_from_partner[i] = cpu.state.task_from_partner[i] + 1;
          }
        }
      }
    }
    return sym_baby;
  }


  /**
   * Input: None
   *
   * Output: The pointer to the newly created organism
   *
   * Purpose: To produce a new symbiont, identical to the original
   */
  emp::Ptr<Organism> MakeNew() {
    emp::Ptr<SGPSymbiont> sym_baby = emp::NewPtr<SGPSymbiont>(
        random, my_world, sgp_config, cpu.GetProgram(), GetIntVal());
    return sym_baby;
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To mutate the code in the genome of this symbiont.
   */
  void Mutate() {
    Symbiont::Mutate();

    cpu.Mutate();
  }
};

#endif