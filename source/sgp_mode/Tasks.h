#ifndef TASKS_H
#define TASKS_H

#include "../default_mode/SymWorld.h"
#include "CPUState.h"
#include <atomic>
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
  
  virtual ~Task(){}

  virtual void MarkAlwaysPerformable() {
    dependencies.clear();
    unlimited = true;
  }

  virtual bool CanPerform(const CPUState &state, size_t task_id) {
    //only let organisms do one task during lifetime
    // for(int i=0; i<state.tasks_performed->size(); i++){
    //   if(i==task_id) continue; //skip this task
    //   if(state.tasks_performed->Get(i)) return false; //if any other task has been performed, can't do this one
    // }
    // if (!state.organism->IsHost()){
    //   //if this is a symbiont 
    //   if (state.used_resources->Get(task_id)) {
    //     //and the task has been performed (therefore must be by host)
    //     //symbiont gets the points from the host
    //     return true;
    //   } else {
    //     //symbionts can't get credit for task if host hasn't done it first
    //     return false;
    //   }
    // }
    
    // if (state.used_resources->Get(task_id) && !unlimited) {
    //   return false;
    // } 

    // if (dependencies.size()) {
    //   size_t actually_completed = std::reduce(
    //       dependencies.begin(), dependencies.end(), 0, [&](auto acc, auto i) {
    //         return acc + state.available_dependencies[i] +
    //                (*state.shared_available_dependencies)[i];
    //       });
    //   if (actually_completed < num_dep_completes) {
    //     return false;
    //   }
    // }
    return true;
  }

  void MarkPerformed(CPUState &state, uint32_t output, size_t task_id,
                             bool shared);

  virtual float CheckOutput(CPUState &state, uint32_t output) = 0;
};

/**
 * An input task computes an expected output based on the inputs, and if the
 * organism's output matches, it gives it a certain reward:
 * `InputTask sum( "SUM", 2, 1.0, [](auto &x) { return x[0] + x[1]; } );`
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
  
  ~InputTask(){}

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


class TaskSet {
  emp::vector<emp::Ptr<Task>> tasks;
  // vector<atomic<>> doesn't work since the vector needs to copy its elements
  // on resize and atomic isn't copiable, so we need pointers
  emp::vector<emp::Ptr<std::atomic<size_t>>> n_succeeds_host;
  emp::vector<emp::Ptr<std::atomic<size_t>>> n_succeeds_sym;

  float MarkPerformedTask(CPUState &state, uint32_t output, size_t task_id,
                          bool shared, float score) {
    if (state.tasks_performed->Get(task_id)){
      //Half points if they did the task before, pushing them to do more tasks instead of cycling
      score = score/2.0;
    }
    if (state.organism->IsHost()){
      score = state.world.Cast<SymWorld>()->PullResources(score);
    }
    if (score == 0.0) {
      return score;
    }

    tasks[task_id]->MarkPerformed(state, output, task_id, shared);

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
      tasks.push_back(emp::NewPtr<InputTask>(*other.tasks[i].DynamicCast<InputTask>()));
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
    //hacky flag for detecting parasite did a task but host hadn't done it
    bool sym_special = false;

    for (size_t i = 0; i < tasks.size(); i++) {
      if (tasks[i]->CanPerform(state, i)) {
        float score = tasks[i]->CheckOutput(state, output);
        if (score > 0.0) {
          score = MarkPerformedTask(state, output, i, shared, score);
          return score;
        }
      // } else if(!state.organism->IsHost()) {
      //   float score = tasks[i]->CheckOutput(state, output);
      //   if (score > 0.0) {
      //     //pity points for symbiont that did a task, but didn't match host
      //     sym_special = true;
      //     //also mark that task is performed in used_resources so offspring have chance of ending up in host that does the task (but don't use MarkPerformedTask since that messes up data collection)
      //     state.tasks_performed->Set(i);
      //   }
      }
    }
    if (sym_special){
      return 2.5;
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
};

// The 9 default logic tasks in Avida
// These are checked top-to-bottom and the reward is given for the first one
// that matches
const InputTask
    NOT = {"NOT", 1, 5.0, [](auto &x) { return ~x[0]; }, true},
    NAND = {"NAND", 2, 5.0, [](auto &x) { return ~(x[0] & x[1]); }, true},
    AND = {"AND", 2, 5.0, [](auto &x) { return x[0] & x[1]; }, true},
    ORN = {"ORN", 2, 5.0, [](auto &x) { return x[0] | ~x[1]; }, true},
    OR = {"OR", 2, 5.0, [](auto &x) { return x[0] | x[1]; }, true},
    ANDN = {"ANDN", 2,        5.0, [](auto &x) { return x[0] & ~x[1]; }, true},
    NOR = {"NOR", 2,        5.0, [](auto &x) { return ~(x[0] | x[1]); },
           true},
    XOR = {"XOR", 2,        5.0, [](auto &x) { return x[0] ^ x[1]; },
           true},
    EQU = {"EQU", 2,        5.0, [](auto &x) { return ~(x[0] ^ x[1]); },
           true};
const TaskSet LogicTasks{
    emp::NewPtr<InputTask>(NOT), emp::NewPtr<InputTask>(NAND),
    emp::NewPtr<InputTask>(AND), emp::NewPtr<InputTask>(ORN),
    emp::NewPtr<InputTask>(OR),  emp::NewPtr<InputTask>(ANDN),
    emp::NewPtr<InputTask>(NOR), emp::NewPtr<InputTask>(XOR),
    emp::NewPtr<InputTask>(EQU)};

#endif