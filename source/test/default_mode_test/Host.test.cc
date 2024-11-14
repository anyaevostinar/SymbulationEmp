#include "../../default_mode/DataNodes.h"
#include "../../default_mode/SymWorld.h"
#include "../../default_mode/Host.h"
#include "../../default_mode/Symbiont.h"

TEST_CASE("Host Constructor", "[default]") {

    emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(4);
    SymConfigBase config;
    SymWorld w(*random, &config);
    SymWorld * world = &w;
    double int_val;

    WHEN("An interaction value of -2 is passed") {
      int_val = -2;
      emp::Ptr<Host> host = emp::NewPtr<Host>(random, world, &config, int_val);
      THEN("The host randomly determines its interaction value") {
        REQUIRE(host->GetIntVal() >= -1);
        REQUIRE(host->GetIntVal() <= 1);
      }
      host.Delete();
    }
    
    WHEN("An interaction value < -1 other than -2 is passed") {
      int_val = -1.5;
      THEN("An excepton is thrown") {
        REQUIRE_THROWS(emp::NewPtr<Host>(random, world, &config, int_val));
      }
    }

    int_val = -1;
    emp::Ptr<Host> host1 = emp::NewPtr<Host>(random, world, &config, int_val);
    CHECK(host1->GetIntVal() == int_val);
    CHECK(host1->GetAge() == 0);
    CHECK(host1->GetPoints() == 0);

    int_val = -1;
    emp::vector<emp::Ptr<Organism>> syms = {};
    emp::vector<emp::Ptr<Organism>> repro_syms = {};
    std::set<int> set = std::set<int>();
    double points = 10;
    emp::Ptr<Host> host2 = emp::NewPtr<Host>(random, world, &config, int_val, syms, repro_syms, points);
    CHECK(host2->GetIntVal() == int_val);
    CHECK(host2->GetAge() == 0);
    CHECK(host2->GetPoints() == points);

    int_val = 1;
    emp::Ptr<Host> host3 = emp::NewPtr<Host>(random, world, &config, int_val);
    CHECK(host3->GetIntVal() == int_val);
    CHECK(host3->GetAge() == 0);
    CHECK(host3->GetPoints() == 0);

    int_val = 2;
    REQUIRE_THROWS(emp::NewPtr<Host>(random, world, &config, int_val) );

    host1.Delete();
    host2.Delete();
    host3.Delete();
    random.Delete();
}

TEST_CASE("Host SetIntVal, GetIntVal", "[default]") {
    emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(4);
    SymConfigBase config;
    SymWorld world(*random, &config);
    double int_val = 1;

    emp::Ptr<Host> host1 = emp::NewPtr<Host>(random, &world, &config);
    double default_int_val = 0.0;
    REQUIRE(host1->GetIntVal() == default_int_val);

    emp::Ptr<Host> host2 = emp::NewPtr<Host>(random, &world, &config, int_val);

    double expected_int_val = 1;
    REQUIRE(host2->GetIntVal() == expected_int_val);

    int_val = -0.7;
    host2->SetIntVal(int_val);
    expected_int_val = -0.7;
    REQUIRE(host2->GetIntVal() == expected_int_val);

    int_val = -1.3;
    REQUIRE_THROWS(emp::NewPtr<Host>(random, &world, &config, int_val));

    int_val = 1.8;
    REQUIRE_THROWS(emp::NewPtr<Host>(random, &world, &config, int_val));

    host1.Delete();
    host2.Delete();
    random.Delete();
}

TEST_CASE("SetPoints, AddPoints, GetPoints", "[default]") {
    emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(4);
    SymConfigBase config;
    SymWorld world(*random, &config);
    double int_val = 1;

    emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, int_val);

    double points = 50;
    host->SetPoints(points);
    double expected_points = 50;
    REQUIRE(host->GetPoints() == expected_points);

    points = 76;
    host->AddPoints(points);
    expected_points = 126;
    REQUIRE(host->GetPoints() == expected_points);

    host.Delete();
    random.Delete();
}

