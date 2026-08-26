#pragma once

#include "event_types/Event.h"
#include "EventTypeDefinition.h"
#include "EventTypeLibrary.h"

#include "../../json/json.hpp"
#include "../../json/json_utils.h"

#include "emp/base/vector.hpp"
#include "emp/bits/Bits.hpp"
#include "emp/tools/string_utils.hpp"
#include "emp/datastructs/set_utils.hpp"
#include "emp/math/math.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <string>
#include <unordered_map>

namespace sgpmode {

/**
 * Purpose: Manages set of events that can be loaded in from a configuration file.
 */
template<typename WORLD_T>
class EventManager {
public:
  using json_t = nlohmann::json;  // json file type
  using event_t = Event;          // event class alias
  using world_t = WORLD_T;        // world type alias
  using event_type_def_t = EventTypeDefinition<world_t>;
  using fun_event_handler_t = typename event_type_def_t::fun_event_handler_t;

protected:

  /**
   * Purpose: Manages known event type definitions.
   *          Must be updated before loading event cfg file.
   */
  EventTypeLibrary<world_t> event_type_library;

  /**
   * Purpose: Manages all one-off events in reverse sorted order by update to apply.
   */
  emp::vector<emp::Ptr<event_t>> one_time_events;

  /**
   * Purpose: Manages all recurring events in reverse sorted order by update to apply.
   */
  emp::vector<emp::Ptr<event_t>> recurring_events;

  /**
   * Purpose: Load a single event from JSON
   *          Each event type should define its own loading function (see ExampleEvent),
   *          so this function grabs the appropriate event type definition and then
   *          calls that event type's load event function.
   *
   * Input: json oject to load an event from, reference to the world
   *
   * Output: Pointer to a new event object
   */
  emp::Ptr<event_t> LoadEventFromJSON(json_t& event_json, world_t& world) {
    // Check that event_json has event type
    emp_assert(event_json.contains("event_type"));
    const std::string event_type(event_json["event_type"]);
    // Check if event type is valid (i.e., exists in the event type library)
    emp_assert(event_type_library.IsValidEventType(event_type));
    const size_t event_type_id = event_type_library.GetEventTypeID(event_type);
    auto& event_type_def = event_type_library.GetEventTypeDefinition(event_type_id);
    // Event json must have required fields (as determined by event type definition)
    emp_assert(
      sym_json::ValidateFieldsJSON(event_json, event_type_def.GetRequiredFields())
    );
    return event_type_def.LoadEventFromJSON(event_json, world);
  }

  /**
   * Purpose: Process all one-type events that should be applied this update.
   *          After processing an event, delete it.
   *          Note: this function assumes that one_time_events is in reverse sorted
   *          order by next update.
   *
   * Input: Reference to the world
   *
   * Output: None.
   */
  void ProcessOneTimeEvents(world_t& world) {
    if (one_time_events.empty()) { return; }
    // One-time events are reverse sorted according to next update.
    //  Sorting should have happened on load (as well as anytime an event was added).
    const size_t current_update = world.GetUpdate();
    // Process one-time events
    const size_t num_events = one_time_events.size();
    // If no events to process, skip.
    emp_assert(num_events > 0);
    size_t events_processed = 0;

    // && event_i < num_events <-- checks for roll over
    for (size_t event_i = num_events - 1; event_i >= 0 && event_i < num_events; --event_i) {
      emp::Ptr<event_t> event = one_time_events[event_i];
      emp_assert(!event->IsRecurring());
      const size_t event_update = event->GetNextUpdate();
      // Event's next update should never be less than current update. If so,
      //  we failed to process it on a previous update. :(
      emp_assert(event_update >= current_update);
      // If event's next update is bigger than current update, no more events
      //  to trigger this update.
      if (event_update > current_update) {
        break;
      }
      emp_assert(event_update == current_update);
      // Process this event
      event_type_library.ProcessEvent(world, event);
      // One-time event processed. Delete, update # events processed.
      event.Delete();
      ++events_processed;
    }
    // Chop off processed events
    emp_assert(events_processed <= num_events);
    one_time_events.resize(num_events - events_processed);
  }

