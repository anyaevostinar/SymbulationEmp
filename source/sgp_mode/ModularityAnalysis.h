#ifndef MODULARITY_ANALYSIS_H
#define MODULARITY_ANALYSIS_H

#include "../../Empirical/include/emp/Evolve/World.hpp"
#include "../../Empirical/include/emp/data/DataFile.hpp"
#include "../../Empirical/include/emp/Evolve/Systematics.hpp"
#include "../../Empirical/include/emp/math/random_utils.hpp"
#include "../../Empirical/include/emp/math/Random.hpp"
#include "../Organism.h"
#include "../default_mode/Host.h"
#include "CPUState.h"
#include "Instructions.h"
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
#include "CPU.h"
#include "CPUState.h"
#include "SGPHost.h"
#include "SGPWorld.h"
#include "Tasks.h"
#include <set>
#include <math.h>

  //Start of physicalModularityCode

  

  /*creates a vector of the position in each alter program of the first !(No-op) instruction to appear
  *
  *
  */
  emp::vector<int>  GetUsefulStarts(emp::vector<emp::vector<int>> task_programs){
      emp::vector<int> list_of_starts ={};

      for(int y=0; y<task_programs.size(); y++){
          for(int e = 0; e<=task_programs[y].size()-1;e++){
              if(task_programs[y][e] == 1){
                  list_of_starts.push_back(e);
                  break;
              }

          }

      }

      return list_of_starts;

  }

  /*creates a vector of the position in each alter program of the last !(No-op) instruction to appear
  *
  *
  */
  emp::vector<int>  GetUsefulEnds(emp::vector<emp::vector<int>> task_programs){
       emp::vector<int> list_of_ends ={};

       for(int y=0; y<task_programs.size(); y++){
          for(int f = task_programs[y].size()-1; f>=0;f--){
              if(task_programs[y][f] == 1){
                  list_of_ends.push_back(f);
                  break;
              }

          }
       }

      return list_of_ends;

  }


  /*
  *
  *
  */
  float  CalcPModularity (int num_tasks, float summed_value,int genome_size){
    int length = genome_size;
    float layer_one = 2/(length*num_tasks);
    float layer_two = layer_one * summed_value;
    float physical_mod = 1 - layer_two;

    return physical_mod;

  }

 

  /*Gets the total number of instruction clusters, without no-ops inside of them, and returns their total amount
  *
  *
  */
  int  GetNumSites(emp::vector<int> alt_genome, int length){
    // for altered genome clusters
    int total_sites = 0;
    int genome_size = length;

    for(int b = 0; b<=(genome_size-2); b++){

        if(alt_genome[b]==1&&alt_genome[b+1]==0){total_sites++;}

    }

    if(alt_genome[genome_size-2]==0 && alt_genome[genome_size-1]==1){ total_sites++;}
  
    return total_sites;

  }

  /*returns the total distance in instruction lines between useful code sites 
  *
  *
  */
  int  GetSumSiteDist(int start_used, int end_used, emp::vector<int> alt_genome){
    //for individual traits of the genome
     int sum_dist =0;

     for(int c = start_used; c <= end_used; c++){

        if(alt_genome[c] == 0){
          sum_dist ++;
        }


     }

    return sum_dist;


  }

   /*the program vector might be turned into an int vector as well depending on the checkUsefulCode return value
  *
  *
  */
  float GetSummedValue (int num_tasks, emp::vector<int> starts_used, emp::vector<int> ends_used, 
  emp::vector<emp::vector<int>>alt_genomes, int genome_size){

      int length = genome_size;
      float final_sum = 0.0;


      for(int a=1; a<=num_tasks; a++){
        float task_sum =0.0;

      //call methods on the altered program a
        int num_sites_a = GetNumSites(alt_genomes[a], length);
        int Sum_site_dist_a = GetSumSiteDist(starts_used[a], ends_used[a], alt_genomes[a]);

        task_sum = Sum_site_dist_a/(num_sites_a*(num_sites_a-1));

        final_sum += task_sum;
      }

      return final_sum;

  }

  /*
  *
  *
  */
  float  GetPModularity (int num_tasks, emp::vector<emp::vector<int>> task_programs){
      int length = task_programs[0].size();
      float physical_mod_val = 0.0; 
    
      emp::vector<int> alt_task_starts = GetUsefulStarts(task_programs);
      emp::vector<int> alt_task_ends = GetUsefulEnds(task_programs);

      float formula_sum = GetSummedValue(tasks_count, alt_task_starts, alt_task_ends,task_programs,length);

      physical_mod_val = CalcPModularity(tasks_count, formula_sum, length);

      return physical_mod_val;

  }

  //end of physical modularity code

#endif
	
	
