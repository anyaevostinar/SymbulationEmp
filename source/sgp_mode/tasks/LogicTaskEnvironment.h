#pragma once

#include "LogicTaskSet.h"

#include "emp/base/vector.hpp"

#include <functional>

/*
  Implements logic task environment.
    - Responsible for configuring LogicTaskSet, etc.
    - Responsible for managing any task dependencies, etc.
*/

namespace sgpmode::tasks {

class LogicTaskEnvironment {
public:
  // using fun_task_value

  // Used to track information about task performance restrictions/requirements.
  struct TaskReqInfo {
    size_t allowed_repeats = 0;       // How many times are organisms allowed to repeat this task for credit?
    double task_value = 0;            // Base value for this task
    bool has_dependencies = false;    // Does credit for this task depend on performing other tasks prior?
    emp::vector<size_t> dependencies; // Which tasks does this task depend on?
  };

protected:
  LogicTaskSet task_set;
  emp::vector<TaskReqInfo> task_req_info;
  // TODO - track task performance?

public:
  // LogicTaskEnvironment

};

}