#pragma once

#include "TaskSet.h"
#include "boolean_logic_impls.h"

#include <string>
#include <algorithm>
#include <functional>
#include <map>
#include <unordered_set>
#include <set>

namespace sgpmode::tasks {

// TODO - write tests

/*
  Adapted from AvidaGPTaskSet from
    https://github.com/amlalejini/directed-digital-evolution

  Boolean logic tasks.
  - *Only* responsible for tracking task functionality
    (knows nothing about world, any configured task dependencies, etc)
*/
class LogicTaskSet : public TaskSet<uint32_t, uint32_t> {
public:
  using this_t = LogicTaskSet;
  using base_t = TaskSet<uint32_t, uint32_t>;
  using input_t = uint32_t;     // Task input type
  using output_t = uint32_t;    // Task output type
  using task_def_t = typename base_t::TaskDefinition;
  using calc_fun_t = typename base_t::calc_output_fun_t;

  /*
    Helper struct includes everything needed to define a logic task minus the
    task ID (needed for TaskSet::TaskDefinition).
  */
  struct LogicTaskSpec {
    std::string name;
    calc_fun_t calc;
    size_t num_inputs;
    std::string desc;
    LogicTaskSpec(
      const std::string& a_name,
      const calc_fun_t& a_calc,
      size_t a_num_inputs,
      const std::string& a_desc
    ) :
      name(a_name), calc(a_calc), num_inputs(a_num_inputs), desc(a_desc)
    { ; }
  };

protected:


  // Static map of valid pre-defined tasks.
  static const std::map<std::string, LogicTaskSpec> predefined_tasks;

public:

  static bool IsPredefined(const std::string& name) {
    return emp::Has(this_t::predefined_tasks, name);
  }

  /*
    Add tasks by name from a set of pre-defined tasks
  */
  void AddTasksByName(const emp::vector<std::string>& names) {
    std::vector<std::string> unused_names;
    for (const std::string& name : names) {
      if (emp::Has(this_t::predefined_tasks, name)) {
        auto& task_spec = this_t::predefined_tasks.at(name);
        AddTask(
          task_spec.name,
          task_spec.calc,
          task_spec.num_inputs,
          task_spec.desc
        );
      } else {
        unused_names.emplace_back(name);
      }
    }
    emp_assert(unused_names.size()==0, unused_names);
  }

  /*
    Add new logic task from given logic task spec.
  */
  size_t AddLogicTask(const LogicTaskSpec& spec) {
    return AddTask(
      spec.name,
      spec.calc,
      spec.num_inputs,
      spec.desc
    );
  }

  size_t AddLogicTask(const std::string& name) {
    emp_assert(emp::Has(this_t::predefined_tasks, name));
    const auto& task_spec = this_t::predefined_tasks.at(name);
    return AddTask(
      task_spec.name,
      task_spec.calc,
      task_spec.num_inputs,
      task_spec.desc
    );
  }

};

/// Pre-defined tasks for the logic task set
const std::map<
  std::string,
  LogicTaskSet::LogicTaskSpec
> LogicTaskSet::predefined_tasks = {
  {
    "ECHO",
    LogicTaskSet::LogicTaskSpec{
      "ECHO",
      [](const emp::vector<LogicTaskSet::input_t>& inputs) -> uint32_t {
        emp_assert(inputs.size() >= 1);
        return sgpmode::logic::ECHO(inputs[0]);
      },
      1,
      "ECHO function"
    }
  },

  {
    "NOT",
    LogicTaskSet::LogicTaskSpec{
      "NOT",
      [](const emp::vector<LogicTaskSet::input_t>& inputs) -> uint32_t {
        emp_assert(inputs.size() >= 1);
        return logic::NOT(inputs[0]);
      },
      1,
      "NOT boolean logic function"
    }
  },

  {
    "NAND",
    LogicTaskSet::LogicTaskSpec{
      "NAND",
      [](const emp::vector<LogicTaskSet::input_t>& inputs) -> uint32_t {
        emp_assert(inputs.size() >= 2);
        return logic::NAND(inputs[0], inputs[1]);
      },
      2,
      "NAND boolean logic function"
    }
  },

  {
    "OR_NOT",
    LogicTaskSet::LogicTaskSpec{
      "OR_NOT",
      [](const emp::vector<LogicTaskSet::input_t>& inputs) -> uint32_t {
        emp_assert(inputs.size() >= 2);
        return logic::OR_NOT(inputs[0], inputs[1]);
      },
      2,
      "OR_NOT boolean logic function"
    }
  },

  {
    "AND",
    LogicTaskSet::LogicTaskSpec{
      "AND",
      [](const emp::vector<LogicTaskSet::input_t>& inputs) -> uint32_t {
        emp_assert(inputs.size() >= 2);
        return logic::AND(inputs[0], inputs[1]);
      },
      2,
      "AND boolean logic function"
    }
  },

  {
    "OR",
    LogicTaskSet::LogicTaskSpec{
      "OR",
      [](const emp::vector<LogicTaskSet::input_t>& inputs) -> uint32_t {
        emp_assert(inputs.size() >= 2);
        return logic::OR(inputs[0], inputs[1]);
      },
      2,
      "OR boolean logic function"
    }
  },

  {
    "AND_NOT",
    LogicTaskSet::LogicTaskSpec{
      "AND_NOT",
      [](const emp::vector<LogicTaskSet::input_t>& inputs) -> uint32_t {
        emp_assert(inputs.size() >= 2);
        return logic::AND_NOT(inputs[0], inputs[1]);
      },
      2,
      "AND_NOT boolean logic function"
    }
  },

  {
    "NOR",
    LogicTaskSet::LogicTaskSpec{
      "NOR",
      [](const emp::vector<LogicTaskSet::input_t>& inputs) -> uint32_t {
        emp_assert(inputs.size() >= 2);
        return logic::NOR(inputs[0], inputs[1]);
      },
      2,
      "NOR boolean logic function"
    }
  },

  {
    "XOR",
    LogicTaskSet::LogicTaskSpec{
      "XOR",
      [](const emp::vector<LogicTaskSet::input_t>& inputs) -> uint32_t {
        emp_assert(inputs.size() >= 2);
        return logic::XOR(inputs[0], inputs[1]);
      },
      2,
      "XOR boolean logic function"
    }
  },

  {
    "EQU",
    LogicTaskSet::LogicTaskSpec{
      "EQU",
      [](const emp::vector<LogicTaskSet::input_t>& inputs) -> uint32_t {
        emp_assert(inputs.size() >= 2);
        return logic::EQU(inputs[0], inputs[1]);
      },
      2,
      "EQU boolean logic function"
    }
  }

};

} // end sgpmode namespace