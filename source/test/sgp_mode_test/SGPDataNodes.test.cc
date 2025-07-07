#include "../../sgp_mode/GenomeLibrary.h"
#include "../../sgp_mode/CPU.h"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPHost.cc"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPWorldSetup.cc"
#include "../../sgp_mode/SGPDataNodes.h"

TEST_CASE("SGP GetHostCountDataNode", "[dsg]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigSGP config;
    int int_val = 0;
    SGPWorld world(random, &config, LogicTasks);
    size_t world_size = 4;
    world.Resize(world_size);

    emp::DataMonitor<int>& host_count_node = world.GetHostCountDataNode();
    REQUIRE(host_count_node.GetTotal() == 0);
    REQUIRE(world.GetNumOrgs() == 0);

    WHEN("a host is added"){
      size_t num_hosts = 4;
      for(size_t i = 0; i < num_hosts; i++){
        world.AddOrgAt(emp::NewPtr<SGPHost>(&random, &world, &config, int_val), i);
      }
      world.Update();
      host_count_node = world.GetHostCountDataNode();
      THEN("the host is tracked by the data node"){
        REQUIRE(world.GetNumOrgs() == num_hosts);
        REQUIRE(host_count_node.GetTotal() == num_hosts);
      }
    }
  }
}

TEST_CASE("SGP GetSymCountDataNode", "[dsg]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigSGP config;
    int int_val = 0;
    int world_size = 4;
    SGPWorld world(random, &config, LogicTasks);
    world.Resize(world_size);

    config.FREE_LIVING_SYMS(1);
    config.SYM_INFECTION_CHANCE(0);
    config.SYM_LIMIT(2);

    emp::DataMonitor<int>& sym_count_node = world.GetSymCountDataNode();
    REQUIRE(sym_count_node.GetTotal() == 0);
    REQUIRE(world.GetNumOrgs() == 0);

    WHEN("free and hosted syms are added"){
      size_t num_free_syms = 4;
      size_t num_hosted_syms = 2;
      emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, int_val);
      world.AddOrgAt(host, 0);
      for(size_t i = 0; i < num_free_syms; i++){
        world.AddOrgAt(emp::NewPtr<SGPSymbiont>(&random, &world, &config, int_val), emp::WorldPosition(0,i));
      }
      for(size_t i = 0; i < num_hosted_syms; i++){
        host->AddSymbiont(emp::NewPtr<SGPSymbiont>(&random, &world, &config, int_val));
      }

      world.Update();

      THEN("they are tracked by the data node"){
        REQUIRE(world.GetNumOrgs() == num_free_syms + 1); // and the host
        REQUIRE(sym_count_node.GetTotal() == num_free_syms + num_hosted_syms);
      }
    }
  }
}

TEST_CASE("SGP GetCountHostedSymsDataNode", "[dsg]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigSGP config;
    int int_val = 0;
    SGPWorld world(random, &config, LogicTasks);
    config.SYM_LIMIT(4);
    size_t world_size = 4;
    world.Resize(world_size);

    emp::DataMonitor<int>& hosted_sym_count_node = world.GetCountHostedSymsDataNode();
    REQUIRE(hosted_sym_count_node.GetTotal() == 0);
    REQUIRE(world.GetNumOrgs() == 0);

    WHEN("hosted syms are added"){
      size_t num_syms_per_host = 4;
      size_t num_hosts = 2;
      for(size_t i = 0; i < num_hosts; i++){
        emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, int_val);
        world.AddOrgAt(host, i);
        for(size_t j = 0; j < num_syms_per_host; j++){
          host->AddSymbiont(emp::NewPtr<SGPSymbiont>(&random, &world, &config, int_val));
        }
      }
      world.Update();
      THEN("they are tracked by the data node"){
        REQUIRE(world.GetNumOrgs() == num_hosts);
        REQUIRE(hosted_sym_count_node.GetTotal() == num_syms_per_host * num_hosts);
      }
    }
    WHEN("a free sym is added"){
      world.AddOrgAt(emp::NewPtr<SGPSymbiont>(&random, &world, &config, int_val), emp::WorldPosition(0,0));
      world.Update();
      THEN("the hosted sym data node doesn't track it"){
        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(hosted_sym_count_node.GetTotal() == 0);
      }
    }
  }
}

