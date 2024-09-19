#include "../../Empirical/include/emp/config/ArgManager.hpp"
#include "../ConfigSetup.h"
#include "../default_mode/DataNodes.h"
#include "../default_mode/Host.h"
#include "../default_mode/Symbiont.h"
#include "../sgp_mode/DiversityAnalysis.h"
#include "../sgp_mode/ModularityAnalysis.h"
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

  world.Setup();
  world.CreateDataFiles();

  // Print some debug info for testing purposes
  std::string file_ending = "_SEED" + std::to_string(config.SEED()) + ".data";

  world.OnAnalyzePopulation([&]() {
    emp::vector<CPU> host_cpus = {};
    emp::vector<CPU> sym_cpus = {};
    emp::World<Organism>::pop_t pop = world.GetPop();
    for (size_t i = 0; i < pop.size(); i++) {
      auto sample = pop[i].DynamicCast<SGPHost>();
      host_cpus.push_back(sample->GetCPU());
      if (sample->HasSym()) {
        for (auto sym : sample->GetSymbionts()) {
          sym_cpus.push_back(sym.DynamicCast<SGPSymbiont>()->GetCPU());
        }
      }
    }
  };

  world.RunExperiment();

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
