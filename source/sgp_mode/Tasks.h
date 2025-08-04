#ifndef TASKS_H
#define TASKS_H

#include "../default_mode/SymWorld.h"
#include "CPUState.h"
#include <atomic>
#include <string>
#include <iostream>


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

  /**
   * Input: The current CPU state, the output to check against
   *
   * Output: Boolean for if this task was solved
   *
   * Purpose: Checks whether the organism has completed this task.
   */
  bool IsSolved(CPUState &state, uint32_t output) {
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
        return true;
      }
    }
    return false;
  }
};

class TaskSet {
  emp::vector<emp::Ptr<Task>> tasks;
  // vector<atomic<>> doesn't work since the vector needs to copy its elements
  // on resize and atomic isn't copiable, so we need pointers
  emp::vector<emp::Ptr<std::atomic<size_t>>> n_succeeds_host;
  emp::vector<emp::Ptr<std::atomic<size_t>>> n_succeeds_sym;

  /**
   * Input: The current CPU state, the id of the completed task
   *
   * Output: None
   *
   * Purpose: Marks the organism as having completed the task and increases this task's data counter by 1 for its
   * organism type. 
   */
  void MarkPerformedTask(CPUState &state, size_t task_id) {
    state.tasks_performed->Set(task_id);
   
    if (state.organism->IsHost())
      ++*n_succeeds_host[task_id];
    else{
      ++*n_succeeds_sym[task_id];
    }

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
   * Input: The current CPU state, the output to check against, whehter ONLY_FIRST_TASK_CREDIT is on
   *
   * Output: None
   *
   * Purpose: Checks whether any tasks were completed and if so marks them as completing for reproductive purposes
   * and awards the proper amount of points.  
   */
  void ProcessOutput(CPUState &state, uint32_t output, bool is_only_task_credit){
    int current_task = CheckTasks(state, output, is_only_task_credit);
    if(current_task == -1){
      return;
    }
    MarkPerformedTask(state, current_task);
    int score = tasks[current_task]->value;
    if(!state.organism->IsHost()) {
      //currently only symbionts have special interactions on tasks, such as nutrient mode
      score = state.organism->DoTaskInteraction(score, current_task);
    } 
    else if (state.organism->IsHost()){
      score = state.world.Cast<SymWorld>()->PullResources(score);
    }
    //std::cout << "Added Points " << score << std::end;
    state.organism->AddPoints(score);
  }

  /**
   * Input: The current CPU state, the output to check against, whehter ONLY_FIRST_TASK_CREDIT is on
   *
   * Output: the id of the completed task, if one was completed.
   *
   * Purpose: Checks whether a certain output produced by the organism completes
   * any tasks, and updates necessary state fields if so.
   */
  int CheckTasks(CPUState &state, uint32_t output, bool is_only_task_credit) {
    // Check output tasks
    // Special case so they can't cheat at e.g. NOR (0110, 1011 --> 0)
    int current_task = -1;
    if (output == 0 || output == 1) {
      return current_task;
    }

    for (size_t i = 0; i < tasks.size(); i++) {
      bool is_completed = tasks[i]->IsSolved(state, output);
      if (is_completed) {
        
        current_task = i;
        break;
      }
    }
    if(current_task != -1 && is_only_task_credit){
      if(!IsOnlyTask(state, current_task)){
        current_task = -1;
      }
    }
    return current_task;
  }

  /**
   * Input: The current CPU state, the id of the completed task
   *
   * Output: Whether this task was the first type of task performed for this organism
   *
   * Purpose: Checks whether the organism has completed any tasks that were not this one. 
   * If so then if ONLY_FIRST_TASK_CREDIT is on they should not receive points. 
   */
  bool IsOnlyTask(CPUState &state, int current_task){
    bool no_other_tasks = true;
    for (size_t i = 0; i < tasks.size(); i++) {
      if(state.tasks_performed->Get(i) == 1 && i != current_task){
        no_other_tasks = false;
        break;
      }
    }
    return no_other_tasks;
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