TEST_CASE("HasSym", "[default]") {
    emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(4);
    SymConfigBase config;
    SymWorld world(*random, &config);
    double int_val = 1;

    emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, int_val);
    WHEN("Host has no symbionts") {
        THEN("HasSym is false") {
            bool expected = false;
            REQUIRE(host->HasSym() == expected);
        }
    }

    WHEN("Host has symbionts") {
        host->AddSymbiont(emp::NewPtr<Symbiont>(random, &world, &config, int_val));
        THEN("HasSym is true") {
            bool expected = true;
            REQUIRE(host->HasSym() == expected);
        }
    }
    host.Delete();
    random.Delete();
}

TEST_CASE("Host Mutate", "[default]") {
    //TODO: put in tests for mutation size and mutation rate separately
    emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(3);
    SymConfigBase config;
    SymWorld world(*random, &config);
    double int_val = -0.31;

    //MUTATION RATE
    WHEN("Host mutation rate is -1"){
      THEN("Normal mutation rate is used"){
        config.HOST_MUTATION_RATE(-1);
        config.MUTATION_RATE(1);
        emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, int_val);

        REQUIRE(host->GetIntVal() == int_val);
        host->Mutate();
        REQUIRE(host->GetIntVal() != int_val);
        REQUIRE(host->GetIntVal() <= 1);
        REQUIRE(host->GetIntVal() >= -1);

        host.Delete();
      }
    }
    WHEN("Host mutation rate is not -1"){
      THEN("Host mutation rate is used"){
        config.HOST_MUTATION_RATE(1);
        config.MUTATION_RATE(0);
        emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, int_val);
        REQUIRE(host->GetIntVal() == int_val);
        host->Mutate();
        REQUIRE(host->GetIntVal() != int_val);
        REQUIRE(host->GetIntVal() <= 1);
        REQUIRE(host->GetIntVal() >= -1);

        host.Delete();
      }
    }

    //MUTATION SIZE
    WHEN("Host mutation size is -1"){
      THEN("Normal mutation size is used"){
        config.HOST_MUTATION_SIZE(-1);
        config.MUTATION_RATE(1);
        emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, int_val);
        REQUIRE(host->GetIntVal() == int_val);
        host->Mutate();
        REQUIRE(host->GetIntVal() != int_val);
        REQUIRE(host->GetIntVal() <= 1);
        REQUIRE(host->GetIntVal() >= -1);

        host.Delete();
      }
    }
    WHEN("Host mutation size is not -1"){
      THEN("Host mutation size is used"){
        config.HOST_MUTATION_SIZE(1);
        config.MUTATION_SIZE(0);
        emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, int_val);
        REQUIRE(host->GetIntVal() == int_val);
        host->Mutate();
        REQUIRE(host->GetIntVal() != int_val);
        REQUIRE(host->GetIntVal() <= 1);
        REQUIRE(host->GetIntVal() >= -1);

        host.Delete();
      }
    }

    //TAG MUTATION SIZE
    WHEN("Tag matching is on") {
      THEN("Tags mutate according to tag mutation rate") {
        emp::HammingMetric<32> metric = emp::HammingMetric<32>();
        config.TAG_MATCHING(1);
        config.TAG_MUTATION_SIZE(0.1);
        emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, int_val);
        emp::BitSet<32> bit_set = emp::BitSet<32>();
        host->SetTag(bit_set);

        REQUIRE(metric.calculate(host->GetTag(), bit_set) == 0);
        host->Mutate();
        REQUIRE(metric.calculate(host->GetTag(), bit_set) > 0);
        REQUIRE(metric.calculate(host->GetTag(), bit_set) <= 1);

        host.Delete();
        config.TAG_MATCHING(0); // don't try to delete a non-existent
        // SymWorld metric
      }
    }
    random.Delete();
}

