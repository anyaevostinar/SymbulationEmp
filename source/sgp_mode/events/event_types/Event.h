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

// Helper struct used by derived event types.
struct EventTypeDefinitionSpecs {
  std::string event_type;
  std::string event_description;
  emp::vector<std::string> event_required_fields;

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

// Basic event type. Should be used as base class for event types.
// See ExampleEvent.h for a minimal example of how to implement a new event type.
class Event {
protected:
  size_t event_type_id = 0;       // Event type ID for this event. Filled in when event is loaded from file.
  std::string event_type{"NULL"}; // Human-readable event type.
  EventTiming timing;             // Manages event's timing (one-time vs. recurring start/stop/step)
  bool is_done = false;           // Can be used by an event to end a recurring event before its end update

public:
  size_t GetEventTypeID() const { return event_type_id; }
  void SetEventTypeID(size_t id) { event_type_id = id; }
  bool IsDone() const { return is_done; }
  const std::string& GetEventType() const { return event_type; }
  const EventTiming& GetEventTiming() const { return timing; }
  bool IsRecurring() const { return timing.IsRecurring(); }
  size_t GetStartUpdate() const { return timing.GetStartUpdate(); }
  size_t GetEndUpdate() const { return timing.GetEndUpdate(); }
  size_t GetNextUpdate() const { return timing.GetNextUpdate(); }

  void ResetTiming(size_t start_update) {
    timing.Reset(start_update);
  }

  void ResetTiming(size_t start, size_t end, size_t freq) {
    timing.Reset(start, end, freq);
  }

  // Advance next update, return new next update.
  size_t AdvanceNextUpdate() {
    timing.Step();
    return timing.GetNextUpdate();
  }

};

// Common parsing function used by many event types.
void SetEventTimingFromJSON(
  nlohmann::json& event_json,
  emp::Ptr<Event> event_ptr
) {
  emp_assert(event_json.contains("timing"));
  const std::string timing_str(event_json["timing"]);
  // EventTiming timing;
  // Given as a single (integer) number?
  if (emp::is_digits(timing_str)) {
    const size_t start_u = emp::from_string<size_t>(timing_str);
    event_ptr->ResetTiming(start_u);
  } else {
    // Timing given as Start:Stop:Step
    emp::vector<std::string> recurring_str = emp::slice(timing_str, ':');
    emp_assert(recurring_str.size() == 3);
    const size_t start_u = emp::from_string<size_t>(recurring_str[0]);
    const size_t stop_u = emp::from_string<size_t>(recurring_str[1]);
    const size_t freq = emp::from_string<size_t>(recurring_str[2]);
    event_ptr->ResetTiming(start_u, stop_u, freq);
  }
}

}