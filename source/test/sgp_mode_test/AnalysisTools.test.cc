#include "../../sgp_mode/AnalysisTools.h"
#include "../../../Empirical/include/emp/config/ArgManager.hpp"
#include "../../Organism.h"
#include "../../default_mode/DataNodes.h"
#include "../../default_mode/Host.h"
#include "../../default_mode/Symbiont.h"
#include "../../sgp_mode/CPU.h"
#include "../../sgp_mode/CPUState.h"
#include "../../sgp_mode/GenomeLibrary.h"
#include "../../sgp_mode/Instructions.h"
#include "../../sgp_mode/ModularityAnalysis.h"
#include "../../sgp_mode/SGPDataNodes.h"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPSymbiont.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/Scheduler.h"
#include "../../sgp_mode/Tasks.h"
#include <iostream>
#include <memory>

TEST_CASE("ReturnTaskDone", "[sgp]") {
  emp::Random random(10);
  SymConfigBase config;
  config.RANDOM_ANCESTOR(false);
  config.TASK_TYPE(1);
  int world_size = 1;
  int pop_size = 1;
  TaskSet test_tasks = LogicTasks;
  SGPWorld world(random, &config, test_tasks);
  world.resize(world_size);
  config.POP_SIZE(pop_size);

  emp::Ptr<SGPHost> test_sample =
      emp::NewPtr<SGPHost>(&random, &world, &config);

  bool task_done = false;

  WHEN("The only task is the basic Not-genome") {
    size_t test_id = 0;

    task_done = ReturnTaskDone(test_id, test_sample->GetCPU());

    THEN("Task-0 (Not), can be done") { REQUIRE(task_done == true); }
  }
}

TEST_CASE("GetNecessaryInstructions", "[sgp]") {
  GIVEN("A vector of 1s and 0s representing every site (instruction) in the "
        "actual genome that is either"
        "necessary to perform the designated task or not necessary to perform "
        "the task, respectively") {
    emp::Random random(10);
    SymConfigBase config;
    config.RANDOM_ANCESTOR(false);
    config.TASK_TYPE(1);
    int world_size = 1;
    int pop_size = 1;
    TaskSet test_tasks = LogicTasks;
    SGPWorld world(random, &config, test_tasks);
    world.resize(world_size);
    config.POP_SIZE(pop_size);

    emp::Ptr<SGPHost> test_sample =
        emp::NewPtr<SGPHost>(&random, &world, &config);

    WHEN("The only task is the basic Not-genome") {
      size_t test_id = 0;

      emp::vector<int> program_position_guide =
          GetNecessaryInstructions(test_sample->GetCPU(), test_id);

      emp::vector<int> expected_vector = {
          1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0};
      THEN("There are 4 needed code sites") {
        REQUIRE(program_position_guide == expected_vector);
      }
    }
  }
}

TEST_CASE("GetReducedProgramRepresentations", "[sgp]") {
  GIVEN("A vector of 1s and 0s representing every site (instruction) in the "
        "actual genome that is either"
        "necessary to perform the designated task or not necessary to perform "
        "the task, respectively") {
    emp::Random random(10);
    SymConfigBase config;
    config.RANDOM_ANCESTOR(false);
    config.TASK_TYPE(1);
    int world_size = 1;
    int pop_size = 1;
    TaskSet test_tasks = LogicTasks;
    SGPWorld world(random, &config, test_tasks);
    world.resize(world_size);
    config.POP_SIZE(pop_size);

    WHEN("The only task is the basic Not-genome") {
      emp::vector<int> expected_vector = {
          1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0};

      emp::vector<emp::vector<int>> test_map = {};
      emp::Ptr<SGPHost> test_sample =
          emp::NewPtr<SGPHost>(&random, &world, &config);

      test_map = GetReducedProgramRepresentations(test_sample->GetCPU());
      emp::vector<emp::vector<int>> expected_map = {
          expected_vector, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}};
    
        THEN("There are 4 needed code sites necessary to perform the Not-task "
             "and the genome cannot perform any other tasks") {
          REQUIRE(test_map == expected_map);
        }
    }
  }
}
