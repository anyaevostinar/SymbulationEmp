#pragma once

#include "LogicTaskSet.h"
#include "LogicTaskIOBank.h"

#include "../../json/json.hpp"

#include "emp/base/vector.hpp"
#include "emp/bits/Bits.hpp"
#include "emp/tools/string_utils.hpp"
#include "emp/datastructs/set_utils.hpp"
#include "emp/math/math.hpp"

#include <functional>
#include <string>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <map>

/*
  Implements logic task environment.
    - Responsible for configuring LogicTaskSet, etc.
    - Responsible for managing any task dependencies, etc.
*/

namespace sgpmode::tasks {

// TODO - add functions to attach to config snapshot
class LogicTaskEnvironment {
public:
  struct TaskReqInfo;
  using this_t = LogicTaskEnvironment;
  // NOTE - Could add cpustate as argument (would then need to template class off state type)
  using fun_calc_task_value_t = std::function<double(
    this_t&,            /* The environment, which could contain necessary environment state information */
    const TaskReqInfo&, /* Task requirement information */
    double              /* Current organism 'merit'/'points' value */
  )>;
  using json_t = nlohmann::json;
  using io_bank_t = LogicTaskIOBank;

  // Used to track information about task performance restrictions/requirements.
  struct TaskReqInfo {
    size_t task_id = 0;                      // Task id
    size_t max_repeats = 0;              // How many times are organisms allowed to repeat this task for credit?
    double task_value = 0;                   // Base value for this task
    // bool has_dependencies = false;           // Does credit for this task depend on performing other tasks prior?
    // emp::BitVector dependencies;             // Which tasks does this task depend on? (and how much?) TODO - should this be a bitvector?
    // emp::BitVector blocks;                   // Which tasks does this task block?
    fun_calc_task_value_t fun_calc_task_val; // Returns task value based on task mode
  };

protected:
  static const std::map<std::string, fun_calc_task_value_t> predefined_reward_functions;

  LogicTaskSet task_set;
  std::unordered_map<size_t, size_t> host_tasks;  // Keys: Task IDs (in task_set) of host tasks; Values: associated index into host_task_reqs
  std::unordered_map<size_t, size_t> sym_tasks;   // Keys: Task IDs (in task set) of sym tasks; Values: associated index into sym_task_reqs
  io_bank_t io_bank;

  emp::vector<TaskReqInfo> host_task_reqs;
  emp::vector<TaskReqInfo> sym_task_reqs;

  // TODO - track task performance?

  // TODO - move this into util file in json directory
  template<typename RET_TYPE>
  RET_TYPE GetVal(
    json_t& json,
    const std::string& field,
    RET_TYPE default_val
  ) {
    return (json.contains(field)) ?
      static_cast<RET_TYPE>(json[field]) :
      default_val;
  }

  void SetTaskReqInfo(TaskReqInfo& info, json_t& task_cfg_json) {
    info.task_value = GetVal<double>(task_cfg_json, "value", 1);
    info.max_repeats = GetVal<size_t>(task_cfg_json, "max_repeats", std::numeric_limits<size_t>::max());
    const std::string reward_mode = GetVal<std::string>(task_cfg_json, "reward_mode", "add");
    emp_assert(emp::Has(this_t::predefined_reward_functions, reward_mode));
    info.fun_calc_task_val = this_t::predefined_reward_functions.at(reward_mode);
  }

  // BuildTaskRewardFun_Add() {

  // }

  size_t GetHostTaskReqID(size_t task_id) const {
    emp_assert(IsHostTask(task_id));
    return host_tasks.at(task_id);
  }

  size_t GetSymTaskReqID(size_t task_id) const {
    emp_assert(IsSymTask(task_id));
    return sym_tasks.at(task_id);
  }

  // NOTE - any reason for these to be public? Probably not.
  void AddHostTask(size_t task_id, json_t& task_cfg_json) {
    emp_assert(!IsHostTask(task_id));
    const size_t host_req_idx = host_task_reqs.size();
    host_task_reqs.emplace_back();
    SetTaskReqInfo(host_task_reqs[host_req_idx], task_cfg_json);
    // Keys: Task IDs (in task_set) of host tasks; Values: associated index into host_task_reqs
    host_tasks[task_id] = host_req_idx;
  }

  void AddSymTask(size_t task_id, json_t& task_cfg_json) {
    emp_assert(!IsSymTask(task_id));
    const size_t sym_req_idx = sym_task_reqs.size();
    sym_task_reqs.emplace_back();
    SetTaskReqInfo(sym_task_reqs[sym_req_idx], task_cfg_json);
    // Keys: Task IDs (in task set) of sym tasks; Values: associated index into sym_task_reqs
    sym_tasks[task_id] = sym_req_idx;
  }

  // Load tasks from an environment file.
  void LoadTasks(const std::string& env_filepath);

public:

  LogicTaskEnvironment(
    emp::Random& random
  ) :
    io_bank(random, task_set)
  { }

  void Clear() {
    task_set.Clear();
    host_tasks.clear();
    sym_tasks.clear();
    host_task_reqs.clear();
    sym_task_reqs.clear();
    io_bank.Clear();
  }

  size_t GetTaskCount() const { return task_set.GetSize(); }
  size_t GetHostTaskCount() const { return host_task_reqs.size(); }
  size_t GetSymTaskCount() const { return sym_task_reqs.size(); }

  bool IsHostTask(size_t task_id) const {
    return emp::Has(host_tasks, task_id);
  }

  bool IsSymTask(size_t task_id) const {
    return emp::Has(sym_tasks, task_id);
  }

