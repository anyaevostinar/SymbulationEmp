#include "../../default_mode/Host.h"
#include "../../default_mode/Symbiont.h"
#include "../../sgp_mode/AnalysisTools.h"
#include "../../sgp_mode/CPU.h"
#include "../../sgp_mode/CPUState.h"
#include "../../sgp_mode/GenomeLibrary.h"
#include "../../sgp_mode/Instructions.h"
#include "../../sgp_mode/ModularityAnalysis.h"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPSymbiont.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/Scheduler.h"
#include "../../sgp_mode/Tasks.h"
#include <iostream>
#include <memory>



TEST_CASE("GetUsefulStarts", "[sgp]") {
  GIVEN("A vector of 1s and 0s representing every site (instruction) in the "
        "actual genome that is either"
        "necessary to perform the designated task or not necessary to perform "
        "the task, respectively") {
    emp::vector<int> needed_code_sites_a = {1, 0, 0, 0, 0, 0, 1, 1, 1, 1,
                                            0, 0, 0, 0, 0, 1, 1, 1, 0, 1};
    emp::vector<int> needed_code_sites_b = {0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
                                            0, 0, 0, 0, 0, 1, 1, 1, 0, 1};
    emp::vector<int> needed_code_sites_c = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 1, 1, 1, 0, 1};
    emp::vector<int> needed_code_sites_d = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
    emp::vector<int> needed_code_sites_e = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    emp::vector<emp::vector<int>> needed_code_sites = {
        needed_code_sites_a, needed_code_sites_b, needed_code_sites_c,
        needed_code_sites_d, needed_code_sites_e};

    emp::vector<int> calc_useful_starts = GetUsefulStarts(needed_code_sites);
    emp::vector<int> true_useful_starts = {0, 6, (int)needed_code_sites_c.size() - 5,
                                           (int)needed_code_sites_d.size() - 1};

    REQUIRE(calc_useful_starts == true_useful_starts);
  }
}

TEST_CASE("GetUsefulEnds", "[sgp]") {
  GIVEN("A vector of 1s and 0s representing every site (instruction) in the "
        "actual genome that is either"
        "necessary to perform the designated task or not necessary to perform "
        "the task, respectively") {
    emp::vector<int> needed_code_sites_a = {1, 0, 0, 0, 0, 0, 1, 1, 1, 1,
                                            0, 0, 0, 0, 0, 1, 1, 1, 0, 1};
    emp::vector<int> needed_code_sites_b = {0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    emp::vector<int> needed_code_sites_c = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 1, 0, 0, 1, 1, 1, 0, 0};
    emp::vector<int> needed_code_sites_d = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    emp::vector<int> needed_code_sites_e = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    emp::vector<emp::vector<int>> needed_code_sites = {
        needed_code_sites_a, needed_code_sites_b, needed_code_sites_c,
        needed_code_sites_d, needed_code_sites_e};

    emp::vector<int> calc_useful_ends = GetUsefulEnds(needed_code_sites);
    emp::vector<int> true_useful_ends = {(int)needed_code_sites_a.size() - 1, 9,
                                         (int)needed_code_sites_c.size() - 3, 0};

    REQUIRE(calc_useful_ends == true_useful_ends);
  }
}

TEST_CASE("GetNumSites", "[sgp]") {
  GIVEN("A vector of 1s and 0s representing every site (instruction) in the "
        "actual genome that is either"
        "necessary to perform the designated task or not necessary to perform "
        "the task, respectively") {

    WHEN("there are necessary sites") {
      emp::vector<int> needed_code_sites = {1, 0, 0, 0, 0, 0, 1, 1, 1, 1,
                                            0, 0, 0, 0, 0, 1, 1, 1, 1, 1};
      int site_count = GetNumSites(0, 19, needed_code_sites);

      THEN("GetNumsites returns a number > 0") { REQUIRE(site_count == 10); }
    }

    WHEN("there are no necessary sites") {
      emp::vector<int> needed_code_sites = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
      int site_count = GetNumSites(0, 11, needed_code_sites);

      THEN("GetNumsites returns 0") { REQUIRE(site_count == 0); }
    }
  }
}

