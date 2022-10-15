#ifndef MODULARITY_ANALYSIS_H
#define MODULARITY_ANALYSIS_H

#include "CPU.h"

/**
 * Input: Takes in a CPU and the number identifying a given task
 *
 * Output: Returns a bit array representing the full genome, reduced to 1s and
 * 0s to show either that an instruction is necessary to complete the task, or
 * not respectively
 *
 * Purpose: To return a bit array that acts as a reduced program
 * representation of the necessary code lines to complete the given task
 */
std::optional<emp::BitArray<100>>
GetNecessaryInstructions(const CPU &org_cpu, size_t test_task_id) {
  emp::Random random(-1);
  const sgpl::Program<Spec> &control_program = org_cpu.GetProgram();
  sgpl::Program<Spec> test_program = control_program;

  emp::BitArray<100> reduced_position_guide;

  bool can_do_task = org_cpu.CanPerformTask(test_task_id);

  // catches if a task cannot be done ever
  if (!can_do_task) {
    return std::nullopt;
  } else {
    for (size_t k = 0; k < control_program.size(); k++) {
      test_program[k].op_code = 0;
      CPU temp_cpu(org_cpu.state.host, org_cpu.state.world, test_program);

      can_do_task = temp_cpu.CanPerformTask(test_task_id);

      if (!can_do_task) {
        reduced_position_guide.Set(k, 1);
      } else {
        reduced_position_guide.Set(k, 0);
      }

      // Only need to put back the instruction that was changed
      test_program[k] = control_program[k];
    }
  }

  return reduced_position_guide;
}

/**
 * Input: Takes in an organism's CPU
 *
 * Output: Returns a vector with a reduced program representation taken from the
 * organism for each task in the world's taskset
 *
 * Purpose: To cycle through all the tasks in the world's taskset
 * and return the necessary code sites within the original program
 * to complete each task. If the CPU does not have the necessary code,
 * the `std::optional` for that task will not have a value.
 */
emp::vector<std::optional<emp::BitArray<100>>>
GetReducedProgramRepresentations(const CPU &org_cpu) {
  const TaskSet &all_tasks = org_cpu.state.world->GetTaskSet();
  emp::vector<std::optional<emp::BitArray<100>>> map_of_guides;

  for (size_t j = 0; j < all_tasks.NumTasks(); ++j) {
    map_of_guides.push_back(GetNecessaryInstructions(org_cpu, j));
  }

  return map_of_guides;
}

// Start of physicalModularityCode

/**
 * Input: A vector of bit arrays representing necessary instructions
 *
 * Output: The index of the first necessary instruction in each input array
 *
 * Purpose: Get the index of the first necessary instruction in each genome and
 * adds them to a vector for GetNumSiteDist() to use
 */
