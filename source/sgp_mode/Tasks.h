#ifndef TASKS_H
#define TASKS_H

#include "../default_mode/SymWorld.h"
#include "CPUState.h"
#include <atomic>
#include <string>


/**
 * A task computes an expected output based on the inputs, and if the
 * organism's output matches, it gives it a certain reward:
 * `Task sum( "SUM", 2, 1.0, [](auto &x) { return x[0] + x[1]; } );`
 */
class Task {

public:
  std::string name;
  size_t n_inputs;
  std::function<uint32_t(emp::vector<uint32_t> &)> task_fun;
  float value;

  Task(std::string name, size_t n_inputs, float value,
            std::function<uint32_t(emp::vector<uint32_t> &)> task_fun)
      : name(name),
        n_inputs(n_inputs), task_fun(task_fun), value(value) {}
  
  ~Task(){}

  void MarkPerformed(CPUState &state, uint32_t output, size_t task_id);

  float CheckOutput(CPUState &state, uint32_t output) {
    for (size_t i = 0; i < state.input_buf.size(); i++) {

      
      if (state.input_buf[i] == 0)
        continue;

      emp::vector<uint32_t> inputs{state.input_buf[i], state.input_buf[i + 1]};
      
      //Check to make sure the answer is not equivalent to a input, stops easy points. 
      if(inputs[0] == output || inputs[1] == output){
        continue;
      }
      if (n_inputs > 1 && inputs[1] == 0)
        continue;

      if (task_fun(inputs) == output) {
        return value;
      }
    }
    return 0;
  }
};

class TaskSet {
  emp::vector<emp::Ptr<Task>> tasks;
  // vector<atomic<>> doesn't work since the vector needs to copy its elements
  // on resize and atomic isn't copiable, so we need pointers
  emp::vector<emp::Ptr<std::atomic<size_t>>> n_succeeds_host;
  emp::vector<emp::Ptr<std::atomic<size_t>>> n_succeeds_sym;

  float MarkPerformedTask(CPUState &state, uint32_t output, size_t task_id, float score) {
    if(!state.organism->IsHost()) {
      //currently only symbionts have special interactions on tasks, such as nutrient mode
      score = state.organism->DoTaskInteraction(score, task_id);
    } else if (state.organism->IsHost()){
      score = state.world.Cast<SymWorld>()->PullResources(score);
    }
    if (score == 0.0) {
      return score;
    }

    tasks[task_id]->MarkPerformed(state, output, task_id);

    if (state.organism->IsHost())
      ++*n_succeeds_host[task_id];
    else{
      ++*n_succeeds_sym[task_id];
    }
      

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
  TaskSet(const TaskSet &other){
    tasks = emp::vector<emp::Ptr<Task>>();
    for (size_t i = 0; i < other.tasks.size(); i++) {
      tasks.push_back(emp::NewPtr<Task>(*other.tasks[i].DynamicCast<Task>()));
      n_succeeds_host.push_back(emp::NewPtr<std::atomic<size_t>>(0));
      n_succeeds_sym.push_back(emp::NewPtr<std::atomic<size_t>>(0));
    }
  }

  /**
  * A destructor to clean up the task pointers
  */
  ~TaskSet() {
    for (size_t i = 0; i < tasks.size(); i++) {
      if (tasks[i]) tasks[i].Delete();
      if (n_succeeds_host[i]) n_succeeds_host[i].Delete();
      if (n_succeeds_sym[i]) n_succeeds_sym[i].Delete();
    }
  }

  /**
   * Input: The current CPU state, the output to check against
   *
   * Output: The score that the organism earned from any completed tasks, or 0
   * if no tasks were completed.
   *
   * Purpose: Checks whether a certain output produced by the organism completes
   * any tasks, and updates necessary state fields if so.
   */
  float CheckTasks(CPUState &state, uint32_t output) {
    // Check output tasks
    // Special case so they can't cheat at e.g. NOR (0110, 1011 --> 0)
    if (output == 0 || output == 1) {
      return 0.0;
    }

    for (size_t i = 0; i < tasks.size(); i++) {
      float score = tasks[i]->CheckOutput(state, output);
      if (score > 0.0) {
        score = MarkPerformedTask(state, output, i, score);
        return score;
      }
    }
    return 0.0f;
  }

  size_t NumTasks() const { return tasks.size(); }
  size_t size() const { return tasks.size(); }

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
};

// The 9 default logic tasks in Avida
// These are checked top-to-bottom and the reward is given for the first one
// that matches
const Task
    NOT = {"NOT", 1, 5.0, [](auto &x) { return ~x[0]; }},
    NAND = {"NAND", 2, 5.0, [](auto &x) { return ~(x[0] & x[1]); }},
    AND = {"AND", 2, 5.0, [](auto &x) { return x[0] & x[1]; }},
    ORN = {"ORN", 2, 5.0, [](auto &x) { return x[0] | ~x[1]; }},
    OR = {"OR", 2, 5.0, [](auto &x) { return x[0] | x[1]; }},
    ANDN = {"ANDN", 2,        5.0, [](auto &x) { return x[0] & ~x[1]; }},
    NOR = {"NOR", 2,        5.0, [](auto &x) { return ~(x[0] | x[1]); }},
    XOR = {"XOR", 2,        5.0, [](auto &x) { return x[0] ^ x[1]; }},
    EQU = {"EQU", 2,        5.0, [](auto &x) { return ~(x[0] ^ x[1]); }};
const TaskSet LogicTasks{
    emp::NewPtr<Task>(NOT), emp::NewPtr<Task>(NAND),
    emp::NewPtr<Task>(AND), emp::NewPtr<Task>(ORN),
    emp::NewPtr<Task>(OR),  emp::NewPtr<Task>(ANDN),
    emp::NewPtr<Task>(NOR), emp::NewPtr<Task>(XOR),
    emp::NewPtr<Task>(EQU)};

#endif