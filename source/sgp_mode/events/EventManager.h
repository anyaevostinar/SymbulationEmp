#pragma once
// @AML review: added header guards
// @AML review: switch indentation to 2 spaces for consistency

// @AML review: Moved local includes to top for consistency
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

// @AML review: Don't need to namespace the event handler
//          (unless there's a bunch of internal components that should be isolated
//           from the rest of the repo)
namespace sgpmode {

// TODO: Document event types

template<typename WORLD_T>
class EventManager {
public:
  using json_t = nlohmann::json;  // json file type
  using event_t = Event;          // event class alias
  using world_t = WORLD_T;        // world type alias
  using event_type_def_t = EventTypeDefinition<world_t>;
  using fun_event_handler_t = typename event_type_def_t::fun_event_handler_t;

protected:

  EventTypeLibrary<world_t> event_type_library;
  emp::vector<emp::Ptr<event_t>> one_time_events; // vector of one time events (Event Object type)
  emp::vector<emp::Ptr<event_t>> recurring_events; // vector of reoccuring events (Event Object type)

  emp::Ptr<Event> LoadEventFromJSON(json_t& event_json, world_t& world) {
    // Check that event_json has event type
    emp_assert(event_json.contains("event_type"));
    const std::string event_type(event_json["event_type"]);
    // emp::Ptr<Event> loaded_event;
    // Check if event type is valid (i.e., exists in the event type library)
    emp_assert(event_type_library.IsValidEventType(event_type));
    const size_t event_type_id = event_type_library.GetEventTypeID(event_type);
    auto& event_type_def = event_type_library.GetEventTypeDefinition(event_type_id);
    // Event json must have required fields (as determined by event type definition)
    emp_assert(
      sym_json::ValidateFieldsJSON(event_json, event_type_def.GetRequiredFields())
    );
    return event_type_def.LoadEventFromJSON(event_json, world);

    // // Delegate event loading based on event type
    // if (event_type == "task_value") {
    //   loaded_event = TaskValueEvent::LoadEventFromJSON(event_json, world);
    // } else {
    //   std::cout << "Unknown event type (" << event_type << ") Exiting." << std::endl;
    //   exit(-1);
    // }

    // // Configure loaded event's event_id
    // emp_assert(event_type == loaded_event->GetEventType());
    // loaded_event->SetEventTypeID(event_type_id);
    // return loaded_event;
  }

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
      emp::Ptr<Event> event = one_time_events[event_i];
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
      std::cout << "Processing a recurring event " << event_i << std::endl;
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

  // Re-sort the one-time events.
  // One-time events should be reverse-sorted by their next update (i.e., soonest
  // next update at end). One-time events must be sorted for processing to be correct.
  // i.e., when adding a new event, must resort!
  void ReorderOneTimeEvents() {
    std::sort(
      one_time_events.begin(),
      one_time_events.end(),
      [](emp::Ptr<Event> a, emp::Ptr<Event> b) {
        return a->GetNextUpdate() > b->GetNextUpdate();
      }
    );
  }

  // Reorder recurring events. Should be reverse sorted by each event's next update
  // (i.e., soonest next update at end of vector). Recurring events must be sorted
  // for processing to be correct.
  // I.e., when adding a new event, must sort! And, when re-queueing a recurring
  //  event, we must make sure it ends up in the appropriate spot by next update.
  void ReorderRecurringEvents() {
    std::sort(
      recurring_events.begin(),
      recurring_events.end(),
      [](emp::Ptr<Event> a, emp::Ptr<Event> b) {
        return a->GetNextUpdate() > b->GetNextUpdate();
      }
    );
  }

// @AML review: missing public designation here
public:

  ~EventManager() {
    ClearEvents();
  }

  const EventTypeLibrary<world_t>& GetEventTypeLibrary() const { return event_type_library; }

  // Delete all current events info
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

  // load in and process events.json file (includes creating events and checking if they are valid)
  void LoadEventsFromJSON(const std::string& event_filepath, world_t& world) {
    std::cout << "Loading events from event file." << std::endl;
    ClearEvents();
    // === Parse events file ===
    // Check if given events file exists. Exit if not.
    const bool event_file_exists = std::filesystem::exists(event_filepath);
    if (!event_file_exists) {
      std::cout << "Event file does not exist: " << event_filepath << std::endl;
      std::exit(EXIT_FAILURE);
    }
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

  void ProcessEvents(world_t& world) {
    // Get current update in the world. Process all events that should occur on this update.
    // Options:
    //  1. Maintain unordered list of events. Loop over entire list each update,
    //      triggering any events where event->NextUpdate() == current update.
    //    - Pro: no need to maintain sorted order, simple insertion/deletion
    //    - Con: need to loop over all events no matter what. Could be costly if
    //        there are many events that occur throughout the run.
    //  2. Keep events sorted by their next update to trigger on. Trigger any events
    //      where event->NextUpdate() == current update.
    //    - Pro: Very efficient to check if any events need to be triggered. Will
    //        end up looping over just events that need to be triggered each update.
    //    - Con: Recurring events need to be resorted if they need to be triggered
    //        again in the future.
    // AML thoughts: Leaning toward option 2. In option 1, we pay the expensive part
    //    every update. In option 2, we only pay the resorting cost when a recurring
    //    event triggers (most recurring events will not happen every update).
    // Process all one-time events that need to be triggered this update.
    ProcessOneTimeEvents(world);
    // Next, process all recurring events that need to be triggered this update.
    ProcessRecurringEvents(world);
  }

  // TODO - manual 'AddEvent'

};

}