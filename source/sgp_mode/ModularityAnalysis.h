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

using Library =
      sgpl::OpLibrary<sgpl::Nop<>, inst::JumpIfNEq, inst::JumpIfLess,
                      // if-label doesn't make sense for SGP, same with *-head
                      // and set-flow but this is required
                      sgpl::global::Anchor,
                      // single argument math
                      inst::ShiftLeft, inst::ShiftRight, inst::Increment,
                      inst::Decrement,
                      // Stack manipulation
                      inst::Push, inst::Pop, inst::SwapStack, inst::Swap,
                      // double argument math
                      inst::Add, inst::Subtract, inst::Nand,
                      // biological operations
                      // no copy or alloc
                      inst::Reproduce, inst::IO,
                      // no h-search
                      inst::Donate>;
using Spec = sgpl::Spec<Library, CPUState>;

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
                std::cout<<"???"<<std::endl;
            } else {
                std::cout<<score<< " check point 1"<<std::endl;
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

  //Start of physicalModularityCode

  float GetPModularity (int numTasks, emp::vector<emp::Ptr<emp::vector<emp::Ptr<int>>>> taskPrograms, int genomeSize){
      int length = genomeSize;
      float physicalModVal = 0.0; 
    
      emp::vector<emp::Ptr<int>> altTaskStarts = GetUsefulStarts(taskPrograms);
      emp::vector<emp::Ptr<int>> altTaskEnds = GetUsefulEnds(taskPrograms);

      float formulaSum = getSummedValue(tasksCount, altTaskStarts, altTaskEnds,taskPrograms,length);

      physicalModVal = calcPModularity(tasksCount, formulaSum, length);

      return physicalModVal;

  }

  //creates a vector of the position in each alter program of the first !(No-op) instruction to appear

  emp::vector<emp::Ptr<int>> GetUsefulStarts(emp::vector<emp::Ptr<emp::vector<emp::Ptr<int>>>> taskPrograms){
      emp::vector<emp::Ptr<int>> listOfStarts ={};

      for(int y=0; y<taskPrograms.size(); y++)){
          for(int e = 0; e<=taskPrograms[y].size()-1;e++){
              if(taskPrograms[y][e] == 1){
                  listOfStarts.pushback(e);
                  break;
              }

          }

      }

      return listOfStarts;

  }

  //creates a vector of the position in each alter program of the last !(No-op) instruction to appear

  emp::vector<emp::Ptr<int>> GetUsefulEnds(emp::vector<emp::Ptr<emp::vector<emp::Ptr<int>>>> taskPrograms){
      emp::vector<emp::Ptr<int>> listOfEnds ={};

       for(int y=0; y<taskPrograms.size(); y++)){
          for(int f = taskPrograms[y].size()-1; f>=0;f--){
              if(taskPrograms[y][e] == 1){
                  listOfEnds.pushback(e);
                  break;
              }

          }

      return listOfEnds;

  }

  float CalcPModularity (int numTasks, float summedValue,int genomeSize){
    int length = genomeSize;
    float layerOne = 2/(length*numTasks);
    float layerTwo = layerOne * summedValue;
    float physicalMod = 1 - LayerTwo;

    return physicalMod;

  }

  //the program vector might be turned into an int vector as well depending on the checkUsefulCode return value
  float GetSummedValue (int numTasks, emp::vector<emp::Ptr<int>> startsUsed, emp::vector<emp::Ptr<int>> endsUsed, 
  emp::vector<emp::Ptr<emp::vector<emp::Ptr<int>>>> altGenomes, int programSize){

      int length = programSize;
      float finalSum = 0.0;


      for(int a=1; a<=numTasks; a++){
        float taskSum =0.0;

      //call methods on the altered program a
        int numSitesA = .GetNumSites(altGenomes{a}, length);
        int SumSiteDistA = .GetSumSiteDist(startsUsed{a}, endsUsed{a}, altGenomes{a});

        taskSum = SumSiteDistA/(numSitesA*(numSitesA-1));

        finalSum += taskSum;
      }

      return finalSum;

  }

  //Gets the total number of instruction clusters, without no-ops inside of them, and returns their total amount
  int GetNumSites(emp::vector<emp::Ptr<int>> altGenome, int length){
    // for altered genome clusters
    int totalSites = 0;
    int genomeSize = length;

    for(int b = 0; b<=(genomeSize-2); b++){

        if(altGenome[b]==1&&altGenome[b+1]==0){totalSites++;}

    }

    if(altGenome[genomeSize-2]==0 && altGenome[genomeSize-1]==1){ totalSites++;}
  
    return totalSites;

  }

  //returns the total distance in instruction lines between useful code sites 
  int GetSumSiteDist(int startUsed, int endUsed, emp::vector<emp::Ptr<int>> altGenome){
    //for individual traits of the genome
     int sumDist =0;

     for(int c = startUsed; c <= endUsed; c++){

        if(altGenome[c] == 0){
          sumDist ++;
        }


     }

    return sumDist;


  }

  //end of physical modularity code

#endif
	
	
