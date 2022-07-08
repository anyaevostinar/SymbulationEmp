#ifndef MODULARITY_H
#define MODULARITY_H

#include "../../Empirical/include/emp/Evolve/World.hpp"
#include "../../Empirical/include/emp/data/DataFile.hpp"
#include "../../Empirical/include/emp/Evolve/Systematics.hpp"
#include "../../Empirical/include/emp/math/random_utils.hpp"
#include "../../Empirical/include/emp/math/Random.hpp"
#include "../Organism.h"
#include "SGPCpu.h"
#include "SGPHost.h"
#include "SGPWorld.h"
#include <set>
#include <math.h>



//Physical ModularityCode
 //NEXTSTEPS: needs to get the tasks being checked and the number of them being checked. ____
  // Then needs to get their useful Genome vector _____
  // Then needs to plug that into getTotalSum and getNumSites/getSiteDist within getTotalSum. v/
  //then return physical modularity and print it out v/
  //create tests for the code


//int getNumTasks(...){}
  // will need to get from the phenotype/tag methods I think

// binary<64 or 9> getDoneTasks(...) needed to getNumTraits ---> ask katrina

//GetUsefulLists(...){}
  //will need the getUsefulCode(takes in host, task) method

//GetUsefulStarts(emp::vector<emp::Ptr<emp::vector<emp::Ptr<int>>>> alteredInstrList){}
//GetUsefulEnds(emp::vector<emp::Ptr<emp::vector<emp::Ptr<int>>>> alteredInstrList){}

separated code from GetPModularity
 // int tasksCount = getNumTasks();
 //  emp::vector<emp::Ptr<emp::vector<emp::Ptr<int>>>> taskPrograms = GetUsefulLists();

  //top level method, only the frame right now, several methods not finished yet
  float GetPModularity (int numTasks, emp::vector<emp::Ptr<emp::vector<emp::Ptr<int>>>> taskPrograms){
      float physicalModVal = 0.0; 
    
      emp::vector<emp::Ptr<int>> altTaskStarts = GetUsefulStarts(taskPrograms);
      emp::vector<emp::Ptr<int>> altTaskEnds = GetUsefulEnds(taskPrograms);

      float formulaSum = getSummedValue(tasksCount, altTaskStarts, altTaskEnds,taskPrograms);

      physicalModVal = calcPModularity(tasksCount, formulaSum);

      return physicalModVal;

  }

  //creates a vector of the position in each alter program of the first !(No-op) instruction to appear

  emp::vector<emp::Ptr<int>> GetUsefulStarts(emp::vector<emp::Ptr<emp::vector<emp::Ptr<int>>>> taskPrograms){
      emp::vector<emp::Ptr<int>> listOfStarts ={};

      for("iterate through list of programs"){
          for(int e = 0; e<=99;e++){
              if(taskProgram[e])

          }

      }

      return listOfStarts;

  }

  //creates a vector of the position in each alter program of the last !(No-op) instruction to appear

  emp::vector<emp::Ptr<int>> GetUsefulEnds(emp::vector<emp::Ptr<emp::vector<emp::Ptr<int>>>> taskPrograms){
      emp::vector<emp::Ptr<int>> listOfEnds ={};

      return listOfEnds;

  }

  float CalcPModularity (int numTasks, float summedValue){
    int length = 100;
    float layerOne = 2/(length*numTraits);
    float layerTwo = layerOne * summedValue;
    float physicalMod = 1 - LayerTwo;

    return physicalMod;

  }

  //the program vector might be turned into an int vector as well depending on the checkUsefulCode return value
  float GetSummedValue (int numTasks, emp::vector<emp::Ptr<int>> startsUsed, emp::vector<emp::Ptr<int>> endsUsed, 
  emp::vector<emp::Ptr<emp::vector<emp::Ptr<int>>>> altGenomes){

      float finalSum = 0.0;


      for(int a=1; a<=numTasks; a++){
        float taskSum =0.0;

      //call methods on the altered program a
        int numSitesA = .getNumSites(altGenomes{a});
        int SumSiteDistA = .getSumSiteDist(startsUsed{a}, endsUsed{a}, altGenomes{a});

        taskSum = SumSiteDistA/(numSitesA*(numSitesA-1));

        finalSum += traitSum;
      }

      return finalSum;

  }

  //Gets the total number of instruction clusters, without no-ops inside of them, and returns their total amount
  int GetNumSites(emp::vector<emp::Ptr<int>> altGenome){
    // for altered genome clusters
    int totalSites = 0;
    int genomeSize =100;

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
   
int CheckHammingDistance(SGPHost other) {
    int dis = 0;
    // sgpl::Program<AvidaSpec> &program;
    // sgpl::Program<AvidaSpec> &otherProgram;
    sgpl::Program<AvidaSpec> program = this->getCpu().getProgram();
    sgpl::Program<AvidaSpec> otherProgram = other.getCpu().getProgram();
    
    for (int i=0;i<=99; ++i) {
      if (!compareOp(program[i],otherProgram[i])){
        dis++;
      }
    }
    return dis;
  }

	
	
	
