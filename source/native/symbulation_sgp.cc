#include "../../Empirical/include/emp/config/ArgManager.hpp"
#include "../ConfigSetup.h"
#include "../default_mode/DataNodes.h"
#include "../default_mode/Host.h"
#include "../default_mode/Symbiont.h"
#include "../sgp_mode/SGPDataNodes.h"
#include "../sgp_mode/SGPWorld.h"
#include "../sgp_mode/Scheduler.h"
#include "../sgp_mode/SymbiontImpact.h"
#include "symbulation.h"

// Empirical doesn't support more than one translation unit, so any CC files are
// included last. It still fixes include issues, but doesn't improve build time.
#include "../default_mode/WorldSetup.cc"
#include "../sgp_mode/SGPWorldSetup.cc"
#include "../sgp_mode/Tasks.cc"
#include "../sgp_mode/SGPHost.cc"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

using namespace std;

// This is the main function for the NATIVE version of this project.

int symbulation_main(int argc, char *argv[]) {
  SymConfigSGP config;
  CheckConfigFile(config, argc, argv);

  if (config.FREE_LIVING_SYMS()) {
    std::cout << "Free living symbionts aren't currently supported" << std::endl;
    exit(1);
  }

  // stress hard-coded transmission modes
  if (config.INTERACTION_MECHANISM() == 2) {
    //TODO: remove this since it should just be set correctly
    if (config.SYMBIONT_TYPE() == 0) {
      // mutualists
      config.VERTICAL_TRANSMISSION(1.0);
      config.HORIZ_TRANS(0);
    }
    else if (config.SYMBIONT_TYPE() == 1) {
      // parasites
      config.VERTICAL_TRANSMISSION(0);
      config.HORIZ_TRANS(1);
    }
  }

  config.Write(std::cout);
  emp::Random random(config.SEED());

  TaskSet task_set = LogicTasks;

  SGPWorld world(random, &config, task_set);

  world.Setup();
  world.CreateDataFiles();

  // Print some debug info for testing purposes
  std::string file_ending = "_SEED" + std::to_string(config.SEED()) + ".data";

  world.RunExperiment();
 
  world.WriteOrgReproHistFile(config.FILE_PATH() + "OrgReproHist" + config.FILE_NAME() +
    file_ending);
  world.WriteTaskCombinationsFile(config.FILE_PATH() + "EndingTaskCombinations" + config.FILE_NAME() +
    file_ending);

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
