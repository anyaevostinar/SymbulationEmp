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

  /**
  * 
  * Input: Takes in a vector of ints either 0 or 1 representing No-op or necessary instructions respectively. 
  * the vector is a modified version of a normal genome that has the positions of all the necessary 
  * and unnecessary instructions for a task
  *
  * Output: Outputs an integer of the number of sites of useful instructions within a modified program
  *
  *Purpose: Gets the total number of instruction clusters, without no-ops inside of them, and returns their total amount
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

  

   /**
  * 
  * Input: 
  *
  * Output: 
  *
  *Purpose: 
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

  /**
  * 
  * Input: 
  *
  * Output: 
  *
  *Purpose: 
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

  /**
  * 
  * Input: 
  *
  * Output: 
  *
  *Purpose: 
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

 /**
  * 
  * Input: 
  *
  * Output: 
  *
  *Purpose: 
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

  /**
  * 
  * Input: 
  *
  * Output: 
  *
  *Purpose: 
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


  /**
  * 
  * Input: 
  *
  * Output: 
  *
  *Purpose: Takes all the calculation methods and calls them in order of having a simplified way of getting an organism's Physical Modularity
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

  //start of getUsefulGenomes methods

  using Library =
      sgpl::OpLibrary<sgpl::Nop<>,
                      // single argument math
                      inst::ShiftLeft, inst::ShiftRight, inst::Increment,
                      inst::Decrement,
                      // biological operations
                      // no copy or alloc
                      inst::Reproduce, inst::PrivateIO, inst::SharedIO,
                      // double argument math
                      inst::Add, inst::Subtract, inst::Nand,
                      // Stack manipulation
                      inst::Push, inst::Pop, inst::SwapStack, inst::Swap,
                      // no h-search
                      inst::Donate, inst::JumpIfNEq, inst::JumpIfLess,
                      // if-label doesn't make sense for SGP, same with *-head
                      // and set-flow but this is required
                      sgpl::global::Anchor>;

  using Spec = sgpl::Spec<Library, CPUState>;


  /**
  * 
  * Input: Takes in a host
  *
  * Output: Outputs a vector of int vectors that represents all the necessary instructions to do a task for each task the host can do
  *
  *Purpose: To get modified versions of the host's genome to give to top level modularity methods
  *
  */
  emp::vector<emp::vector<int>> PhysicalModularityHelper(SGPHost host){

    CPUState state = host.GetCPU().state;
    
    emp::vector<Task> full_tasks = state.world->GetTaskSet().GetTasks();

    
    
    sgpl::Program<Spec> program = host.GetCPU().GetProgram();
    emp::vector<emp::vector<int>> result;

    for (int j=0;j<(int)full_tasks.size();++j){
        //iterate through every task
        
        emp::vector<int> useful;
        emp::vector<int> useless;
        emp::vector<int> binary_string;

        // emp::vector<Task> one_task;
        // one_task.push_back(full_tasks[j]);

        std::initializer_list<Task> one_task = {full_tasks[j]};

        TaskSet test_task_set = TaskSet(one_task);
        sgpl::Program<Spec> useful_program;

        for (int i=0;i<100; ++i) {
            //iterate through every line of instruction
            sgpl::Program<Spec> test_program = program;
            // test_program[i].NopOut();
            test_program[i].op_code = 0; // change that line of instruction to no-op
            host.GetCPU().SetProgram(test_program);    
            float score = test_task_set.CheckTasks(host.GetCPU().state, 2);
            if(score != 0){
                std::cout<<"???";
            } else {
                std::cout<<score<< " check point 1";
            }//currently it is printing 100 things so it successfully went through the first task
            if (score != 0.0){
                useful.push_back(i);
                binary_string.push_back(1);
            } else {
                useless.push_back(i);
                binary_string.push_back(0);
            }
            
            
        }
        
        for (int i=0;i<(int)useless.size(); ++i){
            int &line = useless[i];
            useful_program[line].op_code = 0;//0 means nop
        }
        host.GetCPU().SetProgram(useful_program);
        host.GetCPU().PrintCode();
        // host.GetCPU().SetProgram(program);
        result.push_back(binary_string);
    }
    host.GetCPU().SetProgram(program);
    return result;
}






#endif
	
	