#include "../../default_mode/DataNodes.h"
#include "../../default_mode/Symbiont.h"
#include "../../default_mode/Host.h"

TEST_CASE("GetHostCountDataNode", "[default]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    SymWorld world(random, &config);
    size_t world_size = 4;
    world.Resize(world_size);

    emp::DataMonitor<int>& host_count_node = world.GetHostCountDataNode();
    REQUIRE(host_count_node.GetTotal() == 0);
    REQUIRE(world.GetNumOrgs() == 0);

    WHEN("a host is added"){
      size_t num_hosts = 4;
      for(size_t i = 0; i < num_hosts; i++){
        world.AddOrgAt(emp::NewPtr<Host>(&random, &world, &config, int_val), i);
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

TEST_CASE("GetSymCountDataNode", "[default]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    int world_size = 4;
    SymWorld world(random, &config);
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
      emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      world.AddOrgAt(host, 0);
      for(size_t i = 0; i < num_free_syms; i++){
        world.AddOrgAt(emp::NewPtr<Symbiont>(&random, &world, &config, int_val), emp::WorldPosition(0,i));
      }
      for(size_t i = 0; i < num_hosted_syms; i++){
        host->AddSymbiont(emp::NewPtr<Symbiont>(&random, &world, &config, int_val));
      }

      world.Update();

      THEN("they are tracked by the data node"){
        REQUIRE(world.GetNumOrgs() == num_free_syms + 1); // and the host
        REQUIRE(sym_count_node.GetTotal() == num_free_syms + num_hosted_syms);
      }
    }
  }
}

TEST_CASE("GetCountHostedSymsDataNode", "[default]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    SymWorld world(random, &config);
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
        emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
        world.AddOrgAt(host, i);
        for(size_t j = 0; j < num_syms_per_host; j++){
          host->AddSymbiont(emp::NewPtr<Symbiont>(&random, &world, &config, int_val));
        }
      }
      world.Update();
      THEN("they are tracked by the data node"){
        REQUIRE(world.GetNumOrgs() == num_hosts);
        REQUIRE(hosted_sym_count_node.GetTotal() == num_syms_per_host * num_hosts);
      }
    }
    WHEN("a free sym is added"){
      world.AddOrgAt(emp::NewPtr<Symbiont>(&random, &world, &config, int_val), emp::WorldPosition(0,0));
      world.Update();
      THEN("the hosted sym data node doesn't track it"){
        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(hosted_sym_count_node.GetTotal() == 0);
      }
    }
  }
}

TEST_CASE("GetCountFreeSymsDataNode", "[default]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    SymWorld world(random, &config);
    config.SYM_LIMIT(4);
    size_t world_size = 4;
    world.Resize(world_size);

    emp::DataMonitor<int>& free_sym_count_node = world.GetCountFreeSymsDataNode();
    REQUIRE(free_sym_count_node.GetTotal() == 0);
    REQUIRE(world.GetNumOrgs() == 0);

    WHEN("free syms are added"){
      size_t num_free_syms = 4;

      for(size_t i = 0; i < num_free_syms; i++){
        world.AddOrgAt(emp::NewPtr<Symbiont>(&random, &world, &config, int_val), emp::WorldPosition(0, i));
      }

      world.Update();
      THEN("they are tracked by the data node"){
        REQUIRE(world.GetNumOrgs() == num_free_syms);
        REQUIRE(free_sym_count_node.GetTotal() == num_free_syms);
      }
    }

    WHEN("a hosted sym is added"){
      emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      world.AddOrgAt(host, 0);
      host->AddSymbiont(emp::NewPtr<Symbiont>(&random, &world, &config, int_val));
      world.Update();
      THEN("it isn't tracked by the free sym data node"){
        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(free_sym_count_node.GetTotal() == 0);
      }
    }
  }
}

