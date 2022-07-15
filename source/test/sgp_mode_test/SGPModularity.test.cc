#include "../../sgp_mode/CPUState.h"
#include "../../sgp_mode/Instructions.h"
#include "../../sgp_mode/Tasks.h"
#include "../../sgp_mode/CPU.h"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPSymbiont.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/ModularityAnalysis.h"
#include "../../default_mode/Symbiont.h"
#include "../../default_mode/Host.h"
#include "../../../Empirical/include/emp/config/ArgManager.hpp"
#include <iostream>
#include <memory>
#include "../../sgp_mode/Scheduler.h"

//have world call these methods

TEST_CASE("GetNumSites", "[sgp]"){
       
        WHEN("last position is in a site"){
        emp::vector<int> useful_genome = {1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,1};
        int site_count = GetNumSites(useful_genome);
        REQUIRE(site_count==3);
        }

        WHEN("when last position is a one-instruction size site"){
        emp::vector<int> useful_genome = {1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,0,1};
        int site_count = GetNumSites(useful_genome);
        REQUIRE(site_count==4);
        }

}

TEST_CASE("GetUsefulStarts", "[sgp]"){
        emp::vector<int> useful_genome_a = {1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,0,1};
        emp::vector<int> useful_genome_b = {0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,0,1};
        emp::vector<int> useful_genome_c = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,1};
        emp::vector<int> useful_genome_d = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
        emp::vector<int> useful_genome_e = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

        emp::vector<emp::vector<int>> useful_genomes = {useful_genome_a,useful_genome_b,useful_genome_c,useful_genome_d,useful_genome_e};

        emp::vector<int> calc_useful_starts = GetUsefulStarts(useful_genomes);
        emp::vector<int> true_useful_starts = {0,6,useful_genome_c.size()-5,useful_genome_d.size()-1, 0};

        REQUIRE(calc_useful_starts == true_useful_starts);

}

