#pragma once

#include "emp/base/assert.hpp"

namespace sgpmode {

// note: add event timing helper class to manage event timing?

// Helper class for managing event timing
class EventTiming {
protected:
  size_t start_update;
  size_t end_update;
  size_t frequency;
  size_t next_update;
  bool recurring;
public:
  EventTiming() = default;

  // Constructor for recurring events
  EventTiming(size_t start_u, size_t end_u, size_t freq_u) :
    start_update(start_u),
    end_update(end_u),
    frequency(freq_u),
    recurring(true)
  {
    next_update = start_update;
  }

  // Constructor for one-time events
  EventTiming(size_t start_u) :
    start_update(start_u),
    end_update((size_t)-1),
    frequency((size_t)-1),
    recurring(false)
  {
    next_update = start_update;
  }

  void Reset(size_t start_u) {
    start_update = start_u;
    end_update = (size_t)-1;
    frequency = (size_t)-1;
    recurring = false;
    next_update = start_update;
  }

  void Reset(size_t start_u, size_t end_u, size_t freq_u) {
    emp_assert(start_u <= end_u);
    start_update = start_u;
    end_update = end_u;
    frequency = freq_u;
    recurring = true;
    next_update = start_update;
  }

  size_t GetStartUpdate() const { return start_update; }
  size_t GetEndUpdate() const { return end_update; }
  size_t GetFrequency() const { return frequency; }
  bool IsRecurring() const { return recurring; }
  size_t GetNextUpdate() const { return next_update; }

  void Step() {
    next_update += (recurring) ? frequency : 0;
  }

};

}