  /**
   * Purpose: Process any recurring events that should be applied this update.
   *          After processing an event, delete the event if its next update is
   *          past its end update or if the event was marked as done by the event's
   *          process function. Otherwise, advance the event's timing and keep
   *          for future processing.
   *          Note: this function assumes that recurring_events is in reverse sorted order
   *          by each event's next update. This function will resort recurring
   *          events after processing.
   *
   * Input: Reference to the world.
   *
   * Output: None.
   */
  void ProcessRecurringEvents(world_t& world) {
    if (recurring_events.empty()) { return; }
    // Recurring events are reverse sorted by the next update they should trigger
    // on.
    const size_t current_update = world.GetUpdate();
    const size_t num_events = recurring_events.size();
    emp_assert(num_events > 0);
    size_t events_deleted = 0;
    size_t events_recurred = 0;
    for (size_t event_i = num_events - 1; event_i >= 0 && event_i < num_events; --event_i) {
      emp::Ptr<Event> event = recurring_events[event_i];
      emp_assert(event->IsRecurring());
      const size_t event_update = event->GetNextUpdate();
      // Event's next update should never be less than current update. If so,
      //  we failed to process it on a previous update. :(
      emp_assert(event_update >= current_update);
      // If event's next update is bigger than current update, no more events
      //  to trigger this update.
      if (event_update > current_update) {
        break;
      }
      // Otherwise, process this event.
      event_type_library.ProcessEvent(world, event);
      const size_t next_update = event->AdvanceNextUpdate();
      const size_t end_update = event->GetEndUpdate();
      const bool is_done = event->IsDone();
      if (next_update > end_update || is_done) {
        event.Delete();
        recurring_events[event_i] = nullptr;
        ++events_deleted;
        // Move deleted events to end of vector.
        // - If we haven't recurred any events, either this is the first event
        //   or all events procssed this update have been deleted.
        // - If we have recurred any events, we know that all prior deleted events
        //   have been swapped to the end of the vector. So, all recurred events
        //   are in a chunk between this (deleted) event and the rest of the deleted
        //   events (if any). We want to swap this nullptr with the last recurred
        //   event in that chunk.
        if (events_recurred > 0) {
          // There was an event recurred before this, potentially breaking up
          //  the sequence of deleted events.
          // Swap current event with last event in recurred events chunk
          emp_assert(recurring_events[event_i + events_recurred] != nullptr);
          std::swap(
            recurring_events[event_i],
            recurring_events[event_i + events_recurred]
          );
        }
      } else {
        // If this event wasn't deleted, it will occur again later.
        ++events_recurred;
      }

    }
    // Resize away the deleted events (that have all been swapped to the end)
    recurring_events.resize(num_events - events_deleted);
    // Resort! Perfect application for powersort because most events should
    //           already be sorted... (versus likely std library's introsort)
    ReorderRecurringEvents();
  }

public:
  /**
   * Purpose: EventManager destructor. Responsible for cleaning up any event objects.
   */
  ~EventManager() {
    ClearEvents();
  }

  /**
   * Purpose: Accessor for event type library.
   *
   * Input: None.
   *
   * Output: Const reference to this event manager's event type library.
   */
  const EventTypeLibrary<world_t>& GetEventTypeLibrary() const { return event_type_library; }


  /**
   * Purpose: Delete all current events info (one-time and recurring)
   *
   * Input: None.
   *
   * Output: None.
   */
  void ClearEvents() {
    for (emp::Ptr<event_t> event : one_time_events) {
      event.Delete();
    }
    one_time_events.clear();
    for (emp::Ptr<event_t> event : recurring_events) {
      event.Delete();
    }
    recurring_events.clear();
  }

  /**
   * Purpose: Accessor for recurring events.
   *          CAUTION: the event manager makes assumptions about the order of recurring
   *          events based on their timing. This accessor is used primarily for testing.
   *          If recurring events are modified via this function, you must resort them (ReorderRecurringEvents)
   */
  emp::vector<emp::Ptr<event_t>>& GetRecurringEvents() {
    return recurring_events;
  }

  /**
   * Purpose: Accessor for one-time events.
   *          CAUTION: the event manager makes assumptions about the order of one-time
   *          events based on their timing. This accessor is used primarily for testing.
   *          If one-time events are modified via this function, you must resort them (ReorderOneTimeEvents)
   */
  emp::vector<emp::Ptr<event_t>>& GetOneTimeEvents() {
    return one_time_events;
  }

  /**
   * Purpose: load in and process events configuration json file (includes
   *          creating events and checking if they are valid)
   *
   * Input: String path to event configuration file, reference to the world object
   *
   * Output: None.
   */
  void LoadEventsFromJSON(const std::string& event_filepath, world_t& world) {
    ClearEvents();
    // === Parse events file ===
    // Assert event file existence.
    const bool event_file_exists = std::filesystem::exists(event_filepath);
    emp_assert(event_file_exists, "Event file does not exist.");
    // read event.json file
    std::ifstream event_ifstream(event_filepath);
    nlohmann::json events_json;
    event_ifstream >> events_json;
    emp_assert(events_json.contains("events"));
    // For each event line in events
    for (auto& event_json : events_json["events"]) {
      emp::Ptr<Event> new_event_ptr = LoadEventFromJSON(event_json, world);
      // Categorize event as recurring or one-time
      if (new_event_ptr->IsRecurring()) {
        recurring_events.emplace_back(new_event_ptr);
      } else {
        one_time_events.emplace_back(new_event_ptr);
      }
    }
    // Sort events according to their next update
    ReorderOneTimeEvents();
    ReorderRecurringEvents();
  }

