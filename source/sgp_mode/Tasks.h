#ifndef TASKS_H
#define TASKS_H

#include "../default_mode/SymWorld.h"
#include "CPUState.h"
#include <atomic>
#include <variant>

/**
 * An input task computes an expected output based on the inputs, and if the
 * organism's output matches, it gives it a certain reward:
 * `InputTask sum{ 2, [](auto &x) { return x[0] + x[1]; }, 1.0 };`
 */
struct InputTask {
  size_t n_inputs;
  std::function<uint32_t(emp::vector<uint32_t> &)> taskFun;
  float value;
};

/**
 * An output task returns a reward based on the output the organism produced:
 * `OutputTask is42{ [](uint32_t x) { return x == 42 ? 2.0 : 0.0; } };`
 */
struct OutputTask {
  std::function<float(uint32_t)> taskFun;
};

struct Task {
  std::string name;
  std::variant<InputTask, OutputTask> kind;
  bool unlimited = true;
  emp::vector<size_t> dependencies;
  /// The total number of times this task's dependencies must be completed for
  /// each use of this task
  size_t num_dep_completes = 1;
};

class TaskSet {
  emp::vector<Task> tasks;
  // vector<atomic<>> doesn't work since the vector needs to copy its elements
  // on resize and atomic isn't copiable, so we need pointers
  //&& !task.unlimited
  emp::vector<emp::Ptr<std::atomic<size_t>>> n_succeeds_host;
  emp::vector<emp::Ptr<std::atomic<size_t>>> n_succeeds_sym;

  bool CanPerformTask(const CPUState &state, size_t task_id) {
    Task &task = tasks[task_id];

    if (state.used_resources->Get(task_id) && !task.unlimited) {
      return false;
    }
    if (task.dependencies.size()) {
      size_t num_dep_completes = std::reduce(
          task.dependencies.begin(), task.dependencies.end(), 0,
          [&](auto acc, auto i) {
            return acc + state.self_completed[i] + (*state.shared_completed)[i];
          });
      if (num_dep_completes < task.num_dep_completes) {
        return false;
      }
    }
    return true;
  }

  float MarkPerformedTask(CPUState &state, size_t task_id, bool shared,
                          float score) {
    score = state.world.Cast<SymWorld>()->PullResources(score);
    if (score == 0.0) {
      return score;
    }

    Task &task = tasks[task_id];
    state.used_resources->Set(task_id);

    if (task.dependencies.size()) {
      // TODO does it make sense to reset to 0, or to let them accumulate
      // resources?
      size_t total = task.num_dep_completes;
      for (size_t i : task.dependencies) {
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
  TaskSet(std::initializer_list<Task> tasks) : tasks(tasks) {
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
      Task &task = tasks[i];
      if (std::holds_alternative<OutputTask>(task.kind) &&
          CanPerformTask(state, i)) {
        float score = std::get<OutputTask>(task.kind).taskFun(output);
        if (score > 0.0) {
          score = MarkPerformedTask(state, i, shared, score);
          state.internalEnvironment->insert(state.internalEnvironment->begin(),
                                            sqrt(output));
          return score;
        }
      }
    }
    // Check input tasks
    emp::vector<uint32_t> inputs;
    for (size_t i = 0; i < state.input_buf.size(); i++) {
      if (state.input_buf[i] == 0)
        continue;

      inputs = {state.input_buf[i], state.input_buf[i + 1]};
      for (size_t i = 0; i < tasks.size(); i++) {
        Task &task = tasks[i];
        if (std::holds_alternative<InputTask>(task.kind) &&
            CanPerformTask(state, i)) {
          InputTask &itask = std::get<InputTask>(task.kind);
          if (itask.n_inputs > 1 && inputs[1] == 0)
            continue;

          if (itask.taskFun(inputs) == output) {
            float score = MarkPerformedTask(state, i, shared, itask.value);
            return score;
          }
        }
      }
    }
    return 0.0f;
  }

  size_t NumTasks() { return tasks.size(); }

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
      return TaskData{task_set.tasks[index], *task_set.n_succeeds_host[index],
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
};

// The 9 default logic tasks in Avida
// These are checked top-to-bottom and the reward is given for the first one
// that matches
const Task NOT = {"NOT", InputTask{1, [](auto &x) { return ~x[0]; }, 5.0},
                  false},
           NAND = {"NAND",
                   InputTask{2, [](auto &x) { return ~(x[0] & x[1]); }, 5.0},
                   false},
           AND = {"AND",
                  InputTask{2, [](auto &x) { return x[0] & x[1]; }, 40.0},
                  true,
                  {0, 1}},
           ORN = {"ORN",
                  InputTask{2, [](auto &x) { return x[0] | ~x[1]; }, 40.0},
                  true,
                  {0, 1}},
           OR = {"OR",
                 InputTask{2, [](auto &x) { return x[0] | x[1]; }, 80.0},
                 true,
                 {0, 1}},
           ANDN = {"ANDN",
                   InputTask{2, [](auto &x) { return x[0] & ~x[1]; }, 80.0},
                   true,
                   {2, 3, 4}},
           NOR = {"NOR",
                  InputTask{2, [](auto &x) { return ~(x[0] | x[1]); }, 160.0},
                  true,
                  {2, 3, 4}},
           XOR = {"XOR",
                  InputTask{2, [](auto &x) { return x[0] ^ x[1]; }, 160.0},
                  true,
                  {2, 3, 4}},
           EQU = {"EQU",
                  InputTask{2, [](auto &x) { return ~(x[0] ^ x[1]); }, 320.0},
                  true,
                  {5, 6, 7}};
TaskSet LogicTasks{NOT, NAND, AND, ORN, OR, ANDN, NOR, XOR, EQU};

const Task SQU = {"SQU", OutputTask{[](uint32_t x) {
                    return sqrt(x) - floor(sqrt(x)) == 0 ? 40.0 : 0.0;
                  }}};
TaskSet SquareTasks{SQU};

#endif