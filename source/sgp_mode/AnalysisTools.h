#ifndef ANALYSIS_TOOLS_H
#define ANALYSIS_TOOLS_H

#include "../../Empirical/include/emp/Evolve/Systematics.hpp"
#include "../../Empirical/include/emp/Evolve/World.hpp"
#include "../../Empirical/include/emp/data/DataFile.hpp"
#include "../../Empirical/include/emp/math/Random.hpp"
#include "../../Empirical/include/emp/math/random_utils.hpp"
#include "../Organism.h"
#include "../default_mode/Host.h"
#include "CPU.h"
#include "CPUState.h"
#include "GenomeLibrary.h"
#include "Instructions.h"
#include "SGPHost.h"
#include "SGPWorld.h"
#include "Tasks.h"
#include "sgpl/algorithm/execute_cpu.hpp"
#include "sgpl/hardware/Cpu.hpp"
#include "sgpl/library/OpLibraryCoupler.hpp"
#include "sgpl/library/prefab/ArithmeticOpLibrary.hpp"
#include "sgpl/library/prefab/ControlFlowOpLibrary.hpp"
#include "sgpl/operations/flow_global/Anchor.hpp"
#include "sgpl/operations/unary/Increment.hpp"
#include "sgpl/operations/unary/Terminal.hpp"
#include "sgpl/program/Program.hpp"
#include "sgpl/spec/Spec.hpp"
#include "sgpl/utility/ThreadLocalRandom.hpp"
#include <iostream>
#include <math.h>
#include <set>

// start of getNecessarySites methods

/*
 *
 * Input: Takes in a cpu and the identifier for a specific task
 *
 * Output: Returns a boolean representing a program's ability to do a logic task
 *
 *Purpose: To return whether or not the organism can perform the given task
 *
 *
 */

bool ReturnTaskDone(size_t task_id, CPU org_cpu) {
  bool if_task_true = false;
  org_cpu.Reset();
  org_cpu.state.self_completed = {1, 1, 1, 1, 1, 1, 1, 1, 1};

  org_cpu.RunCPUStep(emp::WorldPosition::invalid_id, 100);

  if (org_cpu.state.used_resources->Get(task_id)) {
    if_task_true = true;
  }

  return if_task_true;
}

/**
 *
 * Input: Takes in a CPU and the number identifying a given task
 *
 * Output: Returns a vector representing the full genome, reduced to 1s and 0s to show either
 *that an instruction is necessary to complete the task, or not respectively
 *
 *Purpose: Is to return a vector that acts as a reduced program representation
 *of the necessary code lines to complete the given task
 *
 */
emp::vector<int> GetNecessaryInstructions(CPU org_cpu, size_t test_task_id) {
  emp::Random random(-1);
  sgpl::Program<Spec> const control_program = org_cpu.GetProgram();
  sgpl::Program<Spec> test_program = control_program;

  emp::vector<int> reduced_position_guide = {};

  bool can_do_task = ReturnTaskDone(test_task_id, org_cpu);

  // catches if a task cannot be done ever
  if (!can_do_task) {
    reduced_position_guide.push_back(-1);
    return reduced_position_guide;
  }

  if (can_do_task) {

    for (size_t k = 0; k <= control_program.size() - 1; k++) {

      test_program[k].op_code = 0;
      CPU temp_cpu = CPU(org_cpu.state.host, org_cpu.state.world, test_program);

      can_do_task = ReturnTaskDone(test_task_id, temp_cpu);

      if (!can_do_task) {
        reduced_position_guide.push_back(1);
      }

      else {
        reduced_position_guide.push_back(0);
      }

      test_program = control_program;
    }
  }

  return reduced_position_guide;
}

/**
 *
 * Input: Takes in an organism's CPU
 *
 * Output:Returns a vector with a reduced program representation taken from the organism
 * for each task in the world's taskset
 *
 *Purpose: To cycle through all the tasks in the world's taskset
 *and return the necessary code sites within the original program 
 *to complete each task. If the CPU does not have the necessary code,
 * then a (-1) is returned in the first and only position of the reduced program representation.
 *
 */
emp::vector<emp::vector<int>> GetReducedProgramRepresentations(CPU org_cpu) {
  CPUState condition = org_cpu.state;
  TaskSet all_tasks = condition.world->GetTaskSet();
  sgpl::Program<Spec> original_program = org_cpu.GetProgram();
  emp::vector<emp::vector<int>> map_of_guides;

  for (size_t j = 0; j < all_tasks.NumTasks(); ++j) {
    emp::vector<int> position_guide = {};
    position_guide = GetNecessaryInstructions(org_cpu, j);
    map_of_guides.push_back(position_guide);
  }

  return map_of_guides;
}

#endif