  /**
   * Purpose: Process any events that need to be processed on the current world
   *          update.
   *
   * Input: Reference to the world.
   *
   * Output: None.
   */
  void ProcessEvents(world_t& world) {
    // Process all one-time events that need to be triggered this update.
    ProcessOneTimeEvents(world);
    // Next, process all recurring events that need to be triggered this update.
    ProcessRecurringEvents(world);
  }

  /**
   * Purpose: Manual add event function. Used when an event that didn't originate
   *          from a config file needs to be added to the event system.
   *
   * Input: Pointer to the event to add to the event manager.
   *
   * Output: None.
   */
  void AddEvent(emp::Ptr<event_t> event) {
    // Check that this event is who they say it is
    const auto& event_type_name = event->GetEventType();
    // Check that incoming event is a known event type
    emp_assert(event_type_library.IsValidEventType(event_type_name));
    // Set event's type id
    event->SetEventTypeID(event_type_library.GetEventTypeID(event_type_name));
    // Add event to either recurring or one-time event set, then reorder.
    if (event->IsRecurring()) {
      recurring_events.emplace_back(event);
      ReorderRecurringEvents();
    } else {
      one_time_events.emplace_back(event);
      ReorderOneTimeEvents();
    }
  }

  /**
   * Purpose: Manually add multiple events to the event manager. Used for events
   *          that don't originate from the events configuration file. Faster to
   *          to bulk add multiple events than adding one at a time to avoid
   *          repeated resorting.
   *
   * Input: List of event pointers to be added.
   *
   * Output: None.
   */
  void AddEvents(emp::vector<emp::Ptr<event_t>> events) {
    bool resort_recurring = false;
    bool resort_one_time = false;
    for (size_t i = 0; i < events.size(); ++i) {
      // Check that this event is who they say it is
      const auto& event_type_name = events[i]->GetEventType();
      // Check that incoming event is a known event type
      emp_assert(event_type_library.IsValidEventType(event_type_name));
      // Set event's type id
      events[i]->SetEventTypeID(event_type_library.GetEventTypeID(event_type_name));
      // Add event to either recurring or one-time event set.
      if (events[i]->IsRecurring()) {
        recurring_events.emplace_back(events[i]);
        resort_recurring = true;
      } else {
        one_time_events.emplace_back(events[i]);
        resort_one_time = true;
      }
    }
    // Resort as needed
    if (resort_recurring) {
      ReorderRecurringEvents();
    }
    if (resort_one_time) {
      ReorderOneTimeEvents();
    }
  }

    /**
   * Purpose: Helper function to resort the one-time events.
   *          One-time events should be reverse-sorted by their next update
   *          (i.e., soonest next update at end). One-time events must be sorted
   *          for processing to be correct. i.e., when adding a new event, must resort!
   *
   * Input: None.
   *
   * Output: None.
   */
  void ReorderOneTimeEvents() {
    std::sort(
      one_time_events.begin(),
      one_time_events.end(),
      [](emp::Ptr<event_t> a, emp::Ptr<event_t> b) {
        return a->GetNextUpdate() > b->GetNextUpdate();
      }
    );
  }

  /**
   * Purpose: Helper function to reorder recurring events.
   *          Should be reverse sorted by each event's next update (i.e., soonest
   *          next update at end of vector). Recurring events must be sorted for
   *          processing to be correct. I.e., when adding a new event, must sort!
   *          And, when re-queueing a recurring event, we must make sure it ends
   *          up in the appropriate spot by next update.
   *
   * Input: None.
   *
   * Output. None.
   *
   *
   */
  void ReorderRecurringEvents() {
    std::sort(
      recurring_events.begin(),
      recurring_events.end(),
      [](emp::Ptr<event_t> a, emp::Ptr<event_t> b) {
        return a->GetNextUpdate() > b->GetNextUpdate();
      }
    );
  }

};

void GenerateEmptyEventsJSON(const std::string& event_filepath) {
  nlohmann::json empty_events;
  empty_events["events"] = nlohmann::json::array();
  std::ofstream empty_events_file(event_filepath);
  empty_events_file << empty_events << std::endl;
  empty_events_file.close();
}

}