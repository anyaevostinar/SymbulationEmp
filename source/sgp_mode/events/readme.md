# Events system notes

## Organization

Two layers of organization:

1. Event types - Classes or types of events (e.g., change task value, add new task, volcano, tsunami, acid rain, etc). An event type is specified by an instance of the EventTypeDefinition class.
2. Events - Individual instances of a particular type of event. Each of these events corresponds to an event included in the events file that needs to be triggered during the run.

### EventTypeLibrary

The EventTypeLibrary manages EventTypeDefinitions.
Every event type is specified by an instance of EventTypeDefinition.
The EventTypeLibrary includes definitions for default event types and allows for runtime specification of additional event types.

### EventManager

The EventManager owns an EventTypeLibrary to keep track of valid event types.
The EventManager is responsible for loading a user-defined events file. Each event type in the events file must match a valid event type in the EventTypeLibrary.
For each event in the events file, the EventManager creates an Event object.
The event manager handles triggering events each update.

The world object owns an event manager, and calls the process events function in the event manager each update.
