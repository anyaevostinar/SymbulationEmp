#include "../../Empirical/include/emp/config/ArgManager.hpp"
#include "../ConfigSetup.h"
#include "../default_mode/DataNodes.h"
#include "../default_mode/Host.h"
#include "../default_mode/Symbiont.h"
#include "../sgp_mode/SGPDataNodes.h"
#include "../sgp_mode/SGPWorld.h"
#include "../sgp_mode/SGPWorldSetup.cc"
#include "../sgp_mode/Scheduler.h"
#include "../sgp_mode/SymbiontImpact.h"
#include "../sgp_mode/ModularityAnalysis.h"
#include "../sgp_mode/DiversityAnalysis.h"
#include "symbulation.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

using namespace std;

// This is the main function for the NATIVE version of this project.

int symbulation_main(int argc, char *argv[]) {
  SymConfigBase config;
  CheckConfigFile(config, argc, argv);

  config.Write(std::cout);
  emp::Random random(config.SEED());

  TaskSet task_set = LogicTasks;
  if (config.TASK_TYPE() == 0) {
    task_set = SquareTasks;
  } else if (config.TASK_TYPE() == 1) {
    task_set = LogicTasks;
  }

  SGPWorld world(random, &config, task_set);

  worldSetup(&world, &config);
  world.CreateDataFiles();
  
  // Print some debug info for testing purposes
  std::string file_ending = "_SEED" + std::to_string(config.SEED()) + ".data";

  world.OnAnalyzePopulation([&](){
    emp::vector<CPU> host_cpus = {};
    emp::vector<CPU> sym_cpus = {};
    emp::World<Organism>::pop_t pop = world.GetPop();
    for (size_t i = 0; i < pop.size(); i++){
      auto sample = pop[i].DynamicCast<SGPHost>();
      host_cpus.push_back(sample->GetCPU());
      if(sample->HasSym()){
        for (auto sym: sample->GetSymbionts()){
          sym_cpus.push_back(sym.DynamicCast<SGPSymbiont>()->GetCPU());
        }
      }
    }
    double host_alpha = AlphaDiversity(random, host_cpus);
    emp::unordered_map<emp::BitSet<64>, int> host_phenotype_map = GetPhenotypeMap(host_cpus);
    double host_shannon;
    if (host_cpus.size() == 0){
        host_shannon = 0.0;
    } else {
        host_shannon = ShannonDiversityHelper(host_phenotype_map);
    }
    int host_richness = host_phenotype_map.size();

    double sym_alpha = AlphaDiversity(random, sym_cpus);
    emp::unordered_map<emp::BitSet<64>, int> sym_phenotype_map = GetPhenotypeMap(sym_cpus);
    double sym_shannon;
    if (sym_cpus.size() == 0){
        sym_shannon = 0.0;
    } else {
        sym_shannon = ShannonDiversityHelper(sym_phenotype_map);
    }
    int sym_richness = sym_phenotype_map.size();

    ofstream diversity_file;
    std::string diversity_path =
        config.FILE_PATH()+ "_diversity_" + config.FILE_NAME() + file_ending;
    diversity_file.open(diversity_path);

    diversity_file << "alpha_diversity, shannon_diversity, species_richness, partner" << std::endl;
    diversity_file << host_alpha <<" " << host_shannon << " " << host_richness << " host" << std::endl;
    diversity_file << sym_alpha <<" " << sym_shannon << " " << sym_richness << " symbiont" << std::endl;


    ofstream phenotype_counts_file;
    std::string phenotype_counts_path =
        config.FILE_PATH()+ "_phenotype_counts_" + config.FILE_NAME() + file_ending;
    phenotype_counts_file.open(phenotype_counts_path);
    
    phenotype_counts_file << "phenotype, count, partner" << std::endl;
    for (auto const &pair : host_phenotype_map){
      phenotype_counts_file << pair.first << " " << pair.second << " host" << std::endl;
    }
    for (auto const &pair : sym_phenotype_map){
      phenotype_counts_file << pair.first << " " << pair.second << " sym" << std::endl;
    }
  });

  world.RunExperiment();

  emp::vector<std::pair<emp::Ptr<Organism>, size_t>> dominant_organisms =
      world.GetDominantInfo();
  std::cout << "Dominant count: " << dominant_organisms.front().second
            << std::endl;

  {
    size_t idx = 0;
    for (auto pair : dominant_organisms) {
      auto sample = pair.first.DynamicCast<SGPHost>();

      ofstream genome_file;
      std::string genome_path = config.FILE_PATH() + "Genome_Host" +
                                std::to_string(idx) + config.FILE_NAME() +
                                file_ending;
      genome_file.open(genome_path);
      sample->GetCPU().PrintCode(genome_file);

      for (auto &sym : sample->GetSymbionts()) {
        ofstream genome_file;
        std::string genome_path = config.FILE_PATH() + "Genome_Sym" +
                                  std::to_string(idx) + config.FILE_NAME() +
                                  file_ending;
        genome_file.open(genome_path);
        sym.DynamicCast<SGPSymbiont>()->GetCPU().PrintCode(genome_file);
      }

      idx++;
    }
  }

  {
    ofstream mutualism_file;
    std::string mutualism_path =
        config.FILE_PATH() + "SymImpact" + config.FILE_NAME() + file_ending;
    mutualism_file.open(mutualism_path);

    for (auto pair : dominant_organisms) {
      auto sample = pair.first.DynamicCast<SGPHost>();
      if (sample->HasSym()) {
        mutualism_file
            << CheckSymbiont(
                   *sample,
                   *sample->GetSymbionts().front().DynamicCast<SGPSymbiont>(),
                   world)
            << std::endl;
      } else {
        // We want something in the file so it overwrites previous data, and NA
        // is something R generally understands
        mutualism_file << "NA" << std::endl;
      }
    }
  }

  {
    ofstream modularity_file;
    std::string modularity_path =
        config.FILE_PATH()+ "_modularity_" + config.FILE_NAME() + file_ending;
    modularity_file.open(modularity_path);

    modularity_file << "host_pm, host_fm, sym_pm, sym_fm" << std::endl;

    for (auto pair : dominant_organisms) {
      auto sample = pair.first.DynamicCast<SGPHost>();
      modularity_file << GetPMFromCPU(sample->GetCPU()) << " ";
      modularity_file << GetFMFromCPU(sample->GetCPU()) << " ";
      if (sample->HasSym()) {
        modularity_file
            << GetPMFromCPU(sample->GetSymbionts().front().DynamicCast<SGPSymbiont>()->GetCPU()) << " ";
        modularity_file
            << GetFMFromCPU(sample->GetSymbionts().front().DynamicCast<SGPSymbiont>()->GetCPU()) << std::endl;

      } else {
        modularity_file << "NA ";
        modularity_file << "NA " << std::endl;
      }
    }
  }
  


  // retrieve the dominant taxons for each organism and write them to a file
  if (config.PHYLOGENY() == 1) {
    world.WritePhylogenyFile(config.FILE_PATH() + "Phylogeny_" +
                             config.FILE_NAME() + file_ending);
  }
  return 0;
}

/*
This definition guard prevents main from being defined twice during testing.
In testing, Catch will define a main function which will initiate tests
(including testing the symbulation_main function above).
*/
#ifndef CATCH_CONFIG_MAIN
int main(int argc, char *argv[]) { return symbulation_main(argc, argv); }
#endif
