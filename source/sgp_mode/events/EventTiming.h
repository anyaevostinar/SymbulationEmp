#pragma once

#include "emp/base/assert.hpp"

namespace sgpmode {

/**
 * Purpose: Manages timing for events.
 */
class EventTiming {
protected:
  /**
   * Purpose: Start update for this event.
   */
  size_t start_update;

  /**
   * Purpose: End update for this event if recurring.
   */
  size_t end_update;

  /**
   * Purpose: Update interval for this event if recurring.
   */
  size_t frequency;

  /**
   * Purpose: Next update that this event should occur if recurring.
   */
  size_t next_update;

  /**
   * Purpose: Boolean indicating whether this is a recurring or one-time event.
   */
  bool recurring;

public:
  /**
   * Purpose: Default constructor.
   */
  EventTiming() = default;

  /**
   * Purpose: Constructor for recurring events.
   *
   * Inputs: Specify start update, end update, and frequency for recurring event.
   */
  EventTiming(size_t start_u, size_t end_u, size_t freq_u) :
    start_update(start_u),
    end_update(end_u),
    frequency(freq_u),
    recurring(true)
  {
    next_update = start_update;
  }

  /**
   * Purpose: Constructor for one-time events.
   *
   * Inputs: Specify the start update for this one-time event.
   */
  EventTiming(size_t start_u) :
    start_update(start_u),
    end_update((size_t)-1),
    frequency((size_t)-1),
    recurring(false)
  {
    next_update = start_update;
  }

  /**
   * Purpose: Reset this event as a one-time event.
   *
   * Inputs: Start update.
   *
   * Outputs: None.
   */
  void Reset(size_t start_u) {
    start_update = start_u;
    end_update = (size_t)-1;
    frequency = (size_t)-1;
    recurring = false;
    next_update = start_update;
  }

  /**
   * Purpose: Reset this event as a recurring event.
   *
   * Inputs: Start update, end update, and frequency.
   *
   * Outputs: None.
   */
  void Reset(size_t start_u, size_t end_u, size_t freq_u) {
    emp_assert(start_u <= end_u);
    start_update = start_u;
    end_update = end_u;
    frequency = freq_u;
    recurring = true;
    next_update = start_update;
  }

  /**
   * Purpose: Get this event's start update.
   *
   * Inputs: None.
   *
   * Outputs: This event's start update.
   */
  size_t GetStartUpdate() const { return start_update; }

  /**
   * Purpose: Get this event's end update.
   *
   * Inputs: None.
   *
   * Outputs: This event's end update.
   */
  size_t GetEndUpdate() const { return end_update; }

  /**
   * Purpose: Get this event's frequency.
   *
   * Inputs: None.
   *
   * Outputs: This event's update interval frequency.
   */
  size_t GetFrequency() const { return frequency; }

  /**
   * Purpose: Check whether this event is recurring.
   *
   * Inputs: None.
   *
   * Outputs: Boolean indicating whether this event is recurring.
   */
  bool IsRecurring() const { return recurring; }

  /**
   * Purpose: Get this event's next update.
   *
   * Inputs: None.
   *
   * Outputs: The update that this event will trigger next.
   */
  size_t GetNextUpdate() const { return next_update; }

  /**
   * Purpose: Step the event timing forward if it is recurring.
   *
   * Inputs: None.
   *
   * Outputs: None.
   */
  void Step() {
    next_update += (recurring) ? frequency : 0;
  }

};

}