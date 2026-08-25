#pragma once

#include "Event.h"

namespace sgpmode {

/**
 * Purpose: Example event class. Used for example purposes only.
 */
class ExampleEvent : public Event {
public:
  using json_t = nlohmann::json;

  /**
   * Purpose: Event specification object shared by all instances of this event
   *          type.
   */
  static const EventTypeDefinitionSpecs event_specs;

  /**
   * Purpose: All event type classes should define a "LoadEventFromJSON" function
   *          that configures an instance of this event type.
   *          See TaskValueEvent for another example.
   *
   * Input: Json object to load event information from, reference to the world.
   *
   * Output: Pointer to new event instance created and configured by this load
   *         function.
   */
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
  // -- TO FILL IN --
  // Put any internal non-public variables used by your event here.
  // ----------------

public:
  /**
   * Purpose: Constructor. Set the base class's event_type based on this event
   *          type's event specs.
   */
  ExampleEvent() {
    // Set event type in base class.
    event_type = event_specs.event_type;
  }

  /**
   * Purpose: Process this event. Called by the event manager when this event should
   *          be triggered based on its timing.
   *
   * Input: Reference to the world (will likely be modified by the event)
   *
   * Output: None.
   */
  template<typename WORLD_T>
  void Process(WORLD_T& world) {
    // --- TO FILL IN ---
    // Code to process an instance of this event should go here.
    // ------------------
  }
};

// Every event type needs to have its event specs defined.
const EventTypeDefinitionSpecs ExampleEvent::event_specs = EventTypeDefinitionSpecs{
  "example_event",
  "Example of a minimally defined event type",
  {"list", "required", "fields", "for", "this", "event", "here"}
};

}