  TaskReqInfo& GetHostTaskReq(size_t task_id) {
    emp_assert(IsHostTask(task_id));
    return host_task_reqs[GetHostTaskReqID(task_id)];
  }

  TaskReqInfo& GetSymTaskReq(size_t task_id) {
    emp_assert(IsSymTask(task_id));
    return sym_task_reqs[GetSymTaskReqID(task_id)];
  }

  const io_bank_t& GetIOBank() const { return io_bank; }

  void Setup(const std::string& env_filepath, size_t io_bank_size, bool io_unique_outputs) {
    LoadTasks(env_filepath); // Will reset current bank, etc.
    io_bank.GenerateBank(io_bank_size, io_unique_outputs);
  }

  // NOTE - can have a process output buffer function that triggers signals that world can attach functions to

};

void LogicTaskEnvironment::LoadTasks(const std::string& env_filepath) {
  std::cout << "Loading tasks from environment file." << std::endl;
  // Clear any existing task information.
  Clear();

  // TODO - add support for limited resource pools
  //     E.g., {"resource_pools": {"name": "A", "in-flow": 5, "out-flow": 5}}
  //     Then, tasks can be associated with a particular resource pool.

  // === Parse environment file ===
  // Check if given environment file exists. Exit if not.
  const bool env_file_exists = std::filesystem::exists(env_filepath);
  if (!env_file_exists) {
    std::cout << "Environment file does not exist: " << env_filepath << std::endl;
    std::exit(EXIT_FAILURE);
  }
  // If environment file exists, read it.
  std::ifstream env_ifstream(env_filepath);
  nlohmann::json env_json;
  env_ifstream >> env_json;
  // (1) Take first pass overall task categories to add all tasks to task set
  const emp::vector<std::string> task_categories = {"shared", "host", "symbiont"};
  for (const std::string& cat : task_categories) {
    if (!env_json.contains(cat)) {
      continue;
    }
    std::cout << "  Identified " << cat << " tasks:" << std::endl;
    emp_assert(env_json[cat].contains("tasks"));
    auto& cat_tasks = env_json[cat]["tasks"];
    for (auto& task : cat_tasks) {
      emp_assert(task.contains("name"));
      std::cout << "    - " << task["name"];
      // If we've already added this task to the task_set, skip.
      if (task_set.HasTask(task["name"])) continue;
      // Next, check that this is a valid pre-defined task name.
      if (!LogicTaskSet::IsPredefined(task["name"])) {
        std::cout << " (invalid). Exiting." << std::endl;
        std::exit(EXIT_FAILURE);
      }
      std::cout << std::endl;
      // Task name is valid, add to task_set.
      task_set.AddLogicTask(task["name"]);
    }
  }
  // (2) Process shared tasks.
  if (env_json.contains("shared")) {
    auto& shared_tasks = env_json["shared"]["tasks"];
    for (auto& task : shared_tasks) {
      // TODO - create configure task function, clean up by adding constructor to req info
      // Configure task requirement info from json
      const size_t task_id = task_set.GetID(task["name"]);
      AddHostTask(task_id, task);
      AddSymTask(task_id, task);
    }
  }
  // (3) Process host tasks.
  //     NOTE - Overrides any task settings set under "shared" tasks.
  if (env_json.contains("host")) {
    auto& host_tasks = env_json["host"]["tasks"];
    for (auto& task : host_tasks) {
      const size_t task_id = task_set.GetID(task["name"]);
      // Is this task already in host_tasks?
      if (IsHostTask(task_id)) {
        // Update settings.
        TaskReqInfo& info = GetHostTaskReq(task_id);
        // NOTE - discuss how defaults are applied, how override works
        SetTaskReqInfo(info, task);
      } else {
        // Add new host-specific task
        AddHostTask(task_id, task);
      }
    }
  }

  // (4) Process symbiont tasks.
  //     NOTE - Overrides any task settings set under "shared" tasks.
  if (env_json.contains("symbiont")) {
    auto& symbiont_tasks = env_json["symbiont"]["tasks"];
    for (auto& task : symbiont_tasks) {
      const size_t task_id = task_set.GetID(task["name"]);
      // Is this task already in host_tasks?
      if (IsSymTask(task_id)) {
        // Update settings.
        TaskReqInfo& info = GetSymTaskReq(task_id);
        // NOTE - discuss how defaults are applied, how override works
        SetTaskReqInfo(info, task);
      } else {
        // Add new host-specific task
        AddSymTask(task_id, task);
      }
    }
  }

}

// NOTE - not convinced that this is the cleanest way to do this.
//        Essentially, we need a way to associate reward mode strings with functions.
//        This will likely need to change as we figure out what influences
const std::map<
  std::string,
  LogicTaskEnvironment::fun_calc_task_value_t
> LogicTaskEnvironment::predefined_reward_functions = {
  {
    "add",
    [](
      LogicTaskEnvironment& env,
      const LogicTaskEnvironment::TaskReqInfo& task_req,
      double org_cur_value
    ) -> double {
      return org_cur_value + task_req.task_value;
    }
  },
  {
    "mult",
    [](
      LogicTaskEnvironment& env,
      const LogicTaskEnvironment::TaskReqInfo& task_req,
      double org_cur_value
    ) -> double {
      return org_cur_value * task_req.task_value;
    }
  },
  {
    "pow",
    [](
      LogicTaskEnvironment& env,
      const LogicTaskEnvironment::TaskReqInfo& task_req,
      double org_cur_value
    ) -> double {
      return org_cur_value * emp::Pow2(task_req.task_value);
    }
  }
};

} // End sgpmode::tasks namespace