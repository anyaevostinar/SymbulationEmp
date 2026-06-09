#pragma once

#include "Event.h"

#include "emp/base/Ptr.hpp"

#include <string>
#include <functional>

namespace sgp_mode {

// The event type definition contains the information necessary to define a
// type of event:
// - event_id: unique, used to lookup handler function when processing events
// - event_name: unique, human-readable event type name, used to identify event
//               in the events file
// - event_function:
template<typename WORLD_T>
class EventTypeDefinition {
public:
  using event_t = Event;          // event class alias
  using world_t = WORLD_T;        // world type alias

  // Event handler function type
  using fun_event_handler_t = std::function<void(
    world_t&,         /* sgp world to use helper functions and avoid circualr dependency */
    emp::Ptr<event_t> /* event being processed */
  )>;

protected:
  // @AML Review: if a variable should never by negative, prefer size_t over int
  size_t event_id;                        // Event definition ID
  std::string event_name;                 // Human-readable event type name
  event_handler_func_t event_handler_fun; // Event handler function
  std::string description;                // Event type description
  emp::vector<std::string> required_fields;
  // TODO: Any other type parameters?

public:
  EventDefinition(
    size_t a_event_id,
    const std::string& a_event_name,
    const fun_event_handler_t& a_event_handler,
    const emp::vector<std::string>& a_required_fields,
    const std::string& a_desc
  ) :
    event_id(a_event_id),
    event_name(a_event_name),
    event_handler_fun(a_event_handler),
    required_fields(a_required_fields),
    description(a_desc)
  { ; }

  // Run event handler function
  void Process(world_t& world, emp::Ptr<Event> event) {
    event_handler_fun(world, event);
  }

};

}