TEST_CASE("DistributeResources", "[default]") {
    emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(5);
    SymConfigBase config;
    SymWorld world(*random, &config);

    WHEN("There are no symbionts and interaction value is between 0 and 1") {

        double int_val = 0.6;
        double resources = 80;
        double orig_points = 0; // call this default_points instead? (i'm not setting this val)
        config.SYNERGY(5);

        emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, int_val);
        host->DistribResources(resources);

        THEN("Points increase") {
            double expected_points = resources - (resources * int_val); // 48
            double points = host->GetPoints();
            REQUIRE(points == expected_points);
            REQUIRE(points > orig_points);
        }

        host.Delete();
    }

    WHEN("There are no symbionts and interaction value is 0") {

        double int_val = 0;
        double resources = 10;
        double orig_points = 0;
        config.SYNERGY(5);

        emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, int_val);
        host->DistribResources(resources);

        THEN("Resources are added to points") {
            double expected_points = orig_points + resources; // 0
            double points = host->GetPoints();
            REQUIRE(points == expected_points);
        }

        host.Delete();
    }

    WHEN("There are no symbionts and interaction value is between -1 and 0") {

        double int_val = -0.4;
        double resources = 30;
        double orig_points = 27;
        config.SYNERGY(5);

        emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, int_val);
        host->AddPoints(orig_points);
        host->DistribResources(resources);

        THEN("Points increase") {
            double host_defense =  -1.0 * int_val * resources; // the resources spent on defense
            double add_points  = resources - host_defense;
            double expected_points = orig_points + add_points;
            double points = host->GetPoints();
            REQUIRE(points == expected_points);
            REQUIRE(points > orig_points);
        }

        host.Delete();
    }
    random.Delete();
}

TEST_CASE("SetResInProcess, GetResInProcess", "[default]") {
  emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(3);
  SymConfigBase config;
  SymWorld world(*random, &config);
  double int_val = 1;

  emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, int_val);

  double expected_res_in_process = 0;
  REQUIRE(host->GetResInProcess() == expected_res_in_process);

  host->SetResInProcess(126);
  expected_res_in_process = 126;
  REQUIRE(host->GetResInProcess() == expected_res_in_process);

  host.Delete();
  random.Delete();
}

TEST_CASE("Steal resources unit test", "[default]"){
    emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(3);
    SymConfigBase config;
    SymWorld world(*random, &config);


    WHEN ("sym_int_val < host_int_val"){
        double sym_int_val = -0.6;

        WHEN("host_int_val > 0"){
            double host_int_val = 0.2;
            emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, host_int_val);

            host->SetResInProcess(100);
            double expected_stolen = 60; // sym_int_val * res_in_process * -1
            double expected_res_in_process = 40; // res_in_process - expected_stolen

            THEN("Amount stolen is dependent only on sym_int_val"){
                REQUIRE(host->StealResources(sym_int_val) == expected_stolen);
                REQUIRE(host->GetResInProcess() == expected_res_in_process);
            }
            host.Delete();
        }
        WHEN("host_int_val < 0"){
            double host_int_val = -0.2;
            emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, host_int_val);

            host->SetResInProcess(100);
            double expected_stolen = 40; // (host_int_val - sym_int_val) * res_in_process
            double expected_res_in_process = 60; // res_in_process - expected_stolen

            THEN("Amount stolen is dependent on both sym_int_val and host_int_val"){
                REQUIRE(host->StealResources(sym_int_val) == expected_stolen);
                REQUIRE(host->GetResInProcess() == expected_res_in_process);
            }
            host.Delete();
        }
    }

    WHEN("host_int_val > sym_int_val"){
        double sym_int_val = -0.3;
        double host_int_val = -0.5;
        emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, host_int_val);

        host->SetResInProcess(100);
        double expected_stolen = 0;
        double expected_res_in_process = 100;

        THEN("Symbiont fails to steal resources"){
            REQUIRE(host->StealResources(sym_int_val) == expected_stolen);
            REQUIRE(host->GetResInProcess() == expected_res_in_process);
        }
        host.Delete();
    }
    random.Delete();
}

