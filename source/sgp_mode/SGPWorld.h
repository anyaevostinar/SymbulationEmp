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
      : SymWorld(r, _config), task_set(task_set) {}

  TaskSet &GetTaskSet() { return task_set; }

  void Update() {
    // These must be done here because we don't call SymWorld::Update()
    // That may change in the future
    emp::World<Organism>::Update();
    if (my_config->PHYLOGENY())
      sym_sys->Update();

    scheduler.ProcessOrgs(*this, [&](emp::WorldPosition pos, Organism &org) {
      org.Process(pos);
      if (pop[pos.GetIndex()]->GetDead()) { // Check if the host died
        DoDeath(pos);
      }
    });

    std::unordered_set<uint32_t> replaced;
    for (auto org : to_reproduce) {
      if (replaced.count(org.second.GetIndex())) {
        // This organism has been replaced, it's dead
        continue;
      }
      emp::Ptr<Organism> child = org.first->Reproduce();
      if (child->IsHost()) {
        // Host::Reproduce() doesn't take care of vertical transmission, that
        // happens here
        for (auto &sym : org.first->GetSymbionts()) {
          sym->VerticalTransmission(child);
        }
        emp::WorldPosition new_pos = DoBirth(child, org.second);
        replaced.insert(new_pos.GetIndex());
      } else {
        SymDoBirth(child, org.second);
        // A host and its sym can't both reproduce in the same update, but that
        // doesn't really matter
        replaced.insert(org.second.GetIndex());
      }
    }
    to_reproduce.clear();
  }
};

#endif