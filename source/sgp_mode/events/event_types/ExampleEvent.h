#pragma once

#include "Event.h"

namespace sgpmode {

// Example event class. Used for example purposes only.
class ExampleEvent : public Event {
public:
  using json_t = nlohmann::json;

  static const EventTypeDefinitionSpecs event_specs;

  template <typename WORLD_T>
  static emp::Ptr<ExampleEvent> LoadEventFromJSON(
    json_t& event_json,
    WORLD_T& world
  ) {
    // This should be a task_value event
    emp_assert(event_json["event_type"] == event_specs.event_type);
    // NOTE: Caller is responsible for event deletion.
    emp::Ptr<ExampleEvent> event = emp::NewPtr<ExampleEvent>();
    // --- TO FILL IN ---
    // Configure new event object based on given event_json
    // ------------------
    return event;
  }

protected:
public:
  ExampleEvent() {
    // Set event type in base class.
    event_type = event_specs.event_type;
  }

  template<typename WORLD_T>
  void Process(WORLD_T& world) {
    // --- TO FILL IN ---
    // Code to process an instance of this event should go here.
    // ------------------
  }
};

const EventTypeDefinitionSpecs ExampleEvent::event_specs = EventTypeDefinitionSpecs{
  "example_event",
  "Example of a minimally defined event type",
   {"list", "required", "fields", "for", "this", "event", "here"}
};

}