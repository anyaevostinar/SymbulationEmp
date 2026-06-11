#pragma once
// @AML review: added header guards
// @AML review: switch indentation to 2 spaces for consistency

// @AML review: Moved local includes to top for consistency
#include "Event.h"
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

  emp::Ptr<Event> LoadEventFromJSON(nlohmann::json& event_json, world_t& world) {
    // Check that event_json has event type
    emp_assert(event_json.contains("event_type"));
    const std::string event_type(event_json["event_type"]);
    emp::Ptr<Event> loaded_event;
    // Check if event type is valid (i.e., exists in the event type library)
    emp_assert(event_type_library.IsValidEventType(event_type));
    const size_t event_type_id = event_type_library.GetEventTypeID(event_type);
    auto& event_type_def = event_type_library.GetEventTypeDefinition(event_type_id);
    // Event json must have required fields (as determined by event type definition)
    emp_assert(
      sym_json::ValidateFieldsJSON(event_json, event_type_def.GetRequiredFields())
    );
    // Delegate event loading based on event type
    if (event_type == "task_value") {
      loaded_event = TaskValueEvent::LoadEventFromJSON(event_json, world);
    } else {
      std::cout << "Unknown event type (" << event_type << ") Exiting." << std::endl;
      exit(-1);
    }

    // Configure loaded event's event_id
    emp_assert(event_type == loaded_event->GetEventType());
    loaded_event->SetEventTypeID(event_type_id);
    return loaded_event;
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

      // Sort events according to their next update
      // TODO - Move into reorder function?
      std::sort(
        recurring_events.begin(),
        recurring_events.end(),
        [](emp::Ptr<Event> a, emp::Ptr<Event> b) {
          return a->GetNextUpdate() > b->GetNextUpdate();
        }
      );
      std::sort(
        one_time_events.begin(),
        one_time_events.end(),
        [](emp::Ptr<Event> a, emp::Ptr<Event> b) {
          return a->GetNextUpdate() > b->GetNextUpdate();
        }
      );
    }
  }

  void ProcessEvents(world_t& world) {
    // Process one-time events
    // TODO
    // Process recurring events
    // TODO
  }

  // TODO - manual 'AddEvent'

  // // delete an event from event info vector (single or reoccur)
  // void DeleteOneEvent(const bool reoccur, const int & index){
  //     (reoccur) ? reoccur_event_info.erase(reoccur_event_info.begin()+index) :
  //     single_event_info.erase(single_event_info.begin()+index);
  // }

  // // Delete all finished events from an event_info vector
  // void DeleteEvents(const emp::vector<event_object_t> & event_vect){
  //   // erase remove idiom https://en.wikipedia.org/wiki/Erase%E2%80%93remove_idiom
  //   event_vect.erase(std::remove_if(event_vect.begin(), event_vect.end(),
  //   [](const event_object_t & eve){ return eve.GetIsDone(); }),
  //   event_vect.end());
  // }

  // // call event_func_t from current_events if possible based on update_indices
  // void ProcessEvent(const world_t& world){
  //   // // set update variable to world.GetUpdate() or whaterver gets the world's update
  //   int update = world.GetUpdate();

  //   // loop through single time events
  //   for(auto& it = single_event_info.begin(); it != single_event_info.end(); it++){
  //     if(*it.GetStartUpdate() > update){
  //       break;
  //     }
  //     if(*it.GetStartUpdate() == update){
  //       // call event function look at how logic task does it
  //       event_types[*it.GetEventId()].event_function;
  //       // set to is_done true
  //       *it.SetIsDone();
  //     }
  //   }

  //   // loop through reoccur time events
  //   for(auto& it = reoccur_event_info.begin(); it != reoccur_event_info.end(); it++){
  //     if(*it.GetStartUpdate() > update){
  //       break;
  //     }
  //     if(*it.GetStartUpdate() == update){
  //       // call event function look at how logictask does it
  //       event_types[*it.GetEventId()].event_function;
  //       // reset start update
  //       int new_start = *it.GetStartUpdate() + *it.GetUpdateStep();
  //       *it.SetStartUpdate(new_start);
  //       // check if event is done
  //       if (update >= *it.GetEndUpdate() || *it.GetStartUpdate() > *it.GetEndUpdate()){
  //         *it.SetIsDone();
  //       }
  //     }
  //   }

  //   // clean up
  //   DeleteEvents(single_event_info);
  //   DeleteEvents(reoccur_event_info);
  //   // don't need to resort single_event_info at the moment
  //   SortEvents(reoccur_event_info, 0, reoccur_event.size() - 1);
  // }

};

}