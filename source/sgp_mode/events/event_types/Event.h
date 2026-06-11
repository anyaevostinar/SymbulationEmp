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

// Basic event type
class Event {
protected:
  size_t event_type_id = 0;
  std::string event_type{"NULL"};
  EventTiming timing;
  bool is_done = false; // Can be used by an event to end a recurring event before its end update

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
  // Advance next update, return new next update.
  size_t AdvanceNextUpdate() {
    timing.Step();
    return timing.GetNextUpdate();
  }

};

// class ChangeTaskRewardTypeEvent : Event {

// };

class StressEvent : Event {

};

}