TEST_CASE("SGP GetCountFreeSymsDataNode", "[dsg]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigSGP config;
    int int_val = 0;
    SGPWorld world(random, &config, LogicTasks);
    config.SYM_LIMIT(4);
    size_t world_size = 4;
    world.Resize(world_size);

    emp::DataMonitor<int>& free_sym_count_node = world.GetCountFreeSymsDataNode();
    REQUIRE(free_sym_count_node.GetTotal() == 0);
    REQUIRE(world.GetNumOrgs() == 0);

    WHEN("free syms are added"){
      size_t num_free_syms = 4;

      for(size_t i = 0; i < num_free_syms; i++){
        world.AddOrgAt(emp::NewPtr<SGPSymbiont>(&random, &world, &config, int_val), emp::WorldPosition(0, i));
      }

      world.Update();
      THEN("they are tracked by the data node"){
        REQUIRE(world.GetNumOrgs() == num_free_syms);
        REQUIRE(free_sym_count_node.GetTotal() == num_free_syms);
      }
    }

    WHEN("a hosted sym is added"){
      emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, int_val);
      world.AddOrgAt(host, 0);
      host->AddSymbiont(emp::NewPtr<SGPSymbiont>(&random, &world, &config, int_val));
      world.Update();
      THEN("it isn't tracked by the free sym data node"){
        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(free_sym_count_node.GetTotal() == 0);
      }
    }
  }
}

TEST_CASE("SGP GetUninfectedHostsDataNode", "[dsg]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigSGP config;
    int int_val = 0;
    SGPWorld world(random, &config, LogicTasks);
    config.SYM_LIMIT(4);
    size_t world_size = 10;
    world.Resize(world_size);

    //keep track of host organisms that are uninfected
    emp::DataMonitor<int>& uninfected_hosts_data_node = world.GetUninfectedHostsDataNode();
    REQUIRE(uninfected_hosts_data_node.GetTotal() == 0);
    REQUIRE(world.GetNumOrgs() == 0);

    WHEN("uninfected hosts are added"){
      size_t num_hosts = 10;

      for(size_t i = 0; i < num_hosts; i++){
        world.AddOrgAt(emp::NewPtr<SGPHost>(&random, &world, &config, int_val), i);
      }

      world.Update();
      THEN("they are tracked by the data node"){
        REQUIRE(world.GetNumOrgs() == num_hosts);
        REQUIRE(uninfected_hosts_data_node.GetTotal() == num_hosts);
      }

      WHEN("some hosts are infected "){
        size_t num_infections = 2;
        for(size_t i = 0; i < num_infections; i++){
          world.GetOrg(i).AddSymbiont(emp::NewPtr<SGPSymbiont>(&random, &world, &config, int_val));
        }
        world.Update();
        THEN("infected hosts are excluded from the uninfected host count"){
          REQUIRE(world.GetNumOrgs() == num_hosts);
          REQUIRE(uninfected_hosts_data_node.GetTotal() == (num_hosts - num_infections));
        }
      }
    }
  }
}

