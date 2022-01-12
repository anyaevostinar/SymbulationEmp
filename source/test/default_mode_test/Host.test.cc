#include "../../default_mode/Host.h"
#include "../../default_mode/Symbiont.h"
#include <set>

TEST_CASE("Host SetIntVal, GetIntVal", "[default]") {
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

TEST_CASE("SetPoints, AddPoints, GetPoints", "[default]") {
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

TEST_CASE("SetResTypes, GetResTypes", "[default]") {
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

TEST_CASE("HasSym", "[default]") {
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

TEST_CASE("Host Mutate", "[default]") {
    //TODO: put in tests for mutation size and mutation rate separately
    emp::Ptr<emp::Random> random = new emp::Random(3);
    SymConfigBase config;
    SymWorld w(*random);
    double int_val = -0.31;

    //MUTATION RATE
    WHEN("Host mutation rate is -1"){
      THEN("Normal mutation rate is used"){
        config.HOST_MUTATION_RATE(-1);
        config.MUTATION_RATE(1);
        Host * h = new Host(random, &w, &config, int_val);

        REQUIRE(h->GetIntVal() == int_val);
        h->mutate();
        REQUIRE(h->GetIntVal() != int_val);
        REQUIRE(h->GetIntVal() <= 1);
        REQUIRE(h->GetIntVal() >= -1);
      }
    }
    WHEN("Host mutation rate is not -1"){
      THEN("Host mutation rate is used"){
        config.HOST_MUTATION_RATE(1);
        config.MUTATION_RATE(0);
        Host * h = new Host(random, &w, &config, int_val);
        REQUIRE(h->GetIntVal() == int_val);
        h->mutate();
        REQUIRE(h->GetIntVal() != int_val);
        REQUIRE(h->GetIntVal() <= 1);
        REQUIRE(h->GetIntVal() >= -1);
      }
    }

    //MUTATION SIZE
    WHEN("Host mutation size is -1"){
      THEN("Normal mutation size is used"){
        config.HOST_MUTATION_SIZE(-1);
        config.MUTATION_RATE(1);
        Host * h = new Host(random, &w, &config, int_val);
        REQUIRE(h->GetIntVal() == int_val);
        h->mutate();
        REQUIRE(h->GetIntVal() != int_val);
        REQUIRE(h->GetIntVal() <= 1);
        REQUIRE(h->GetIntVal() >= -1);
      }
    }
    WHEN("Host mutation size is not -1"){
      THEN("Host mutation size is used"){
        config.HOST_MUTATION_SIZE(1);
        config.MUTATION_SIZE(0);
        Host * h = new Host(random, &w, &config, int_val);
        REQUIRE(h->GetIntVal() == int_val);
        h->mutate();
        REQUIRE(h->GetIntVal() != int_val);
        REQUIRE(h->GetIntVal() <= 1);
        REQUIRE(h->GetIntVal() >= -1);
      }
    }


}

TEST_CASE("DistributeResources", "[default]") {
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

TEST_CASE("Steal resources unit test", "[default]"){
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

TEST_CASE("GetDoEctosymbiosis"){
  GIVEN("A world"){
    emp::Ptr<emp::Random> random = new emp::Random(17);
    SymWorld w(*random);
    SymConfigBase config;
    w.Resize(2,2);
    double int_val = 0.5;
    size_t host_pos = 0;

    WHEN("Ectosymbiosis is off but other conditions are met"){
      config.ECTOSYMBIOSIS(0);
      emp::Ptr<Host> host = new Host(random, &w, &config, int_val);
      emp::Ptr<Organism> sym = new Symbiont(random, &w, &config, int_val);
      w.AddOrgAt(host, host_pos);
      w.AddOrgAt(sym, host_pos);
      THEN("Returns false"){
        REQUIRE(host->GetDoEctosymbiosis(host_pos) == false);
      }
    }
    WHEN("There is no parallel sym but other conditions are met"){
      config.ECTOSYMBIOSIS(1);
      emp::Ptr<Host> host = new Host(random, &w, &config, int_val);
      emp::Ptr<Organism> sym = new Symbiont(random, &w, &config, int_val);
      w.AddOrgAt(host, host_pos);
      w.AddOrgAt(sym, host_pos + 1);
      THEN("Returns false"){
        REQUIRE(host->GetDoEctosymbiosis(host_pos) == false);
      }
    }
    WHEN("There is a parallel sym but it is dead, and other conditions are met"){
      config.ECTOSYMBIOSIS(1);
      emp::Ptr<Host> host = new Host(random, &w, &config, int_val);
      emp::Ptr<Organism> sym = new Symbiont(random, &w, &config, int_val);
      sym->SetDead();
      w.AddOrgAt(host, host_pos);
      w.AddOrgAt(sym, host_pos + 1);
      THEN("Returns false"){
        REQUIRE(host->GetDoEctosymbiosis(host_pos) == false);
      }
    }
    WHEN("Ectosymbiotic immunity is on and the host has a sym, but other conditions are met"){
      config.ECTOSYMBIOSIS(1);
      config.ECTOSYMBIOTIC_IMMUNITY(1);
      emp::Ptr<Host> host = new Host(random, &w, &config, int_val);
      emp::Ptr<Organism> sym = new Symbiont(random, &w, &config, int_val);
      emp::Ptr<Organism> hosted_sym = new Symbiont(random, &w, &config, int_val);
      w.AddOrgAt(host, host_pos);
      w.AddOrgAt(sym, host_pos);
      host->AddSymbiont(hosted_sym);

      THEN("Returns false"){
        REQUIRE(host->GetDoEctosymbiosis(host_pos) == false);
      }
    }
    WHEN("Ectosymbiosis is on, there is a parallel sym, ectosymbiotic immunity is off, and the host has a sym"){
      config.ECTOSYMBIOSIS(1);
      config.ECTOSYMBIOTIC_IMMUNITY(0);
      emp::Ptr<Host> host = new Host(random, &w, &config, int_val);
      emp::Ptr<Organism> sym = new Symbiont(random, &w, &config, int_val);
      emp::Ptr<Organism> hosted_sym = new Symbiont(random, &w, &config, int_val);
      w.AddOrgAt(host, host_pos);
      w.AddOrgAt(sym, host_pos);
      host->AddSymbiont(hosted_sym);

      THEN("Returns true"){
        REQUIRE(host->GetDoEctosymbiosis(host_pos) == true);
      }
    }
    WHEN("Ectosymbiosis is on, there is a parallel sym, ectosymbiotic immunity is on, and the host does not have a sym"){
      config.ECTOSYMBIOSIS(1);
      config.ECTOSYMBIOTIC_IMMUNITY(1);
      emp::Ptr<Host> host = new Host(random, &w, &config, int_val);
      emp::Ptr<Organism> sym = new Symbiont(random, &w, &config, int_val);
      emp::Ptr<Organism> hosted_sym = new Symbiont(random, &w, &config, int_val);
      w.AddOrgAt(host, host_pos);
      w.AddOrgAt(sym, host_pos);

      THEN("Returns true"){
        REQUIRE(host->GetDoEctosymbiosis(host_pos) == true);
      }
    }
  }
}

TEST_CASE("Host GrowOlder"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymWorld w(*random);
    SymConfigBase config;
    config.HOST_AGE_MAX(2);

    WHEN ("A host reaches its maximum age"){
      Host * h = new Host(random, &w, &config, 1);
      w.AddOrgAt(h, 1);
      THEN("The host dies and is removed from the world"){
        REQUIRE(h->GetDead() == false);
        REQUIRE(w.GetNumOrgs() == 1);
        REQUIRE(h->GetAge() == 0);
        w.Update();
        REQUIRE(h->GetAge() == 1);
        w.Update();
        REQUIRE(h->GetAge() == 2);
        w.Update();
        REQUIRE(w.GetNumOrgs() == 0);
      }
    }
}
TEST_CASE("makeNew", "[default][efficient][lysis][pgg]"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymWorld w(*random);
    SymConfigBase config;

    double host_int_val = 0.2;
    Organism * h1 = new Host(random, &w, &config, host_int_val);
    Organism * h2 = h1->makeNew();
    THEN("The new host has properties of the original host"){
      REQUIRE(h1->GetIntVal() == h2->GetIntVal());
    }
}

TEST_CASE("Host reproduce", "[default][efficient][lysis][pgg]"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymWorld w(*random);
    SymConfigBase config;

    double host_int_val = 0.2;
    Organism * h1 = new Host(random, &w, &config, host_int_val);
    Organism * h2 = h1->reproduce();
    THEN("The host baby has mutated interaction value"){
      REQUIRE(h1->GetIntVal() != h2->GetIntVal());
    }
    THEN("The host parent's points are set to 0"){
      REQUIRE(h1->GetPoints() == 0);
    }
}
