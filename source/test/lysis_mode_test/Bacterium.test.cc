#include "../../lysis_mode/Bacterium.h"

TEST_CASE("Bacterium constructor, host_incorporation_val", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(5);
    LysisWorld w(*random);
    SymConfigBase config;
    LysisWorld * world = &w;
    double int_val = -1;

    config.HOST_INC_VAL(-1);
    Bacterium * b = new Bacterium(random, world, &config, int_val);
    CHECK(b->GetIncVal() >= 0.0);
    CHECK(b->GetIncVal() <= 1.0);
    CHECK(b->GetAge() == 0); 
    CHECK(b->GetPoints() == 0);

    config.HOST_INC_VAL(0.8);
    Bacterium * b2 = new Bacterium(random, world, &config, int_val);
    double expected_inc_val = 0.8;
    CHECK(b2->GetIncVal()==expected_inc_val);
    CHECK(b2->GetAge() == 0); 
    CHECK(b2->GetPoints() == 0);

    delete b;
    delete b2;
}

TEST_CASE("Bacterium SetIncVal, GetIncVal", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(4);
    LysisWorld w(*random);
    LysisWorld * world = &w;
    SymConfigBase config;
    double int_val = -1;
    emp::Ptr<Bacterium> b = new Bacterium(random, world, &config, int_val);

    double host_incorporation_val = 0.2;
    b->SetIncVal(host_incorporation_val);
    double expected_inc_val = 0.2;
    REQUIRE(b->GetIncVal()==expected_inc_val);

}

TEST_CASE("Bacterium mutate", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(12);
    LysisWorld w(*random);
    LysisWorld * world = &w;
    SymConfigBase config;
    double int_val = 0;
    double orig_host_inc_val = 0.5;
    config.HOST_INC_VAL(orig_host_inc_val);


    WHEN("Mutation rate is not zero and host_inc_val mutations are enabled"){
        config.MUTATION_SIZE(0.002);
        config.MUTATION_RATE(1);
        config.MUTATE_INC_VAL(1);
        emp::Ptr<Organism> b = new Bacterium(random, world, &config, int_val);
        b->mutate();

        THEN("Then mutation occurs and the bacterium's host_inc_val mutates"){
            REQUIRE(b->GetIncVal() != orig_host_inc_val);
            REQUIRE(b->GetIncVal() >= 0.0);
            REQUIRE(b->GetIncVal() <= 1.0);
        }

    delete b;
    }

    WHEN("Mutation rate is not zero and host_inc_val mutations are not enabled"){
        config.MUTATION_SIZE(0.002);
        config.MUTATION_RATE(1);
        config.MUTATE_INC_VAL(0);
        emp::Ptr<Organism> b = new Bacterium(random, world, &config, int_val);
        b->mutate();

        THEN("Then mutations occur but do not occur in the host_inc_val"){
            REQUIRE(b->GetIncVal() ==  orig_host_inc_val);
        }

    delete b;
    }

    WHEN("Mutation rate is 0 and host_inc_val mutations are enabled"){
        config.MUTATION_RATE(0.0);
        config.MUTATION_SIZE(0.0);
        config.MUTATE_INC_VAL(1);
        emp::Ptr<Organism> b = new Bacterium(random, world, &config, int_val);
        b->mutate();

        THEN("Mutations do not occur"){
            REQUIRE(b->GetIncVal() ==  orig_host_inc_val);
        }

    delete b;
    }

    WHEN("Mutation rate is 0 and host_inc_val mutations are not enabled"){
        config.MUTATION_RATE(0.0);
        config.MUTATION_SIZE(0.0);
        config.MUTATE_INC_VAL(0);
        emp::Ptr<Organism> b = new Bacterium(random, world, &config, int_val);
        b->mutate();

        THEN("Mutations do not occur"){
            REQUIRE(b->GetIncVal() ==  orig_host_inc_val);
        }

    delete b;
    }
}

