#ifndef TASKS_H
#define TASKS_H

#include "../default_mode/SymWorld.h"
#include "CPUState.h"
#include <atomic>
#include <variant>
#include <map>
#include <string>

class Task {
  bool unlimited = true;
  emp::vector<size_t> dependencies;
  /// The total number of times this task's dependencies must be completed for
  /// each use of this task
  size_t num_dep_completes = 1;

public:
  std::string name;

  Task(std::string name, bool unlimited = true,
       emp::vector<size_t> dependencies = {}, size_t num_dep_completes = 1)
      : unlimited(unlimited), dependencies(dependencies),
        num_dep_completes(num_dep_completes), name(name) {}

  virtual void MarkAlwaysPerformable() {
    dependencies.clear();
    unlimited = true;
  }

  virtual bool CanPerform(const CPUState &state, size_t task_id) {
    if (state.used_resources->Get(task_id) && !unlimited) {
      return false;
    }
    if (dependencies.size()) {
      size_t actually_completed = std::reduce(
          dependencies.begin(), dependencies.end(), 0, [&](auto acc, auto i) {
            return acc + state.self_completed[i] + (*state.shared_completed)[i];
          });
      if (actually_completed < num_dep_completes) {
        return false;
      }
    }
    return true;
  }

  virtual void MarkPerformed(CPUState &state, uint32_t output, size_t task_id,
                             bool shared) {
    state.used_resources->Set(task_id);

    if (dependencies.size()) {
      // TODO does it make sense to reset to 0, or to let them accumulate
      // resources?
      size_t total = num_dep_completes;
      for (size_t i : dependencies) {
        // Subtract just as much as needed from each dependency until we've
        // accumulated `num_dep_completes` completions
        size_t subtract = std::min(state.self_completed[i], total);
        total -= subtract;
        state.self_completed[i] -= subtract;

        subtract = std::min((*state.shared_completed)[i], total);
        total -= subtract;
        (*state.shared_completed)[i] -= subtract;

        if (total == 0) {
          break;
        }
      }
    }

    if (shared) {
      (*state.shared_completed)[task_id]++;
    } else {
      state.self_completed[task_id]++;
    }
  }

  virtual float CheckOutput(CPUState &state, uint32_t output) = 0;
};

/**
 * An input task computes an expected output based on the inputs, and if the
 * organism's output matches, it gives it a certain reward:
 * `InputTask sum{ "SUM", 2, 1.0, [](auto &x) { return x[0] + x[1]; } };`
 */
class InputTask : public Task {
  size_t n_inputs;
  std::function<uint32_t(emp::vector<uint32_t> &)> task_fun;
  float value;

public:
  InputTask(std::string name, size_t n_inputs, float value,
            std::function<uint32_t(emp::vector<uint32_t> &)> task_fun,
            bool unlimited = true, emp::vector<size_t> dependencies = {},
            size_t num_dep_completes = 1)
      : Task(name, unlimited, dependencies, num_dep_completes),
        n_inputs(n_inputs), task_fun(task_fun), value(value) {}

  float CheckOutput(CPUState &state, uint32_t output) override {
    for (size_t i = 0; i < state.input_buf.size(); i++) {
      if (state.input_buf[i] == 0)
        continue;

      emp::vector<uint32_t> inputs{state.input_buf[i], state.input_buf[i + 1]};
      if (n_inputs > 1 && inputs[1] == 0)
        continue;

      if (task_fun(inputs) == output) {
        return value;
      }
    }
    return 0;
  }
};

/**
 * An output task returns a reward based on the output the organism produced:
 * `OutputTask is42{ "IS42", [](uint32_t x) { return x == 42 ? 2.0 : 0.0; } };`
 */
class OutputTask : public Task {
  std::function<float(uint32_t)> task_fun;

public:
  OutputTask(std::string name, std::function<float(uint32_t)> task_fun,
             bool unlimited = true, emp::vector<size_t> dependencies = {},
             size_t num_dep_completes = 1)
      : Task(name, unlimited, dependencies, num_dep_completes),
        task_fun(task_fun) {}

  float CheckOutput(CPUState &state, uint32_t output) override {
    return task_fun(output);
  }
};

class SquareTask : public OutputTask {
public:
  std::map<uint32_t, uint32_t> hostCalculationTable;
  std::map<uint32_t, uint32_t> symCalculationTable;
  SquareTask(std::string name, std::function<float(uint32_t)> task_fun,
             bool unlimited = true, emp::vector<size_t> dependencies = {},
             size_t num_dep_completes = 1)
      : OutputTask(name, task_fun, unlimited, dependencies, num_dep_completes) {
  }

