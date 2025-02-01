#include "../../Empirical/include/emp/config/ArgManager.hpp"
#include "../ConfigSetup.h"
#include "../default_mode/DataNodes.h"
#include "../default_mode/Host.h"
#include "../default_mode/Symbiont.h"
#include "../sgp_mode/SGPWorld.h"
#include "../sgp_mode/Scheduler.h"
#include "../sgp_mode/SymbiontImpact.h"
#include "symbulation.h"

// Empirical doesn't support more than one translation unit, so any CC files are
// included last. It still fixes include issues, but doesn't improve build time.
#include "../default_mode/WorldSetup.cc"
#include "../sgp_mode/SGPWorld.cc"
#include "../sgp_mode/SGPWorldSetup.cc"
#include "../sgp_mode/SGPWorldDataNodes.cc"
#include "../sgp_mode/Tasks.cc"
#include "../sgp_mode/SGPHost.cc"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

// This is the main function for the NATIVE version of this project.

int symbulation_main(int argc, char *argv[]) {
  sgpmode::SymConfigSGP config;
  CheckConfigFile(config, argc, argv);

  emp::Random random(config.SEED());

  sgpmode::TaskSet task_set = sgpmode::LogicTasks;

  sgpmode::SGPWorld world(random, &config, task_set);

  world.Setup();
  world.CreateDataFiles();

  // Write config after world setup to accomodate world potentially changing
  // config values on setup.
  config.Write(std::cout);

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
      auto sample = pair.first.DynamicCast<sgpmode::SGPHost>();

      std::ofstream genome_file;
      std::string genome_path = config.FILE_PATH() + "Genome_Host" +
                                std::to_string(idx) + config.FILE_NAME() +
                                file_ending;
      genome_file.open(genome_path);
      sample->GetCPU().PrintCode(genome_file);

      for (auto &sym : sample->GetSymbionts()) {
        std::ofstream g1enome_file;
        std::string genome_path = config.FILE_PATH() + "Genome_Sym" +
                                  std::to_string(idx) + config.FILE_NAME() +
                                  file_ending;
        genome_file.open(genome_path);
        sym.DynamicCast<sgpmode::SGPSymbiont>()->GetCPU().PrintCode(genome_file);
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
