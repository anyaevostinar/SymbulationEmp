#pragma once

#include "../Organism.h"

#include <deque>

namespace sgpmode {

// TODO - tests!

// Encapsulates information about a reproduction request
struct ReproEvent {
  emp::Ptr<Organism> org; // Organism to reproduce
  emp::WorldPosition pos; // Location of reproduction event in world
  bool valid = true;
  // size_t event_id = 0;
};

/*
  Tracks organisms queued for reproduction.
*/
class ReproductionQueue {
public:
protected:
  // TODO - queue vector
  emp::vector<ReproEvent> queue;
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

  emp::vector<ReproEvent>& GetQueue() {
    return queue;
  }

  const emp::vector<ReproEvent>& GetQueue() const {
    return queue;
  }


};

}