#pragma once

#include "EventTiming.h"

#include "../../json/json.hpp"
#include "../../json/json_utils.h"

#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/bits/Bits.hpp"
#include "emp/datastructs/set_utils.hpp"
#include "emp/datastructs/map_utils.hpp"
#include "emp/math/math.hpp"
#include "emp/tools/string_utils.hpp"

#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <string>
#include <unordered_map>

/*

Event types:
- task_value_change: change the task value of an existing task to a new value
  - Parameters:
    - task_name: name of task to change
    - value: value to change to
    - timing: event timing. For one-time event, provide a single number. For a recurring event, provide start:stop:step.
    - group: [optional] Which task group to apply change to? shared / symbiont / host
- task_value_add: change the task value of an existing task by adding the given value
- task_value_mul: change the task value of an existing task by multiplying the given value


*/

namespace sgpmode {

// Basic event type
class Event {
protected:
  size_t event_type_id = 0;
  std::string event_type{"NULL"};
  EventTiming timing;
  bool is_done = false;

public:

  size_t GetEventTypeID() const { return event_type_id; }
  void SetEventTypeID(size_t id) { event_type_id = id; }

  const std::string& GetEventType() const { return event_type; }

  const EventTiming& GetEventTiming() const { return timing; }
  bool IsRecurring() const { return timing.IsRecurring(); }
  size_t GetStartUpdate() const { return timing.GetStartUpdate(); }
  size_t GetEndUpdate() const { return timing.GetEndUpdate(); }
  size_t GetNextUpdate() const { return timing.GetNextUpdate(); }

};

class TaskValueEvent : public Event {
public:
  enum class ACTION_TYPE { CHANGE, ADD, MULT };
  enum class TASK_GROUP { SHARED, HOST, SYM };
  static const std::unordered_map<std::string, ACTION_TYPE> valid_action_types;
  static const std::unordered_map<std::string, TASK_GROUP> valid_task_groups;

  using json_t = nlohmann::json;
  using action_t = ACTION_TYPE;
  using task_group_t = TASK_GROUP;

  // TODO: make adding new event types as easy / centralized as possible
  //        i.e., relocate these loaders to centralized location where other
  //            event type definitions are defined.
  // World is necessary for parsing the task id
  template <typename WORLD_T>
  static emp::Ptr<TaskValueEvent> LoadEventFromJSON(json_t& event_json, WORLD_T& world) {
    // This should be a task_value event
    emp_assert(event_json["event_type"] == "task_value");
    // NOTE: Caller is responsible for event deletion.
    emp::Ptr<TaskValueEvent> event = emp::NewPtr<TaskValueEvent>();

    // Get event parameters out of json

    // --- Extract task name(s) ---
    //   If multiple tasks are given as an array, accept as vector.
    //   otherwise, wrap single given task into vector.
    emp::vector<std::string> task_names;
    if (event_json["task_name"].is_array()) {
      task_names = event_json["task_name"];
    } else {
      const std::string task_name = event_json["task_name"];
      task_names = {task_name};
    }
    std::cout << task_names << std::endl;

    // Convert task names to task_ids as known by the task set.
    const auto& world_task_set = world.GetTaskEnv().GetTaskSet();
    emp::vector<size_t> task_ids(task_names.size());
    for (size_t task_i = 0; task_i < task_names.size(); ++task_i) {
      auto& task_name = task_names[task_i];
      emp_assert(world_task_set.HasTask(task_name));
      task_ids[task_i] = world_task_set.GetID(task_name);
    }

    // --- Set the action ---
    const std::string action_str(event_json["action"]);
    emp_assert(emp::Has(valid_action_types, action_str));
    const action_t action = valid_action_types.at(action_str);

    // --- Set the task value ---
    const double task_value = sym_json::GetVal<double>(event_json, "value");
    // event_json.get<double>("value");

    // --- Set the timing ---
    // TODO - move into a function to be shared by other event types
    const std::string timing_str(event_json["timing"]);
    // EventTiming timing;
    // Given as a single (integer) number?
    if (emp::is_digits(timing_str)) {
      const size_t start_u = emp::from_string<size_t>(timing_str);
      event->timing.Reset(start_u);
    } else {
      // Timing given as Start:Stop:Step
      emp::vector<std::string> recurring_str = emp::slice(timing_str, ':');
      emp_assert(recurring_str.size() == 3);
      const size_t start_u = emp::from_string<size_t>(recurring_str[0]);
      const size_t stop_u = emp::from_string<size_t>(recurring_str[1]);
      const size_t freq = emp::from_string<size_t>(recurring_str[2]);
      event->timing.Reset(start_u, stop_u, freq);
    }

    // --- Task group ---
    // Is there a group specification? If not, assume shared.
    const std::string group_str = (event_json.contains("group")) ? event_json["group"] : "shared";
    emp_assert(emp::Has(valid_task_groups, group_str));
    const task_group_t task_group = valid_task_groups.at(group_str);

    // Create new event to pass out of function.
    // TODO: incorporate these lines above (i.e., no need to create temp variables)
    event->action = action;
    event->task_ids = task_ids;
    event->task_names = task_names;
    event->task_group = task_group;
    event->value = task_value;

    return event;
  }

protected:
  action_t action;         // What task value action does this event apply?
  emp::vector<size_t> task_ids;
  emp::vector<std::string> task_names; // Kept mainly for debugging asserts
  task_group_t task_group;
  double value;

public:
  TaskValueEvent() {
    // Set event type in base class.
    event_type = "task_value";
  }

  template<typename WORLD_T>
  void Process(WORLD_T& world) {
    // TODO
  }
};

// TODO - encapsulate all task value event stuff into single file / namespace(?)
const std::unordered_map<std::string, TaskValueEvent::ACTION_TYPE> TaskValueEvent::valid_action_types = {
    {"change", ACTION_TYPE::CHANGE},
    {"add", ACTION_TYPE::ADD},
    {"mult", ACTION_TYPE::MULT}
  };

const std::unordered_map<std::string, TaskValueEvent::TASK_GROUP> TaskValueEvent::valid_task_groups = {
  {"shared", TASK_GROUP::SHARED},
  {"host", TASK_GROUP::HOST},
  {"symbiont", TASK_GROUP::SYM}
};

// class ChangeTaskRewardTypeEvent : Event {

// };

class StressEvent : Event {

};

}