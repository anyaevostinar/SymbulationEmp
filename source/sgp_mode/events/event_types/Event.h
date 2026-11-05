#pragma once

#include "../EventTiming.h"

#include "../../../json/json.hpp"
#include "../../../json/json_utils.h"

#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/bits/Bits.hpp"
#include "emp/datastructs/set_utils.hpp"
#include "emp/datastructs/map_utils.hpp"
#include "emp/math/math.hpp"
#include "emp/tools/string_utils.hpp"

#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <string>
#include <unordered_map>


namespace sgpmode {

/**
 * Purpose: Helper struct used to manage minimum required specifications for new
 *          event types. Each event type contains a static const EventTypeDefinitionSpecs
 *          object, containing definitional information about the event type.
 */
struct EventTypeDefinitionSpecs {
  /**
   * Purpose: String that names this type of event.
   */
  std::string event_type;

  /**
   *  Purpose: String that provides a brief description of this event type.
   */
  std::string event_description;

  /**
   * Purpose: Vector of strings that lists the required fields for configuring
   *          an event of this type.
   */
  emp::vector<std::string> event_required_fields;

  /**
   * Purpose: Constructor.
   *
   * Input:
   *  - e_type: String specifying name of this event type.
   *  - e_desc: String specifying a description of this event type.
   *  - e_req_fields: Vector of strings specifying required fields for configuring
   *                  an event of this type.
   */
  EventTypeDefinitionSpecs(
    const std::string& e_type,
    const std::string& e_desc = "",
    const emp::vector<std::string>& e_req_fields = {}
  ) :
    event_type(e_type),
    event_description(e_desc),
    event_required_fields(e_req_fields)
  { ; }
};

/**
 * Purpose: Basic event type. Should be used as base class for event types.
 *          See ExampleEvent.h for a minimal example of how to implement a new
 *          event type. Each instance of any event will have this as its base class.
 */
class Event {
protected:
  /**
   * Purpose: Event type ID for this event. Filled in by the event manager when
   *          event is loaded from file.
   */
  size_t event_type_id = 0;

  /**
   * Purpose: Human-readable event type name.
   */
  std::string event_type{"NULL"}; //

  /**
   * Purpose: EventTiming object that specifies/manages this event's timing
   *          (one-time vs. recurring start/stop/step)
   */
  EventTiming timing;

  /**
   * Purpose: Boolean indicating whether this event is finished. Can be set by
   *          an event to end a recurring event before its end update.
   */
  bool is_done = false;

public:

  /**
   * Purpose: Get this event's type id.
   *
   * Input: None.
   *
   * Output: This event's event type id.
   *
   */
  size_t GetEventTypeID() const { return event_type_id; }

  /**
   * Purpose: Set this event's event type id.
   *
   * Input: New event type id.
   *
   * Output: None.
   *
   */
  void SetEventTypeID(size_t id) { event_type_id = id; }

  /**
   * Purpose: Check whether this event has been flagged as finished.
   *
   * Input: None.
   *
   * Output: Boolean indicating whether this event is finished.
   *
   */
  bool IsDone() const { return is_done; }

  /**
   * Purpose: Get this event's type string.
   *
   * Input: None.
   *
   * Output: String indicating this event's type.
   *
   */
  const std::string& GetEventType() const { return event_type; }

  /**
   * Purpose: Get this event's event timing object.
   *
   * Input: None.
   *
   * Output: This event's event timing object.
   *
   */
  const EventTiming& GetEventTiming() const { return timing; }

  /**
   * Purpose: Get whether this event is recurring.
   *
   * Input: None.
   *
   * Output: Boolean indicating whether this event is recurring.
   *
   */
  bool IsRecurring() const { return timing.IsRecurring(); }

  /**
   * Purpose: Get this event's start update.
   *
   * Input: None.
   *
   * Output: Unsigned integer indicating this event's start update.
   *
   */
  size_t GetStartUpdate() const { return timing.GetStartUpdate(); }

  /**
   * Purpose: Get this event's end update.
   *
   * Input: None.
   *
   * Output: Unsigned integer indicating this event's ending update.
   *
   */
  size_t GetEndUpdate() const { return timing.GetEndUpdate(); }

  /**
   * Purpose: Get this event's update frequency
   *
   * Input: None.
   *
   * Output: Unsigned integer indicating this event's update frequency.
   */
  size_t GetFrequency() const { return timing.GetFrequency(); }

  /**
   * Purpose: Get the update that this event will next occur.
   *
   * Input: None.
   *
   * Output: Unsigned integer indicating the update that this event will next occur.
   *
   */
  size_t GetNextUpdate() const { return timing.GetNextUpdate(); }

  /**
   * Purpose: Reset this event's timing as a non-recurring event with the given
   *          trigger update.
   *
   * Input: Start update to reset this event to.
   *
   * Output: None.
   *
   */
  void ResetTiming(size_t start_update) {
    timing.Reset(start_update);
  }

  /**
   * Purpose: Reset this event's timing as a recurring event with specified start
   *          update, end update, and frequency.
   *
   * Input: Unsigned integers indicating new start update, end update, and frequency.
   *
   * Output: None.
   *
   */
  void ResetTiming(size_t start, size_t end, size_t freq) {
    timing.Reset(start, end, freq);
  }

  /**
   * Purpose: Advance event timing (used when event is processed).
   *
   * Input: None.
   *
   * Output: Next event update after advancing.
   *
   */
  size_t AdvanceNextUpdate() {
    timing.Step();
    return timing.GetNextUpdate();
  }

};

/**
 * Purpose: Basic parsing function for event timing used by many event types.
 *
 * Input: json object with timing information, pointer to the event to configure
 *
 * Output: None.
 */
template <typename WORLD_T>
void SetEventTimingFromJSON(
  nlohmann::json& event_json,
  emp::Ptr<Event> event_ptr,
  WORLD_T& world
) {
  emp_assert(event_json.contains("timing"));
  const std::string timing_str(event_json["timing"]);
  // EventTiming timing;
  // Given as a single (integer) number?
  if (emp::is_digits(timing_str)) {
    const size_t start_update = emp::from_string<size_t>(timing_str);
    event_ptr->ResetTiming(start_update);
  } else {
    // Timing given as Start:Stop:Step
    emp::vector<std::string> recurring_str = emp::slice(timing_str, ':');
    emp_assert(recurring_str.size() == 3);
    const size_t start_update = emp::from_string<size_t>(recurring_str[0]);
    int stop_update = emp::from_string<int>(recurring_str[1]);
    if(stop_update < 0) {
      stop_update = world.GetConfig().UPDATES(); 
    }
    const size_t frequency = emp::from_string<size_t>(recurring_str[2]);
    event_ptr->ResetTiming(start_update, stop_update, frequency);
  }
}

}