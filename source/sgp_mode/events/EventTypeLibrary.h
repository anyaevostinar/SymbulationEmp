#pragma once

#include "Event.h"
#include "EventTypeDefinition.h"

#include "emp/base/vector.hpp"
#include "emp/datastructs/map_utils.hpp"

namespace sgp_mode {

template<typename WORLD_T>
class EventTypeLibrary {
public:
  using world_t = WORLD_T;
  using event_type_def_t = EventTypeDefinition<world_t>;
  using fun_event_handler_t = typename event_type_def_t::fun_event_handler_t;
protected:
  emp::vector<EventDefinition> event_definitions;
  std::unordered_map<std::string, size_t> event_name_to_id; // event type/name mapped to index of event in event_types. Index is used as event_id

public:
  EventTypeLibrary(bool add_default_events=true) {
    if (add_default_events) {
      AddDefaultEventTypes();
    }
  }

  // Clear all event types from the library
  void Clear() {
    event_definitions.clear();
  }

  // Get event type id using string name
  size_t GetEventTypeID(const std::string& event_name) const {
    emp_assert(IsValidEventType(event_name));
    return event_name_to_id[event_name];
  }

  // check if event type is valid (aka there is a function in all_event_functions that cna perform the event)
  bool IsValidEventType(const std::string& event_name) const {
    return emp::Has(event_name_to_id, event_name);
  }

  // Run appropriate event handler for given event.
  void ProcessEvent(world_t& world, emp::Ptr<Event> event_ptr) {
    const size_t event_id = event_ptr->GetID();
    event_definitions[event_id].Process(world, event_ptr);
  }

  // Add default event types
  // NOTE: Called by constructor by default.
  //        Should not be called a second time without clearing first.
  void AddDefaultEventTypes();

  // Add a new event type to the event library
  void AddEventType(
    const std::string& event_name,
    const fun_event_handler_t& event_handler,
    const std::string& event_description = ""
  ) {
    // Should not be duplicate event type names.
    emp_assert(!emp::Has(event_name, event_name_to_id));
    const size_t event_id = event_definitions.size();
    event_definitions.emplace_back(
      event_id,
      event_name,
      event_handler,
      event_description
    );
    event_name_to_id[event_name] = event_id;
  }

  template<typename EVENT_T>
  void AddEventType(
    const std::string& event_name,
    const std::string& event_description
  ) {
    AddEventType(
      event_name,
      [](world_t& world, emp::Ptr<Event> event_ptr) {
        emp::Ptr<EVENT_T> event = static_cast<EVENT_T*>(event_ptr.Raw());
        event->Process(world);
      },
      event_description
    )
  }

};

// TODO - create handler factories?
template<typename WORLD_T>
void EventTypeLibrary<WORLD_T>::AddDefaultEventTypes() {
  // Add task value replace
  // TODO - Might be able to compress this into a templated function
  // AddEventType(
  //   "task_value",
  //   [](world_t& world, emp::Ptr<Event> event_ptr) {
  //     // Cast event to correct type
  //     emp::Ptr<TaskValueEvent> task_event_ptr = static_cast<TaskValueEvent*>(event_ptr.Raw());
  //     task_event_ptr->Process(world);
  //     // TODO: define function
  //   },
  //   "replace a specific preexisting task value with another value"
  // );

  AddEventType<TaskValueEvent>(
    "task_value",
    "replace a specific preexisting task value with another value"
  );

  // AddEventType(
  //   "task_value_add",
  //   [] (world_t& world, emp::Ptr<Event> event_ptr) {
  //     // Cast event to correct type
  //     emp::Ptr<ChangeTaskValueEvent> task_event_ptr = static_cast<ChangeTaskValueEvent*>(event_ptr.Raw());
  //     // TODO: define function
  //   },
  //   {"task_name", "value", "task_group", "timing"},
  //   "add a specific amount to the current value of a preexisting task"
  // );

  // AddEventType(
  //   "task_value_mul",
  //   [](world_t& world, emp::Ptr<Event> event_ptr) {
  //     // Cast event to correct type
  //     emp::Ptr<ChangeTaskValueEvent> task_event_ptr = static_cast<ChangeTaskValueEvent*>(event_ptr.Raw());
  //     // TODO: define function
  //   },
  //   {"task_name", "value", "task_group", "timing"},
  //   "multiply a specific amount to the current value of a preexisting task"
  // );
}

}