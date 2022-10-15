#ifndef DIVERSITY_ANALYSIS_H
#define DIVERSITY_ANALYSIS_H

#include "CPU.h"

/**
 * Input: A vector of cpus of organisms
 *
 * Output: A map with keys being phenotypes and values being counts of organisms
 * with corresponding phenotypes
 *
 * Purpose: Get the count of phenotypes
 */
emp::unordered_map<emp::BitSet<64>, int>
GetPhenotypeMap(const emp::vector<CPU> &organisms) {
  emp::unordered_map<emp::BitSet<64>, int> phenotype_counts;
  for (const CPU &org_cpu : organisms) {
    emp::BitSet<64> managed_tasks = org_cpu.ReturnTasksDone();
    if (phenotype_counts.count(managed_tasks)) {
      phenotype_counts[managed_tasks] += 1;
    } else {
      // necessary initializing for c++ maps
      phenotype_counts[managed_tasks] = 1;
    }
  }
  return phenotype_counts;
}

/**
 * Input: A vector of cpus of organisms
 *
 * Output: A vector of the same length with the phenotype of each organism in
 * the population
 *
 * Purpose: Get all phenotypes in the population
 */
emp::vector<emp::BitSet<64>>
GetPhenotypeVector(const emp::vector<CPU> &organisms) {
  emp::vector<emp::BitSet<64>> phenotypes;
  for (const CPU &org_cpu : organisms) {
    emp::BitSet<64> managed_tasks = org_cpu.ReturnTasksDone();
    phenotypes.push_back(managed_tasks);
  }
  return phenotypes;
}

/**
 * Input: A vector of cpu of organisms
 *
 * Output: The species richness of the population
 *
 * Purpose: Returns the number of unique phenotypes in the population
 */
int GetRichness(emp::vector<CPU> organisms) {
  emp::unordered_map<emp::BitSet<64>, int> phenotype_counts =
      GetPhenotypeMap(organisms);
  return phenotype_counts.size();
}

/**
 * Input: A random object (of the world), a vector with the phenotype of every
 * organism in the population, an optional integer indicating number of groups
 * to split into, an optional double indicating weight of proportional abundance
 *
 * Output: The alpha diversity of the population
 *
 * Purpose: Calculates the alpha diversity based on the formula
 */
double AlphaDiversityHelper(emp::Random random,
                            const emp::vector<emp::BitSet<64>> &phenotypes,
                            int j = 9, double q = 0.0) {

  int org_count = phenotypes.size();
  double partial = 0.0;
  emp::vector<emp::vector<emp::BitSet<64>>> groups = {};
  for (int i = 0; i < j; i++) { // initializing
    emp::vector<emp::BitSet<64>> shell = {};
    groups.push_back(shell);
  }

  for (auto const &phenotype : phenotypes) {
    int random_number = random.GetUInt(0, j);
    groups[random_number].push_back(phenotype);
  }

  for (auto const &group : groups) {
    int size = group.size();
    emp::unordered_map<emp::BitSet<64>, int> sub_counts = {};

    for (auto const &org : group) {
      if (sub_counts.count(org)) {
        sub_counts[org] += 1;
      } else {
        sub_counts[org] = 1;
      }
    }

    for (auto const &pair : sub_counts) {
      double weight = (double)size / org_count;
      double prop_abundance = pow((double)pair.second / size, (q - 1));

      partial += weight * prop_abundance;
    }
  }
  double alpha = 1 / (pow(partial, 1 / (q - 1)));
  return alpha;
}

/**
 * Input: A random object (of the world), A vector of cpu of organisms
 *
 * Output: The alpha diversity of the population
 *
 * Purpose: Calls and integrates the helper functions
 */
double AlphaDiversity(emp::Random random, const emp::vector<CPU> &organisms) {
  if (organisms.size() == 0) {
    return 1.0;
  }
  emp::vector<emp::BitSet<64>> phenotypes = GetPhenotypeVector(organisms);
  if (organisms.size() < 10) {
    return AlphaDiversityHelper(random, phenotypes, 1);
  }
  return AlphaDiversityHelper(random, phenotypes);
}

/**
 * Input: A map containing the count of phenotypes
 *
 * Output: The shannon diversity of the population
 *
 * Purpose: Calculates the shannon diversity based on the formula
 */
double ShannonDiversityHelper(
    const emp::unordered_map<emp::BitSet<64>, int> &phenotype) {
  int org_count = 0;
  double h = 0;
  for (auto const &pair : phenotype) {
    org_count += pair.second;
  }
  for (auto const &pair : phenotype) {
    double prop = (double)pair.second / org_count;
    h -= log(prop) / log(exp(1.0)) * prop;
  }
  return h;
}

/**
 * Input: A vector of cpu of organisms
 *
 * Output: The shannon diversity of the population
 *
 * Purpose: Calls and integrates the helper functions
 */
double ShannonDiversity(const emp::vector<CPU> &organisms) {
  if (organisms.size() == 0) {
    return 0.0;
  }
  emp::unordered_map<emp::BitSet<64>, int> phenotype =
      GetPhenotypeMap(organisms);
  return ShannonDiversityHelper(phenotype);
}

#endif
