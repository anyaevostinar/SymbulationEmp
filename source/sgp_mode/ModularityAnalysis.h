#ifndef MODULARITY_ANALYSIS_H
#define MODULARITY_ANALYSIS_H

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

// Start of physicalModularityCode

/**
 *
 * Input: Takes in a vector of ints either 0 or 1 representing No-op or
 *necessary instructions respectively. the vector is a modified version of a
 *normal genome that has the positions of all the necessary and unnecessary
 *instructions for a task
 *
 * Output: Outputs an integer of the number of sites of useful instructions
 *within a modified program
 *
 *Purpose: Gets the total number of instruction clusters, without no-ops inside
 *of them, and returns their total amount
 *
 */
int GetNumSites(emp::vector<int> alt_genome) {
  // for altered genome clusters
  int total_sites = 0;
  int genome_size = alt_genome.size();

  for (int b = 0; b <= (genome_size - 2); b++) {

    if (alt_genome[b] == 1 && alt_genome[b + 1] == 0) {
      total_sites++;
    }
  }

  if (alt_genome[genome_size - 1] == 1) {
    total_sites++;
  }

  return total_sites;
}

/**
 *
 * Input: A vector of vectors of ints to represent modified program structures
 *
 * Output: A vector of the first non No-op instruction in each program structure
 *
 *Purpose: Get the first non No-op instruction in the modified genomes and adds
 *them to a vector for GetNumSiteDist() to use
 *
 */
emp::vector<int> GetUsefulStarts(emp::vector<emp::vector<int>> task_programs) {
  emp::vector<int> list_of_starts = {};

  for (int y = 0; y < task_programs.size(); y++) {
    for (int e = 0; e <= task_programs[y].size() - 1; e++) {
      if (task_programs[y][e] == 1) {
        list_of_starts.push_back(e);
        break;
      }
    }

    if ((list_of_starts.size() - 1) != y) {
      list_of_starts.push_back(0);
    }
  }

  return list_of_starts;
}

/**
 *
 * Input:  A vector of vectors of ints to represent modified program structures
 *
 * Output:  A vector of the last non No-op instruction in each program structure
 *
 *Purpose: Get the last non No-op instruction in the modified programs and adds
 *them to a vector for GetNumSiteDist() to use
 *
 */
emp::vector<int> GetUsefulEnds(emp::vector<emp::vector<int>> task_programs) {
  emp::vector<int> list_of_ends = {};

  for (int y = 0; y < task_programs.size(); y++) {
    for (int f = task_programs[y].size() - 1; f >= 0; f--) {
      if (task_programs[y][f] == 1) {
        list_of_ends.push_back(f);
        break;
      }
    }

    if ((list_of_ends.size() - 1) != y) {
      list_of_ends.push_back(0);
    }
  }

  return list_of_ends;
}

/**
 *
 * Input: The positions of the first and last not No-op instructions in the
 *modified program that is also passed into GetSumSiteDist
 *
 * Output: Returns an integer of all the No-op instructions that appear between
 *the start and the end of the not No-op instructions
 *
 *Purpose: Sums all of the No-op instructions between the first and last useful
 *code to get a distance metric to be called in GetSummedValue
 *
 */
int GetSumSiteDist(int start_used, int end_used, emp::vector<int> alt_genome) {
  // for individual traits of the genome
  int sum_dist = 0;

  if (start_used != end_used) {
    for (int c = start_used; c <= end_used; c++) {

      if (alt_genome[c] == 0) {
        sum_dist++;
      }
    }
  }

  return sum_dist;
}

/**
 *
 * Input:
 *
 * Output:
 *
 *Purpose:
 *
 */
double GetSummedValue(int num_tasks, emp::vector<int> starts_used,
                      emp::vector<int> ends_used,
                      emp::vector<emp::vector<int>> alt_genomes) {

  double final_sum = 0.0;

  for (int a = 0; a <= num_tasks - 1; a++) {
    double task_sum = 0.0;

    // call methods on the altered program a

    double sum_site_dist_a =
        GetSumSiteDist(starts_used[a], ends_used[a], alt_genomes[a]);
    double num_sites_a = GetNumSites(alt_genomes[a]);
    if (sum_site_dist_a != 0) {

      task_sum = sum_site_dist_a / (num_sites_a * (num_sites_a - 1));
    }

    final_sum += task_sum;
  }

  return final_sum;
}

/**
 *
 * Input:
 *
 * Output:
 *
 *Purpose:
 *
 */
