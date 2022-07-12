#ifndef SGPWORLD_H
#define SGPWORLD_H

#include "../default_mode/DataNodes.h"
#include "../default_mode/SymWorld.h"
#include "Scheduler.h"
#include "Tasks.h"
#include <atomic>

// Helper to get around std::atomic<double> not having a specialization
struct AtomicDouble {
  std::atomic<double> inner = 0.0;

  operator double() { return inner.load(); }

  void operator+=(double add) {
    // compare-and-swap loop, should almost never loop more than once
    double current = inner.load();
    while (!inner.compare_exchange_weak(current, current + add))
      ;
  }

  void operator=(double val) { inner.store(val); }
};

class SGPWorld : public SymWorld {
private:
  Scheduler scheduler;
  TaskSet task_set;

public:
  AtomicDouble sym_points_donated;
  AtomicDouble sym_points_earned;
  emp::vector<std::pair<emp::Ptr<Organism>, emp::WorldPosition>> to_reproduce;

  SGPWorld(emp::Random &r, emp::Ptr<SymConfigBase> _config, TaskSet task_set)
      : SymWorld(r, _config), scheduler(*this, _config->THREAD_COUNT()),
        task_set(task_set) {}

  /**
   * Input: None
   *
   * Output: The task set used for this world.
   *
   * Purpose: Allows accessing the world's task set.
   */
  TaskSet &GetTaskSet() { return task_set; }

  /**
   * Input: None
   *
   * Output: The configuration used for this world.
   *
   * Purpose: Allows accessing the world's config.
   */
  emp::Ptr<SymConfigBase> GetConfig() { return my_config; }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To simulate a timestep in the world, which includes calling the
   * process functions for hosts and symbionts and updating the data nodes.
   */
  void Update() {
    // These must be done here because we don't call SymWorld::Update()
    // That may change in the future
    emp::World<Organism>::Update();
    if (my_config->PHYLOGENY())
      sym_sys->Update();

    scheduler.ProcessOrgs([&](emp::WorldPosition pos, Organism &org) {
      org.Process(pos);
      if (pop[pos.GetIndex()]->GetDead()) { // Check if the host died
        DoDeath(pos);
      }
    });

    for (auto org : to_reproduce) {
      if (!org.second.IsValid())
        continue;
      emp::Ptr<Organism> child = org.first->Reproduce();
      if (child->IsHost()) {
        // Host::Reproduce() doesn't take care of vertical transmission, that
        // happens here
        for (auto &sym : org.first->GetSymbionts()) {
          sym->VerticalTransmission(child);
        }
        DoBirth(child, org.second);
      } else {
        SymDoBirth(child, org.second);
      }
    }
    to_reproduce.clear();
  }
};

#endif