template <const size_t length>
emp::vector<size_t>
GetUsefulStarts(emp::vector<emp::BitArray<length>> task_programs) {
  emp::vector<size_t> list_of_starts = {};

  for (size_t y = 0; y < task_programs.size(); y++) {
    for (size_t e = 0; e <= task_programs[y].size() - 1; e++) {
      if (task_programs[y][e]) {
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
 * Input: A vector of bit arrays representing necessary instructions
 *
 * Output: The index of the last necessary instruction in each input array
 *
 * Purpose: Get the index of the last necessary instruction in each genome and
 * adds them to a vector for GetNumSiteDist() to use
 */
template <const size_t length>
emp::vector<size_t>
GetUsefulEnds(emp::vector<emp::BitArray<length>> task_programs) {
  emp::vector<size_t> list_of_ends = {};
  for (size_t y = 0; y < task_programs.size(); y++) {
    for (size_t f = task_programs[y].size(); f > 0; f--) {
      if (task_programs[y][f - 1] == 1) {
        list_of_ends.push_back(f - 1);
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
 * Input: A bit array representing necessary instructions for a given task, and
 * the start and end indices of the necessary segment of the genome
 *
 * Output: Outputs the number of sites of useful instructions within a genome
 *
 * Purpose: Gets the total number of necessary instructions; unless there are no
 * sites the method should always return at least 2
 */
template <const size_t length>
int GetNumSites(int start_inst, int end_inst,
                emp::BitArray<length> alt_genome) {
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
 * Input: instruction i and instruction j positions
 *
 * Output: Returns an integer of the the distance between them
 *
 * Purpose: Calculate the distance between two instruction positions in a
 * circular genome
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
 * Input: A vector of bit arrays representing the necessary instructions for
 * each task
 *
 * Output: The physical modularity value as a number between 0 and 1
 *
 * Purpose: Top-level function to calculate physical modularity from necessary
 * instructions
 */
template <const size_t length>
double GetPModularity(emp::vector<emp::BitArray<length>> task_programs) {
  double all_distance = 0.0;
  emp::vector<size_t> useful_starts = GetUsefulStarts(task_programs);
  emp::vector<size_t> useful_ends = GetUsefulEnds(task_programs);

  for (size_t t = 0; t < task_programs.size();
       t++) { // loop through different tasks

    emp::BitArray<length> program = task_programs[t];
    size_t size = program.size();
    double nSt = GetNumSites(useful_starts[t], useful_ends[t], program);
    if (nSt != 1) {

      double sum_distance = 0;

      for (size_t i = useful_starts[t]; i <= useful_ends[t]; i++) {

        if (program[i] == 1) {

          for (size_t j = useful_starts[t]; j <= useful_ends[t]; j++) {

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
      1 -
      (2.0 * all_distance / (task_programs[0].size() * task_programs.size()));
  return physical_mod_val;
}

/**
 * Input: The organism's CPU
 *
 * Output: The physical modularity value as a number between 0 and 1
 *
 * Purpose: Top-level function to calculate physical modularity from a CPU
 */
double GetPMFromCPU(const CPU &org_cpu) {
  emp::vector<std::optional<emp::BitArray<100>>> obtained_positions =
      GetReducedProgramRepresentations(org_cpu);

  emp::vector<emp::BitArray<100>> filtered_obtained_positions;
  for (size_t i = 0; i < obtained_positions.size(); i++) {
    if (obtained_positions[i].has_value()) {
      filtered_obtained_positions.push_back(*obtained_positions[i]);
    }
  }
  if (filtered_obtained_positions.size() == 0) {
    return -1.0;
  }

  double phys_mod = GetPModularity(filtered_obtained_positions);

  return phys_mod;
}
// end of physical modularity code

// start of functional modularity code

/**
 * Input: A vector of bit arrays representing the necessary instructions for
 * each task
 *
 * Output: The functional modularity value as a number between 0 and 1
 *
 * Purpose: Top-level function to calculate functional modularity from necessary
 * instructions
 */
template <const size_t length>
double GetFModularity(emp::vector<emp::BitArray<length>> task_programs) {
  size_t tasks_count = task_programs.size();
  double functional_mod_val = 0.0;
  double sum = 0.0; // sum is everything on the numerator
  for (size_t i = 0; i < task_programs.size(); i++) {   // trait a
    for (size_t j = 0; j < task_programs.size(); j++) { // trait b
      if (i != j) {
        emp::BitArray<length> current_program = task_programs[i];
        for (size_t k = 0; k < current_program.size(); k++) {
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
 * Input: A vector of bit arrays representing the necessary instructions for
 * each task
 *
 * Output: The functional modularity value as a number between 0 and 1
 *
 * Purpose: Top-level function to calculate functional modularity from a CPU
 */
double GetFMFromCPU(const CPU &org_cpu) {
  emp::vector<std::optional<emp::BitArray<100>>> obtained_positions =
      GetReducedProgramRepresentations(org_cpu);

  emp::vector<emp::BitArray<100>> filtered_obtained_positions;
  for (size_t i = 0; i < obtained_positions.size(); i++) {
    if (obtained_positions[i].has_value()) {
      filtered_obtained_positions.push_back(*obtained_positions[i]);
    }
  }

  double func_mod = GetFModularity(filtered_obtained_positions);

  return func_mod;
}

// end of functional modularity

#endif
