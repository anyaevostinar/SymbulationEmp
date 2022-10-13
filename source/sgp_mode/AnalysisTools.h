#ifndef ANALYSIS_TOOLS_H
#define ANALYSIS_TOOLS_H

#include "CPU.h"

/**
 *
 * Input: A cpu of an organism
 *
 * Output: A length 64 emp bitset which describes the phenotype of organism
 * such that the ith 1 in the bitset marks the completion of task i.
 *
 * Purpose: Get the phenotype of an organism
 *
 */
emp::BitSet<64> ReturnTasksDone(CPU org_cpu) {
  org_cpu.Reset();
  org_cpu.state.self_completed = {1, 1, 1, 1, 1, 1, 1, 1, 1};
  // Turn off limited resources for this method
  int old_lim_res = org_cpu.state.world->GetConfig()->LIMITED_RES_TOTAL();
  org_cpu.state.world->GetConfig()->LIMITED_RES_TOTAL(-1);

  org_cpu.RunCPUStep(emp::WorldPosition::invalid_id, 400);

  // and then reset it to the previous value
  org_cpu.state.world->GetConfig()->LIMITED_RES_TOTAL(old_lim_res);
  return *org_cpu.state.used_resources;
}

/*
 *
 * Input: Takes in a cpu and the identifier for a specific task
 *
 * Output: a boolean representing a program's ability to do a logic task
 *
 * Purpose: To return whether or not the organism can perform the given task
 *
 *
 */
bool ReturnTaskDone(size_t task_id, CPU org_cpu) {
  return ReturnTasksDone(org_cpu).Get(task_id);
}

/**
 *
 * Input: Takes in a CPU and the number identifying a given task
 *
 * Output: Returns a vector representing the full genome, reduced to 1s and 0s
 * to show either that an instruction is necessary to complete the task, or not
 * respectively
 *
 * Purpose: Is to return a vector that acts as a reduced program representation
 * of the necessary code lines to complete the given task
 *
 */
std::optional<emp::BitArray<100>>
GetNecessaryInstructions(CPU org_cpu, size_t test_task_id) {
  emp::Random random(-1);
  sgpl::Program<Spec> const control_program = org_cpu.GetProgram();
  sgpl::Program<Spec> test_program = control_program;

  emp::BitArray<100> reduced_position_guide;

  bool can_do_task = ReturnTaskDone(test_task_id, org_cpu);

  // catches if a task cannot be done ever
  if (!can_do_task) {
    return std::nullopt;
  } else {
    for (size_t k = 0; k <= control_program.size() - 1; k++) {

      test_program[k].op_code = 0;
      CPU temp_cpu = CPU(org_cpu.state.host, org_cpu.state.world, test_program);

      can_do_task = ReturnTaskDone(test_task_id, temp_cpu);

      if (!can_do_task) {
        reduced_position_guide.Set(k, 1);
      } else {
        reduced_position_guide.Set(k, 0);
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
 * Output: Returns a vector with a reduced program representation taken from the
 * organism for each task in the world's taskset
 *
 * Purpose: To cycle through all the tasks in the world's taskset
 * and return the necessary code sites within the original program
 * to complete each task. If the CPU does not have the necessary code,
 * then a (-1) is returned in the first and only position of the reduced program
 * representation.
 *
 */
emp::vector<std::optional<emp::BitArray<100>>>
GetReducedProgramRepresentations(CPU org_cpu) {
  const TaskSet &all_tasks = org_cpu.state.world->GetTaskSet();
  emp::vector<std::optional<emp::BitArray<100>>> map_of_guides;

  for (size_t j = 0; j < all_tasks.NumTasks(); ++j) {
    map_of_guides.push_back(GetNecessaryInstructions(org_cpu, j));
  }

  return map_of_guides;
}

#endif
