#pragma once

#include "event_types/Event.h"

#include "../../json/json.hpp"
#include "emp/base/Ptr.hpp"

#include <string>
#include <functional>

namespace sgpmode {

/**
 * Purpose: The event type definition contains the information necessary to define a
 *          type of event.
 * Event definition info:
 *  - event_id: unique, used to lookup handler function when processing events
 *  - event_name: unique, human-readable event type name, used to identify event
 *                in the events file
 *  - event_handler_fun: Function that handles processing an event of this type
 *  - event_json_loader_fun: Function that handles loading an event of this type
 */
template<typename WORLD_T>
class EventTypeDefinition {
public:
  using event_t = Event;          // event class alias
  using world_t = WORLD_T;        // world type alias
  using json_t = nlohmann::json;  // json file type

  // Event handler function type
  using fun_event_handler_t = std::function<void(
    world_t&,         // sgp world to use helper functions and avoid circualr dependency
    emp::Ptr<event_t> // event being processed
  )>;

  using fun_json_loader_t = std::function<emp::Ptr<event_t>(json_t&, world_t&)>;

protected:

  /**
   * Purpose: Event definition ID
   */
  size_t event_id;

  /**
   * Purpose: Human-readable event type name
   */
  std::string event_name;

  /**
   * Purpose: Event handler function
   */
  fun_event_handler_t event_handler_fun;

  /**
   * Purpose: Event's json loader function
   */
  fun_json_loader_t event_json_loader_fun;

  /**
   * Purpose: Event type description
   */
  std::string description;

  /**
   * Purpose: List of required fields for loading event configuration
   */
  emp::vector<std::string> required_fields;

public:

  /**
   * Purpose: EventTypeDefinition constructor.
   */
  EventTypeDefinition(
    size_t a_event_id,
    const std::string& a_event_name,
    const fun_event_handler_t& a_event_handler,
    const fun_json_loader_t& a_event_json_loader_fun,
    const std::string& a_desc,
    const emp::vector<std::string>& a_required_fields
  ) :
    event_id(a_event_id),
    event_name(a_event_name),
    event_handler_fun(a_event_handler),
    event_json_loader_fun(a_event_json_loader_fun),
    description(a_desc),
    required_fields(a_required_fields)
  { ; }


  /**
   * Purpose: Get this event's required fields.
   *
   * Input: None.
   *
   * Output: List of strings indicating this event type's set of required fields
   *         for configuration.
   */
  const emp::vector<std::string>& GetRequiredFields() const {
    return required_fields;
  }

  /**
   * Purpose: Runs the given event through its event handler.
   *
   * Input: Reference to the world and a pointer to the event to be processed.
   *
   * Output: None.
   */
  void Process(world_t& world, emp::Ptr<Event> event) const {
    event_handler_fun(world, event);
  }

  /**
   * Purpose: Loads event from json using event_json_loader_fun.
   *
   * Input: Json to load from, reference to the world.
   *
   * Output: Pointer to a new event object created by the event loader.
   */
  emp::Ptr<event_t> LoadEventFromJSON(json_t& json, world_t& world) const {
    emp::Ptr<event_t> event = event_json_loader_fun(json, world);
    event->SetEventTypeID(event_id);
    return event;
  }

};

}