TEST_CASE("GetDistance", "[sgp]") {
  GIVEN(
      "For when the genome is treated as circular, the max distance is 1/2 the"
      "genome length, and the genome_length is always 100") {
    int length = 100;
    int site_i = 0;
    int site_j = 0;
    int expected_distance = 0;
    int obtained_distance = 0;

    WHEN("the distance value (i-j) is less than the max distance") {
      site_i = 50;
      site_j = 40;
      expected_distance = 10;
      obtained_distance = GetDistance(site_i, site_j, length);

      THEN("Get distance returns a value less than the max distance") {
        REQUIRE(obtained_distance == expected_distance);
      }
    }

    WHEN("The distance value (i-j) is more than the max distance") {
      site_i = 1;
      site_j = 71;
      expected_distance = 30;
      obtained_distance = GetDistance(site_i, site_j, length);

      THEN("It returns a value less than or equal to the max distance") {
        REQUIRE(obtained_distance == expected_distance);
      }
    }

    WHEN("there is no distance between the sites") {
      site_i = 30;
      site_j = 30;
      expected_distance = 0;
      obtained_distance = GetDistance(site_i, site_j, length);

      THEN("There is no distance returned") {
        REQUIRE(obtained_distance == expected_distance);
      }
    }
  }
}

TEST_CASE("GetPModularity", "[sgp]") {
  GIVEN("A vector of 1s and 0s representing every site (instruction) in the "
        "actual genome that is either"
        "necessary to perform the designated task or not necessary to perform "
        "the task, respectively") {
    emp::vector<int> needed_code_sites_b = {0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    emp::vector<int> needed_code_sites_c = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            1, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    emp::vector<int> needed_code_sites_d = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
    emp::vector<int> needed_code_sites_e = {0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    emp::vector<emp::vector<int>> needed_code_sites = {needed_code_sites_b};


    WHEN("The only reduced program that can do a task is needed_code_sites_b") {
      double test_phys_mod_a = GetPModularity(needed_code_sites);
      double found_value_a = 0.8333333;

      THEN("The physical modularity will be high") { REQUIRE(test_phys_mod_a == Approx(found_value_a)); }
    }

    needed_code_sites.push_back(needed_code_sites_d);
    WHEN("When there are two reduced program that can do tasks") {

      double test_phys_mod_b = GetPModularity(needed_code_sites);
      double found_value_b = 0.8666666667;
      THEN("The physical modularity will inc as compared to first case due to n_c_s_d")
       { REQUIRE(test_phys_mod_b == Approx(found_value_b)); }
    }

    needed_code_sites.pop_back();
    needed_code_sites.pop_back();
    needed_code_sites.push_back(needed_code_sites_e);
    WHEN("The only reduced program that can do a task has one site") {
      double test_phys_mod_c = GetPModularity(needed_code_sites);
      double found_value_c = 1.0;

      THEN("There is perfect physical modularity") { REQUIRE(test_phys_mod_c == Approx(found_value_c)); }
    }

    needed_code_sites.pop_back();
    needed_code_sites.push_back(needed_code_sites_c);
    WHEN("The only reduced program that can do a task has the max distance") {
      double test_phys_mod_d = GetPModularity(needed_code_sites);
      double found_value_d = 0.0;

      THEN("There is no physical modularity") { REQUIRE(test_phys_mod_d == Approx(found_value_d)); }
    }
  }
}

TEST_CASE("GetPMFromCPU", "[sgp]") {

  WHEN("The only task is the basic not genome") {
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

    emp::Ptr<SGPHost> test_sample =
        emp::NewPtr<SGPHost>(&random, &world, &config);

    double expected_phys_mod = .956666667;
    double test_phys_mod = GetPMFromCPU(test_sample->GetCPU());

    THEN("Physical modularity is extremely high") { REQUIRE(Approx(expected_phys_mod) == test_phys_mod); }
  }

  WHEN("It is a random genome and the host can do no task") {
    emp::Random random(5);
    SymConfigBase config;
    config.RANDOM_ANCESTOR(true);
    int world_size = 1;
    int pop_size = 1;
    TaskSet test_tasks = LogicTasks;
    SGPWorld world(random, &config, test_tasks);
    world.resize(world_size);
    config.POP_SIZE(pop_size);

    emp::Ptr<SGPHost> test_sample =
        emp::NewPtr<SGPHost>(&random, &world, &config);
    double expected_phys_mod = -1.0;
    double test_phys_mod = GetPMFromCPU(test_sample->GetCPU());

    THEN("The method cannot take the program's physical modularity") { REQUIRE(Approx(expected_phys_mod) == test_phys_mod); }
  }

  WHEN("using logic tasks") {

    emp::Random random(5);
    SymConfigBase config;
    config.RANDOM_ANCESTOR(false);
    int world_size = 1;
    int pop_size = 1;
    size_t length = 100;
    TaskSet test_tasks = LogicTasks;
    SGPWorld world(random, &config, test_tasks);
    world.resize(world_size);
    config.POP_SIZE(pop_size);

    emp::Ptr<SGPHost> test_sample =
        emp::NewPtr<SGPHost>(&random, &world, &config);

    WHEN("The genome is a Nand") {
      ProgramBuilder builder;
      builder.AddNand();
      sgpl::Program<Spec> test_program = builder.Build(length);
      CPU temp_cpu =
          CPU(test_sample->GetCPU().state.host,
              test_sample->GetCPU().state.world, test_program);
      test_sample->GetCPU() = temp_cpu;

      double expected_phys_mod = .952;
      double test_phys_mod = GetPMFromCPU(test_sample->GetCPU());

      THEN("The program has high physical modularity") 
      { REQUIRE(Approx(expected_phys_mod) == test_phys_mod); }
    }

    WHEN("The genome is one of the other logic tasks only") {
      ProgramBuilder builder;
      builder.AddOrn();
      sgpl::Program<Spec> test_program = builder.Build(length);
      CPU temp_cpu =
          CPU(test_sample->GetCPU().state.host,
              test_sample->GetCPU().state.world,test_program);
      test_sample->GetCPU() = temp_cpu;

      double expected_phys_mod = .9466667;
      double test_phys_mod = GetPMFromCPU(test_sample->GetCPU());

      THEN("The physical modularity will be lower than the basic tasks'") 
      { REQUIRE(Approx(expected_phys_mod) == test_phys_mod); }
    }
  }
}

TEST_CASE("GetFModularity", "[sgp]") {
  GIVEN("A vector of 1s and 0s representing every site (instruction) in the "
        "actual genome that is either"
        "necessary to perform the designated task or not necessary to perform "
        "the task, respectively") {
    emp::vector<int> needed_code_sites_a = {0, 0, 0, 0, 1, 0, 1, 1, 1, 1,
                                            0, 0, 0, 0, 0, 1, 1, 1, 0, 1};
    emp::vector<int> needed_code_sites_b = {0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    emp::vector<int> needed_code_sites_c = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            1, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    emp::vector<int> needed_code_sites_d = {1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    emp::vector<int> needed_code_sites_e = {0, 0, 0, 0, 0, 0, 0, 1, 1, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    emp::vector<int> needed_code_sites_f = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    emp::vector<emp::vector<int>> needed_code_sites = {needed_code_sites_b,
                                                       needed_code_sites_e};

    WHEN("There is partial overlap between the two reduced programs") {
      double test_funct_mod_a = GetFModularity(needed_code_sites);
      double found_value_a = 0.05;

      THEN("There is low functional modularity") { REQUIRE(test_funct_mod_a == Approx(found_value_a)); }
    }
    // change to conceptual framework and add thens
    needed_code_sites.pop_back();
    needed_code_sites.push_back(needed_code_sites_c);
    WHEN("There is no overlap between the two reduced programs") {
      double test_funct_mod_b = GetFModularity(needed_code_sites);
      double found_value_b = 0.15;

      THEN("There is high functional modularity") { REQUIRE(test_funct_mod_b == Approx(found_value_b)); }
    }
    needed_code_sites.push_back(needed_code_sites_d);
    WHEN("There are more than 2 succesful genomes, partial overlap") {
      double test_funct_mod_c = GetFModularity(needed_code_sites);
      double found_value_c = 0.1;

      THEN("There is medium functional modularity") { REQUIRE(test_funct_mod_c == Approx(found_value_c)); }
    }

    needed_code_sites.pop_back();
    needed_code_sites.pop_back();
    needed_code_sites.pop_back();
    needed_code_sites.push_back(needed_code_sites_a);
    needed_code_sites.push_back(needed_code_sites_a);
    WHEN(
        "There is complete overlap between the  2 reduced programs that can do "
        "tasks") {
      double test_funct_mod_d = GetFModularity(needed_code_sites);
      double found_value_d = 0.0;

      THEN("The functional modularity is 0.0") { REQUIRE(test_funct_mod_d == Approx(found_value_d)); }
    }
  }
}

TEST_CASE("GetFMFromCPU", "[sgp]") {

  WHEN("A genome that can perform Nand & Not is passed in") {
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

    emp::Ptr<SGPHost> test_sample =
        emp::NewPtr<SGPHost>(&random, &world, &config);
    ProgramBuilder builder;
    builder.AddNand();
    builder.AddNot();
    sgpl::Program<Spec> test_program = builder.Build(length);
    CPU temp_cpu =
        CPU(test_sample->GetCPU().state.host, test_sample->GetCPU().state.world,test_program);
    test_sample->GetCPU() = temp_cpu;

    double expected_phys_mod = 0.025;
    double test_phys_mod = GetFMFromCPU(test_sample->GetCPU());

    THEN("Functional Modularity is very low") { REQUIRE(Approx(expected_phys_mod) == test_phys_mod); }
  }
}
