#ifndef MODULARITY_ANALYSIS_H
#define MODULARITY_ANALYSIS_H

#include "../../Empirical/include/emp/Evolve/Systematics.hpp"
#include "../../Empirical/include/emp/Evolve/World.hpp"
#include "../../Empirical/include/emp/data/DataFile.hpp"
#include "../../Empirical/include/emp/math/Random.hpp"
#include "../../Empirical/include/emp/math/random_utils.hpp"
#include "../Organism.h"
#include "../default_mode/Host.h"
#include "AnalysisTools.h"
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
#include <math.h>
#include <set>


// Start of physicalModularityCode

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

  for (int y = 0; y < (int)task_programs.size(); y++) {
    for (int e = 0; e <= (int)task_programs[y].size() - 1; e++) {
      if (task_programs[y][e] == 1) {
        list_of_starts.push_back(e);
        break;
      }
    }

    // if ((list_of_starts.size() - 1) != y) {
    //   list_of_starts.push_back(0);
    // }
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

  for (int y = 0; y < (int)task_programs.size(); y++) {
    for (int f = (int)task_programs[y].size() - 1; f >= 0; f--) {
      if (task_programs[y][f] == 1) {
        list_of_ends.push_back(f);
        break;
      }
    }

    // if ((list_of_ends.size() - 1) != y) {
    //   list_of_ends.push_back(0);
    // }
  }

  return list_of_ends;
}

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
 *Purpose: Gets the total number of instruction, without no-ops inside
 *of them, and returns their total amount
 *unless there are no sites the method should always return at least 2
 */
int GetNumSites(int start_inst, int end_inst, emp::vector<int> alt_genome) {
  // for altered genome clusters
  int total_sites = 0;

  for (int b = start_inst; b <= end_inst; b++) {

    if (alt_genome[b] == 1) {
      total_sites++;
    }
  }

  return total_sites;
}

/**
 *
 * Input: instruction i and instruction j positions
 *
 * Output: Returns an integer of the the distance between them
 *
 *Purpose: takes the distance between two sites and returns the value
 *
 */
int GetDistance(int i, int j, int length) {
  // return std::abs(i-j); // if we treat genome as non-circular
  int genome_length = length;
  int dis = std::abs(i - j);
  if (dis <= genome_length / 2) {
    return dis;
  }
  return genome_length - dis;
}

/**
 *
 * Input: Takes in the number of tasks and the 2d vector representing the
 *positions of all the tasks' sites
 *
 * Output: A number between 0 and 1 that, as the physical modularity value
 *
 *Purpose: Takes all the calculation methods and calls them in order of having a
 *simplified way of getting an organism's Physical Modularity from a 2d vector
 *of all the sites of the tasks it can do
 *
 */

double GetPModularity(emp::vector<emp::vector<int>> task_programs) {
  double all_distance = 0.0;
  emp::vector<int> useful_starts = GetUsefulStarts(task_programs);
  emp::vector<int> useful_ends = GetUsefulEnds(task_programs);

  for (int t = 0; t < (int)task_programs.size();
       t++) { // loop through different tasks

    emp::vector<int> program = task_programs[t];
    int size = (int)program.size();
    double nSt = GetNumSites(useful_starts[t], useful_ends[t], program);
    if (nSt != 1) {

      double sum_distance = 0;

      for (int i = useful_starts[t]; i <= useful_ends[t]; i++) {

        if (program[i] == 1) {

          for (int j = useful_starts[t]; j <= useful_ends[t]; j++) {

            if ((i != j) && (program[j] == 1)) {
              sum_distance += GetDistance(i, j, size);
            }
          }
        }
      }

      double task_distance = sum_distance / (nSt * (nSt - 1));
      all_distance += task_distance;
    }
  }

  double physical_mod_val =
      1 - (2.0 * all_distance /
           (task_programs[0].size() * (int)task_programs.size()));
  return physical_mod_val;
}

/**
 *
 * Input:Takes in the number of tasks in the taskset and a CPU reference
 *
 * Output: A number between 0 and 1 that, as the physical modularity value
 *
 *Purpose: Takes all the calculation methods and calls them in order of having a
 *simplified way of getting an organism's Physical Modularity
 *
 */

double GetPMFromCPU(CPU org_cpu) {
  emp::vector<emp::vector<int>> obtained_positions =
      GetReducedProgramRepresentations(org_cpu);

  emp::vector<emp::vector<int>> filtered_obtained_positions;
  for (int i = 0; i < (int)obtained_positions.size(); i++) {
    if (obtained_positions[i].size() != 1) {
      filtered_obtained_positions.push_back(obtained_positions[i]);
    }
  }
  if ((int)filtered_obtained_positions.size() == 0) {
    return -1.0;
  }

  double phys_mod = GetPModularity(filtered_obtained_positions);

  return phys_mod;
}
// end of physical modularity code

// start of functional modularity code

/**
 *
 * Input: Takes in the number of tasks and the 2d vector representing the
 *positions of all the tasks' sites
 *
 * Output: A number between 0 and 1 that, as the functional modularity value
 *
 *Purpose: Takes all the calculation methods and calls them in order of having a
 *simplified way of getting an organism's Functional Modularity from a 2d vector
 *of all the sites of the tasks it can do
 *
 */

double GetFModularity(emp::vector<emp::vector<int>> task_programs) {
  int tasks_count = (int)task_programs.size();
  int length = task_programs[0].size();
  double functional_mod_val = 0.0;
  double sum = 0.0; // sum is everything on the numerator
  for (int i = 0; i < (int)task_programs.size(); i++) {   // trait a
    for (int j = 0; j < (int)task_programs.size(); j++) { // trait b
      if (i != j) {
        emp::vector<int> current_program = task_programs[i];
        for (int k = 0; k < (int)current_program.size(); k++) {
          if (current_program[k] == 1) {
            sum += (1 - task_programs[j][k]);
            // task_programs[j][k] is the M(s,b) in the paper,
            // which specifies whether site s is required for expression of
            // trait b
          }
        }
      }
    }
  }
  functional_mod_val = sum / (length * tasks_count * (tasks_count - 1));
  return functional_mod_val;
}

/**
 *
 * Input:Takes in the number of tasks in the taskset and a CPU reference
 *
 * Output: A number between 0 and 1 that, as the funtional modularity value
 *
 *Purpose: Takes all the calculation methods and calls them in order of having a
 *simplified way of getting an organism's Functional Modularity
 *
 */
double GetFMFromCPU(CPU org_cpu) {
  emp::vector<emp::vector<int>> obtained_positions =
      GetReducedProgramRepresentations(org_cpu);

  emp::vector<emp::vector<int>> filtered_obtained_positions;
  for (int i = 0; i < (int)obtained_positions.size(); i++) {
    if (obtained_positions[i].size() != 1) {
      filtered_obtained_positions.push_back(obtained_positions[i]);
    }
  }

  double func_mod = GetFModularity(filtered_obtained_positions);

  return func_mod;
}

// end of functional modularity

#endif
