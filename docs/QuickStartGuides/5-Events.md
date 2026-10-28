# Event system overview
Symbulation has a customizable event system. The event system is currently limited to SGP mode, but will be moved to work in all modes in the future (if this is particularly interesting to you, let us know and we'll prioritize it or take a shot at doing it yourself and contribute it!).

You specify what events you want to occur during an experiment via the `EVENTS_CFG_PATH` configuration setting. You can find examples in the folder `source/test/sgp_mode_test/task-value-events-cfg` and a default empty `events.json` will be generated if you run without it.

There is a built in event type `TaskValueEvent` (found in `source/sgp_mode/events/event_types`) that changes the value of a task. You can implement additional event types in the `event_types` folder to change any other aspects of the system as well.

## TaskValueEvents
The `TaskValueEvent` type has many customization options. Here is an annotated version of an events file (note that `//` indicate annotations but are not valid JSON comments, so you can't use this example directly):

```
{
  "events": [ // Must lead with `events` top level key
    {
      "event_type": "task_value", // Indicates which event type, if you make your own custom type, indicate that here,                        // everything that follows is custom to this event type
      "task_name": "NAND",          // Specify which of the nine logic tasks should be affected,
                                    // note that you can specify multiple using [] as seen below
      "action": "add",              // Valid actions are "add", "change", or "mult". 
                            // Add will add this amount to the existing, change will just directly change it, and mult will multiply.
      "value": 50,          // The amount to add/change/multiply to the existing value
      "timing": "1",        // Events can recur. "1" is how you signify a one-time event (see the next example for recurring)
      "group": "shared"     // Events can apply to only hosts ("host"), only symbionts ("symbiont"), or both ("shared"). 
    },
    {
      "event_type": "task_value",
      "task_name": ["NAND","NOT"], // Note that this will apply to both NAND and NOT
      "action": "mult", 
      "value": -1,                  // Because this is multiplying by -1, it will flip the reward from positive to negative or vice versa
      "timing": "2:5:2",            // This is a recurring event. The format is start:stop:step and the ends are inclusive. The event will therefore start at update 2, recur at update 4 and stop at update 5
      "group": "shared"
    }
  ]
}
```

You can create an infinitely recurring events by using -1 (or anything else less than 0) as the stop value, which the system will replace with the value specified in the configuration file for UPDATES.

## New Event Type
To create a new event type, you need to do the following.

1. In `source/sgp_mode/events/event_types` copy the `ExampleEvent.h` and name it appropriately for your new event. Make sure to update the class name as well inside the file.

2. In the `protected` section of the class, decide on your internal event variables, i.e. what information your event will be based on. If you are planning on using tasks, `emp::vector<size_t> task_ids;` and `emp::vector<std::string> task_names;` are good choices. You can additionally have whatever other variable types you need.

3. Scroll down to the bottom of the file and update `EventTypeDefinitionSpecs` based on your internal event variables. The first item is the name of your event in the JSON file, the second is a useful description, and the third is a set of the keys that are expectd in your event's JSON entry.

4. Go back up and implement your `LoadEventFromJSON`
  * Extract event parameters from the JSON with `sym_json::GetVal<type of variable>(event_json, "name in json file")` and save them to the event with `event->var_name =`
  * If your event has timing component, there is a helper method that extracts that information, so you just need to call `SetEventTimingFromJSON(event_json, event, world);`

5. `Process` is the heart of your event and is where the actual logic is defined of what should happen when the event is triggered. 
  * You can access all the world methods through the `world` parameter
  * If you are doing something specific to `SGPHost` or `SGPSymbiont` (or otherwise SGP specific), you can use the types defined in the `SGPWorld` by creating a type alias at the start of `Process` such as `using sgp_host_t = typename WORLD_T::sgp_host_t;` to do `SGPHost` specific things.
  * If you want to do something to a specific organism based on its internal state, you'll need to also static cast to the correct type, for example `sgp_host_t& sgp_org = static_cast<sgp_host_t&>(world.GetOrg(0));`

6. Finally, you need to add your event type class to `EventTypeLibrary.h`'s `AddDefaultEventTypes()`.

7. Remember that it's a good idea to create a test for your new event type. You can see an example of such a test in `source/test/sgp_mode_test/functional_tests/TaskValueEvent.test.cc`