TEST_CASE("GetUninfectedHostsDataNode", "[default]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    SymWorld world(random, &config);
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
        world.AddOrgAt(emp::NewPtr<Host>(&random, &world, &config, int_val), i);
      }

      world.Update();
      THEN("they are tracked by the data node"){
        REQUIRE(world.GetNumOrgs() == num_hosts);
        REQUIRE(uninfected_hosts_data_node.GetTotal() == num_hosts);
      }

      WHEN("some hosts are infected "){
        size_t num_infections = 2;
        for(size_t i = 0; i < num_infections; i++){
          world.GetOrg(i).AddSymbiont(emp::NewPtr<Symbiont>(&random, &world, &config, int_val));
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

TEST_CASE("GetSymIntValDataNode", "[default]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    SymWorld world(random, &config);
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

      emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      world.AddOrgAt(host, 0);
      for(size_t i = 0; i < 3; i++){
        world.AddOrgAt(emp::NewPtr<Symbiont>(&random, &world, &config, free_sym_int_vals[i]), emp::WorldPosition(0,i));
        host->AddSymbiont(emp::NewPtr<Symbiont>(&random, &world, &config, hosted_sym_int_vals[i]));
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

TEST_CASE("GetFreeSymIntValDataNode", "[default]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    SymWorld world(random, &config);
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

      emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      world.AddOrgAt(host, 0);
      for(size_t i = 0; i < 3; i++){
        world.AddOrgAt(emp::NewPtr<Symbiont>(&random, &world, &config, free_sym_int_vals[i]), emp::WorldPosition(0,i));
        host->AddSymbiont(emp::NewPtr<Symbiont>(&random, &world, &config, hosted_sym_int_vals[i]));
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

TEST_CASE("GetHostedSymIntValDataNode", "[default]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    SymWorld world(random, &config);
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

      emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      world.AddOrgAt(host, 0);
      for(size_t i = 0; i < 3; i++){
        world.AddOrgAt(emp::NewPtr<Symbiont>(&random, &world, &config, free_sym_int_vals[i]), emp::WorldPosition(0,i));
        host->AddSymbiont(emp::NewPtr<Symbiont>(&random, &world, &config, hosted_sym_int_vals[i]));
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

TEST_CASE("GetHostIntValDataNode", "[default]"){
  GIVEN("a world"){
    emp::Random random(17);
    SymConfigBase config;
    SymWorld world(random, &config);
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
        world.AddOrgAt(emp::NewPtr<Host>(&random, &world, &config, int_vals[i]), i);
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

TEST_CASE("GetSymInfectChanceDataNode", "[default]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    SymWorld world(random, &config);
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

      emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      world.AddOrgAt(host, 0);

      for(size_t i = 0; i < 3; i++){
        emp::Ptr<Symbiont> sym1 = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
        emp::Ptr<Symbiont> sym2 = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);

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

TEST_CASE("GetFreeSymInfectChanceDataNode", "[default]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    SymWorld world(random, &config);
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

      emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      world.AddOrgAt(host, 0);

      for(size_t i = 0; i < 3; i++){
        emp::Ptr<Symbiont> sym1 = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
        emp::Ptr<Symbiont> sym2 = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);

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

TEST_CASE("GetHostedSymInfectChanceDataNode", "[default]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    SymWorld world(random, &config);
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

      emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      world.AddOrgAt(host, 0);

      for(size_t i = 0; i < 3; i++){
        emp::Ptr<Symbiont> sym1 = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
        emp::Ptr<Symbiont> sym2 = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);

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

TEST_CASE("GetHorizontalTransmissionAttemptCount", "[default]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    double int_val = 1;
    SymWorld world(random, &config);
    size_t world_size = 4;
    world.Resize(world_size);
    config.SYM_HORIZ_TRANS_RES(0);

    emp::DataMonitor<double, emp::data::Histogram>& data_node_attempts_horiztrans = world.GetHorizontalTransmissionAttemptCount();
    emp::WorldPosition parent_pos = emp::WorldPosition(0, 0);
    REQUIRE(data_node_attempts_horiztrans.GetCount() == 0);
    REQUIRE(data_node_attempts_horiztrans.GetHistCounts()[9] == 0);

    WHEN("Free living symbionts are allowed"){
      config.FREE_LIVING_SYMS(1);
      emp::Ptr<Symbiont> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      world.AddOrgAt(symbiont, parent_pos);

      WHEN("A symbiont successfully transmits into a free living cell"){
        symbiont->HorizontalTransmission(parent_pos);
        REQUIRE(world.GetNumOrgs() == 2);

        THEN("The count of attempted horizontal transmissions increments"){
          REQUIRE(data_node_attempts_horiztrans.GetCount() == 1);
          REQUIRE(data_node_attempts_horiztrans.GetHistCounts()[9] == 1);
        }
      }
        WHEN("There are no valid cells to transmit into and the symbiont dies trying to transmit"){
        world.Resize(0);
        symbiont->HorizontalTransmission(parent_pos);
        REQUIRE(world.GetNumOrgs() == 1);
        THEN("The count of attempted horizontal transmissions increments"){
          REQUIRE(data_node_attempts_horiztrans.GetCount() == 1);
          REQUIRE(data_node_attempts_horiztrans.GetHistCounts()[9] == 1);
        }
        symbiont.Delete(); // won't be caught by symworld destructor due to resize
      }
    }
    WHEN("Free living symbionts are not allowed"){
      config.FREE_LIVING_SYMS(0);
      emp::Ptr<Symbiont> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      world.AddOrgAt(host, 1);

      WHEN("A symbiont successfully horizontally transmits into a host"){
        symbiont->HorizontalTransmission(parent_pos);
        REQUIRE(host->HasSym() == true);
        THEN("The count of attempted horizontal transmissions increments"){
          REQUIRE(data_node_attempts_horiztrans.GetCount() == 1);
          REQUIRE(data_node_attempts_horiztrans.GetHistCounts()[9] == 1);
        }
      }
      WHEN("A symbiont dies trying to horizontally transmit into a host"){
        config.SYM_LIMIT(1);
        emp::Ptr<Organism> parent_symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
        host->AddSymbiont(parent_symbiont);
        symbiont->HorizontalTransmission(emp::WorldPosition(1,1));
        REQUIRE(host->HasSym() == true);
        REQUIRE(host->GetSymbionts().size() == 1);
        REQUIRE(host->GetSymbionts().at(0) == parent_symbiont);
        THEN("The count of attempted horizontal transmissions increments"){
          REQUIRE(data_node_attempts_horiztrans.GetCount() == 1);
          REQUIRE(data_node_attempts_horiztrans.GetHistCounts()[9] == 1);
        }
      }
      symbiont.Delete();
    }

  }
}

TEST_CASE("GetHorizontalTransmissionTagFailCount", "[default]") {
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigBase config;
    double int_val = 1;
    SymWorld world(random, &config);
    size_t world_size = 4;
    world.Resize(world_size);
    config.SYM_HORIZ_TRANS_RES(0);

    emp::DataMonitor<double, emp::data::Histogram>& data_node_tagfail_horiztrans = world.GetHorizontalTransmissionTagFailCount();
    emp::WorldPosition parent_pos = emp::WorldPosition(0, 0);
    emp::WorldPosition target_pos = emp::WorldPosition(1, 0);
    REQUIRE(data_node_tagfail_horiztrans.GetCount() == 0);

    WHEN("A symbiont successfully horizontally transmits into a host") {
      emp::Ptr<Symbiont> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      world.AddOrgAt(host, target_pos);
      symbiont->HorizontalTransmission(parent_pos);
      REQUIRE(host->HasSym() == true);
      THEN("The tag failure count is NOT incremented") {
        REQUIRE(data_node_tagfail_horiztrans.GetCount() == 0);
        REQUIRE(data_node_tagfail_horiztrans.GetHistCounts()[9] == 0);
      }
      symbiont.Delete();
    }
    WHEN("A symbiont unsuccessfully horizontally transmits into a host") {
      config.TAG_MATCHING(1);
      world.SetTagMetric(emp::NewPtr<emp::HammingMetric<TAG_LENGTH>>());
      emp::WorldPosition sym_parent_pos = emp::WorldPosition(1, 0);
      emp::Ptr<Symbiont> parent_symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      emp::Ptr<Host> parent_host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      emp::Ptr<Host> target_host = emp::NewPtr<Host>(&random, &world, &config, int_val);

      world.AddOrgAt(parent_host, parent_pos);
      world.AddOrgAt(target_host, target_pos);
      parent_host->AddSymbiont(parent_symbiont);

      emp::BitSet<TAG_LENGTH> tag = emp::BitSet<TAG_LENGTH>(TAG_LENGTH, random, TAG_LENGTH/8);
      emp::BitSet<TAG_LENGTH> dissimilar_tag = emp::BitSet<TAG_LENGTH>(TAG_LENGTH, random, TAG_LENGTH-2);
      parent_symbiont->SetTag(tag);

      WHEN("The failure is only due to tag mismatch") {
        target_host->SetTag(dissimilar_tag);
        parent_symbiont->HorizontalTransmission(sym_parent_pos);
        REQUIRE(target_host->HasSym() == false);
        THEN("The tag failure count is incremented") {
          REQUIRE(data_node_tagfail_horiztrans.GetCount() == 1);
          REQUIRE(data_node_tagfail_horiztrans.GetHistCounts()[9] == 1);
        }
      }
      WHEN("The failure is only due to size") {
        emp::Ptr<Organism> filler_symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
        target_host->AddSymbiont(filler_symbiont);
        target_host->SetTag(tag);
        parent_symbiont->HorizontalTransmission(sym_parent_pos);
        REQUIRE(target_host->HasSym() == true);
        REQUIRE(target_host->GetSymbionts().at(0) == filler_symbiont);
        THEN("The tag failure count is NOT incremented") {
          REQUIRE(data_node_tagfail_horiztrans.GetCount() == 0);
          REQUIRE(data_node_tagfail_horiztrans.GetHistCounts()[9] == 0);
        }
      }
      WHEN("The failure is due to size and tag mismatch") {
        emp::Ptr<Organism> filler_symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
        target_host->AddSymbiont(filler_symbiont);
        target_host->SetTag(dissimilar_tag);
        parent_symbiont->HorizontalTransmission(sym_parent_pos);
        REQUIRE(target_host->HasSym() == true);
        REQUIRE(target_host->GetSymbionts().at(0) == filler_symbiont);
        THEN("The tag failure count is NOT incremented") {
          REQUIRE(data_node_tagfail_horiztrans.GetCount() == 0);
          REQUIRE(data_node_tagfail_horiztrans.GetHistCounts()[9] == 0);
        }
      }
    }
  }
}

TEST_CASE("GetHorizontalTransmissionSizeFailCount", "[default]") {
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigBase config;
    double int_val = 1;
    SymWorld world(random, &config);
    size_t world_size = 4;
    world.Resize(world_size);
    config.SYM_HORIZ_TRANS_RES(0);

    emp::DataMonitor<double, emp::data::Histogram>& data_node_sizefail_horiztrans = world.GetHorizontalTransmissionSizeFailCount();
    emp::WorldPosition parent_pos = emp::WorldPosition(0, 0);
    emp::WorldPosition target_pos = emp::WorldPosition(1, 0);
    REQUIRE(data_node_sizefail_horiztrans.GetCount() == 0);

    WHEN("A symbiont successfully horizontally transmits into a host") {
      emp::Ptr<Symbiont> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      world.AddOrgAt(host, target_pos);
      symbiont->HorizontalTransmission(parent_pos);
      REQUIRE(host->HasSym() == true);
      THEN("The tag failure count is NOT incremented") {
        REQUIRE(data_node_sizefail_horiztrans.GetCount() == 0);
        REQUIRE(data_node_sizefail_horiztrans.GetHistCounts()[9] == 0);
      }
      symbiont.Delete();
    }
    WHEN("A symbiont unsuccessfully horizontally transmits into a host") {
      config.TAG_MATCHING(1);
      world.SetTagMetric(emp::NewPtr<emp::HammingMetric<TAG_LENGTH>>());
      emp::WorldPosition sym_parent_pos = emp::WorldPosition(1, 0);
      emp::Ptr<Symbiont> parent_symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      emp::Ptr<Host> parent_host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      emp::Ptr<Host> target_host = emp::NewPtr<Host>(&random, &world, &config, int_val);

      world.AddOrgAt(parent_host, parent_pos);
      world.AddOrgAt(target_host, target_pos);
      parent_host->AddSymbiont(parent_symbiont);

      emp::BitSet<TAG_LENGTH> tag = emp::BitSet<TAG_LENGTH>(TAG_LENGTH, random, TAG_LENGTH/8);
      emp::BitSet<TAG_LENGTH> dissimilar_tag = emp::BitSet<TAG_LENGTH>(TAG_LENGTH, random, TAG_LENGTH-2);
      parent_symbiont->SetTag(tag);

      WHEN("The failure is only due to tag mismatch") {
        target_host->SetTag(dissimilar_tag);
        parent_symbiont->HorizontalTransmission(sym_parent_pos);
        REQUIRE(target_host->HasSym() == false);
        THEN("The size failure count is NOT incremented") {
          REQUIRE(data_node_sizefail_horiztrans.GetCount() == 0);
          REQUIRE(data_node_sizefail_horiztrans.GetHistCounts()[9] == 0);
        }
      }
      WHEN("The failure is only due to size") {
        emp::Ptr<Organism> filler_symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
        target_host->AddSymbiont(filler_symbiont);
        target_host->SetTag(tag);
        parent_symbiont->HorizontalTransmission(sym_parent_pos);
        REQUIRE(target_host->HasSym() == true);
        REQUIRE(target_host->GetSymbionts().at(0) == filler_symbiont);
        THEN("The size failure count is incremented") {
          REQUIRE(data_node_sizefail_horiztrans.GetCount() == 1);
          REQUIRE(data_node_sizefail_horiztrans.GetHistCounts()[9] == 1);
        }
      }
      WHEN("The failure is due to size and tag mismatch") {
        emp::Ptr<Organism> filler_symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
        target_host->AddSymbiont(filler_symbiont);
        target_host->SetTag(dissimilar_tag);
        parent_symbiont->HorizontalTransmission(sym_parent_pos);
        REQUIRE(target_host->HasSym() == true);
        REQUIRE(target_host->GetSymbionts().at(0) == filler_symbiont);
        THEN("The size failure count is NOT incremented") {
          REQUIRE(data_node_sizefail_horiztrans.GetCount() == 0);
          REQUIRE(data_node_sizefail_horiztrans.GetHistCounts()[9] == 0);
        }
      }
    }
  }
}

TEST_CASE("GetHorizontalTransmissionSuccessCount", "[default]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    double int_val = 1;
    SymWorld world(random, &config);
    size_t world_size = 4;
    world.Resize(world_size);
    config.SYM_HORIZ_TRANS_RES(0);

    emp::DataMonitor<double, emp::data::Histogram>& data_node_successes_horiztrans = world.GetHorizontalTransmissionSuccessCount();
    emp::WorldPosition parent_pos = emp::WorldPosition(0, 0);
    REQUIRE(data_node_successes_horiztrans.GetCount() == 0);

    WHEN("Free living symbionts are allowed"){
      config.FREE_LIVING_SYMS(1);
      emp::Ptr<Symbiont> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      world.AddOrgAt(symbiont, parent_pos);

      WHEN("A symbiont successfully transmits into a free living cell"){
        symbiont->HorizontalTransmission(parent_pos);
        REQUIRE(world.GetNumOrgs() == 2);

        THEN("The count of successful horizontal transmissions increments"){
          REQUIRE(data_node_successes_horiztrans.GetCount() == 1);
          REQUIRE(data_node_successes_horiztrans.GetHistCounts()[9] == 1);
        }
      }
      WHEN("There are no valid cells to transmit into and the symbiont dies trying to transmit"){
        world.Resize(0);
        symbiont->HorizontalTransmission(parent_pos);
        REQUIRE(world.GetNumOrgs() == 1);
        THEN("The count of successful horizontal transmissions does not change"){
          REQUIRE(data_node_successes_horiztrans.GetCount() == 0);
          REQUIRE(data_node_successes_horiztrans.GetHistCounts()[9] == 0);
        }
        symbiont.Delete(); // won't be caught by symworld destructor due to resize 
      }
    }
    WHEN("Free living symbionts are not allowed"){
      config.FREE_LIVING_SYMS(0);
      emp::Ptr<Symbiont> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      world.AddOrgAt(host, 1);

      WHEN("A symbiont successfully horizontally transmits into a host"){
        symbiont->HorizontalTransmission(parent_pos);
        REQUIRE(host->HasSym() == true);
        THEN("The count of successful horizontal transmissions increments"){
          REQUIRE(data_node_successes_horiztrans.GetCount() == 1);
          REQUIRE(data_node_successes_horiztrans.GetHistCounts()[9] == 1);
        }
      }
      WHEN("A symbiont dies trying to horizontally transmit into a host"){
        config.SYM_LIMIT(1);
        emp::Ptr<Organism> parent_symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
        host->AddSymbiont(parent_symbiont);
        symbiont->HorizontalTransmission(emp::WorldPosition(1, 1));
        REQUIRE(host->HasSym() == true);
        REQUIRE(host->GetSymbionts().size() == 1);
        REQUIRE(host->GetSymbionts().at(0) == parent_symbiont);
        THEN("The count of successful horizontal transmissions does not change") {
          REQUIRE(data_node_successes_horiztrans.GetCount() == 0);
          REQUIRE(data_node_successes_horiztrans.GetHistCounts()[9] == 0);
        }
      }
      symbiont.Delete();
    }

  }
}

TEST_CASE("GetVerticalTransmissionAttemptCount", "[default]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    config.TAG_MATCHING(1);
    SymWorld world(random, &config);
    size_t world_size = 4;
    world.Resize(world_size);
    config.SYM_VERT_TRANS_RES(0);
    config.VERTICAL_TRANSMISSION(1);
    
    emp::DataMonitor<double, emp::data::Histogram>& data_node_attempts_verttrans = world.GetVerticalTransmissionAttemptCount();
    REQUIRE(data_node_attempts_verttrans.GetCount() == 0);
    
    WHEN("A symbiont baby gets vertically transmitted into a host baby"){
      emp::Ptr<Symbiont> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      emp::Ptr<Host> host_baby = emp::NewPtr<Host>(&random, &world, &config, int_val);

      WHEN("The symbiont's parent has a very high interaction value") {
        symbiont->SetIntVal(1.0);
        symbiont->VerticalTransmission(host_baby);
        THEN("The count of successful vertical transmissions increments and the correct histogram bin is incremented") {
          REQUIRE(host_baby->HasSym() == true);
          REQUIRE(data_node_attempts_verttrans.GetCount() == 1);
          REQUIRE(data_node_attempts_verttrans.GetHistCounts()[9] == 1);
        }
      }
      WHEN("The symbiont's parent has a middling interaction value") {
        symbiont->SetIntVal(-0.2);
        symbiont->VerticalTransmission(host_baby);
        THEN("The count of successful vertical transmissions increments and the correct histogram bin is incremented") {
          REQUIRE(host_baby->HasSym() == true);
          REQUIRE(data_node_attempts_verttrans.GetCount() == 1);
          REQUIRE(data_node_attempts_verttrans.GetHistCounts()[4] == 1);
        }
      }
      WHEN("The symbiont's parent has a very low interaction value") {
        symbiont->SetIntVal(-1);
        symbiont->VerticalTransmission(host_baby);
        THEN("The count of successful vertical transmissions increments and the correct histogram bin is incremented") {
          REQUIRE(host_baby->HasSym() == true);
          REQUIRE(data_node_attempts_verttrans.GetCount() == 1);
          REQUIRE(data_node_attempts_verttrans.GetHistCounts()[0] == 1);
        }
      }

      symbiont.Delete();
      host_baby.Delete();
    }
    
    WHEN("A symbiont baby tries to vertically transmitted into a host baby but their tags mismatch") {
      emp::Ptr<Symbiont> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      emp::Ptr<Host> host_baby = emp::NewPtr<Host>(&random, &world, &config, int_val);
      
      emp::BitSet<TAG_LENGTH> sym_bit_set = emp::BitSet<TAG_LENGTH>();
      emp::BitSet<TAG_LENGTH> host_bit_set = emp::BitSet<TAG_LENGTH>(TAG_LENGTH, random, TAG_LENGTH/2);
      symbiont->SetTag(sym_bit_set);
      host_baby->SetTag(host_bit_set);

      WHEN("The symbiont's parent has a very high interaction value") {
        symbiont->SetIntVal(1.0);
        symbiont->VerticalTransmission(host_baby);
        THEN("The count of successful vertical transmissions increments and the correct histogram bin is incremented") {
          REQUIRE(host_baby->HasSym() == false);
          REQUIRE(data_node_attempts_verttrans.GetCount() == 1);
          REQUIRE(data_node_attempts_verttrans.GetHistCounts()[9] == 1);
        }
      }
      WHEN("The symbiont's parent has a middling interaction value") {
        symbiont->SetIntVal(-0.2);
        symbiont->VerticalTransmission(host_baby);
        THEN("The count of successful vertical transmissions increments and the correct histogram bin is incremented") {
          REQUIRE(host_baby->HasSym() == false);
          REQUIRE(data_node_attempts_verttrans.GetCount() == 1);
          REQUIRE(data_node_attempts_verttrans.GetHistCounts()[4] == 1);
        }
      }
      WHEN("The symbiont's parent has a very low interaction value") {
        symbiont->SetIntVal(-1);
        symbiont->VerticalTransmission(host_baby);
        THEN("The count of successful vertical transmissions increments and the correct histogram bin is incremented") {
          REQUIRE(host_baby->HasSym() == false);
          REQUIRE(data_node_attempts_verttrans.GetCount() == 1);
          REQUIRE(data_node_attempts_verttrans.GetHistCounts()[0] == 1);
        }
      }
      
      symbiont.Delete();
      host_baby.Delete();
    }
  }
}

TEST_CASE("GetVerticalTransmissionSuccessCount", "[default]") {
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigBase config;
    double int_val = 1;
    config.TAG_MATCHING(1);
    SymWorld world(random, &config);
    size_t world_size = 4;
    world.Resize(world_size);
    config.SYM_VERT_TRANS_RES(0);
    config.VERTICAL_TRANSMISSION(1);

    emp::DataMonitor<double, emp::data::Histogram>& data_node_successes_verttrans = world.GetVerticalTransmissionSuccessCount();
    REQUIRE(data_node_successes_verttrans.GetCount() == 0);

    WHEN("A symbiont baby gets vertically transmitted into a host baby") {
      emp::Ptr<Symbiont> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      emp::Ptr<Host> host_baby = emp::NewPtr<Host>(&random, &world, &config, int_val);

      emp::BitSet<TAG_LENGTH> sym_bit_set = emp::BitSet<TAG_LENGTH>();
      emp::BitSet<TAG_LENGTH> host_bit_set = emp::BitSet<TAG_LENGTH>();
      symbiont->SetTag(sym_bit_set);
      host_baby->SetTag(host_bit_set);

      WHEN("The symbiont's parent has a very high interaction value"){
        symbiont->SetIntVal(1.0);
        symbiont->VerticalTransmission(host_baby);
        THEN("The count of successful vertical transmissions increments and the correct histogram bin is incremented") {
          REQUIRE(host_baby->HasSym() == true);
          REQUIRE(data_node_successes_verttrans.GetCount() == 1);
          REQUIRE(data_node_successes_verttrans.GetHistCounts()[9]);
        }
      }
      WHEN("The symbiont's parent has a middling interaction value"){
        symbiont->SetIntVal(-0.2);
        symbiont->VerticalTransmission(host_baby);
        THEN("The count of successful vertical transmissions increments and the correct histogram bin is incremented") {
          REQUIRE(host_baby->HasSym() == true);
          REQUIRE(data_node_successes_verttrans.GetCount() == 1);
          REQUIRE(data_node_successes_verttrans.GetHistCounts()[4] == 1);
        }
      }
      WHEN("The symbiont's parent has a very low interaction value"){
        symbiont->SetIntVal(-1);
        symbiont->VerticalTransmission(host_baby);
        THEN("The count of successful vertical transmissions increments and the correct histogram bin is incremented") {
          REQUIRE(host_baby->HasSym() == true);
          REQUIRE(data_node_successes_verttrans.GetCount() == 1);
          REQUIRE(data_node_successes_verttrans.GetHistCounts()[0] == 1);
        }
      }
      
      symbiont.Delete();
      host_baby.Delete();
    }
    WHEN("A symbiont baby tries to vertically transmitted into a host baby but their tags mismatch") {
      emp::Ptr<Symbiont> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      emp::Ptr<Host> host_baby = emp::NewPtr<Host>(&random, &world, &config, int_val);

      emp::BitSet<TAG_LENGTH> sym_bit_set = emp::BitSet<TAG_LENGTH>();
      emp::BitSet<TAG_LENGTH> host_bit_set = emp::BitSet<TAG_LENGTH>(TAG_LENGTH, random, TAG_LENGTH/2);
      symbiont->SetTag(sym_bit_set);
      host_baby->SetTag(host_bit_set);

      symbiont->VerticalTransmission(host_baby);

      THEN("The count of successful vertical transmissions does not increment") {
        REQUIRE(host_baby->HasSym() == false);
        REQUIRE(data_node_successes_verttrans.GetCount() == 0);
      }

      symbiont.Delete();
      host_baby.Delete();
    }
  }
}

TEST_CASE("GetHostTagPermissiveness", "[default]") {
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    config.TAG_MATCHING(1);
    SymWorld world(random, &config);

    emp::DataMonitor<double>& data_node_host_permissiveness = world.GetHostTagPermissiveness();
    REQUIRE(data_node_host_permissiveness.GetCount() == 0);
    REQUIRE(std::isnan(data_node_host_permissiveness.GetMean()));

    double host_1_permissiveness = 0.125;
    emp::Ptr<Host> host_1 = emp::NewPtr<Host>(&random, &world, &config, int_val);
    host_1->SetTagPermissiveness(host_1_permissiveness);
    world.InjectHost(host_1);
    world.Update();
    REQUIRE(data_node_host_permissiveness.GetCount() == 1);
    REQUIRE(data_node_host_permissiveness.GetMean() == host_1_permissiveness);

    WHEN("A host with new permissiveness value is added to the world") {
      double host_2_permissiveness = 0.375;
      emp::Ptr<Host> host_2 = emp::NewPtr<Host>(&random, &world, &config, int_val);
      host_2->SetTagPermissiveness(host_2_permissiveness);
      world.InjectHost(host_2);

      REQUIRE(data_node_host_permissiveness.GetCount() == 1);
      REQUIRE(data_node_host_permissiveness.GetMean() == host_1_permissiveness);
      world.Update();

      THEN("The host's permissiveness gets included in the permissiveness mean data node") {
        REQUIRE(data_node_host_permissiveness.GetCount() == 2);
        REQUIRE(data_node_host_permissiveness.GetMean() == (host_1_permissiveness + host_2_permissiveness) / 2.0);
      }
    }
  }
}