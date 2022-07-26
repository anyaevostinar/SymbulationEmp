#include "../../sgp_mode/CPUState.h"
#include "../../sgp_mode/Instructions.h"
#include "../../sgp_mode/Tasks.h"
#include "../../sgp_mode/CPU.h"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPSymbiont.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/ModularityAnalysis.h"
#include "../../sgp_mode/AnalysisTools.h"
#include "../../default_mode/Symbiont.h"
#include "../../default_mode/Host.h"
#include "../../../Empirical/include/emp/config/ArgManager.hpp"
#include <iostream>
#include <memory>
#include "../../sgp_mode/Scheduler.h"


///the tests below are incomplete and in-progress

TEST_CASE("ReturnTaskDone","[sgp]"){
// currently focusing on getting this code up and running
// the conditional inside is probably still not up to par
        emp::Random random(10);
        SymConfigBase config;
        config.RANDOM_ANCESTOR(false);
        int world_size = 1;
        int pop_size = 1;
        TaskSet test_tasks = LogicTasks;
        SGPWorld world(random, &config, test_tasks);
        world.resize(world_size);
        config.POP_SIZE(pop_size);
        bool task_done = false;

        emp::Ptr<SGPHost> test_sample = emp::NewPtr<SGPHost>(&random, &world, &config);
        std::cout<< world.GetFullPop().size();
        size_t test_id = 0;

        test_sample->GetCPU().PrintCode();
        //conclusion: Get a better genome

        task_done = ReturnTaskDone(test_tasks, test_id,test_sample->GetCPU());

        
        REQUIRE(task_done==true);


}

TEST_CASE("GetNecessaryInstructions", "[sgp]"){
        emp::Random random(10);
        SymConfigBase config;
        config.RANDOM_ANCESTOR(false);
        int world_size = 1;
        int pop_size = 1;
        TaskSet test_tasks = LogicTasks;
        SGPWorld world(random, &config, test_tasks);
        world.resize(world_size);
        config.POP_SIZE(pop_size);

        emp::Ptr<SGPHost> test_sample = emp::NewPtr<SGPHost>(&random, &world, &config);
        std::cout<< world.GetFullPop().size();
        
        size_t test_id =0;
        test_sample->GetCPU().PrintCode();
       emp::vector<int> program_position_guide = GetNecessaryInstructions(test_sample, test_id, test_tasks); 
        emp::vector<int> zero_vector = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,1,1,1,0};
       REQUIRE(program_position_guide == zero_vector);
        //right now this returns a vector of  vectors of ints that are all 0

        std::cout<<" -GugGWQ ";
        
        
        


}





TEST_CASE("GetReducedProgramRepresentations", "[sgp]"){
        emp::Random random(10);
        SymConfigBase config;
        config.RANDOM_ANCESTOR(false);
        int world_size = 1;
        int pop_size = 1;
        TaskSet test_tasks = LogicTasks;
        SGPWorld world(random, &config, test_tasks);
        world.resize(world_size);
        config.POP_SIZE(pop_size);

        emp::vector<int> zero_vector = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0};

        emp::vector<emp::vector<int>> test_map = {};
        emp::Ptr<SGPHost> test_sample = emp::NewPtr<SGPHost>(&random, &world, &config);
        std::cout<< world.GetFullPop().size();
       test_map = GetReducedProgramRepresentations(test_sample);
       emp::vector<emp::vector<int>>zero_map = {};
        for(int guide = 0; guide<test_map.size();guide++){
              REQUIRE( test_map ==zero_map);
             
               //right now this returns a vector of  vectors of ints that are all 0
        }
        
       // GetReducedPrograms(test_sample); // gets segfault

        std::cout<<"-Malpha";

       



 }
