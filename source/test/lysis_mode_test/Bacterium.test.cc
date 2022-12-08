#include "../../lysis_mode/Bacterium.h"

TEST_CASE("Bacterium constructor, host_incorporation_val", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(5);
    SymConfigLysis config;
    LysisWorld w(*random, &config);
    LysisWorld * world = &w;
    double int_val = -1;

    config.HOST_INC_VAL(-1);
    emp::Ptr<Bacterium> bacterium = emp::NewPtr<Bacterium>(random, world, &config, int_val);
    CHECK(bacterium->GetIncVal() >= 0.0);
    CHECK(bacterium->GetIncVal() <= 1.0);
    CHECK(bacterium->GetAge() == 0);
    CHECK(bacterium->GetPoints() == 0);

    config.HOST_INC_VAL(0.8);
    emp::Ptr<Bacterium> bacterium2 = emp::NewPtr<Bacterium>(random, world, &config, int_val);
    double expected_inc_val = 0.8;
    CHECK(bacterium2->GetIncVal()==expected_inc_val);
    CHECK(bacterium2->GetAge() == 0);
    CHECK(bacterium2->GetPoints() == 0);

    bacterium.Delete();
    bacterium2.Delete();
}

TEST_CASE("Bacterium SetIncVal, GetIncVal", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(4);
    SymConfigLysis config;
    LysisWorld w(*random, &config);
    LysisWorld * world = &w;
    double int_val = -1;
    emp::Ptr<Bacterium> bacterium = emp::NewPtr<Bacterium>(random, world, &config, int_val);

    double host_incorporation_val = 0.2;
    bacterium->SetIncVal(host_incorporation_val);
    double expected_inc_val = 0.2;
    REQUIRE(bacterium->GetIncVal()==expected_inc_val);

    bacterium.Delete();
}

TEST_CASE("Bacterium mutate", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(12);
    SymConfigLysis config;
    LysisWorld w(*random, &config);
    LysisWorld * world = &w;
    double int_val = 0;
    double orig_host_inc_val = 0.5;
    config.HOST_INC_VAL(orig_host_inc_val);


    WHEN("Mutation rate is not zero and host_inc_val mutations are enabled"){
        config.MUTATION_SIZE(0.002);
        config.MUTATION_RATE(1);
        config.MUTATE_INC_VAL(1);

        emp::Ptr<Organism> bacterium = emp::NewPtr<Bacterium>(random, world, &config, int_val);
        bacterium->Mutate();

        THEN("Then mutation occurs and the bacterium's host_inc_val mutates"){
            REQUIRE(bacterium->GetIncVal() != orig_host_inc_val);
            REQUIRE(bacterium->GetIncVal() >= 0.0);
            REQUIRE(bacterium->GetIncVal() <= 1.0);
        }

    bacterium.Delete();
    }

    WHEN("Mutation rate is not zero and host_inc_val mutations are not enabled"){
        config.MUTATION_SIZE(0.002);
        config.MUTATION_RATE(1);
        config.MUTATE_INC_VAL(0);
        emp::Ptr<Organism> bacterium = emp::NewPtr<Bacterium>(random, world, &config, int_val);
        bacterium->Mutate();

        THEN("Then mutations occur but do not occur in the host_inc_val"){
            REQUIRE(bacterium->GetIncVal() ==  orig_host_inc_val);
        }

    bacterium.Delete();
    }

    WHEN("Mutation rate is 0 and host_inc_val mutations are enabled"){
        config.MUTATION_RATE(0.0);
        config.MUTATION_SIZE(0.0);
        config.MUTATE_INC_VAL(1);
        emp::Ptr<Organism> bacterium = emp::NewPtr<Bacterium>(random, world, &config, int_val);
        bacterium->Mutate();

        THEN("Mutations do not occur"){
            REQUIRE(bacterium->GetIncVal() ==  orig_host_inc_val);
        }

    bacterium.Delete();
    }

    WHEN("Mutation rate is 0 and host_inc_val mutations are not enabled"){
        config.MUTATION_RATE(0.0);
        config.MUTATION_SIZE(0.0);
        config.MUTATE_INC_VAL(0);
        emp::Ptr<Organism> bacterium = emp::NewPtr<Bacterium>(random, world, &config, int_val);
        bacterium->Mutate();

        THEN("Mutations do not occur"){
            REQUIRE(bacterium->GetIncVal() ==  orig_host_inc_val);
        }

    bacterium.Delete();
  }
}

TEST_CASE("ProcessLysogenResources", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(12);
    SymConfigLysis config;
    LysisWorld w(*random, &config);
    LysisWorld * world = &w;
    config.HOST_INC_VAL(0);
    config.SYNERGY(2);

    double orig_host_resources = 10;
    double int_val = 0;
    emp::Ptr<Bacterium> bacterium = emp::NewPtr<Bacterium>(random, world, &config, int_val);

    WHEN("The incorporation values are similar"){
        double phage_inc_val = 0;
        bacterium->SetResInProcess(orig_host_resources);
        double expected_resources = 20; //synergy * bacterium->GetResInProcess() * (1 - abs(host_inc_val - phage_inc_val))

        THEN("The host resources increase"){
            REQUIRE(bacterium->ProcessLysogenResources(phage_inc_val) == expected_resources);
            REQUIRE(bacterium->GetResInProcess() == 0);
        }
    }

    WHEN("The incorporation values are neutral"){
        double phage_inc_val = 0.5;
        bacterium->SetResInProcess(orig_host_resources);
        double expected_resources = 10; //synergy * bacterium->GetResInProcess() * (1 - abs(host_inc_val - phage_inc_val))

        THEN("The host resources stay the same"){
            REQUIRE(bacterium->ProcessLysogenResources(phage_inc_val) == expected_resources);
            REQUIRE(bacterium->GetResInProcess() == 0);
        }
    }

    WHEN("The incorporation values are far apart"){
        double phage_inc_val = 1;
        bacterium->SetResInProcess(orig_host_resources);
        double expected_resources = 0; //synergy * bacterium->GetResInProcess() * (1 - abs(host_inc_val - phage_inc_val))

        THEN("The host resources are diminished"){
            REQUIRE(bacterium->ProcessLysogenResources(phage_inc_val) == expected_resources);
            REQUIRE(bacterium->GetResInProcess() == 0);
        }
    }
    bacterium.Delete();
}

