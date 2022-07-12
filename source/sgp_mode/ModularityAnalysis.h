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

  /*Gets the total number of instruction clusters, without no-ops inside of them, and returns their total amount
  *
  *
  */
  int  GetNumSites(emp::vector<int> alt_genome){
    // for altered genome clusters
    int total_sites = 0;
    int genome_size = alt_genome.size();

    for(int b = 0; b<=(genome_size-2); b++){

        if(alt_genome[b]==1&&alt_genome[b+1]==0){total_sites++;}

    }

    if(alt_genome[genome_size-1]==1){ total_sites++;}
  
    return total_sites;

  }

  

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

          if ((list_of_starts.size()-1)!=y) {
              list_of_starts.push_back(0);
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

          if ((list_of_ends.size()-1)!=y) {
              list_of_ends.push_back(0);
          }
          
       }

      return list_of_ends;

  }

  /*returns the total distance in instruction lines between useful code sites 
  *
  *
  */
  int  GetSumSiteDist(int start_used, int end_used, emp::vector<int> alt_genome){
    //for individual traits of the genome
     int sum_dist =0;

     if(start_used!=end_used){
        for(int c = start_used; c <= end_used; c++){

            if(alt_genome[c] == 0){
              sum_dist ++;
            }


        }
     }

    return sum_dist;


  }

 /*the program vector might be turned into an int vector as well depending on the checkUsefulCode return value
  *
  *
  */
  double GetSummedValue (int num_tasks, emp::vector<int> starts_used, emp::vector<int> ends_used, 
  emp::vector<emp::vector<int>>alt_genomes){

      double final_sum = 0.0;


      for(int a=0; a<=num_tasks-1; a++){
          double task_sum =0.0;

          //call methods on the altered program a
          
          
          
          double sum_site_dist_a = GetSumSiteDist(starts_used[a], ends_used[a], alt_genomes[a]);
          double num_sites_a = GetNumSites(alt_genomes[a]);
          if (sum_site_dist_a!=0) { 

              task_sum = sum_site_dist_a/(num_sites_a*(num_sites_a-1));

          }

          final_sum += task_sum;
      }

      return final_sum;

  }

  /*
  *
  *
  */
  double  CalcPModularity (int num_tasks, double summed_value,int genome_size){
    if (summed_value==0||num_tasks==0) {
      return  -1;    
    }
    double length = genome_size;
    double layer_one = 2.0/(length*num_tasks);
    double layer_two = layer_one * summed_value;
    double physical_mod = 1 - layer_two;

    return physical_mod;

  }


  /*
  * Frame function to call all the necessary functions inside
  *
  */
  float  GetPModularity (int tasks_count, emp::vector<emp::vector<int>> task_programs){
      int length = task_programs[0].size();
      double physical_mod_val = 0.0; 
    
      emp::vector<int> alt_task_starts = GetUsefulStarts(task_programs);
      emp::vector<int> alt_task_ends = GetUsefulEnds(task_programs);

      double formula_sum = GetSummedValue(tasks_count, alt_task_starts, alt_task_ends,task_programs);

      physical_mod_val = CalcPModularity(tasks_count, formula_sum, length);

      return physical_mod_val;

  }

  //end of physical modularity code

  


#endif
	
	