TEST_CASE("ProcessLysogenResources", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(12);
    LysisWorld w(*random);
    LysisWorld * world = &w;
    SymConfigBase config;
    config.HOST_INC_VAL(0);
    config.SYNERGY(2);

    double orig_host_resources = 10;
    double int_val = 0;
    emp::Ptr<Bacterium> b = new Bacterium(random, world, &config, int_val);

    WHEN("The incorporation values are similar"){
        double phage_inc_val = 0;
        b->SetResInProcess(orig_host_resources);
        double expected_resources = 20; //synergy * b->GetResInProcess() * (1 - abs(host_inc_val - phage_inc_val))

        THEN("The host resources increase"){
            REQUIRE(b->ProcessLysogenResources(phage_inc_val) == expected_resources);
            REQUIRE(b->GetResInProcess() == 0);
        }
    }

    WHEN("The incorporation values are neutral"){
        double phage_inc_val = 0.5;
        b->SetResInProcess(orig_host_resources);
        double expected_resources = 10; //synergy * b->GetResInProcess() * (1 - abs(host_inc_val - phage_inc_val))

        THEN("The host resources stay the same"){
            REQUIRE(b->ProcessLysogenResources(phage_inc_val) == expected_resources);
            REQUIRE(b->GetResInProcess() == 0);
        }
    }

    WHEN("The incorporation values are far apart"){
        double phage_inc_val = 1;
        b->SetResInProcess(orig_host_resources);
        double expected_resources = 0; //synergy * b->GetResInProcess() * (1 - abs(host_inc_val - phage_inc_val))

        THEN("The host resources are diminished"){
            REQUIRE(b->ProcessLysogenResources(phage_inc_val) == expected_resources);
            REQUIRE(b->GetResInProcess() == 0);
        }
    }
}

TEST_CASE("Bacterium Process", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(12);
    LysisWorld w(*random);
    LysisWorld * world = &w;
    SymConfigBase config;
    size_t location = 0;

    WHEN("The bacterium doesn't have a symbiont"){

        WHEN("The bacterium doesn't have enough resources to reproduce"){
            int res_distribute = 5;
            config.HOST_REPRO_RES(10);
            config.RES_DISTRIBUTE(5);

            double int_val = 0;
            emp::Ptr<Bacterium> b = new Bacterium(random, world, &config, int_val);
            b->Process(location);

            THEN("The bacterium's points increase by res distribute"){
                REQUIRE(b->GetPoints() == res_distribute);
            }
        }

        WHEN("The bacterium does have enough resources to reproduce"){
            int res_distribute = 10;
            config.HOST_REPRO_RES(10);
            world->SetResPerUpdate(res_distribute);

            double int_val = 0;
            emp::Ptr<Bacterium> b = new Bacterium(random, world, &config, int_val);
            b->Process(location);

            THEN("The bacterium will reproduce and its points will be set to zero"){
                REQUIRE(b->GetPoints() == 0);
            }
        }
    }
}

TEST_CASE("Phage Exclude", "[lysis]") {
    emp::Ptr<emp::Random> random = new emp::Random(3);
    SymWorld w(*random);

    SymConfigBase config;
    int sym_limit = 4;
    config.SYM_LIMIT(sym_limit);

    double int_val = 0;

    WHEN("Phage exclude is set to false"){
      bool phage_exclude = 0;
      config.PHAGE_EXCLUDE(phage_exclude);
      Host * h = new Host(random, &w, &config, int_val);

      THEN("syms are added without issue"){
        for(int i = 0; i < sym_limit; i++){
          h->AddSymbiont(new Symbiont(random, &w, &config, int_val));
        }
        int num_syms = (h->GetSymbionts()).size();

        REQUIRE(num_syms==sym_limit);
        //with random seed 3 and phage exclusion on,
        //num_syms not reach the sym_limit (would be 2 not 4)
      }
    }

    WHEN("Phage exclude is set to true"){
      bool phage_exclude = 1;
      config.PHAGE_EXCLUDE(phage_exclude);

      THEN("syms have a decreasing change of entering the host"){
        int goal_num_syms[] = {3,3,3,3};

        for(int i = 0; i < 4; i ++){
          emp::Ptr<emp::Random> random = new emp::Random(i+1);
          SymWorld w(*random);

          Host * h = new Host(random, &w, &config, int_val);
          for(double i = 0; i < 10; i++){
            h->AddSymbiont(new Symbiont(random, &w, &config, int_val));
          }
          int host_num_syms = (h->GetSymbionts()).size();

          REQUIRE(goal_num_syms[i] == host_num_syms);
        }
      }
    }
}