  void MarkPerformed(CPUState &state, uint32_t output, size_t task_id,
                     bool shared) override {
    OutputTask::MarkPerformed(state, output, task_id, shared);
    state.internalEnvironment->insert(state.internalEnvironment->begin(),
                                      sqrt(output));
    if(state.host->IsHost()){
      IncrementSquareMap(output, true);
    }
    else{
      IncrementSquareMap(output, false);
    }
  }

   /**
   * Input: The value of the current output and a boolean indicating whether the organism
   is a host or symbiont
   *
   * Output: None
   *
   * Purpose: Adds the output to the SquareMap with a count of 1 if not already included; 
   * otherwise, increments the count for that output
   */
  void IncrementSquareMap(uint32_t output, bool isHost){
      std::map<uint32_t, uint32_t>& calculationMap = isHost ? hostCalculationTable : symCalculationTable;
      if (calculationMap.empty()){//Base case for when the map is empty
        calculationMap.insert(std::pair<uint32_t, uint32_t>(output, 1));
      }else{
        std::map<uint32_t, uint32_t>::iterator placemark;
        placemark = calculationMap.find(output);
        if(placemark == calculationMap.end()){//If output does not exist in the map, add it with a count of 1
          calculationMap.insert(std::pair<uint32_t, uint32_t>(output, 1));
        }else{//If output does exist in the map, increment its count
          placemark->second++;
        }
    }
  }
};

class TaskSet {
  emp::vector<emp::Ptr<Task>> tasks;
  // vector<atomic<>> doesn't work since the vector needs to copy its elements
  // on resize and atomic isn't copiable, so we need pointers
  emp::vector<emp::Ptr<std::atomic<size_t>>> n_succeeds_host;
  emp::vector<emp::Ptr<std::atomic<size_t>>> n_succeeds_sym;

  float MarkPerformedTask(CPUState &state, uint32_t output, size_t task_id,
                          bool shared, float score) {
    score = state.world.Cast<SymWorld>()->PullResources(score);
    if (score == 0.0) {
      return score;
    }

    tasks[task_id]->MarkPerformed(state, output, task_id, shared);

    if (state.host->IsHost())
      ++*n_succeeds_host[task_id];
    else
      ++*n_succeeds_sym[task_id];

    return score;
  }

public:
  /**
   * Construct a TaskSet from a list of tasks
   */
  TaskSet(std::initializer_list<emp::Ptr<Task>> tasks) : tasks(tasks) {
    for (size_t i = 0; i < tasks.size(); i++) {
      n_succeeds_host.push_back(emp::NewPtr<std::atomic<size_t>>(0));
      n_succeeds_sym.push_back(emp::NewPtr<std::atomic<size_t>>(0));
    }
  }


  /**
   * A custom copy constructor so that task completions aren't shared between
   * TaskSets, which would be a problem for tests
   */
  TaskSet(const TaskSet &other) : tasks(other.tasks) {
    for (size_t i = 0; i < tasks.size(); i++) {
      n_succeeds_host.push_back(emp::NewPtr<std::atomic<size_t>>(0));
      n_succeeds_sym.push_back(emp::NewPtr<std::atomic<size_t>>(0));
    }
  }

  /**
   * Input: The current CPU state, the output to check against, and whether to
   * update shared or private completed pools for dependent tasks.
   *
   * Output: The score that the organism earned from any completed tasks, or 0
   * if no tasks were completed.
   *
   * Purpose: Checks whether a certain output produced by the organism completes
   * any tasks, and updates necessary state fields if so.
   */
  float CheckTasks(CPUState &state, uint32_t output, bool shared) {
    // Check output tasks
    // Special case so they can't cheat at e.g. NOR (0110, 1011 --> 0)
    if (output == 0 || output == 1) {
      return 0.0;
    }
    for (size_t i = 0; i < tasks.size(); i++) {
      if (tasks[i]->CanPerform(state, i)) {
        float score = tasks[i]->CheckOutput(state, output);
        if (score > 0.0) {
          score = MarkPerformedTask(state, output, i, shared, score);
          return score;
        }
      }
    }
    return 0.0f;
  }

  size_t NumTasks() const { return tasks.size(); }

