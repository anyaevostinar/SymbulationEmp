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
#include "../../sgp_mode/DiversityAnalysis.h"
#include "../../sgp_mode/SGPDataNodes.h"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPSymbiont.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/Scheduler.h"
#include "../../sgp_mode/Tasks.h"
#include <iostream>
#include <memory>


TEST_CASE("GetPhenotypeMap", "[sgp]") {

  WHEN("For two identical CPUs") {
    emp::Random random(5);
    SymConfigBase config;
    config.RANDOM_ANCESTOR(false);
    config.TASK_TYPE(1);
    int world_size = 1;
    int pop_size = 1;
    TaskSet test_tasks = LogicTasks;
    SGPWorld world(random, &config, test_tasks);
    world.resize(world_size);
    config.POP_SIZE(pop_size);

    size_t length = 100;

    ProgramBuilder builder;
    sgpl::Program<Spec> test_program = builder.Build(length);

    emp::Ptr<SGPHost> test_sample1 =
        emp::NewPtr<SGPHost>(&random, &world, &config, test_program);
    emp::Ptr<SGPHost> test_sample2 =
        emp::NewPtr<SGPHost>(&random, &world, &config, test_program);


    emp::BitSet<64> zeros;
  
    
    emp::unordered_map<emp::BitSet<64> , int> expected_map;
    expected_map.insert({zeros,2});
    emp::vector<CPU> test_vec = {};
    test_vec.push_back((*test_sample1).GetCPU());
    test_vec.push_back((*test_sample2).GetCPU());
    
    
    emp::unordered_map<emp::BitSet<64>, int> test_map = GetPhenotypeMap(test_vec);

    THEN("GetPhenotypeMap successfully count the phenotype twice") { 
      REQUIRE(test_map[zeros] == expected_map[zeros]); 
    }
  }
}

TEST_CASE("GetRichness", "[sgp]") {
  WHEN("For three different CPUs") {
    emp::Random random(5);
    SymConfigBase config;
    config.RANDOM_ANCESTOR(false);
    config.TASK_TYPE(1);
    int world_size = 1;
    int pop_size = 1;
    TaskSet test_tasks = LogicTasks;
    SGPWorld world(random, &config, test_tasks);
    world.resize(world_size);
    config.POP_SIZE(pop_size);

    size_t length = 100;

    ProgramBuilder builder;
    sgpl::Program<Spec> test_program = builder.Build(length);

    emp::Ptr<SGPHost> test_sample1 =
        emp::NewPtr<SGPHost>(&random, &world, &config, test_program);

    
    builder.AddNand();
    test_program = builder.Build(length);
    emp::Ptr<SGPHost> test_sample2 =
        emp::NewPtr<SGPHost>(&random, &world, &config, test_program);

    
    builder.AddOrn();
    test_program = builder.Build(length);
    emp::Ptr<SGPHost> test_sample3 =
        emp::NewPtr<SGPHost>(&random, &world, &config, test_program);

    emp::BitSet<64> zeros;
  
    int expected_types = 3;
    emp::vector<CPU> test_vec = {};
    test_vec.push_back((*test_sample1).GetCPU());
    test_vec.push_back((*test_sample2).GetCPU());
    test_vec.push_back((*test_sample3).GetCPU());
    
    int test_size = GetRichness(test_vec);

    THEN("Species richness is equal to 3") { 
      REQUIRE(test_size == expected_types); 
    }
  }
}




TEST_CASE("AlphaDiversity", "[sgp]") {

  WHEN("For a world with just two identical CPUs") {
    emp::Random random(5);
    SymConfigBase config;
    config.RANDOM_ANCESTOR(false);
    config.TASK_TYPE(1);
    int world_size = 1;
    int pop_size = 1;
    TaskSet test_tasks = LogicTasks;
    SGPWorld world(random, &config, test_tasks);
    world.resize(world_size);
    config.POP_SIZE(pop_size);

    size_t length = 100;

    ProgramBuilder builder;
    sgpl::Program<Spec> test_program = builder.Build(length);

    emp::Ptr<SGPHost> test_sample1 =
        emp::NewPtr<SGPHost>(&random, &world, &config, test_program);
    emp::Ptr<SGPHost> test_sample2 =
        emp::NewPtr<SGPHost>(&random, &world, &config, test_program);


    emp::BitSet<64> zeros;
  
    
    emp::unordered_map<emp::BitSet<64> , int> expected_map;
    expected_map.insert({zeros,2});
    double expected_value = 1.0; 
    emp::vector<CPU> test_vec = {};
    test_vec.push_back((*test_sample1).GetCPU());
    test_vec.push_back((*test_sample2).GetCPU());

    
    double test_value = AlphaDiversity(random,test_vec);

    THEN("Alpha diversity is equal to 1") { 
      REQUIRE(test_value == expected_value); 
      }
  }
}


TEST_CASE("ShannonDiversity", "[sgp]") {

  WHEN("For a world with just two identical CPUs") {
    emp::Random random(5);
    SymConfigBase config;
    config.RANDOM_ANCESTOR(false);
    config.TASK_TYPE(1);
    int world_size = 1;
    int pop_size = 1;
    TaskSet test_tasks = LogicTasks;
    SGPWorld world(random, &config, test_tasks);
    world.resize(world_size);
    config.POP_SIZE(pop_size);

    size_t length = 100;

    ProgramBuilder builder;
    sgpl::Program<Spec> test_program = builder.Build(length);

    emp::Ptr<SGPHost> test_sample1 =
        emp::NewPtr<SGPHost>(&random, &world, &config, test_program);
    emp::Ptr<SGPHost> test_sample2 =
        emp::NewPtr<SGPHost>(&random, &world, &config, test_program);


    emp::BitSet<64> zeros;
  
    
    emp::unordered_map<emp::BitSet<64> , int> expected_map;
    expected_map.insert({zeros,2});
    double expected_value = 0.0;
    emp::vector<CPU> test_vec = {};
    test_vec.push_back((*test_sample1).GetCPU());
    test_vec.push_back((*test_sample2).GetCPU());


    
    double test_value = ShannonDiversity(test_vec);

    THEN("Shannon diversity is equal to 0") { 
      REQUIRE(test_value == expected_value); 
    }
  }

  

  
}

