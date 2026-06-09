#pragma once
// @AML review: added header guards
// @AML review: switch indentation to 2 spaces for consistency

// @AML review: Moved local includes to top for consistency
#include "Event.h"
#include "EventTypeDefinition.h"
#include "EventTypeLibrary.h"

#include "../../json/json.hpp"

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
  using event_type_def_t = EvenTypeDefinition<world_t>;
  using event_handler_func_t = typename event_type_def_t::event_handler_func_t;

protected:

  EventTypeLibrary<world_t> event_type_library;
  emp::vector<emp::Ptr<event_t>> one_time_events; // vector of one time events (Event Object type)
  emp::vector<emp::Ptr<event_t>> recurring_events; // vector of reoccuring events (Event Object type)

  // // create instance of event object using EventObject class, return event object
  // event_t CreateEventObject(const size_t event_id, const std::string & event_name, const std::string & task_name, const double & task_value, const std:string & update_indices, const std::vector<std::string> & parameters, const bool & reocccur){

  //   if(reoccur){
  //     // slice and convert update indices to individula integers
  //     std::vector<std::string> indices_vect;
  //     emp::slice(update_indices, indices_vect, ":");
  //     int start_index = static_cast<int>(indices_vect[0]);
  //     int end_index = static_cast<int>(indices_vect[1]);
  //     int step_index = static_cast<int>(indices_vect[2]);
  //     event.start_update = start_index;
  //     event.end_update = end_index;
  //     event.update_step = step_index;

  //     event_object_t event(event_id, event_name, task_name, task_value, start_index, end_index, step_index, parameters, reoccur);
  //   }
  //   else {
  //     int start_index = static_cast<int>(update_indices);
  //     event_object_t event(event_id, event_name, task_name, task_value, start_index, parameters, reoccur);
  //   }
  //   return event;
  // }

  // @AML review: renamed, fixed inner loop
  bool ValidateFieldsJSON(
    const emp::vector<std::string>& fields,
    auto& json_line
  ) {
    // @AML review: Can use const string reference to avoid copying string here
    for (const std::string& name : fields) {
      if (!json_line.contains(name)) {
        return false;
      }
    }
    return true;
  }

  emp::Ptr<Event> LoadEventFromJSON(nlohmann::json& event_json) {
    // Check that event_json has event type
    emp_assert(event_json.contains("event_type"));
    const std::string event_type(event_json["event_type"]);
    // Check if event type is valid (i.e., exists in the event type library)
    emp_assert(event_type_library.IsValidEventType(event_type));
    // Delegate event loading based on event type
    if (event_type == "task_value_change") {
      return LoadChangeTaskValueEventFromJSON(event_json);
    } else if (event_type == "task_value_add") {
      return LoadChangeTaskValueEventFromJSON(event_json);
    } else if (event_type == "task_value_mul") {
      return LoadChangeTaskValueEventFromJSON(event_json);
    } else {
      std::cout << "Unknown event type (" << event_type << ") Exiting." << std::endl;
      exit(-1);
    }
  }

  // TODO: make adding new event types as easy / centralized as possible
  //        i.e., relocate these loaders to centralized location where other
  //            event type definitions are defined.
  emp::Ptr<ChangeTaskValueEvent> LoadChangeTaskValueEventFromJSON(nlohmann::json& event_json) {
    // TODO
  }