TEST_CASE("GetDoEctosymbiosis", "[default]"){
  GIVEN("A world"){
    emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(17);
    SymConfigBase config;
    SymWorld world(*random, &config);
    world.Resize(2,2);
    double int_val = 0.5;
    size_t host_pos = 0;

    WHEN("Ectosymbiosis is off but other conditions are met"){
      config.ECTOSYMBIOSIS(0);
      emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, int_val);
      emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(random, &world, &config, int_val);
      world.AddOrgAt(host, host_pos);
      world.AddOrgAt(sym, emp::WorldPosition(0, host_pos));
      THEN("Returns false"){
        REQUIRE(host->GetDoEctosymbiosis(host_pos) == false);
      }
    }
    WHEN("There is no parallel sym but other conditions are met"){
      config.ECTOSYMBIOSIS(1);
      emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, int_val);
      emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(random, &world, &config, int_val);
      world.AddOrgAt(host, host_pos);
      world.AddOrgAt(sym, emp::WorldPosition(0, host_pos + 1));
      THEN("Returns false"){
        REQUIRE(host->GetDoEctosymbiosis(host_pos) == false);
      }
    }
    WHEN("There is a parallel sym but it is dead, and other conditions are met"){
      config.ECTOSYMBIOSIS(1);
      emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, int_val);
      emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(random, &world, &config, int_val);
      sym->SetDead();
      world.AddOrgAt(host, host_pos);
      world.AddOrgAt(sym, emp::WorldPosition(0, host_pos + 1));
      THEN("Returns false"){
        REQUIRE(host->GetDoEctosymbiosis(host_pos) == false);
      }
    }
    WHEN("Ectosymbiotic immunity is on and the host has a sym, but other conditions are met"){
      config.ECTOSYMBIOSIS(1);
      config.ECTOSYMBIOTIC_IMMUNITY(1);
      emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, int_val);
      emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(random, &world, &config, int_val);
      emp::Ptr<Organism> hosted_sym = emp::NewPtr<Symbiont>(random, &world, &config, int_val);
      world.AddOrgAt(host, host_pos);
      world.AddOrgAt(sym, emp::WorldPosition(0, host_pos));
      host->AddSymbiont(hosted_sym);

      THEN("Returns false"){
        REQUIRE(host->GetDoEctosymbiosis(host_pos) == false);
      }
    }
    WHEN("Ectosymbiosis is on, there is a parallel sym, ectosymbiotic immunity is off, and the host has a sym"){
      config.ECTOSYMBIOSIS(1);
      config.ECTOSYMBIOTIC_IMMUNITY(0);
      emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, int_val);
      emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(random, &world, &config, int_val);
      emp::Ptr<Organism> hosted_sym = emp::NewPtr<Symbiont>(random, &world, &config, int_val);
      world.AddOrgAt(host, host_pos);
      world.AddOrgAt(sym, emp::WorldPosition(0, host_pos));
      host->AddSymbiont(hosted_sym);

      THEN("Returns true"){
        REQUIRE(host->GetDoEctosymbiosis(host_pos) == true);
      }
    }
    WHEN("Ectosymbiosis is on, there is a parallel sym, ectosymbiotic immunity is on, and the host does not have a sym"){
      config.ECTOSYMBIOSIS(1);
      config.ECTOSYMBIOTIC_IMMUNITY(1);
      emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, int_val);
      emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(random, &world, &config, int_val);

      world.AddOrgAt(host, host_pos);
      world.AddOrgAt(sym, emp::WorldPosition(0, host_pos));

      THEN("Returns true"){
        REQUIRE(host->GetDoEctosymbiosis(host_pos) == true);
      }
    }
    random.Delete();
  }
}

TEST_CASE("Host GrowOlder", "[default]"){
    emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(4);
    SymConfigBase config;
    SymWorld world(*random, &config);
    config.HOST_AGE_MAX(2);

    WHEN ("A host reaches its maximum age"){
      emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, 1);
      world.AddOrgAt(host, 1);
      THEN("The host dies and is removed from the world"){
        REQUIRE(host->GetDead() == false);
        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(host->GetAge() == 0);
        world.Update();
        REQUIRE(host->GetAge() == 1);
        world.Update();
        REQUIRE(host->GetAge() == 2);
        world.Update();
        REQUIRE(world.GetNumOrgs() == 0);
      }
    }
    random.Delete();
}

TEST_CASE("Host MakeNew", "[default]"){
    emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(4);
    SymConfigBase config;
    SymWorld world(*random, &config);

    double host_int_val = 0.2;
    emp::Ptr<Organism> host1 = emp::NewPtr<Host>(random, &world, &config, host_int_val);
    emp::Ptr<Organism> host2 = host1->MakeNew();
    THEN("The new host has properties of the original host and has 0 points and 0 age"){
      REQUIRE(host1->GetIntVal() == host2->GetIntVal());
      REQUIRE(host2->GetPoints() == 0);
      REQUIRE(host2->GetAge() == 0);
      //check that the offspring is the correct class
      REQUIRE(host2->GetName() == "Host");
    }

    host1.Delete();
    host2.Delete();
    random.Delete();
}

