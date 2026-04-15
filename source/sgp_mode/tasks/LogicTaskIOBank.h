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

// Each numeric "environment" has four input numbers.
// We precompute all combinations of outputs (4x4 = 16 for two input tasks)

// Precompute l9 environments to be used during an experiment
namespace sgpmode::tasks {

// Bank of L9 instances
// TODO - save and load functionality
class LogicTaskIOBank {
public:
  using this_t = LogicTaskIOBank;
  using task_set_t = LogicTaskSet;
  using input_t = typename task_set_t::input_t;   // uint32_t
  using output_t = typename task_set_t::output_t; // uint32_t

  // This assumes that input_t is something with valid numeric_limits
  static constexpr input_t MIN_LOGIC_TASK_INPUT = std::numeric_limits<input_t>::min();
  static constexpr input_t MAX_LOGIC_TASK_INPUT = std::numeric_limits<input_t>::max();
  static constexpr size_t MAX_ENV_BUILD_TRIES = 10000;

  // Used by TaskIO struct to associate a set of inputs with their associated output for a task
  struct IOSet {
    emp::vector<input_t> inputs;
    output_t output;

    IOSet() = default;
    IOSet(const emp::vector<input_t>& ins, output_t out)
      : inputs(ins), output(out) { ; }

    bool operator==(const IOSet& other) const {
      return std::tie(inputs, output) == std::tie(other.inputs, other.output);
    }
  };

  // Numeric environment (specifies input buffer for an organism + all correct outputs)
  struct TaskIO {
    emp::vector<input_t> input_buffer;                  // Specifies the inputs for this environment instance.
    std::unordered_set<output_t> valid_outputs;         // Set of valid output values for this environment.
    emp::vector<emp::vector<IOSet>> correct_outputs; // Indexed by task id. For each task id, correct outputs associated with particular inputs.
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

    bool IsValidOutput(float output) const {
      // Reinterpet floating point output value as output_t (uint usually)
      return IsValidOutput(*(reinterpret_cast<output_t*>(&output)));
    }

    bool IsValidOutput(output_t output) const {
      return emp::Has(valid_outputs, output);
    }

    const emp::vector<size_t>& GetTaskIDs(output_t output) const {
      emp_assert(IsValidOutput(output));
      return task_lookup.at(output);
    }

    // Get number of distinct possible outputs stored for this task
    size_t GetNumTaskOutputs(size_t task_id) const {
      emp_assert(task_id < correct_outputs.size());
      return correct_outputs[task_id].size();
    }
  };

protected:
  emp::Random& random;         // Random number generator (unowned)
  task_set_t& task_set;        // Task set associated with this IO bank
  emp::vector<TaskIO> io_bank; // Bank of environments.

  /// Internal helper function to add a task (task_id) output (output_value) to an environment (env)
  // task_io: TaskIO instance to modify
  // task_id: TaskID to set output for
  // input_values: Particular input values associated with given output_value
  // output_value: Particular output value associated with given input_values
  void SetTaskOutput(
    TaskIO& task_io,
    size_t task_id,
    const IOSet& io_set
  ) {
    if (task_id >= task_io.correct_outputs.size()) {
      task_io.correct_outputs.resize(task_id+1, {});
    }
    const output_t output_val = io_set.output;
    task_io.correct_outputs[task_id].emplace_back(io_set);  // Add this input-output pairing for this task
    task_io.is_collision = emp::Has(task_io.valid_outputs, output_val); // Mark if io contains an output collision.
    task_io.valid_outputs.emplace(output_val);                          // Add output value to valid outputs set.
    // Add output value to task lookup (regardless of whether or not we've seen this output value before)
    if (emp::Has(task_io.task_lookup, output_val)) {
      task_io.task_lookup[output_val].emplace_back(task_id);
    } else {
      task_io.task_lookup.emplace(
        std::make_pair(output_val, emp::vector<size_t>({task_id}))
      );
    }
  }

  TaskIO BuildTaskIO(bool unique_outputs, size_t num_inputs_per_task = 4) {
    TaskIO task_io;
    task_io.is_collision=false;
    bool collision_bail = false; // Should we bail because of a collision?
    size_t build_tries = 0;
    // For each task, build IO combinations for num_inputs_per_task inputs
    do {
      // Reset task io
      task_io.Clear();
      // Prepare correct outputs to hold IO combinations for each task.
      task_io.correct_outputs.resize(task_set.GetSize(), {});
      // Build input buffer with random values.
      auto& input_buffer = task_io.input_buffer;
      input_buffer.resize(num_inputs_per_task);
      for (size_t i = 0; i < num_inputs_per_task; ++i) {
        const input_t rand_in = (input_t)random.GetUInt(
          this_t::MIN_LOGIC_TASK_INPUT,
          this_t::MAX_LOGIC_TASK_INPUT
        );
        input_buffer[i] = rand_in;
      }
      // For each task, generate output values for input pairings
      for (size_t task_id = 0; (task_id < task_set.GetSize()) && !collision_bail; ++task_id) {
        const auto& task_def = task_set.GetTaskDef(task_id);
        for (size_t input_idx = 0; input_idx < input_buffer.size(); ++input_idx) {
          // Call task function with rotated inputs
          // I.e., Add input pairings for all sequential inputs (not all combinations)
          emp::vector<input_t> buffer(input_buffer.size());
          for (size_t i = 0; i < buffer.size(); ++i) {
            buffer[i] = input_buffer[(i + input_idx) % buffer.size()];
          }
          const output_t task_output = task_def.CalcOutput(buffer);
          SetTaskOutput(task_io, task_id, IOSet{buffer, task_output});
        }
        // Any collisions?
        collision_bail = unique_outputs && task_io.is_collision;
      }
      ++build_tries;
    } while (collision_bail && (build_tries < this_t::MAX_ENV_BUILD_TRIES));
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
  void GenerateBank(size_t count, bool unique_outputs=true, size_t input_buffer_size=4) {
    Clear();
    io_bank.resize(count);
    for (size_t n = 0; n < count; ++n) {
      io_bank[n] = BuildTaskIO(unique_outputs, input_buffer_size);
    }
  }

  void Clear() {
    io_bank.clear();
  }

  size_t GetSize() const { return io_bank.size(); }

  const TaskIO& GetIO(size_t i) const {
    emp_assert(i < GetSize());
    return io_bank[i];
  }

  const TaskIO& GetRandIO() {
    emp_assert(GetSize(), "IO bank is empty", GetSize());
    return io_bank[random.GetUInt(io_bank.size())];
  }

};

}