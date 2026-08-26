# Event system overview
Symbulation has a customizable event system. The event system is currently limited to SGP mode, but will be moved to work in all modes in the future (if this is particularly interesting to you, let us know and we'll prioritize it or take a shot at doing it yourself and contribute it!).

You specify what events you want to occur during an experiment via the `EVENTS_CFG_PATH` configuration setting. You can find examples in the folder `source/test/sgp_mode_test/task-value-events-cfg` and a default empty `events.json` will be generated if you run without it.

There is a built in event type `TaskValueEvent` (found in `source/sgp_mode/events/event_types`) that changes the value of a task. You can implement additional event types in the `event_types` folder to change any other aspects of the system as well.

# TaskValueEvents
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