TEST_CASE("GetUsefulEnds", "[sgp]"){
        emp::vector<int> useful_genome_a = {1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,0,1};
        emp::vector<int> useful_genome_b = {0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0};
        emp::vector<int> useful_genome_c = {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,1,0,0};
        emp::vector<int> useful_genome_d = {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        emp::vector<int> useful_genome_e = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

        emp::vector<emp::vector<int>> useful_genomes = {useful_genome_a,useful_genome_b,useful_genome_c,useful_genome_d,useful_genome_e};

        emp::vector<int> calc_useful_ends = GetUsefulEnds(useful_genomes);
        emp::vector<int> true_useful_ends = {useful_genome_a.size()-1,9,useful_genome_c.size()-3,0, 0};

        REQUIRE(calc_useful_ends == true_useful_ends);

}

TEST_CASE("GetSumSiteDist", "[sgp]"){
        
        emp::vector<int> useful_genome_a = {0,0,0,0,1,0,1,1,1,1,0,0,0,0,0,1,1,1,0,1};
        emp::vector<int> useful_genome_b = {0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0};
        emp::vector<int> useful_genome_c = {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,1,0,0};
        emp::vector<int> useful_genome_d = {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
        emp::vector<int> useful_genome_e = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

        emp::vector<emp::vector<int>> useful_genomes = {useful_genome_a,useful_genome_b,useful_genome_c,useful_genome_d,useful_genome_e};
        emp::vector<int> calc_useful_starts = GetUsefulStarts(useful_genomes);
        emp::vector<int> calc_useful_ends = GetUsefulEnds(useful_genomes);
        
        emp::vector<int> test_distances = {};
        emp::vector<int> true_distances = {7,0,2,useful_genome_d.size()-2,0};
        
        for(int i = 0; i<=useful_genomes.size()-1;i++){
                test_distances.push_back(GetSumSiteDist(calc_useful_starts[i],calc_useful_ends[i], useful_genomes[i]));
        }
        
        REQUIRE(test_distances==true_distances);

}

TEST_CASE("GetSummedValue", "[sgp]"){
        emp::vector<int> useful_genome_a = {0,0,0,0,1,0,1,1,1,1,0,0,0,0,0,1,1,1,0,1};
        emp::vector<int> useful_genome_b = {0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0};
        emp::vector<int> useful_genome_c = {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,1,0,0};
        emp::vector<int> useful_genome_d = {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
        emp::vector<int> useful_genome_e = {0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0};

        emp::vector<emp::vector<int>> useful_genomes = {useful_genome_a,useful_genome_b,useful_genome_c,useful_genome_d,useful_genome_e};
        emp::vector<int> calc_useful_starts = GetUsefulStarts(useful_genomes);
        emp::vector<int> calc_useful_ends = GetUsefulEnds(useful_genomes);

        double test_sum = GetSummedValue(useful_genomes.size(), calc_useful_starts, calc_useful_ends, useful_genomes);
        double found_value = 10.5833333333;
        
        REQUIRE(test_sum==Approx(found_value));



}

TEST_CASE("CalcPModularity", "[sgp]"){
        double summed_value = 10.58333;
        int task_num = 5;
        int genome_size = 20;

        double test_phys_mod_a = CalcPModularity(task_num, summed_value, genome_size);
        double found_value_a = 0.788333;
        REQUIRE(test_phys_mod_a==Approx(found_value_a));
        

        summed_value = 1.58333;
        task_num=4;
        double test_phys_mod_b = CalcPModularity(task_num, summed_value, genome_size);
        double found_value_b = 0.96041675;
        REQUIRE(test_phys_mod_b==Approx(found_value_b));
        

        summed_value = 0.0;
        double test_phys_mod_c = CalcPModularity(task_num, summed_value, genome_size);
        double found_value_c = -1;
        REQUIRE(test_phys_mod_c==Approx(found_value_c));


}

TEST_CASE("GetPModularity", "[sgp]"){
        emp::vector<int> useful_genome_a = {0,0,0,0,1,0,1,1,1,1,0,0,0,0,0,1,1,1,0,1};
        emp::vector<int> useful_genome_b = {0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0};
        emp::vector<int> useful_genome_c = {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,1,0,0};
        emp::vector<int> useful_genome_d = {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
        emp::vector<int> useful_genome_e = {0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0};

        emp::vector<emp::vector<int>> useful_genomes = {useful_genome_a,useful_genome_b,useful_genome_c,useful_genome_d,useful_genome_e};
        
        int num_tasks = 5;
        double test_phys_mod_a = GetPModularity(num_tasks, useful_genomes);
        double found_value_a = 0.788333;
        REQUIRE(test_phys_mod_a==Approx(found_value_a));

        num_tasks = 4;
        useful_genomes.pop_back();
        useful_genomes.pop_back();
        useful_genomes.push_back(useful_genome_e);

        double test_phys_mod_b = GetPModularity(num_tasks, useful_genomes);
        double found_value_b = 0.96041675;
        REQUIRE(test_phys_mod_b==Approx(found_value_b));


}

TEST_CASE("GetNecessaryInstructions", "[sgp]"){
        emp::Random random(10);
        SymConfigBase config;
        int world_size = 1;
        int pop_size = 1;
        TaskSet test_tasks = LogicTasks;
        SGPWorld world(random, &config, test_tasks);
        world.resize(world_size);
        config.POP_SIZE(pop_size);

        emp::Ptr<SGPHost> test_sample = emp::NewPtr<SGPHost>(&random, &world, &config);
        std::cout<< world.GetFullPop().size();
        std::cout<<"-Alamo ";
     

        size_t test_id = 0;

        emp::vector<int> program_position_guide = GetNecessaryInstructions(test_sample, test_id, test_tasks); //gives Segfault currently 
        //issue with RunCpuStep
        


}





TEST_CASE("GetReducedProgramRepresentations", "[sgp]"){
        emp::Random random(-1);
        SymConfigBase config;
        int world_size = 1;
        int pop_size = 1;
        TaskSet test_tasks = LogicTasks;
        SGPWorld world(random, &config, test_tasks);
        world.resize(world_size);
        config.POP_SIZE(pop_size);

        emp::vector<emp::vector<int>> test_map = {};
        emp::Ptr<SGPHost> test_sample = emp::NewPtr<SGPHost>(&random, &world, &config);
        std::cout<< world.GetFullPop().size();
        test_map = GetReducedProgramRepresentations(test_sample);

        std::cout<<"-Malpha";

        //that it works for both hosts and syms but just hosts is fine for now

        //when there are tasks
        //it returns a vector of vectors filled with 0 and 1 all the same length as the full genome
        //that it only gets the tasks that the host can actually do
                //compare to tag of the host
        //that it doesn't crash due to unexpected issues like: the host not having a cpu or a program

       //when there aren't ->return empty vector of vectors




}



