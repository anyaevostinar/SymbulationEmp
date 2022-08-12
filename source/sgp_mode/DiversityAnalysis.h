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

/**
 *
 * Input: A cpu of an organism
 *
 * Output: A length 64 emp bitset which describes the phenotype of organism
 * such that the ith 1 in the bitset marks the completion of task i.
 *
 * Purpose: Get the phenotype of an organism
 *
 */
emp::BitSet<64> ReturnTasksDone(CPU org_cpu) {
  
  org_cpu.RunCPUStep(emp::WorldPosition::invalid_id, 100);
  
  return *org_cpu.state.used_resources;
}


/**
 *
 * Input: A vector of cpus of organisms
 *
 * Output: A map with keys being phenotypes and values being counts of organisms
 * with corresponding phenotypes
 *
 * Purpose: Get the count of phenotypes
 *
 */
emp::unordered_map<emp::BitSet<64>, int> GetPhenotypeMap(emp::vector<CPU> organisms){
    emp::unordered_map<emp::BitSet<64> , int> phenotype_counts;
    for (size_t i = 0; i < organisms.size(); i++){
        CPU org_cpu = organisms[i];
        emp::BitSet<64> managed_tasks = ReturnTasksDone(org_cpu);        
        if (phenotype_counts.count(managed_tasks)){
            phenotype_counts[managed_tasks] += 1; 
        } else {
            phenotype_counts[managed_tasks] = 1; //necessary initializing for c++ maps
        }
    }
    return phenotype_counts;
}

/**
 *
 * Input: A map containing the count of phenotypes
 *
 * Output: The alpha diversity of the org_countulation
 *
 * Purpose: Calculates the alpha diversity based on the formula
 *
 */
double AlphaDiversityHelper(emp::unordered_map<emp::BitSet<64> , int> phenotype, double q = 0.0) {
    int org_count = 0;
    double partial = 0.0;

    emp::vector<double> proportion;
    
    for (auto const &pair: phenotype){
        org_count += pair.second;
    }
    for (auto const &pair: phenotype){
        double prop = (double)pair.second/org_count;
        proportion.push_back(prop);
    }
    for (auto const &i: proportion){
        partial += i;
    }
    double alpha = pow(partial, 1/(1-q));
    return alpha;
}

/**
 *
 * Input: A vector of cpu of organisms
 *
 * Output: The alpha diversity of the org_countulation
 *
 * Purpose: Calls and integrates the helper functions
 *
 */
double AlphaDiversity(emp::vector<CPU> organisms){
    emp::unordered_map<emp::BitSet<64> , int> phenotype = GetPhenotypeMap(organisms);
    return AlphaDiversityHelper(phenotype);
}

/**
 *
 * Input: A map containing the count of phenotypes
 *
 * Output: The shannon diversity of the org_countulation
 *
 * Purpose: Calculates the shannon diversity based on the formula
 *
 */
double ShannonDiversityHelper(emp::unordered_map<emp::BitSet<64>, int> phenotype) {
    int org_count = 0;
    emp::vector<double> proportion;
    double h = 0;
    for (auto const &pair: phenotype) {
        org_count += pair.second;
    }
    for (auto const &pair: phenotype){
        double prop = (double)pair.second/org_count;
        proportion.push_back(prop);
    }
    for (auto const &i: proportion){
        h -= log(i)/log(exp(1.0))*i;
    }
    return h;
}

/**
 *
 * Input: A vector of cpu of organisms
 *
 * Output: The shannon diversity of the org_countulation
 *
 * Purpose: Calls and integrates the helper functions
 *
 */
double ShannonDiversity(emp::vector<CPU> organisms){
    emp::unordered_map<emp::BitSet<64> , int> phenotype = GetPhenotypeMap(organisms);
    return ShannonDiversityHelper(phenotype);
}


#endif