TEST_CASE("SGP GetSymIntValDataNode", "[dsg]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigSGP config;
    int int_val = 0;
    SGPWorld world(random, &config, LogicTasks);
    size_t world_size = 4;
    world.Resize(world_size);

    config.FREE_LIVING_SYMS(1);
    config.SYM_INFECTION_CHANCE(0);
    config.SYM_LIMIT(3);
    size_t num_bins = 21;

    emp::DataMonitor<double,emp::data::Histogram>& sym_intval_node = world.GetSymIntValDataNode();
    REQUIRE(std::isnan(sym_intval_node.GetMean()));
    for(size_t i = 0; i < num_bins; i++){
      REQUIRE(sym_intval_node.GetHistCounts()[i] == 0);
    }
    REQUIRE(world.GetNumOrgs() == 0);

    WHEN("free and hosted syms are added"){
      double free_sym_int_vals[3] = {-1.0, -0.43, 0.02};
      double hosted_sym_int_vals[3] = {0.71, 0.75, 1};

      double expected_av = 0.175;
      emp::vector<long unsigned int> expected_hist_counts(21);
      std::fill(expected_hist_counts.begin(), expected_hist_counts.end(), 0);
      expected_hist_counts[0] = 1;
      expected_hist_counts[5] = 1;
      expected_hist_counts[10] = 1;
      expected_hist_counts[17] = 2;
      expected_hist_counts[19] = 1;

      emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, int_val);
      world.AddOrgAt(host, 0);
      for(size_t i = 0; i < 3; i++){
        world.AddOrgAt(emp::NewPtr<SGPSymbiont>(&random, &world, &config, free_sym_int_vals[i]), emp::WorldPosition(0,i));
        host->AddSymbiont(emp::NewPtr<SGPSymbiont>(&random, &world, &config, hosted_sym_int_vals[i]));
      }
      world.Update();

      THEN("their average interaction values are tracked"){
        REQUIRE(sym_intval_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(sym_intval_node.GetMean() > (expected_av - 0.0001));
      }
      THEN("they were split into histogram bins correctly"){
        for(size_t i = 0; i < num_bins; i++){
          REQUIRE(sym_intval_node.GetHistCounts()[i] == expected_hist_counts[i]);
        }
      }
    }
  }
}

TEST_CASE("SGP GetFreeSymIntValDataNode", "[dsg]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigSGP config;
    int int_val = 0;
    SGPWorld world(random, &config, LogicTasks);
    size_t world_size = 4;
    world.Resize(world_size);

    config.FREE_LIVING_SYMS(1);
    config.SYM_INFECTION_CHANCE(0);
    config.SYM_LIMIT(3);
    size_t num_bins = 21;

    emp::DataMonitor<double,emp::data::Histogram>& free_sym_intval_node = world.GetFreeSymIntValDataNode();

    //initial checks for emptiness
    REQUIRE(std::isnan(free_sym_intval_node.GetMean()));
    REQUIRE(world.GetNumOrgs() == 0);
    for(size_t i = 0; i < num_bins; i++){
      REQUIRE(free_sym_intval_node.GetHistCounts()[i] == 0);
    }

    WHEN("free and hosted syms are added"){
      double free_sym_int_vals[3] = {-1.0, -0.43, 0.02};
      double hosted_sym_int_vals[3] = {0.71, 0.75, 1};

      double expected_av = -0.47;
      emp::vector<long unsigned int> expected_hist_counts(21);
      std::fill(expected_hist_counts.begin(), expected_hist_counts.end(), 0);
      expected_hist_counts[0] = 1;
      expected_hist_counts[5] = 1;
      expected_hist_counts[10] = 1;

      emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, int_val);
      world.AddOrgAt(host, 0);
      for(size_t i = 0; i < 3; i++){
        world.AddOrgAt(emp::NewPtr<SGPSymbiont>(&random, &world, &config, free_sym_int_vals[i]), emp::WorldPosition(0,i));
        host->AddSymbiont(emp::NewPtr<SGPSymbiont>(&random, &world, &config, hosted_sym_int_vals[i]));
      }
      world.Update();

      THEN("only free symbionts average interaction values are tracked"){
        REQUIRE(free_sym_intval_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(free_sym_intval_node.GetMean() > (expected_av - 0.0001));
      }
      THEN("only free symbionts were split into histogram bins correctly"){
        for(size_t i = 0; i < num_bins; i++){
          REQUIRE(free_sym_intval_node.GetHistCounts()[i] == expected_hist_counts[i]);
        }
      }
    }
  }
}

