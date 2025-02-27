#pragma once

#include "../Organism.h"

#include <deque>
#include <functional>

namespace sgpmode {

// TODO - tests!

// Encapsulates information about a reproduction request
struct ReproEvent {
  emp::Ptr<Organism> org; // Organism to reproduce
  emp::WorldPosition pos; // Location of reproduction event in world
  bool valid = true;
  // size_t event_id = 0;
  ReproEvent() = default;
  ReproEvent(
    emp::Ptr<Organism> in_org,
    const emp::WorldPosition& in_pos,
    bool in_valid=true
  ) : org(in_org), pos(in_pos), valid(in_valid) { }
};

/*
  Tracks organisms queued for reproduction.
*/
class ReproductionQueue {
public:
  using fun_repro_org_t = std::function<void(ReproEvent&)>;
protected:
  // TODO - queue vector
  emp::vector<ReproEvent> queue;
  fun_repro_org_t fun_reproduce_org;
  // TODO - set tracking what is in the queue
  // std::unordered_set in
  // TODO - next_id
  // size_t next_id = 0; // id to assign to next reproduction event (will be unique with respect to all other currently queued events)

public:

  void Clear() {
    queue.clear();
  }

  size_t GetSize() const {
    return queue.size();
  }

  const emp::vector<ReproEvent>& GetQueue() const {
    return queue;
  }

  void SetReproduceOrgFun(fun_repro_org_t fun) {
    fun_reproduce_org = fun;
  }

  // Add organism to queue, return organism's queue id (valid until queue is processed)
  size_t Enqueue(
    emp::Ptr<Organism> org_ptr,
    const emp::WorldPosition& org_pos
  ) {
    const size_t queue_id = queue.size();
    queue.emplace_back(org_ptr, org_pos, true);
    return queue_id;
  }

  // Queue must be processed all at once to avoid
  // invalidating queue_ids.
  // NOTE - could have world template and instead of a configurable functor.
  //        Why functor? Currently resetting repro
  // template<typename WORLD_T>
  // void Process(WORLD_T& world) {

  void Process() {
    for (ReproEvent& repro_info : queue) {
      emp::Ptr<Organism> org_ptr = repro_info.org;
      // If queued organism is dead or repro event has been invalidated,
      // don't reproduce.
      if (!repro_info.valid || org_ptr->GetDead()) {
        continue;
      }
      fun_reproduce_org(repro_info);
      // emp::Ptr<Organism> child = org->Reproduce();
      // (child->IsHost()) ?
      //   world.HostDoBirth(child, org_ptr, repro_info.pos) :
      //   world.SymDoBirth(child, repro_info.pos);
    }
    Clear();
  }

};

}