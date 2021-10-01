#include "default_mode/Host.h"
#include "default_mode/Symbiont.h"
#include <set>

TEST_CASE("Host SetIntVal, GetIntVal") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    SymWorld w(*random);
    double int_val = 1;

    Host * h1 = new Host(random, &w, &config);
    double default_int_val = 0.0;
    REQUIRE(h1->GetIntVal() == default_int_val);

    Host * h2 = new Host(random, &w, &config, int_val);

    double expected_int_val = 1;
    REQUIRE(h2->GetIntVal() == expected_int_val);

    int_val = -0.7;
    h2->SetIntVal(int_val);
    expected_int_val = -0.7;
    REQUIRE(h2->GetIntVal() == expected_int_val);

    int_val = -1.3;
    REQUIRE_THROWS(new Host(random, &w, &config, int_val));

    int_val = 1.8;
    REQUIRE_THROWS(new Host(random, &w, &config, int_val));

}

TEST_CASE("SetPoints, AddPoints, GetPoints") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    SymWorld w(*random);
    double int_val = 1;

    Host * h = new Host(random, &w, &config, int_val);

    double points = 50;
    h->SetPoints(points);
    double expected_points = 50;
    REQUIRE(h->GetPoints() == expected_points);

    points = 76;
    h->AddPoints(points);
    expected_points = 126;
    REQUIRE(h->GetPoints() == expected_points);

}

TEST_CASE("SetResTypes, GetResTypes") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    SymWorld w(*random);
    double int_val = 1;
    emp::vector<emp::Ptr<Organism>> syms = {};
    emp::vector<emp::Ptr<Organism>> repro_syms = {};
    std::set<int> res_types {1,3,5,9,2};

    Host * h = new Host(random, &w, &config, int_val, syms, repro_syms, res_types);

    std::set<int> expected_res_types = h->GetResTypes();
    for (int number : res_types)
    {
        // Tests if each integer from res_types is in expected_res_types
        REQUIRE(expected_res_types.find(number) != expected_res_types.end());
    }

    res_types = {0,1};
    h->SetResTypes(res_types);
    expected_res_types = h->GetResTypes();
    for (int number : res_types)
    {
        // Tests if each integer from res_types is in expected_res_types
        REQUIRE(expected_res_types.find(number) != expected_res_types.end());
    }

}

TEST_CASE("HasSym") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    SymWorld w(*random);
    double int_val = 1;

    WHEN("Host has no symbionts") {
        Host * h = new Host(random, &w, &config, int_val);

        THEN("HasSym is false") {
            bool expected = false;
            REQUIRE(h->HasSym() == expected);
        }
    }
}

TEST_CASE("Host Mutate") {
    //TODO: put in tests for mutation size and mutation rate separately
    emp::Ptr<emp::Random> random = new emp::Random(3);
    SymConfigBase config;
    SymWorld w(*random);
    double int_val = 0;

    Host * h = new Host(random, &w, &config, int_val);
    h->mutate();
    REQUIRE(h->GetIntVal() != int_val);
    REQUIRE(h->GetIntVal() <= 1);
    REQUIRE(h->GetIntVal() >= -1);

    int_val = -0.31;
    h->SetIntVal(int_val);
    h->mutate();
    REQUIRE(h->GetIntVal() != int_val);
    REQUIRE(h->GetIntVal() <= 1);
    REQUIRE(h->GetIntVal() >= -1);

}

TEST_CASE("DistributeResources") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    SymWorld w(*random);

    WHEN("There are no symbionts and interaction value is between 0 and 1") {

        double int_val = 0.6;
        double resources = 80;
        double orig_points = 0; // call this default_points instead? (i'm not setting this val)
        config.SYNERGY(5);

        Host * h = new Host(random, &w, &config, int_val);
        h->DistribResources(resources);

        THEN("Points increase") {
            double expected_points = resources - (resources * int_val); // 48
            double points = h->GetPoints();
            REQUIRE(points == expected_points);
            REQUIRE(points > orig_points);
        }
    }

    WHEN("There are no symbionts and interaction value is 0") {

        double int_val = 0;
        double resources = 10;
        double orig_points = 0;
        config.SYNERGY(5);

        Host * h = new Host(random, &w, &config, int_val);
        h->DistribResources(resources);

        THEN("Resources are added to points") {
            double expected_points = orig_points + resources; // 0
            double points = h->GetPoints();
            REQUIRE(points == expected_points);
        }
    }

    WHEN("There are no symbionts and interaction value is between -1 and 0") {

        double int_val = -0.4;
        double resources = 30;
        double orig_points = 27;
        config.SYNERGY(5);

        Host * h = new Host(random, &w, &config, int_val);
        h->AddPoints(orig_points);
        h->DistribResources(resources);

        THEN("Points increase") {
            double host_defense =  -1.0 * int_val * resources; // the resources spent on defense
            double add_points  = resources - host_defense;
            double expected_points = orig_points + add_points;
            double points = h->GetPoints();
            REQUIRE(points == expected_points);
            REQUIRE(points > orig_points);
        }
    }
}

TEST_CASE("Phage Exclude") {
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

  TEST_CASE("SetResInProcess, GetResInProcess") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    SymWorld w(*random);
    double int_val = 1;

    Host * h = new Host(random, &w, &config, int_val);

    double expected_res_in_process = 0;
    REQUIRE(h->GetResInProcess() == expected_res_in_process);

    h->SetResInProcess(126);
    expected_res_in_process = 126;
    REQUIRE(h->GetResInProcess() == expected_res_in_process);

}

TEST_CASE("Steal resources unit test"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymWorld w(*random);
    SymConfigBase config; 
        

    WHEN ("sym_int_val < host_int_val"){
        double sym_int_val = -0.6;
        
        WHEN("host_int_val > 0"){
            double host_int_val = 0.2;
            Host * h = new Host(random, &w, &config, host_int_val);
            
            h->SetResInProcess(100);
            double expected_stolen = 60; // sym_int_val * res_in_process * -1
            double expected_res_in_process = 40; // res_in_process - expected_stolen
            
            THEN("Amount stolen is dependent only on sym_int_val"){
                REQUIRE(h->StealResources(sym_int_val) == expected_stolen);
                REQUIRE(h->GetResInProcess() == expected_res_in_process);
            }
        }
        WHEN("host_int_val < 0"){
            double host_int_val = -0.2;
            Host * h = new Host(random, &w, &config, host_int_val);
            
            h->SetResInProcess(100);
            double expected_stolen = 40; // (host_int_val - sym_int_val) * res_in_process
            double expected_res_in_process = 60; // res_in_process - expected_stolen
            
            THEN("Amount stolen is dependent on both sym_int_val and host_int_val"){
                REQUIRE(h->StealResources(sym_int_val) == expected_stolen);
                REQUIRE(h->GetResInProcess() == expected_res_in_process);
            }
        }
    }

    WHEN("host_int_val > sym_int_val"){
        double sym_int_val = -0.3;
        double host_int_val = -0.5;
        Host * h = new Host(random, &w, &config, host_int_val);
            
        h->SetResInProcess(100);
        double expected_stolen = 0;
        double expected_res_in_process = 100; 
            
            THEN("Symbiont fails to steal resources"){
                REQUIRE(h->StealResources(sym_int_val) == expected_stolen);
                REQUIRE(h->GetResInProcess() == expected_res_in_process);
            }
        }
}