TEST_CASE("SGP GetHostedSymIntValDataNode", "[dsg]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigSGP config;
    int int_val = 0;
    SGPWorld world(random, &config, LogicTasks);
    size_t world_size = 4;
    world.Resize(world_size);

    config.FREE_LIVING_SYMS(1);
    config.SYM_INFECTION_CHANCE(0);
    config.SYM_LIMIT(3);
    size_t num_bins = 21;

    emp::DataMonitor<double,emp::data::Histogram>& hosted_sym_intval_node = world.GetHostedSymIntValDataNode();
    REQUIRE(std::isnan(hosted_sym_intval_node.GetMean()));
    REQUIRE(world.GetNumOrgs() == 0);
    for(size_t i = 0; i < num_bins; i++){
      REQUIRE(hosted_sym_intval_node.GetHistCounts()[i] == 0);
    }

    WHEN("free and hosted syms are added"){
      double free_sym_int_vals[3] = {-1.0, -0.43, 0.02};
      double hosted_sym_int_vals[3] = {0.71, 0.75, 1};

      double expected_av = 0.82;
      emp::vector<long unsigned int> expected_hist_counts(21);
      std::fill(expected_hist_counts.begin(), expected_hist_counts.end(), 0);
      expected_hist_counts[17] = 2;
      expected_hist_counts[19] = 1;

      emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, int_val);
      world.AddOrgAt(host, 0);
      for(size_t i = 0; i < 3; i++){
        world.AddOrgAt(emp::NewPtr<SGPSymbiont>(&random, &world, &config, free_sym_int_vals[i]), emp::WorldPosition(0,i));
        host->AddSymbiont(emp::NewPtr<SGPSymbiont>(&random, &world, &config, hosted_sym_int_vals[i]));
      }
      world.Update();

      THEN("only hosted symbiont average interaction values are tracked"){
        REQUIRE(hosted_sym_intval_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(hosted_sym_intval_node.GetMean() > (expected_av - 0.0001));
      }
      THEN("only hosted symbionts were split into histogram bins correctly"){
        for(size_t i = 0; i < num_bins; i++){
          REQUIRE(hosted_sym_intval_node.GetHistCounts()[i] == expected_hist_counts[i]);
        }
      }
    }
  }
}

TEST_CASE("SGP GetHostIntValDataNode", "[dsg]"){
  GIVEN("a world"){
    emp::Random random(17);
    SymConfigSGP config;
    SGPWorld world(random, &config, LogicTasks);
    size_t world_size = 4;
    world.Resize(world_size);

    config.FREE_LIVING_SYMS(1);
    config.SYM_INFECTION_CHANCE(0);
    config.SYM_LIMIT(3);
    size_t num_bins = 21;

    emp::DataMonitor<double,emp::data::Histogram>& host_intval_node = world.GetHostIntValDataNode();
    REQUIRE(std::isnan(host_intval_node.GetMean()));
    for(size_t i = 0; i < num_bins; i++){
      REQUIRE(host_intval_node.GetHistCounts()[i] == 0);
    }
    REQUIRE(world.GetNumOrgs() == 0);

    WHEN("hosts are added"){
      double int_vals[4] = {-1.0, -0.92, 0.38, 1.0};
      double expected_av = -0.135;

      emp::vector<long unsigned int> expected_hist_counts(21);
      std::fill(expected_hist_counts.begin(), expected_hist_counts.end(), 0);
      expected_hist_counts[0] = 2;
      expected_hist_counts[13] = 1;
      expected_hist_counts[19] = 1;

      for(size_t i = 0; i < 4; i++){
        world.AddOrgAt(emp::NewPtr<SGPHost>(&random, &world, &config, int_vals[i]), i);
      }
      world.Update();

      THEN("their average interaction values are tracked"){
        REQUIRE(host_intval_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(host_intval_node.GetMean() > (expected_av - 0.0001));
      }
      THEN("they were split into histogram bins correctly"){
        for(size_t i = 0; i < num_bins; i++){
          REQUIRE(host_intval_node.GetHistCounts()[i] == expected_hist_counts[i]);
        }
      }
    }
  }
}

TEST_CASE("SGP GetSymInfectChanceDataNode", "[dsg]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigSGP config;
    int int_val = 0;
    SGPWorld world(random, &config, LogicTasks);
    size_t world_size = 4;
world.Resize(world_size);

    config.FREE_LIVING_SYMS(1);
    config.SYM_INFECTION_CHANCE(0);
    config.SYM_LIMIT(3);
    size_t num_bins = 11;

    emp::DataMonitor<double,emp::data::Histogram>& sym_infectionchance_node = world.GetSymInfectChanceDataNode();
    REQUIRE(std::isnan(sym_infectionchance_node.GetMean()));
    for(size_t i = 0; i < num_bins; i++){
      REQUIRE(sym_infectionchance_node.GetHistCounts()[i] == 0);
    }
    REQUIRE(world.GetNumOrgs() == 0);

    WHEN("free and hosted syms are added"){
      double free_sym_infection_chances[3] = {0, 0.26, 0.73};
      double hosted_sym_infection_chances[3] = {0.32, 0.33, 1.0};

      double expected_av = 0.44;
      emp::vector<long unsigned int> expected_hist_counts(11);
      std::fill(expected_hist_counts.begin(), expected_hist_counts.end(), 0);
      expected_hist_counts[0] = 1;
      expected_hist_counts[2] = 1;
      expected_hist_counts[3] = 2;
      expected_hist_counts[7] = 1;
      expected_hist_counts[9] = 1;

      emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, int_val);
      world.AddOrgAt(host, 0);

      for(size_t i = 0; i < 3; i++){
        emp::Ptr<SGPSymbiont> sym1 = emp::NewPtr<SGPSymbiont>(&random, &world, &config, int_val);
        emp::Ptr<SGPSymbiont> sym2 = emp::NewPtr<SGPSymbiont>(&random, &world, &config, int_val);

        sym1->SetInfectionChance(free_sym_infection_chances[i]);
        sym2->SetInfectionChance(hosted_sym_infection_chances[i]);

        world.AddOrgAt(sym1, emp::WorldPosition(0,i));
        host->AddSymbiont(sym2);
      }
      world.Update();

      THEN("their average infection chance values are tracked"){
        REQUIRE(sym_infectionchance_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(sym_infectionchance_node.GetMean() > (expected_av - 0.0001));
      }
      THEN("they're split into histogram bins correctly"){
        for(size_t i = 0; i < num_bins; i++){
          REQUIRE(sym_infectionchance_node.GetHistCounts()[i] == expected_hist_counts[i]);
        }
      }
    }
  }
}

