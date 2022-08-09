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
#include "symbulation.h"
#include <fstream>
#include <iostream>
#include <memory>

using namespace std;

// This is the main function for the NATIVE version of this project.
/*int PreProcessCheckSymbiont(emp::Ptr<SGPWorld> world){
          uint32_t hostIndex = 0;
          emp::Ptr<SGPHost> host = world->GetFullPop()[hostIndex].DynamicCast<SGPHost>();
          while (hostIndex < world->GetFullPop().size() && (!host->HasSym() || host->GetCPU().state.recentCompletion == 0)){//Not always a high number of hosts that have symbionts
                if(!world->GetSymPop().size()){
                  return -1;
                }
                hostIndex++;
                host = world->GetFullPop()[hostIndex].DynamicCast<SGPHost>();
          }
          emp::Ptr<SGPSymbiont> symbiont;
          if(hostIndex == world->GetFullPop().size()){
            std::cout << "No remaining hosts that have both symbionts and points" << std::endl;
            return -1;
          }else{
             return hostIndex;
          }*/

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
  world.RunExperiment();

  std::pair<emp::Ptr<Organism>, size_t> info = world.GetDominantInfo();
  std::cout << "Dominant count: " << info.second << std::endl;

  // Print some debug info for testing purposes
  emp::Ptr<SGPHost> sample = info.first.DynamicCast<SGPHost>();

  std::string file_ending = "_SEED" + std::to_string(config.SEED()) + ".data";
  {
    ofstream genome_file;
    std::string genome_path =
        config.FILE_PATH() + "Genome_Host" + config.FILE_NAME() + file_ending;
    genome_file.open(genome_path);
    sample->GetCPU().PrintCode(genome_file);

    for (auto &sym : sample->GetSymbionts()) {
      ofstream genome_file;
      std::string genome_path =
          config.FILE_PATH() + "Genome_Sym" + config.FILE_NAME() + file_ending;
      genome_file.open(genome_path);
      sym.DynamicCast<SGPSymbiont>()->GetCPU().PrintCode(genome_file);
    }
  }

  {
    ofstream mutualism_file;
    std::string mutualism_path =
        config.FILE_PATH() + "SymImpact" + config.FILE_NAME() + file_ending;
    mutualism_file.open(mutualism_path);

    if (sample->HasSym()) {
      mutualism_file
          << CheckSymbiont(
                 *sample,
                 *sample->GetSymbionts().front().DynamicCast<SGPSymbiont>(),
                 world)
          << std::endl;
    } else {
      // We want something in the file so it overwrites previous data, and NA is
      // something R generally understands
      mutualism_file << "NA" << std::endl;
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