TEST_CASE("Bacterium Process", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(12);
    SymConfigLysis config;
    LysisWorld w(*random, &config);
    LysisWorld * world = &w;
    size_t location = 0;

    WHEN("The bacterium doesn't have a symbiont"){

        WHEN("The bacterium doesn't have enough resources to reproduce"){
            int res_distribute = 5;
            config.HOST_REPRO_RES(10);
            config.RES_DISTRIBUTE(5);

            double int_val = 0;
            emp::Ptr<Bacterium> bacterium = emp::NewPtr<Bacterium>(random, world, &config, int_val);
            bacterium->Process(location);

            THEN("The bacterium's points increase by res distribute"){
                REQUIRE(bacterium->GetPoints() == res_distribute);
            }
            bacterium.Delete();
        }

        WHEN("The bacterium does have enough resources to reproduce"){
            int res_distribute = 10;
            config.HOST_REPRO_RES(10);
            config.RES_DISTRIBUTE(res_distribute);

            double int_val = 0;
            emp::Ptr<Bacterium> bacterium = emp::NewPtr<Bacterium>(random, world, &config, int_val);
            bacterium->Process(location);

            THEN("The bacterium will reproduce and its points will be set to zero"){
                REQUIRE(bacterium->GetPoints() == 0);
            }
            bacterium.Delete();
        }
    }
}

TEST_CASE("Phage Exclude", "[lysis]") {
    SymConfigLysis config;
    int sym_limit = 4;
    config.SYM_LIMIT(sym_limit);

    double int_val = 0;

    WHEN("Phage exclude is set to false"){
      emp::Ptr<emp::Random> random = new emp::Random(3);
      LysisWorld world(*random, &config);

      bool phage_exclude = 0;
      config.PHAGE_EXCLUDE(phage_exclude);
      emp::Ptr<Bacterium> bacterium = emp::NewPtr<Bacterium>(random, &world, &config, int_val);

      THEN("syms are added without issue"){
        for(int i = 0; i < sym_limit; i++){
          bacterium->AddSymbiont(emp::NewPtr<Phage>(random, &world, &config, int_val));
        }
        int num_syms = (bacterium->GetSymbionts()).size();

        REQUIRE(num_syms==sym_limit);

        bacterium.Delete();
      }
    }

    WHEN("Phage exclude is set to true"){
      bool phage_exclude = 1;
      config.PHAGE_EXCLUDE(phage_exclude);

      THEN("syms have a decreasing change of entering the host"){
        int goal_num_syms[] = {3,3,3,3};

        for(int i = 0; i < 4; i ++){
          emp::Ptr<emp::Random> random = new emp::Random(i+1);
          LysisWorld world(*random, &config);

          emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, int_val);
          for(double i = 0; i < 10; i++){
            host->AddSymbiont(emp::NewPtr<Symbiont>(random, &world, &config, int_val));
          }
          int host_num_syms = (host->GetSymbionts()).size();

          REQUIRE(goal_num_syms[i] == host_num_syms);
          host.Delete();
        }
      }
    }
}

TEST_CASE("Bacterium MakeNew", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigLysis config;
    LysisWorld world(*random, &config);

    double host_int_val = 0.2;
    double host_inc_val = 0.5;
    emp::Ptr<Organism> bacterium = emp::NewPtr<Bacterium>(random, &world, &config, host_int_val);
    bacterium->SetIncVal(host_inc_val);
    emp::Ptr<Organism> new_bacterium = bacterium->MakeNew();
    THEN("The new host has properties of the original host and has 0 points and 0 age"){
      REQUIRE(new_bacterium->GetIntVal() == bacterium->GetIntVal());
      REQUIRE(new_bacterium->GetIncVal() == bacterium->GetIncVal());
      REQUIRE(new_bacterium->GetPoints() == 0);
      REQUIRE(new_bacterium->GetAge() == 0);
      //check that the offspring is the correct class
      REQUIRE(new_bacterium->GetName() == "Bacterium");
    }

    bacterium.Delete();
    new_bacterium.Delete();
}

TEST_CASE("Bacterium reproduce", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigLysis config;
    LysisWorld world(*random, &config);
    config.MUTATION_SIZE(0.002);
    config.MUTATION_RATE(1);
    config.MUTATE_INC_VAL(1);

    double host_int_val = -0.2;
    double host_inc_val = 0.3;
    emp::Ptr<Organism> bacterium = emp::NewPtr<Bacterium>(random, &world, &config, host_int_val);
    bacterium->SetIncVal(host_inc_val);
    emp::Ptr<Organism> bacterium_baby = bacterium->Reproduce();

    THEN("The host baby has a mutated genome and has age and points of 0"){
        REQUIRE(bacterium_baby->GetIntVal() != bacterium->GetIntVal());
        REQUIRE(bacterium_baby->GetIncVal() != bacterium->GetIncVal());
        REQUIRE(bacterium_baby->GetAge() == 0);
        REQUIRE(bacterium_baby->GetPoints() == 0);
    }

    THEN("The host parent's points are set to 0"){
        REQUIRE(bacterium->GetPoints() == 0);
    }

    bacterium.Delete();
    bacterium_baby.Delete();
}