TEST_CASE("SGP GetFreeSymInfectChanceDataNode", "[dsg]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigSGP config;
    int int_val = 0;
    SGPWorld world(random, &config, LogicTasks);
    size_t world_size = 4;
    world.Resize(world_size);

    config.FREE_LIVING_SYMS(1);
    config.SYM_INFECTION_CHANCE(0);
    config.SYM_LIMIT(3);
    size_t num_bins = 11;

    emp::DataMonitor<double,emp::data::Histogram>& free_sym_infectionchance_node = world.GetFreeSymInfectChanceDataNode();
    REQUIRE(std::isnan(free_sym_infectionchance_node.GetMean()));
    for(size_t i = 0; i < num_bins; i++){
      REQUIRE(free_sym_infectionchance_node.GetHistCounts()[i] == 0);
    }
    REQUIRE(world.GetNumOrgs() == 0);

    WHEN("free and hosted syms are added"){
      double free_sym_infection_chances[3] = {0, 0.26, 0.73};
      double hosted_sym_infection_chances[3] = {0.32, 0.33, 1.0};

      double expected_av = 0.33;
      emp::vector<long unsigned int> expected_hist_counts(11);
      std::fill(expected_hist_counts.begin(), expected_hist_counts.end(), 0);
      expected_hist_counts[0] = 1;
      expected_hist_counts[2] = 1;
      expected_hist_counts[7] = 1;

      emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, int_val);
      world.AddOrgAt(host, 0);

      for(size_t i = 0; i < 3; i++){
        emp::Ptr<SGPSymbiont> sym1 = emp::NewPtr<SGPSymbiont>(&random, &world, &config, int_val);
        emp::Ptr<SGPSymbiont> sym2 = emp::NewPtr<SGPSymbiont>(&random, &world, &config, int_val);

        sym1->SetInfectionChance(free_sym_infection_chances[i]);
        sym2->SetInfectionChance(hosted_sym_infection_chances[i]);

        world.AddOrgAt(sym1, emp::WorldPosition(0,i));
        host->AddSymbiont(sym2);
      }
      world.Update();

      THEN("only free symbiont average infection chances are tracked"){
        REQUIRE(free_sym_infectionchance_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(free_sym_infectionchance_node.GetMean() > (expected_av - 0.0001));
      }
      THEN("only free symbionts are split into histogram bins"){
        for(size_t i = 0; i < num_bins; i++){
          REQUIRE(free_sym_infectionchance_node.GetHistCounts()[i] == expected_hist_counts[i]);
        }
      }
    }
  }
}

