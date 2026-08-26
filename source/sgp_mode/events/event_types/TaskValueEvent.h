#pragma once
#ifndef TASK_VALUE_EVENT_H
#define TASK_VALUE_EVENT_H

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

/**
 * Purpose: Definition of a task_value event type.
 *          A task_value event modifies the value of a task.
 *          Event configuration options:
 *            - action: How should this event modify task values?
 *                - Options:
 *                    - change: set the value of this task to the given value
 *                    - add: add the given value to this task's current value
 *                    - mult: multiply the task's current value by the given value
 *            - task_name: Which task(s) should be affected? Can be given as
 *                         either a string indicating a single task or a list of
 *                         tasks. All tasks must be valid.
 *            - value: Value used to update task value according to event action.
 *            - timing: Event timing.
 *            - group: Optional. Should this event affect sym, host, or shared tasks?
 */
class TaskValueEvent : public Event {
public:

  /**
   * Purpose: Event specification object shared by all instances of this event
   *          type.
   */
  static const EventTypeDefinitionSpecs event_specs;

  /**
   * Purpose: Defines valid actions for a task value event.
   */
  enum class ACTION_TYPE { CHANGE, ADD, MULT };

  /**
   * Purpose: Defines valid task groups that can be affected by task value event.
   */
  enum class TASK_GROUP { SHARED, HOST, SYM };

  /**
   * Purpose: Mapping from string to action type. Shared by all instances of this
   *          class.
   */
  static const std::unordered_map<std::string, ACTION_TYPE> valid_action_types;

  /**
   * Purpose: Mapping from string to task group. Shared by all instances of this
   *          class.
   */
  static const std::unordered_map<std::string, TASK_GROUP> valid_task_groups;

  using json_t = nlohmann::json;
  using action_t = ACTION_TYPE;
  using task_group_t = TASK_GROUP;

  /**
   * Purpose: Required LoadEventFromJSON function. Configures this event based on
   *          given json object.
   *
   * Input: json object to load from, reference to world.
   *
   * Output: Pointer to a new TaskValueEvent created by this function.
   *
   */
  template <typename WORLD_T>
  static emp::Ptr<TaskValueEvent> LoadEventFromJSON(
    json_t& event_json,
    WORLD_T& world
  ) {
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

  /**
   * Purpose: What task value action does this event apply?
   */
  action_t action;

  /**
   * Purpose: List of task ids that this event applies to
   */
  emp::vector<size_t> task_ids;

  /**
   * Purpose: List of task names that this event applies to. Corresponds to task_ids.
   *          Redundant information (with task_ids) that is kept for debugging asserts.
   */
   emp::vector<std::string> task_names;

   /**
   * Purpose: What task group does this event apply to? Sym / host / shared?
   */
  task_group_t task_group;

  /**
   * Purpose: Event value to be applied to task value
   */
  double value;

  /**
   * Purpose: Internal function that applies the correct action to the given task
   *          information struct.
   *
   * Input: Reference to the task information struct to be modified.
   *
   * Output: None.
   */
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

  /**
   * Purpose: TaskValueEvent constructor. Sets the base class's event type
   *          based on event specs.
   */
  TaskValueEvent() {
    // Set event type in base class.
    event_type = event_specs.event_type;
  }

  /**
   * Purpose: Process this TaskValueEvent.
   *
   * Input: Reference to the world.
   *
   * Output: None.
   */
  template<typename WORLD_T>
  void Process(WORLD_T& world) {
    // For each task_id, apply action
    auto& world_task_env = world.GetTaskEnv();
    for (size_t task_id : task_ids) {
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

#endif