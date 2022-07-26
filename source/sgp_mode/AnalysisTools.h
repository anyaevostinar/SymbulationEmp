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
#include <math.h>
#include <set>

// start of getUsefulGenomes methods

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
 * Input: Takes in a host
 *
 * Output: Outputs a vector of int vectors that represents all the necessary
 *instructions to do a task for each task the host can do
 *
 *Purpose: To get modified versions of the host's genome to give to top level
 *modularity methods
 *
 */

bool ReturnTaskDone(TaskSet task_list, size_t task_id,CPU host_cpu){
    bool if_task_true = false;

    host_cpu.RunCPUStep(emp::WorldPosition::invalid_id, 100);
    
    for (TaskSet::Iterator one_task = task_list.begin(); one_task!=task_list.end(); ++one_task) {

        std::cout<<" Rascal";
        
        if (one_task.index==task_id){
           
            std::cout<<" Lemur ";

            if(host_cpu.state.used_resources->Get(task_id)){
              if_task_true = true;
              std::cout<<" Yes";
            }
            
        }
    }
    host_cpu.state.used_resources->reset();
    return if_task_true;
}

emp::vector<int> GetNecessaryInstructions(SGPHost *sample_host,
                                          size_t test_task_id,
                                          TaskSet task_passer) {

  sgpl::Program<Spec> control_program = sample_host->GetCPU().GetProgram();

  emp::vector<int> reduced_position_guide = {};
  std::cout<<"HAPPYSUCCESS ";

    //whatever task it is on it should turn out to be true
  sample_host->GetCPU().RunCPUStep(emp::WorldPosition::invalid_id, 100);
  bool can_do_task = ReturnTaskDone(task_passer, test_task_id,sample_host->GetCPU());
  std::cout<<"  HALLLEELUYAH   ";
  

  if (can_do_task) {
    std::cout<<"GREatVentur ";

    for (int k = 0; k <= control_program.size() - 1; k++) {
    
      sample_host->GetCPU().GetProgram()[k].op_code = 0;

      sample_host->GetCPU().RunCPUStep(emp::WorldPosition::invalid_id, 100);
      can_do_task = ReturnTaskDone(task_passer, test_task_id,sample_host->GetCPU());
      

      if (!can_do_task) {
        reduced_position_guide.push_back(1);
      }

      else {
        reduced_position_guide.push_back(0);
      }
      
      sample_host->GetCPU().SetProgram(control_program);
    }
  }

  return reduced_position_guide;
}

emp::vector<emp::vector<int>> GetReducedProgramRepresentations(SGPHost *host) {
  CPUState condition = host->GetCPU().state;
  TaskSet all_tasks = condition.world->GetTaskSet();
  sgpl::Program<Spec> original_program = host->GetCPU().GetProgram();
  emp::vector<emp::vector<int>> map_of_guides;

  for (size_t j = 0; j < all_tasks.NumTasks(); ++j) {
    emp::vector<int> position_guide = {};
    position_guide = GetNecessaryInstructions(host, j, all_tasks);
    map_of_guides.push_back(position_guide);
  }

  return map_of_guides;
}

emp::vector<sgpl::Program<Spec>>
GuideToProgramsConvert(emp::vector<emp::vector<int>> instr_map,
                    sgpl::Program<Spec> original_program) {
  emp::vector<sgpl::Program<Spec>> converted_programs = {};
  for (int guide_num = 0; guide_num <= instr_map.size() - 1; guide_num++) {
    sgpl::Program<Spec> reduced_program = original_program;

    for (int guide_position = 0;
         guide_position <= instr_map[guide_num].size() - 1; guide_position++) {
      if (instr_map[guide_num][guide_position] == 0) {
        reduced_program[guide_position].op_code = 0;
      }
    }

    converted_programs.push_back(reduced_program);
  }

  return converted_programs;
}

emp::vector<sgpl::Program<Spec>> GetReducedPrograms(SGPHost *host) {

  CPUState condition = host->GetCPU().state;
  TaskSet all_tasks = condition.world->GetTaskSet();
  sgpl::Program<Spec> original_program = host->GetCPU().GetProgram();
  emp::vector<emp::vector<int>> position_map =
      GetReducedProgramRepresentations(host);
  emp::vector<sgpl::Program<Spec>> reduced_programs =
      GuideToProgramsConvert(position_map, original_program);

  return reduced_programs;
}



#endif