TEST_CASE("SGP GetHostedSymInfectChanceDataNode", "[dsg]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigSGP config;
    int int_val = 0;
    SGPWorld world(random, &config, LogicTasks);
    size_t world_size = 4;
    world.Resize(world_size);

    config.FREE_LIVING_SYMS(1);
    config.SYM_INFECTION_CHANCE(0);
    config.SYM_LIMIT(3);
    size_t num_bins = 11;

    emp::DataMonitor<double,emp::data::Histogram>& hosted_sym_infectionchance_node = world.GetHostedSymInfectChanceDataNode();
    REQUIRE(std::isnan(hosted_sym_infectionchance_node.GetMean()));
    REQUIRE(world.GetNumOrgs() == 0);

    WHEN("free and hosted syms are added"){
      double free_sym_infection_chances[3] = {0, 0.26, 0.73};
      double hosted_sym_infection_chances[3] = {0.32, 0.33, 1.0};

      double expected_av = 0.55;
      emp::vector<long unsigned int> expected_hist_counts(11);
      std::fill(expected_hist_counts.begin(), expected_hist_counts.end(), 0);
      expected_hist_counts[3] = 2;
      expected_hist_counts[9] = 1;

      emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, int_val);
      world.AddOrgAt(host, 0);

      for(size_t i = 0; i < 3; i++){
        emp::Ptr<SGPSymbiont> sym1 = emp::NewPtr<SGPSymbiont>(&random, &world, &config, int_val);
        emp::Ptr<SGPSymbiont> sym2 = emp::NewPtr<SGPSymbiont>(&random, &world, &config, int_val);

        sym1->SetInfectionChance(free_sym_infection_chances[i]);
        sym2->SetInfectionChance(hosted_sym_infection_chances[i]);

        world.AddOrgAt(sym1, emp::WorldPosition(0,i));
        host->AddSymbiont(sym2);
      }

      world.Update();
      //check they were tracked correctly
      THEN("only hosted symbiont infection chances are tracked"){
        REQUIRE(hosted_sym_infectionchance_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(hosted_sym_infectionchance_node.GetMean() > (expected_av - 0.0001));
      }
      THEN("only hosted symbionts are split into histogram bins"){
        for(size_t i = 0; i < num_bins; i++){
          REQUIRE(hosted_sym_infectionchance_node.GetHistCounts()[i] == expected_hist_counts[i]);
        }
      }
    }
  }
}

// TEST_CASE("SGP GetHorizontalTransmissionAttemptCount", "[dsg]"){
//   GIVEN( "a world" ) {
//     emp::Random random(17);
//     SymConfigSGP config;
//     int int_val = 0;
//     SGPWorld world(random, &config, LogicTasks);
//     size_t world_size = 4;
//     world.Resize(world_size);
//     config.SYM_HORIZ_TRANS_RES(0);
//     config.FREE_LIVING_SYMS(0);

//     emp::DataMonitor<int>& data_node_attempts_horiztrans = world.GetHorizontalTransmissionAttemptCount();
//     emp::WorldPosition parent_pos = emp::WorldPosition(0, 0);
//     REQUIRE(data_node_attempts_horiztrans.GetTotal() == 0);

   
//     WHEN("Hosted Syms are in the world"){
      
//   emp::Ptr<SGPSymbiont> symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(100));
//       emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, int_val);
//        emp::Ptr<SGPHost> host2 = emp::NewPtr<SGPHost>(&random, &world, &config, int_val);
      
//       world.AddOrgAt(host, 1);
//       world.AddOrgAt(host2, 2);
//       host->AddSymbiont(symbiont);
//       host->GetCPU().state.tasks_performed->Set(0);
//       host2->GetCPU().state.tasks_performed->Set(0);

