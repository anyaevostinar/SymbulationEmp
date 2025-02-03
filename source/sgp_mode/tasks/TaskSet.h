#pragma once

#include "emp/base/vector.hpp"
#include "emp/datastructs/map_utils.hpp"

#include <string>
#include <algorithm>
#include <functional>
#include <map>
#include <unordered_set>


namespace sgpmode::tasks {

// TODO - write tests

/*
  TaskSet is adapted from class of the same name from
    https://github.com/amlalejini/directed-digital-evolution

  TaskSet defines a set of tasks where each task can be identified by a numeric ID
  and a given string name.

  A Task defines a mapping from a given set (vector) of inputs (INPUT_T) to a
  given output (OUTPUT_T). This mapping is specified by the task's calc_output_fun,
  which takes a vector of inputs and maps it to the correct output for the given
  task.

*/
template<typename INPUT_T, typename OUTPUT_T>
class TaskSet {
public:

  using input_t = INPUT_T;
  using output_t = OUTPUT_T;
  using calc_output_fun_t = std::function<output_t(const emp::vector<input_t>&)>;

  /*
    Defines a single instance of a task.
    I.e., a particular set of inputs and the correct output as calculated by the
    TaskDefinition.
  */
  struct TaskInstance {
    size_t task_id;               // Task ID for this task (from task definition)
    emp::vector<input_t> inputs;  // Inputs for this task instance
    output_t output;              // Correct outputs for the given inputs.
  };

protected:

  // Task definition
  struct TaskDefinition {
    size_t id;          // Task id
    std::string name;   // Human-readable task name
    calc_output_fun_t calc_output_fun; // Maps inputs to correct output
    size_t num_inputs;  // How many inputs does this task take?
    std::string desc;   // Task description

    TaskDefinition(
      size_t a_id,
      const std::string& a_name,
      const calc_output_fun_t& a_calc_output_fun,
      size_t a_num_inputs,
      const std::string& a_desc
    ) :
      id(a_id),
      name(a_name),
      calc_output_fun(a_calc_output_fun),
      num_inputs(a_num_inputs),
      desc(a_desc)
      { ; }

  };

  emp::vector<TaskDefinition> task_lib;   // Task library
  std::map<std::string, size_t> name_map; // Maps string task names to task ids

public:

  size_t GetSize() const { return task_lib.size(); }
  const std::string& GetName(size_t id) const { return task_lib[id].name; }
  const std::string& GetDesc(size_t id) const { return task_lib[id].desc; }

  size_t GetID(const std::string& name) const {
    emp_assert(emp::Has(name_map, name), name);
    return emp::Find(name_map, name, (size_t)-1);
  }

  /// Retrieve a task by ID
  const Task& GetTask(size_t id) const { return task_lib[id]; }

  bool HasTask(const std::string& name) const { return emp::Has(name_map, name); }

  /// Add a new task to the task set.
  void AddTask(
    const std::string& name,
    const calc_output_fun_t& calc_output_fun,
    size_t num_inputs,
    const std::string& desc =""
  ) {
    const size_t id = task_lib.size();
    task_lib.emplace_back(
      id,
      name,
      calc_output_fun,
      num_inputs,
      desc
    );
    name_map[name] = id;
  }

  /// Reset the task set.
  void Clear() {
    task_lib.clear();
    name_map.clear();
  }

};

} // end sgpmode namespace
