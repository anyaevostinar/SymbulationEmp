#pragma once

#include "Event.h"
#include "../../tasks/LogicTaskEnvironment.h"

#include "../../../json/json.hpp"
#include "../../../json/json_utils.h"

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

namespace sgpmode {

/*
- task_value: change the task value of one or more existing tasks to a new value
  - Parameters:
    - task_name: name of task to change or list of names to change
    - value: value to change to
    - timing: event timing. For one-time event, provide a single number. For a recurring event, provide start:stop:step.
    - group: [optional] Which task group to apply change to? shared / symbiont / host
*/

class TaskValueEvent : public Event {
public:
  static const EventTypeDefinitionSpecs event_specs;

  enum class ACTION_TYPE { CHANGE, ADD, MULT };
  enum class TASK_GROUP { SHARED, HOST, SYM };
  static const std::unordered_map<std::string, ACTION_TYPE> valid_action_types;
  static const std::unordered_map<std::string, TASK_GROUP> valid_task_groups;

  using json_t = nlohmann::json;
  using action_t = ACTION_TYPE;
  using task_group_t = TASK_GROUP;

  // World is necessary for parsing the task id
  template <typename WORLD_T>
  static emp::Ptr<TaskValueEvent> LoadEventFromJSON(json_t& event_json, WORLD_T& world) {
    // This should be a task_value event
    emp_assert(event_json["event_type"] == event_specs.event_type);
    // NOTE: Caller is responsible for event deletion.
    emp::Ptr<TaskValueEvent> event = emp::NewPtr<TaskValueEvent>();

    // Get event parameters out of json

    // --- Extract task name(s) ---
    //   If multiple tasks are given as an array, accept as vector.
    //   otherwise, wrap single given task into vector.
    emp::vector<std::string>& task_names = event->task_names;
    if (event_json["task_name"].is_array()) {
      task_names = event_json["task_name"];
    } else {
      const std::string task_name = event_json["task_name"];
      task_names = {task_name};
    }

    // Convert task names to task_ids as known by the task set.
    const auto& world_task_set = world.GetTaskEnv().GetTaskSet();
    emp::vector<size_t>& task_ids = event->task_ids;
    task_ids.resize(task_names.size());
    for (size_t task_i = 0; task_i < task_names.size(); ++task_i) {
      auto& task_name = task_names[task_i];
      emp_assert(world_task_set.HasTask(task_name));
      task_ids[task_i] = world_task_set.GetID(task_name);
    }

    // --- Set the action ---
    const std::string action_str(event_json["action"]);
    emp_assert(emp::Has(valid_action_types, action_str));
    event->action = valid_action_types.at(action_str);

    // --- Set the task value ---
    event->value = sym_json::GetVal<double>(event_json, "value");

    // --- Set the timing ---
    SetEventTimingFromJSON(event_json, event);

    // --- Task group ---
    // Is there a group specification? If not, assume shared.
    const std::string group_str = (event_json.contains("group")) ? event_json["group"] : "shared";
    emp_assert(emp::Has(valid_task_groups, group_str));
    event->task_group = valid_task_groups.at(group_str);

    return event;
  }

protected:
  action_t action;                     // What task value action does this event apply?
  emp::vector<size_t> task_ids;        // List of task ids that this event applies to
  emp::vector<std::string> task_names; // Kept mainly for debugging asserts
  task_group_t task_group;             // Sym / host / shared?
  double value;                        // Event value to be applied to task value

  void ApplyAction(tasks::LogicTaskEnvironment::TaskReqInfo& task_req) {
    switch (action) {
      case action_t::ADD:
        task_req.task_value += value;
        break;
      case action_t::MULT:
        task_req.task_value *= value;
        break;
      case action_t::CHANGE:
        task_req.task_value = value;
        break;
    }
  }

public:
  TaskValueEvent() {
    // Set event type in base class.
    event_type = event_specs.event_type;
  }

  template<typename WORLD_T>
  void Process(WORLD_T& world) {
    // For each task_id, apply action
    for (size_t task_id : task_ids) {
      auto& world_task_env = world.GetTaskEnv();
      switch(task_group) {
        case task_group_t::SHARED:
          emp_assert(world_task_env.IsHostTask(task_id));
          emp_assert(world_task_env.IsSymTask(task_id));
          ApplyAction(world_task_env.GetHostTaskReq(task_id));
          ApplyAction(world_task_env.GetSymTaskReq(task_id));
          break;
        case task_group_t::HOST:
          ApplyAction(world_task_env.GetHostTaskReq(task_id));
          break;
        case task_group_t::SYM:
          ApplyAction(world_task_env.GetSymTaskReq(task_id));
          break;
      }
    }
  }

};

const EventTypeDefinitionSpecs TaskValueEvent::event_specs = EventTypeDefinitionSpecs{
  "task_value",
  "Modify the value on a current task",
   {"action", "task_name", "value", "timing"}
};

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

}