//       WHEN("A symbiont successfully horizontally transmits into a host"){
//         symbiont->GetCPU().state.tasks_performed->Set(0);
//         symbiont->GetCPU().RunCPUStep(0,100);
//         world.Update();
//         REQUIRE(host2->HasSym() == true);
//         THEN("The count of attempted horizontal transmissions increments"){
//           REQUIRE(data_node_attempts_horiztrans.GetTotal() == 1);
//         }
//       }
//       WHEN("A symbiont dies trying to horizontally transmit into a host"){
//         config.SYM_LIMIT(0);
//         symbiont->GetCPU().state.tasks_performed->Set(0);
//         symbiont->GetCPU().RunCPUStep(0,100);
//         world.Update();
//         REQUIRE(host2->HasSym() == false);
//         THEN("The count of attempted horizontal transmissions increments"){
//           REQUIRE(data_node_attempts_horiztrans.GetTotal() == 1);
//         }
//       }
//       symbiont.Delete();
//     }

//   }
// }

// TEST_CASE("SGP GetHorizontalTransmissionSuccessCount", "[dsg]"){
//   GIVEN( "a world" ) {
//     emp::Random random(17);
//     SymConfigSGP config;
//     int int_val = 0;
//     SGPWorld world(random, &config, LogicTasks);
//     size_t world_size = 4;
//     world.Resize(world_size);
//     config.SYM_HORIZ_TRANS_RES(0);

//     emp::DataMonitor<int>& data_node_successes_horiztrans = world.GetHorizontalTransmissionSuccessCount();
//     emp::WorldPosition parent_pos = emp::WorldPosition(0, 0);
//     REQUIRE(data_node_successes_horiztrans.GetTotal() == 0);

   
//     WHEN("Hosted Syms are in the world"){
//       config.FREE_LIVING_SYMS(0);
//       emp::Ptr<SGPSymbiont> symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config, int_val);
//       emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, int_val);
//        emp::Ptr<SGPHost> host2 = emp::NewPtr<SGPHost>(&random, &world, &config, int_val);
      
//       world.AddOrgAt(host, 1);
//       world.AddOrgAt(host2, 2);
//       host->AddSymbiont(symbiont);
//       host->GetCPU().state.tasks_performed->Set(0);
//       host2->GetCPU().state.tasks_performed->Set(0);

//       WHEN("A symbiont successfully horizontally transmits into a host"){
//         symbiont->GetCPU().state.tasks_performed->Set(0);
//         symbiont->GetCPU().RunCPUStep(0,100);
//         world.Update();
//         REQUIRE(host2->HasSym() == true);
//         THEN("The count of attempted horizontal transmissions increments"){
//           REQUIRE(data_node_successes_horiztrans.GetTotal() == 1);
//         }
//       }
//       WHEN("A symbiont dies trying to horizontally transmit into a host"){
//         config.SYM_LIMIT(0);
//         symbiont->GetCPU().state.tasks_performed->Set(0);
//         symbiont->GetCPU().RunCPUStep(0,100);
//         world.Update();
//         REQUIRE(host2->HasSym() == false);
//         THEN("The count of attempted horizontal transmissions does not increment"){
//           REQUIRE(data_node_successes_horiztrans.GetTotal() == 0);
//         }
//       }
//       symbiont.Delete();
//     }

//   }
// }

TEST_CASE("SGP GetVerticalTransmissionAttemptCount", "[dsg]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigSGP config;
    int int_val = 0;
    SGPWorld world(random, &config, LogicTasks);
    size_t world_size = 4;
    world.Resize(world_size);
    config.SYM_VERT_TRANS_RES(0);
    config.VERTICAL_TRANSMISSION(1);

    emp::DataMonitor<int>& data_node_attempts_verttrans = world.GetVerticalTransmissionAttemptCount();
    REQUIRE(data_node_attempts_verttrans.GetTotal() == 0);

    WHEN("A symbiont baby gets vertically transmitted into a host baby"){
      emp::Ptr<SGPSymbiont> symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config, int_val);
      emp::Ptr<SGPHost> host_baby = emp::NewPtr<SGPHost>(&random, &world, &config, int_val);

      symbiont->VerticalTransmission(host_baby);

      THEN("The count of attempted vertical transmissions increments"){
        REQUIRE(host_baby->HasSym() == true);
        REQUIRE(data_node_attempts_verttrans.GetTotal() == 1);
      }

      symbiont.Delete();
      host_baby.Delete();
    }
  }
}