// @AML review: missing public designation here
public:

  ~EventManager() {
    ClearEvents();
  }

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
  void LoadEvents(const std::string& event_filepath) {
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

    // check for correct json format
    // emp::vector<std::string> event_fields = {"event_type", "task_name", "task_value", "parameters", "update_indices", "reoccuring_event"};

    emp_assert(events_json.contains("events"));
    // For each event line in events
    for (auto& event_json; events_json["events"]) {
      emp::Ptr<Event> new_event_ptr = LoadEventFromJSON(event_json);

      // @AML BOOKMARK

      // CheckJsonFields(event_fields, line);
      // IsValidEvent(line["event_type"]);

      // Is an reoccuring event
      if(line["reoccuring_event"]){
        // set event_id
        int event_id = name_to_id[line["event_type"]];
        // create event object
        event_object_t event_obj = CreateEventObject(event_id, line["event_type"], line["task_name"], line["task_value"], line["update_indices"], line["parameters"], line["reoccuring_event"]);
        reoccur_event_info.emplace_back(event_obj);
      }
      // is a single event
      else {
        // set event_id
        int event_id = name_to_id[line["event_type"]];
        // create event object
        event_object_t event_obj = CreateEventObject(event_id, line["event_type"], line["task_name"], line["task_value"], line["update_indices"], line["parameters"], line["reoccuring_event"]);
        single_event_info.emplace_back(event_obj);
      }
    }
    // sort vectors that hold event objects
    SortSingleEvents();
    SortReoccurEvents();
  }

  // return single event info at a specific index
  void GetSingleEventInfo(int index){
    return single_event_info[index];
  }

  // return reoccuring event info at a specific index
  void GetReoccurEventInfo(int index){
    return reoccur_event_info[index];
  }

  // get predefined event function at certain index
  void GetEventFunctions(size_t event_id){
    return event_types[event_id];
  }



  // // delete an event from event info vector (single or reoccur)
  // void DeleteOneEvent(const bool reoccur, const int & index){
  //     (reoccur) ? reoccur_event_info.erase(reoccur_event_info.begin()+index) :
  //     single_event_info.erase(single_event_info.begin()+index);
  // }

  // Delete all finished events from an event_info vector
  void DeleteEvents(const emp::vector<event_object_t> & event_vect){
    // erase remove idiom https://en.wikipedia.org/wiki/Erase%E2%80%93remove_idiom
    event_vect.erase(std::remove_if(event_vect.begin(), event_vect.end(),
    [](const event_object_t & eve){ return eve.GetIsDone(); }),
    event_vect.end());
  }

  // helper function to SortEvents
  int SortPartition(emp::vector<event_object_t> & event_vect, int & begin_index, int & end_index){
    // sources:  https://www.youtube.com/watch?v=Vtckgz38QHs, https://www.geeksforgeeks.org/dsa/quick-sort-algorithm/
    int piv = event_vect[end_index].GetStartUpdate();
    int i = begin_index - 1;
    for(int j = begin_index; j <= end_index - 1; j++){
      if(event_vect[j].GetStartUpdate() < piv){
        i++;
        event_object_t temp = event_vect[i];
        event_vect[i] = event_vect[j];
        event_vect[j] = temp;
      }
    }
    i++;
    event_object_t temp = event_vect[i];
    event_vect[i] = event_vect[end_index];
    event_vect[end_index] = temp;
    return i;
  }

  // sort events based on start update
  void SortEvents(emp::vector<event_object_t> & event_vect, int & begin_index, int & end_index){
    // use quick sort method
    // sources:  https://www.youtube.com/watch?v=Vtckgz38QHs, https://www.geeksforgeeks.org/dsa/quick-sort-algorithm/

    if(begin_index < end_index){
      int pivot_index = SortPartition(event_vect, begin_index, end_index);
      SortSingelEvent(event_vect, begin_index, pivot_index - 1);
      SortSingleEvent(event_vect, pivot_index + 1, end_index);
    }

  }

  // call event_func_t from current_events if possible based on update_indices
  void ProcessEvent(const world_t& world){
    // // set update variable to world.GetUpdate() or whaterver gets the world's update
    int update = world.GetUpdate();

    // loop through single time events
    for(auto& it = single_event_info.begin(); it != single_event_info.end(); it++){
      if(*it.GetStartUpdate() > update){
        break;
      }
      if(*it.GetStartUpdate() == update){
        // call event function look at how logic task does it
        event_types[*it.GetEventId()].event_function;
        // set to is_done true
        *it.SetIsDone();
      }
    }

    // loop through reoccur time events
    for(auto& it = reoccur_event_info.begin(); it != reoccur_event_info.end(); it++){
      if(*it.GetStartUpdate() > update){
        break;
      }
      if(*it.GetStartUpdate() == update){
        // call event function look at how logictask does it
        event_types[*it.GetEventId()].event_function;
        // reset start update
        int new_start = *it.GetStartUpdate() + *it.GetUpdateStep();
        *it.SetStartUpdate(new_start);
        // check if event is done
        if (update >= *it.GetEndUpdate() || *it.GetStartUpdate() > *it.GetEndUpdate()){
          *it.SetIsDone();
        }
      }
    }

    // clean up
    DeleteEvents(single_event_info);
    DeleteEvents(reoccur_event_info);
    // don't need to resort single_event_info at the moment
    SortEvents(reoccur_event_info, 0, reoccur_event.size() - 1);
  }

  // defined event_types
  const std::vector < EventDefinition >
  ChangingEventHandler::event_types = {
    ChangingEventsHandler::EventDefinition{
      0,
      "task_value_replace",
      [](const world_t & world, const event_object_t & event_info)->{
        // TODO: define function
      },
      "replace a specific preexisting task value with another value"
    },
    ChangingEventsHandler::EventDefinition{
      1,
      "task_value_add",
      [](const world_t & world, const event_object_t & event_info)->{
        // TODO: define function
      },
      "add a specific amount to the current value of a preexisting task"
    },
    ChangingEventsHandler::EventDefinition{
      2,
      "task_value_mul",
      [](const world_t & world, const event_object_t & event_info)->{
        // TODO: define function
      },
      "multiply a specific amount to the current value of a preexisting task"
    }
  }
}

}