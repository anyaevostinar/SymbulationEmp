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

using Library = sgpl::OpLibrary<
    sgpl::Nop<>,
    // single argument math
    inst::ShiftLeft, inst::ShiftRight, inst::Increment, inst::Decrement,
    // biological operations
    // no copy or alloc
    inst::Reproduce, inst::PrivateIO, inst::SharedIO,
    // double argument math
    inst::Add, inst::Subtract, inst::Nand,
    // Stack manipulation
    inst::Push, inst::Pop, inst::SwapStack, inst::Swap,
    // no h-search
    inst::Donate, inst::JumpIfNEq, inst::JumpIfLess, inst::Reuptake,
    // if-label doesn't make sense for SGP, same with *-head
    // and set-flow but this is required
    sgpl::global::Anchor>;

using Spec = sgpl::Spec<Library, CPUState>;

/*
 *
 * Input: Takes in a cpu and the identifier for a specific task
 *
 * Output: a boolean representing a program's ability to do a logic task
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
 * Input:
 *
 * Output:
 *
 *Purpose: Takes all the calculation methods and calls them in order of having a
 *simplified way of getting an organism's Physical Modularity
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

    for (int k = 0; k <= control_program.size() - 1; k++) {

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
 * Input:
 *
 * Output:
 *
 *Purpose: Takes all the calculation methods and calls them in order of having a
 *simplified way of getting an organism's Physical Modularity
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