double CalcPModularity(int num_tasks, double summed_value, int genome_size) {
  if (summed_value == 0 || num_tasks == 0) {
    return -1;
  }
  double length = genome_size;
  double layer_one = 2.0 / (length * num_tasks);
  double layer_two = layer_one * summed_value;
  double physical_mod = 1 - layer_two;

  return physical_mod;
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
float GetPModularity(int tasks_count,
                     emp::vector<emp::vector<int>> task_programs) {
  int length = task_programs[0].size();
  double physical_mod_val = 0.0;

  emp::vector<int> alt_task_starts = GetUsefulStarts(task_programs);
  emp::vector<int> alt_task_ends = GetUsefulEnds(task_programs);

  double formula_sum = GetSummedValue(tasks_count, alt_task_starts,
                                      alt_task_ends, task_programs);

  physical_mod_val = CalcPModularity(tasks_count, formula_sum, length);

  return physical_mod_val;
}

// end of physical modularity code

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
// if(my_config->GRID()) {
//      world->AddOrgAt(new_org, emp::WorldPosition(world->GetRandomCellID()));

//reset task data after calling
//should be using CpuState variables
//task.dependencies.size()
//Task &task = tasks[task_id];
bool ReturnTaskDone(TaskSet task_list, size_t task_id,CPU host_cpu){
    bool if_task_true = false;
    bool share = true;

    host_cpu.RunCPUStep(emp::WorldPosition::invalid_id, 100);
    task_list.MarkPerformedTask(host_cpu.state,  task_id, share);
    
    for (TaskSet::Iterator one_task = task_list.begin(); one_task!=task_list.end(); ++one_task) {

        //loops through all the tasks

        std::cout<<" Rascal";
        
        //need a way to also have it check for smaller tasks done in larger dependent tasks
        
        //catches the target task
        if (one_task.index==task_id){
            //once we have the target then
            //get task data from target

            TaskSet::TaskData task_holder= *one_task;
            Task target_task = task_holder.task;
            size_t n_hosts = task_holder.n_succeeds_host;
            std::cout<<" Lemur ";

            //under what conditions should this return true
            //how can we confirm the orginism did the task?
            //use the information in state
            // if task_holder.dependencies == test_host->GetCPU().state.self_completed
            //loop through elements of dependencies and check if self_completed has larger or equal values in the same places
            
     //None of these commented sections seem right or on target from a logic or a testing perspective. Also looks a bit crazy not gonna lie.     

            // test_host->GetCPU().state.self_completed[task_id] > 0
            //task_holder.n_succeeds_host >0
            //if(host_cpu.state.self_completed[task_id]==host_cpu.state.shared_completed[task_id])
            //if(host_cpu.state.self_completed[dependency_matching_element] >= Task.num_dep_completes)
            //if(host_cpu.state.self_completed[task_id] > 0)
            
            //0 should be a pointer???
            //if(!(host_cpu.state.shared_completed[task_id]==nullptr) && host_cpu.state.shared_completed[task_id]>0){
                if_task_true = true;
           // }
        }
    }
    return if_task_true;
}

emp::vector<int> GetNecessaryInstructions(SGPHost *test_host,
                                          size_t test_task_id,
                                          TaskSet task_passer) {

  test_host->GetCPU().RunCPUStep(emp::WorldPosition::invalid_id, 100);
  sgpl::Program<Spec> control_program = test_host->GetCPU().GetProgram();
  sgpl::Program<Spec> test_program = control_program;
  emp::vector<int> reduced_position_guide = {};
  std::cout<<"HAPPYSUCCESS ";

    //whatever task it is on it should turn out to be true
  bool can_do_task = ReturnTaskDone(task_passer, test_task_id,test_host->GetCPU());
  

  if (can_do_task) {
    std::cout<<"GREatVentur ";

    for (int k = 0; k <= control_program.size() - 1; k++) {
    
      test_program[k].op_code = 0;
      // do I need to reset anything before running CanPerformTask on the
      // altered code?
      test_host->GetCPU().SetProgram(test_program);

      test_host->GetCPU().RunCPUStep(test_host->GetCPU().state.location, 100);
      can_do_task = ReturnTaskDone(task_passer, test_task_id,test_host->GetCPU());
      

      if (!can_do_task) {
        reduced_position_guide.push_back(1);
      }

      else {
        reduced_position_guide.push_back(0);
      }
      
      test_host->GetCPU().SetProgram(control_program);
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
MapToProgramsConvert(emp::vector<emp::vector<int>> instr_map,
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
      MapToProgramsConvert(position_map, original_program);

  return reduced_programs;
}

#endif