TEST_CASE("Host Reproduce", "[default]"){
    emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(4);
    SymConfigBase config;
    SymWorld world(*random, &config);

    double host_int_val = 0.2;
    emp::Ptr<Organism> host1 = emp::NewPtr<Host>(random, &world, &config, host_int_val);
    emp::Ptr<Organism> host2 = host1->Reproduce();
    THEN("The host baby has mutated interaction value"){
      REQUIRE(host1->GetIntVal() != host2->GetIntVal());
    }
    THEN("The host parent's points are set to 0"){
      REQUIRE(host1->GetPoints() == 0);
    }

    host1.Delete();
    host2.Delete();
    random.Delete();
}

TEST_CASE("AddSymbiont", "[default]"){
  emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(4);
  SymConfigBase config;
  SymWorld world(*random, &config);
  double int_val = 0;

   WHEN("A symbiont successfully infects"){
    emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, int_val);
    emp::Ptr<Organism> symbiont = emp::NewPtr<Symbiont>(random, &world, &config, int_val);
    size_t pos = host->AddSymbiont(symbiont);
    emp::vector<emp::Ptr<Organism>>& host_syms = host->GetSymbionts();
    THEN("It is added to the host sym vector and it's position is returned"){
      REQUIRE(host->HasSym() == true);
      REQUIRE(pos == host_syms.size());
      REQUIRE(host_syms.at(pos - 1) == symbiont);
    }
    host.Delete();
  }
  WHEN("A symbiont fails to infect"){
    emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, int_val);
    emp::Ptr<Organism> symbiont = emp::NewPtr<Symbiont>(random, &world, &config, int_val);
    config.SYM_LIMIT(0);
    int pos = host->AddSymbiont(symbiont);
    THEN("It is deleted and 0 is returned"){
      REQUIRE(host->HasSym() == false);
      REQUIRE(pos == 0);
    }
    host.Delete();
  }

  WHEN("A symbiont ousts another") {
    config.OUSTING(1);
    world.Resize(1, 1);
    emp::Ptr<Host> ousting_host = emp::NewPtr<Host>(random, &world, &config, int_val);
    emp::Ptr<Organism> new_symbiont = emp::NewPtr<Symbiont>(random, &world, &config, int_val);
    emp::Ptr<Organism> old_symbiont = emp::NewPtr<Symbiont>(random, &world, &config, int_val);

    world.AddOrgAt(ousting_host, 0);
    REQUIRE(world.GetNumOrgs() == 1);
    ousting_host->AddSymbiont(old_symbiont);
    ousting_host->AddSymbiont(new_symbiont);

    THEN("The new symbiont enters the host and the old is sent to the graveyard and killed") {
      REQUIRE(ousting_host->HasSym() == true);
      REQUIRE(ousting_host->GetSymbionts()[0] == new_symbiont);
      REQUIRE(ousting_host->GetSymbionts().size() == 1);
      REQUIRE(world.GetGraveyard().size() == 1);
      REQUIRE(world.GetGraveyard()[0] == old_symbiont);
      world.Update();
      REQUIRE(world.GetGraveyard().size() == 0);
    }
  }
  random.Delete();
}

TEST_CASE("SymAllowedIn", "[default]") {
  SymConfigBase config;
  double int_val = 0;
  
  int sym_limit = 4;
  config.SYM_LIMIT(sym_limit);

  WHEN("Symbiont exclude is set to false") {
    config.PHAGE_EXCLUDE(0);
    THEN("Symbionts are added without issue") {
      emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(3);
      SymWorld world(*random, &config);
      emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, int_val);
      for (int i = 0; i < sym_limit; i++) {
        host->AddSymbiont(emp::NewPtr<Symbiont>(random, &world, &config, int_val));
      }
      int num_syms = (host->GetSymbionts()).size();
      REQUIRE(num_syms == sym_limit);
      host.Delete();
      random.Delete();
    }
  }

  WHEN("Symbiont exclude is set to true") {
    config.PHAGE_EXCLUDE(1);
    THEN("Symbionts have a decreasing change of entering the host") {
      int goal_num_syms[] = { 3,3,3,3 };
      for (int i = 0; i < 4; i++) {
        emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(i + 1);
        SymWorld world(*random, &config);
        emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, int_val);

        for (double i = 0; i < 10; i++) {
          host->AddSymbiont(emp::NewPtr<Symbiont>(random, &world, &config, int_val));
        }
        int host_num_syms = (host->GetSymbionts()).size();
        REQUIRE(goal_num_syms[i] == host_num_syms);
        host.Delete();
        random.Delete();
      }
    }
  } 
}
