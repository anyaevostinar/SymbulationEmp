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

    emp::Ptr<SGPHost> test_sample1 =
        emp::NewPtr<SGPHost>(&random, &world, &config);
    emp::Ptr<SGPHost> test_sample2 =
        emp::NewPtr<SGPHost>(&random, &world, &config);
    // emp::Ptr<SGPHost> test_sample3 =
        // emp::NewPtr<SGPHost>(&random, &world, &config);

    ProgramBuilder builder;
    // builder.AddNand();
    // builder.AddNot();
    sgpl::Program<Spec> test_program = builder.Build(length);
    CPU temp_cpu1 =
        CPU(test_sample1->GetCPU().state.host, test_sample1->GetCPU().state.world,
            &random, test_program);
    CPU temp_cpu2 =
        CPU(test_sample2->GetCPU().state.host, test_sample2->GetCPU().state.world,
            &random, test_program);
    test_sample1->GetCPU() = temp_cpu1;
    test_sample2->GetCPU() = temp_cpu2;

    std::bitset<64> zeros;
    zeros[0] = 1; //not sure why the two samples have 00....001
    
    emp::unordered_map<std::bitset<64> , int> expected_map;
    expected_map.insert({zeros,2});
    emp::vector<CPU> test_vec = {};
    test_vec.push_back((*test_sample1).GetCPU());
    test_vec.push_back((*test_sample2).GetCPU());

    // emp::Ptr<emp::vector<Organism>> test_ptr =
        // emp::NewPtr<emp::vector<Organism>>(test_vec);
    
    
    emp::unordered_map<std::bitset<64>, int> test_map = GetPhenotypeMap(test_vec);

    THEN("") { REQUIRE(test_map[zeros] == expected_map[zeros]); }
  }
}