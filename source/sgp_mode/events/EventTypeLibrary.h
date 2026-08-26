#pragma once

#include "event_types/Event.h"
#include "event_types/TaskValueEvent.h"
#include "EventTypeDefinition.h"

#include "emp/base/vector.hpp"
#include "emp/datastructs/map_utils.hpp"

namespace sgpmode {

/**
 * Purpose: Manages a library of event type definitions.
 */
template<typename WORLD_T>
class EventTypeLibrary {
public:
  using world_t = WORLD_T;
  using event_type_def_t = EventTypeDefinition<world_t>;
  using json_t = nlohmann::json;  // json file type
  using fun_event_handler_t = typename event_type_def_t::fun_event_handler_t;
  using fun_json_loader_t = typename event_type_def_t::fun_json_loader_t;

protected:

  /**
   * Purpose: List of event type definitions
   */
  emp::vector<event_type_def_t> event_definitions;

  /**
   * Purpose: Mapping of event type names to their index in the event_definitions
   *          vector.
   */
  std::unordered_map<std::string, size_t> event_name_to_id;

public:
  /**
   * Purpose: EventTypeLibrary constructor.
   *
   * Inputs: Boolean whether to add all default event types to this library on
   *         construction.
   */
  EventTypeLibrary(bool add_default_events=true) {
    if (add_default_events) {
      AddDefaultEventTypes();
    }
  }

  /**
   * Purpose: Clear all event types from the library
   *
   * Input: None.
   *
   * Output: None.
   */
  void Clear() {
    event_definitions.clear();
  }

  /**
   * Purpose: Get event type id using string name
   *
   * Input: String event type name.
   *
   * Output: ID of that event type in the library.
   */
  size_t GetEventTypeID(const std::string& event_name) const {
    emp_assert(IsValidEventType(event_name));
    return event_name_to_id.at(event_name);
  }

  /**
   * Purpose: Get an event type definition.
   *
   * Input: ID of the event type.
   *
   * Output: Event type definition associated with given event type id.
   */
  const event_type_def_t& GetEventTypeDefinition(size_t event_type_id) const {
    emp_assert(event_type_id < event_definitions.size());
    return event_definitions[event_type_id];
  }

  /**
   * Purpose: Check if event type is contained in this library.
   *
   * Input: Event type name to check.
   *
   * Output: Boolean indicating whether event type name is in this library.
   */
  bool IsValidEventType(const std::string& event_name) const {
    return emp::Has(event_name_to_id, event_name);
  }

  /**
   * Purpose: Run appropriate event handler for given event.
   *
   * Input: Reference to the world, pointer to the event to process.
   *
   * Output: None.
   */
  void ProcessEvent(world_t& world, emp::Ptr<Event> event_ptr) {
    const size_t event_type_id = event_ptr->GetEventTypeID();
    event_definitions[event_type_id].Process(world, event_ptr);
  }

  /**
   * Purpose: Add all default event types to this library. Called by constructor
   *          by default. Should not be called a second time without clearing
   *          the library first to avoid adding duplicate event types.
   *
   * Input: None.
   *
   * Output: None.
   */
  void AddDefaultEventTypes() {
    AddEventType<TaskValueEvent>();
  }

  /**
   * Purpose: Add a new event type to the event library. Can be used to add event
   *          types that don't conform to expectations (e.g., Event.h / ExampleEvent.h)
   *
   * Inputs:
   *  - event_name: String indicating event type name
   *  - event_handler: Function for handling events of this type
   *  - event_loader: Function for loading events of this type
   *  - event_description: Description of this event type
   *  - event_required_cfg_fields: Required fields for configuring events of this type
   *
   * Output: None.
   */
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

  /**
   * Purpose: Add event type to this event type library.
   *
   * Input:
   *  - event_name: String event type name
   *  - event_description: String event type description
   *  - event_required_cfg_fields: List of required fields for configuring events
   *      of this type.
   *
   * Output: None.
   */
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

  /**
   * Purpose: Add event type assuming EVENT_T has static event specs + a process function
   *
   * Input: None.
   *
   * Output: None.
   */
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