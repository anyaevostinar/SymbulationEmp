#ifndef DIVERSITY_ANALYSIS_H
#define DIVERSITY_ANALYSIS_H

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


bool ReturnTasksDone(CPU org_cpu) {
  
  org_cpu.RunCPUStep(emp::WorldPosition::invalid_id, 100);

  return org_cpu.state.used_resources;
}




emp::unordered_map<std::bitset<64>, int> GetPhenotypeMap(emp::vector<CPU> organisms){ // SGPWorld world
//pointer??
    emp::unordered_map<std::bitset<64> , int> phenotype;
    for (size_t i = 0; i < organisms.size(); i++){
        CPU org_cpu = organisms[i];//.GetCPU();//??
        std::bitset<64> managed_tasks = ReturnTasksDone(org_cpu);
        
        // std::cout<<"got phenotype: " << managed_tasks << std::endl; //00....001
        phenotype[managed_tasks] += 1; // this works with / without key exists (i.e. just like python)
        
    }
    return phenotype;


}



double AlphaDiversityHelper(emp::unordered_map<std::bitset<64> , int> phenotype, double q = 0.0) {
    int pop = 0;
    double partial = 0.0;
        
    // emp::vector<int> sizes;
    emp::vector<double> proportion;
    // double h = 0;
    
    for (auto const &pair: phenotype){
        pop += pair.second;
    }
    for (auto const &pair: phenotype){
        double prop = (double)pair.second/pop;
        proportion.push_back(prop);
    }
    for (auto const &i: proportion){
        partial += i;
    }
    double alpha = pow(partial, 1/(1-q));
    // std::cout << "Population: " << pop << "\n" << std::flush;
    // std::cout << "Alpha: " << partial << "\n"<<std::flush;
    return alpha;
}

double AlphaDiversity(emp::vector<CPU> organisms){
    emp::unordered_map<std::bitset<64> , int> phenotype = GetPhenotypeMap(organisms);
    return AlphaDiversityHelper(phenotype);
}


double ShannonDiversityHelper(emp::unordered_map<std::bitset<64> , int> phenotype) {
    int pop = 0;
    emp::vector<double> proportion;
    double h = 0;
    for (auto const &pair: phenotype) {
        pop += pair.second;
    }
    for (auto const &pair: phenotype){
        double prop = (double)pair.second/pop;
        proportion.push_back(prop);
    }
    for (auto const &i: proportion){
        h -= log(i)/log(exp(1.0))*i;
    }
    // std::cout << "Population: " << pop << "\n" << std::flush;
    // std::cout << "Shannon: " << h << "\n"<<std::flush;
    return h;
}

double ShannonDiversity(emp::vector<CPU> organisms){
    emp::unordered_map<std::bitset<64> , int> phenotype = GetPhenotypeMap(organisms);
    return ShannonDiversityHelper(phenotype);
}


#endif
