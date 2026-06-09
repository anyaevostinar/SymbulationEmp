#pragma once
// @AML review: added header guards
// @AML review: switch indentation to 2 spaces for consistency

// @AML review: Moved local includes to top for consistency
#include "Event.h"
#include "EventTypeDefinition.h"

#include "emp/base/vector.hpp"
#include "emp/bits/Bits.hpp"
#include "emp/tools/string_utils.hpp"
#include "emp/datastructs/set_utils.hpp"
#include "emp/math/math.hpp"
#include "../../../json/json.hpp"

#include <functional>
#include <string>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <map>
#include <algorithm>

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




  emp::vector<event_object_t> single_event_info; // vector of one time events (Event Object type)
  emp::vector<event_object_t> reoccur_event_info; // vector of reoccuring events (Event Object type)

  // // from LogicTaskEnvironment.h get json field values
  // template<typename RET_TYPE>
  // RET_TYPE GetVal(
  //     json_t& json,
  //     const std::string& field,
  //     RET_TYPE default_val
  // ) {
  //     return (json.contains(field)) ?
  //     static_cast<RET_TYPE>(json[field]) :
  //     default_val;
  // }

  // create instance of event object using EventObject class, return event object
  event_object_t CreateEventObject(const size_t event_id, const std::string & event_name, const std::string & task_name, const double & task_value, const std:string & update_indices, const std::vector<std::string> & parameters, const bool & reocccur){

    if(reoccur){
      // slice and convert update indices to individula integers
      std::vector<std::string> indices_vect;
      emp::slice(update_indices, indices_vect, ":");
      int start_index = static_cast<int>(indices_vect[0]);
      int end_index = static_cast<int>(indices_vect[1]);
      int step_index = static_cast<int>(indices_vect[2]);
      event.start_update = start_index;
      event.end_update = end_index;
      event.update_step = step_index;

      event_object_t event(event_id, event_name, task_name, task_value, start_index, end_index, step_index, parameters, reoccur);
    }
    else {
      int start_index = static_cast<int>(update_indices);
      event_object_t event(event_id, event_name, task_name, task_value, start_index, parameters, reoccur);
    }
    return event;
  }

  // should i add emp:: to funct?
  bool CheckJsonField(const emp::vector<std::string> & fields, auto& json_line){
    for(std::string name : fields){
      (emp_assert(json_line.contains(name)))? continue :
      return false;
    }
  }

  // load in and process events.json file (includes creating events and checking if they are valid)
  void LoadEvents(const std::string& event_filepath){
    // from LogicTaskEnvironment.h
    std::cout << "Loading tasks from event file." << std::endl;
    ClearEvents();
    // === Parse environment file ===
    // Check if given environment file exists. Exit if not.
    const bool event_file_exists = std::filesystem::exists(event_filepath);
    if (!event_file_exists) {
      std::cout << "Event file does not exist: " << event_filepath << std::endl;
      std::exit(EXIT_FAILURE);
    }

    // read event.json file
    std::ifstream event_ifstream(event_filepath);
    nlohmann::json eve_json;
    event_ifstream >> eve_json;

    // check for correct json format
    emp::vector<std::string> event_fields = {"event_type", "task_name", "task_value", "parameters", "update_indices", "reoccuring_event"};

    emp_assert(eve_json.contains("events"));
    for(auto& line; eve_json["events"]){

      CheckJsonFields(event_fields, line);
      IsValidEvent(line["event_type"]);

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

  // delete all current events info
  void ClearEvents(){
    single_event_info.clear();
    reoccur_event_info.clear();
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