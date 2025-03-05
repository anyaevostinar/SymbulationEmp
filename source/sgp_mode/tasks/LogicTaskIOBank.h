#pragma once

#include "LogicTaskSet.h"

#include "emp/base/assert_warning.hpp"
#include "emp/base/vector.hpp"
#include "emp/math/Random.hpp"
#include "emp/datastructs/map_utils.hpp"
#include "emp/datastructs/set_utils.hpp"
#include "emp/base/Ptr.hpp"

#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <limits>

// Precompute l9 environments to be used during an experiment
namespace sgpmode::tasks {

// Bank of L9 instances
// TODO - save and load functionality
class LogicTaskIOBank {
public:
  using this_t = LogicTaskIOBank;
  using task_set_t = LogicTaskSet;
  using input_t = typename task_set_t::input_t;
  using output_t = typename task_set_t::output_t;

  // This assumes that input_t is something with valid numeric_limits
  static constexpr input_t MIN_LOGIC_TASK_INPUT = std::numeric_limits<input_t>::min();
  static constexpr input_t MAX_LOGIC_TASK_INPUT = std::numeric_limits<input_t>::max();
  static constexpr size_t MAX_ENV_BUILD_TRIES = 10000;

  // Numeric environment (specifies input buffer for an organism + all correct outputs)
  struct TaskIO {
    emp::vector<input_t> input_buffer;                // Specifies the inputs for this environment instance.
    std::unordered_set<output_t> valid_outputs;       // Set of valid output values for this environment.
    emp::vector<output_t> correct_outputs;            // Correct outputs indexed by task id.
    std::unordered_map<output_t, emp::vector<size_t>> task_lookup; // Which output belongs to which task?
    bool is_collision=false;

    // Clear task io
    void Clear() {
      input_buffer.clear();
      valid_outputs.clear();
      correct_outputs.clear();
      task_lookup.clear();
      is_collision=false;
    }

    bool operator==(const TaskIO& other) const {
      return std::tie(
        input_buffer, valid_outputs, correct_outputs, task_lookup
      ) == std::tie(
        other.input_buffer, other.valid_outputs, other.correct_outputs, other.task_lookup
      );
    }

  };

protected:

  emp::Random& random;
  task_set_t& task_set;
  // emp::vector<emp::Ptr<TaskIO>> io_bank; // Bank of environments.
  emp::vector<TaskIO> io_bank; // Bank of environments.

  /// Internal helper function to add a task (task_id) output (output_value) to an environment (env)
  void SetTaskOutput(TaskIO& task_io, size_t task_id, output_t output_value) {
    if (task_id >= task_io.correct_outputs.size()) {
      task_io.correct_outputs.resize(task_id+1, 0);
    }
    task_io.correct_outputs[task_id] = output_value;                      // Set correct output for this task id.
    task_io.is_collision = emp::Has(task_io.valid_outputs, output_value); // Mark if io contains an output collision.
    task_io.valid_outputs.emplace(output_value);                          // Add output value to valid outputs set.
    // Add output value to task lookup (regardless of whether or not we've seen this output value before)
    if (emp::Has(task_io.task_lookup, output_value)) {
      // task_io.task_lookup.find(output_value)->second.emplace_back(task_id);
      task_io.task_lookup[output_value].emplace_back(task_id);
    } else {
      task_io.task_lookup.emplace(
        std::make_pair(output_value, emp::vector<size_t>({task_id}))
      );
    }
  }

  TaskIO BuildTaskIO(bool unique_outputs) {
    TaskIO task_io;
    task_io.is_collision=true;
    size_t build_tries = 0;
    do {
      task_io.Clear();
      task_io.correct_outputs.resize(task_set.GetSize(), (uint32_t)-1);
      task_io.input_buffer = {
        (input_t)random.GetUInt(this_t::MIN_LOGIC_TASK_INPUT, this_t::MAX_LOGIC_TASK_INPUT),
        (input_t)random.GetUInt(this_t::MIN_LOGIC_TASK_INPUT, this_t::MAX_LOGIC_TASK_INPUT)
      };
      for (size_t task_id = 0; (task_id < task_set.GetSize()) && !task_io.is_collision; ++task_id) {
        const auto& task_def = task_set.GetTaskDef(task_id);
        const output_t task_output = task_def.calc_output_fun(
          (task_def.num_inputs > 1) ? task_io.input_buffer : emp::vector<input_t>({task_io.input_buffer[0]})
        );
        SetTaskOutput(task_io, task_id, task_output);
      }
      ++build_tries;
    } while (task_io.is_collision && unique_outputs && (build_tries < this_t::MAX_ENV_BUILD_TRIES));
    emp_assert_warning(build_tries <= this_t::MAX_ENV_BUILD_TRIES, "Failed to build environment with unique outputs for each task.");
    return task_io;
  }

public:

  LogicTaskIOBank(
    emp::Random& rnd,
    task_set_t& a_task_set
  ) :
    random(rnd),
    task_set(a_task_set)
  { ; }

  ~LogicTaskIOBank() {
    Clear();
  }

  // Generate count number of task io instances, adding each to the io bank.
  // Each task io is guaranteed to have unique outputs for teach possible task.
  // WARNING - calling this function will delete any existing task ios in this bank, invalidating references to them.
  void GenerateBank(size_t count, bool unique_outputs=true) {
    Clear();
    io_bank.resize(count);
    for (size_t n = 0; n < count; ++n) {
      // io_bank[n] = emp::NewPtr<Environment>(BuildEnvironment(unique_outputs));
      io_bank[n] = BuildTaskIO(unique_outputs);
    }
  }

  void Clear() {
    // for (emp::Ptr<Environment> env : environment_bank) {
    //   env.Delete();
    // }
    io_bank.clear();
  }

  size_t GetSize() const { return io_bank.size(); }

  // const Environment& GetEnvironment(size_t i) const { emp_assert(i < GetSize()); return *(environment_bank[i]); }
  const TaskIO& GetIO(size_t i) const {
    emp_assert(i < GetSize());
    return io_bank[i];
  }

  // const Environment& GetRandEnv() {
  //   emp_assert(GetSize(), "Environment bank is empty", GetSize());
  //   return *(environment_bank[random.GetUInt(environment_bank.size())]);
  // }
  const TaskIO& GetRandIO() {
    emp_assert(GetSize(), "IO bank is empty", GetSize());
    return io_bank[random.GetUInt(io_bank.size())];
  }

};

}