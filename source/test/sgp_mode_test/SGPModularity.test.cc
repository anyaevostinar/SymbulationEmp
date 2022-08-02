#include "../../../Empirical/include/emp/config/ArgManager.hpp"
#include "../../default_mode/Host.h"
#include "../../default_mode/Symbiont.h"
#include "../../sgp_mode/AnalysisTools.h"
#include "../../sgp_mode/CPU.h"
#include "../../sgp_mode/CPUState.h"
#include "../../sgp_mode/Instructions.h"
#include "../../sgp_mode/ModularityAnalysis.h"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPSymbiont.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/Scheduler.h"
#include "../../sgp_mode/Tasks.h"
#include <iostream>
#include <memory>


// have world call these methods

TEST_CASE("GetNumSites", "[sgp]") {

  WHEN("last position is in a site") {
    emp::vector<int> useful_genome = {1, 0, 0, 0, 0, 0, 1, 1, 1, 1,
                                      0, 0, 0, 0, 0, 1, 1, 1, 1, 1};
    int site_count = GetNumSites(useful_genome);
    REQUIRE(site_count == 10);
  }

  WHEN("when there are no sites") {
    emp::vector<int> useful_genome = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int site_count = GetNumSites(useful_genome);
    REQUIRE(site_count == 0);
  }
}

TEST_CASE("GetDistance", "[sgp]") {
  //For when the genome is treated as circular, the max distance is 1/2 the genome length,
  //and the genome_length is always 100
  int length = 100;
  int site_i = 0;
  int site_j = 0;
  int expected_distance =0;
  int obtained_distance =0;

  
  WHEN("the distance value is less than the max distance") {
        int site_i = 50;
        int site_j = 40;
        int expected_distance =10;
        obtained_distance = GetDistance(site_i,site_j);
        REQUIRE(obtained_distance == expected_distance);


  }

  WHEN("the distance value is more than the max distance") {
        int site_i = 1;
        int site_j = 71;
        int expected_distance = 30;
        obtained_distance = GetDistance(site_i,site_j);
        REQUIRE(obtained_distance == expected_distance);

  }

  WHEN("there is no distance between the sites"){
        int site_i = 30;
        int site_j = 30;
        int expected_distance = 0;
        obtained_distance = GetDistance(site_i,site_j);
        REQUIRE(obtained_distance == expected_distance);


  }
}

// needs editing

TEST_CASE("GetPModularity", "[sgp]") {
  emp::vector<int> useful_genome_a = {0, 0, 0, 0, 1, 0, 1, 1, 1, 1,
                                      0, 0, 0, 0, 0, 1, 1, 1, 0, 1};
  emp::vector<int> useful_genome_b = {0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
                                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  emp::vector<int> useful_genome_c = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 1, 0, 0, 1, 1, 1, 0, 0};
  emp::vector<int> useful_genome_d = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
  emp::vector<int> useful_genome_e = {0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
                                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  emp::vector<emp::vector<int>> useful_genomes = {
      useful_genome_a, useful_genome_b, useful_genome_c, useful_genome_d,
      useful_genome_e};

  int num_tasks = 5;
  double test_phys_mod_a = GetPModularity(num_tasks, useful_genomes);
  double found_value_a = 0.788333;
  REQUIRE(test_phys_mod_a == Approx(found_value_a));

  num_tasks = 4;
  useful_genomes.pop_back();
  useful_genomes.pop_back();
  useful_genomes.push_back(useful_genome_e);

  double test_phys_mod_b = GetPModularity(num_tasks, useful_genomes);
  double found_value_b = 0.96041675;
  REQUIRE(test_phys_mod_b == Approx(found_value_b));
}

TEST_CASE("GetPMFromHost", "[sgp]") {
  emp::Random random(5);
  SymConfigBase config;
  config.RANDOM_ANCESTOR(false);
  int world_size = 1;
  int pop_size = 1;
  TaskSet test_tasks = LogicTasks;
  SGPWorld world(random, &config, test_tasks);
  world.resize(world_size);
  config.POP_SIZE(pop_size);
  bool task_done = false;

  emp::Ptr<SGPHost> test_sample =
      emp::NewPtr<SGPHost>(&random, &world, &config);

  WHEN("The only task is the basic not genome") {
    size_t task_set_size = test_tasks.NumTasks();
    double expected_phys_mod = .6;
    double test_phys_mod = GetPMFromHost(task_set_size, test_sample);

    REQUIRE(expected_phys_mod == test_phys_mod);
  }
}
