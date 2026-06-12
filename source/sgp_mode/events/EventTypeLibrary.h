#pragma once

#include "event_types/Event.h"
#include "event_types/TaskValueEvent.h"
#include "EventTypeDefinition.h"

#include "emp/base/vector.hpp"
#include "emp/datastructs/map_utils.hpp"

namespace sgpmode {

template<typename WORLD_T>
class EventTypeLibrary {
public:
  using world_t = WORLD_T;
  using event_type_def_t = EventTypeDefinition<world_t>;
  using json_t = nlohmann::json;  // json file type
  using fun_event_handler_t = typename event_type_def_t::fun_event_handler_t;
  using fun_json_loader_t = typename event_type_def_t::fun_json_loader_t;

protected:
  emp::vector<event_type_def_t> event_definitions;
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
    return event_name_to_id.at(event_name);
  }

  const event_type_def_t& GetEventTypeDefinition(size_t event_type_id) const {
    emp_assert(event_type_id < event_definitions.size());
    return event_definitions[event_type_id];
  }

  // check if event type is valid (aka there is a function in all_event_functions that cna perform the event)
  bool IsValidEventType(const std::string& event_name) const {
    return emp::Has(event_name_to_id, event_name);
  }

  // Run appropriate event handler for given event.
  void ProcessEvent(world_t& world, emp::Ptr<Event> event_ptr) {
    const size_t event_type_id = event_ptr->GetEventTypeID();
    event_definitions[event_type_id].Process(world, event_ptr);
  }

  // Add default event types
  // NOTE: Called by constructor by default.
  //        Should not be called a second time without clearing first.
  void AddDefaultEventTypes() {
    AddEventType<TaskValueEvent>();
  }

  // Add a new event type to the event library
  void AddEventType(
    const std::string& event_name,
    const fun_event_handler_t& event_handler,
    const fun_json_loader_t& event_loader,
    const std::string& event_description = "",
    const emp::vector<std::string>& event_required_cfg_fields = {}
  ) {
    // Should not be duplicate event type names.
    emp_assert(!emp::Has(event_name_to_id, event_name));
    const size_t event_id = event_definitions.size();
    event_definitions.emplace_back(
      event_id,
      event_name,
      event_handler,
      event_loader,
      event_description,
      event_required_cfg_fields
    );
    event_name_to_id[event_name] = event_id;
  }

  template<typename EVENT_T>
  void AddEventType(
    const std::string& event_name,
    const std::string& event_description = "",
    const emp::vector<std::string>& event_required_cfg_fields = {}
  ) {
    AddEventType(
      event_name,
      [](world_t& world, emp::Ptr<Event> event_ptr) {
        emp::Ptr<EVENT_T> event = static_cast<EVENT_T*>(event_ptr.Raw());
        event->Process(world);
      },
      [](json_t& event_json, world_t& world) -> emp::Ptr<Event> {
        return EVENT_T::LoadEventFromJSON(event_json, world);
      },
      event_description,
      event_required_cfg_fields
    );
  }

  // Add event type assuming EVENT_T has static event spects + a process function
  template<typename EVENT_T>
  void AddEventType() {
    AddEventType<EVENT_T>(
      EVENT_T::event_specs.event_type,
      EVENT_T::event_specs.event_description,
      EVENT_T::event_specs.event_required_fields
    );
  }

};

}