  // Provide access to data about task completion with an iterator
  struct TaskData {
    const Task &task;
    size_t n_succeeds_host;
    size_t n_succeeds_sym;
  };

  struct Iterator {
    const TaskSet &task_set;
    size_t index;

    Iterator &operator++() {
      index++;
      return *this;
    }

    bool operator!=(const Iterator &other) { return index != other.index; }

    TaskData operator*() const {
      return TaskData{*task_set.tasks[index], *task_set.n_succeeds_host[index],
                      *task_set.n_succeeds_sym[index]};
    }
  };

  Iterator begin() const { return Iterator{*this, 0}; }

  Iterator end() const { return Iterator{*this, tasks.size()}; }

  void ResetTaskData() {
    for (size_t i = 0; i < tasks.size(); i++) {
      n_succeeds_host[i]->store(0);
      n_succeeds_sym[i]->store(0);
    }
  }
  /**
   * Input: A boolean identifying a given organism as a host or a symbiont
   *
   * Output: The map of square frequencies that matches the organism's type
   *
   * Purpose: Returns either hostCalculationTable or symCalculationTable, depending
   on whether the organism is a host or a symbiont. 
   */
  std::map<uint32_t, uint32_t> GetSquareFrequencyData(bool isHost){
    std::map<uint32_t, uint32_t> myMap;
    emp::Ptr<Task> curTask = tasks[0];
    emp::Ptr<SquareTask> squareTask = curTask.DynamicCast<SquareTask>();
    if (isHost){
      myMap = squareTask->hostCalculationTable;
    }
    else {
      myMap = squareTask->symCalculationTable;
    }
    return myMap;
  }
  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Empties the host and symbiont calculation tables 
   */
  void ClearSquareFrequencyData(bool IsHost){
      emp::Ptr<Task> curTask = tasks[0];
      emp::Ptr<SquareTask> squareTask = curTask.DynamicCast<SquareTask>();
      if (IsHost){
          squareTask->hostCalculationTable.clear();
      }
      else{
          squareTask->symCalculationTable.clear();
      }
      
  }
};




// The 9 default logic tasks in Avida
// These are checked top-to-bottom and the reward is given for the first one
// that matches
const InputTask
    NOT = {"NOT", 1, 5.0, [](auto &x) { return ~x[0]; }, false},
    NAND = {"NAND", 2, 5.0, [](auto &x) { return ~(x[0] & x[1]); }, false},
    AND = {"AND", 2, 40.0, [](auto &x) { return x[0] & x[1]; }, true, {0, 1}},
    ORN = {"ORN", 2, 40.0, [](auto &x) { return x[0] | ~x[1]; }, true, {0, 1}},
    OR = {"OR", 2, 80.0, [](auto &x) { return x[0] | x[1]; }, true, {0, 1}},
    ANDN = {"ANDN", 2,        80.0, [](auto &x) { return x[0] & ~x[1]; },
            true,   {2, 3, 4}},
    NOR = {"NOR", 2,        160.0, [](auto &x) { return ~(x[0] | x[1]); },
           true,  {2, 3, 4}},
    XOR = {"XOR", 2,        160.0, [](auto &x) { return x[0] ^ x[1]; },
           true,  {2, 3, 4}},
    EQU = {"EQU", 2,        320.0, [](auto &x) { return ~(x[0] ^ x[1]); },
           true,  {5, 6, 7}};
const TaskSet LogicTasks{
    emp::NewPtr<InputTask>(NOT), emp::NewPtr<InputTask>(NAND),
    emp::NewPtr<InputTask>(AND), emp::NewPtr<InputTask>(ORN),
    emp::NewPtr<InputTask>(OR),  emp::NewPtr<InputTask>(ANDN),
    emp::NewPtr<InputTask>(NOR), emp::NewPtr<InputTask>(XOR),
    emp::NewPtr<InputTask>(EQU)};

  const SquareTask SQU = {"SQU", [](uint32_t x) {
                    uint32_t largest_int = 4294967295;
                    if (sqrt(x) - floor(sqrt(x)) == 0){
                      if (x > (largest_int/2)){//Awards points based on a number's distance from 0 rather than absolute size
                        return 0.5 * (0 - x);
                      }else{
                        return (0.5 * x);
                      }
                    }else{
                      return 0.0;
                    }
                    return 0.0;
                  }};
const TaskSet SquareTasks{emp::NewPtr<SquareTask>(SQU)};

#endif