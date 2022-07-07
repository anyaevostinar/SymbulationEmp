#ifndef TASKS_H
#define TASKS_H

#include "CPUState.h"
#include <atomic>
#include <variant>

// An input task computes an expected output based on the inputs, and if the
// organism's output matches, it gives it a certain reward:
// `InputTask sum{ 2, [](auto &x) { return x[0] + x[1]; }, 1.0 };`
struct InputTask {
  size_t n_inputs;
  std::function<uint32_t(emp::vector<uint32_t> &)> taskFun;
  float value;
};

// An output task returns a reward based on the output the organism produced:
// OutputTask is42{ [](uint32_t x) { return x == 42 ? 2.0 : 0.0; } };`
struct OutputTask {
  std::function<float(uint32_t)> taskFun;
};

struct Task {
  std::string name;
  std::variant<InputTask, OutputTask> kind;
  bool unlimited = true;
};
class TaskSet {
  emp::vector<Task> tasks;
  // vector<atomic<>> doesn't work since the vector needs to copy its elements
  // on resize and atomic isn't copiable, so we need pointers
  emp::vector<emp::Ptr<std::atomic<size_t>>> n_succeeds_host;
  emp::vector<emp::Ptr<std::atomic<size_t>>> n_succeeds_sym;

public:
  TaskSet(std::initializer_list<Task> tasks) : tasks(tasks) {
    for (size_t i = 0; i < tasks.size(); i++) {
      n_succeeds_host.push_back(emp::NewPtr<std::atomic<size_t>>(0));
      n_succeeds_sym.push_back(emp::NewPtr<std::atomic<size_t>>(0));
    }
  }

  float CheckTasks(CPUState &state, uint32_t output) {
    // Special case so they can't cheat at e.g. NOR (0110, 1011 --> 0)
    if (output == 0 || output == 1) {
      return 0.0;
    }
    // Check output tasks
    for (size_t i = 0; i < tasks.size(); i++) {
      Task &task = tasks[i];
      if (std::holds_alternative<OutputTask>(task.kind) &&
          !state.used_resources->Get(i)) {
        state.used_resources->Set(i, !task.unlimited);
        float score = std::get<OutputTask>(task.kind).taskFun(output);
        if (score > 0.0) {
          if (state.host->IsHost())
            ++*n_succeeds_host[i];
          else
            ++*n_succeeds_sym[i];
          state.internalEnvironment->insert(state.internalEnvironment->begin(), sqrt(output));
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
            !state.used_resources->Get(i)) {
          InputTask &itask = std::get<InputTask>(task.kind);
          if (itask.n_inputs > 1 && inputs[1] == 0)
            continue;

          if (itask.taskFun(inputs) == output) {
            state.used_resources->Set(i, !task.unlimited);
            if (state.host->IsHost())
              ++*n_succeeds_host[i];
            else
              ++*n_succeeds_sym[i];
            return itask.value;
          }
        }
      }
    }
    return 0.0f;
  }

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
TaskSet DefaultTasks{
    {"NOT", InputTask{1, [](auto &x) { return ~x[0]; }, 1.0}, false},
    {"NAND", InputTask{2, [](auto &x) { return ~(x[0] & x[1]); }, 1.0}, false},
    {"AND", InputTask{2, [](auto &x) { return x[0] & x[1]; }, 2.0}, false},
    {"ORN", InputTask{2, [](auto &x) { return x[0] | ~x[1]; }, 2.0}, false},
    {"OR", InputTask{2, [](auto &x) { return x[0] | x[1]; }, 3.0}, false},
    {"ANDN", InputTask{2, [](auto &x) { return x[0] & ~x[1]; }, 3.0}, false},
    {"NOR", InputTask{2, [](auto &x) { return ~(x[0] | x[1]); }, 4.0}, false},
    {"XOR", InputTask{2, [](auto &x) { return x[0] ^ x[1]; }, 4.0}, false},
    {"EQU", InputTask{2, [](auto &x) { return ~(x[0] ^ x[1]); }, 5.0}, false},
    {"SQU", OutputTask{[](uint32_t x) { return sqrt(x) - floor(sqrt(x)) == 0 ? 1.0 : 0.0; } }}};

#endif