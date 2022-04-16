#include "../../default_mode/DataNodes.h"
#include "../../default_mode/Symbiont.h"
#include "../../default_mode/Host.h"

TEST_CASE("GetHostCountDataNode", "[default]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    SymWorld w(random);
    w.Resize(4);

    emp::DataMonitor<int>& host_count_node = w.GetHostCountDataNode();
    REQUIRE(host_count_node.GetTotal() == 0);
    REQUIRE(w.GetNumOrgs() == 0);

    WHEN("a host is added"){
      size_t num_hosts = 4;
      for(size_t i = 0; i < num_hosts; i++){
        w.AddOrgAt(new Host(&random, &w, &config, int_val), i);
      }
      w.Update();
      host_count_node = w.GetHostCountDataNode();
      THEN("the host is tracked by the data node"){
        REQUIRE(w.GetNumOrgs() == num_hosts);
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
    SymWorld w(random);
    w.Resize(4);
    w.SetFreeLivingSyms(1);
    config.SYM_LIMIT(2);

    emp::DataMonitor<int>& sym_count_node = w.GetSymCountDataNode();
    REQUIRE(sym_count_node.GetTotal() == 0);
    REQUIRE(w.GetNumOrgs() == 0);

    WHEN("free and hosted syms are added"){
      size_t num_free_syms = 4;
      size_t num_hosted_syms = 2;
      Host *h = new Host(&random, &w, &config, int_val);
      w.AddOrgAt(h, 0);
      for(size_t i = 0; i < num_free_syms; i++){
        w.AddOrgAt(new Symbiont(&random, &w, &config, int_val), emp::WorldPosition(0,i));
      }
      for(size_t i = 0; i < num_hosted_syms; i++){
        h->AddSymbiont(new Symbiont(&random, &w, &config, int_val));
      }

      w.Update();
      sym_count_node = w.GetSymCountDataNode();
      THEN("they are tracked by the data node"){
        REQUIRE(w.GetNumOrgs() == num_free_syms + 1); // and the host
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
    SymWorld w(random);
    config.SYM_LIMIT(4);
    w.Resize(4);

    emp::DataMonitor<int>& hosted_sym_count_node = w.GetCountHostedSymsDataNode();
    REQUIRE(hosted_sym_count_node.GetTotal() == 0);
    REQUIRE(w.GetNumOrgs() == 0);

    WHEN("hosted syms are added"){
      size_t num_syms_per_host = 4;
      size_t num_hosts = 2;
      for(size_t i = 0; i < num_hosts; i++){
        Host *h = new Host(&random, &w, &config, int_val);
        w.AddOrgAt(h, i);
        for(size_t j = 0; j < num_syms_per_host; j++){
          h->AddSymbiont(new Symbiont(&random, &w, &config, int_val));
        }
      }
      w.Update();
      THEN("they are tracked by the data node"){
        REQUIRE(w.GetNumOrgs() == num_hosts);
        REQUIRE(hosted_sym_count_node.GetTotal() == num_syms_per_host * num_hosts);
      }
    }
    WHEN("a free sym is added"){
      w.AddOrgAt(new Symbiont(&random, &w, &config, int_val), emp::WorldPosition(0,0));
      w.Update();
      THEN("the hosted sym data node doesn't track it"){
        REQUIRE(w.GetNumOrgs() == 1);
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
    SymWorld w(random);
    config.SYM_LIMIT(4);
    w.Resize(4);

    emp::DataMonitor<int>& free_sym_count_node = w.GetCountFreeSymsDataNode();
    REQUIRE(free_sym_count_node.GetTotal() == 0);
    REQUIRE(w.GetNumOrgs() == 0);

    WHEN("free syms are added"){
      size_t num_free_syms = 4;

      for(size_t i = 0; i < num_free_syms; i++){
        w.AddOrgAt(new Symbiont(&random, &w, &config, int_val), emp::WorldPosition(0, i));
      }

      w.Update();
      THEN("they are tracked by the data node"){
        REQUIRE(w.GetNumOrgs() == num_free_syms);
        REQUIRE(free_sym_count_node.GetTotal() == num_free_syms);
      }
    }

    WHEN("a hosted sym is added"){
      Host *h = new Host(&random, &w, &config, int_val);
      w.AddOrgAt(h, 0);
      h->AddSymbiont(new Symbiont(&random, &w, &config, int_val));
      w.Update();
      THEN("it isn't tracked by the free sym data node"){
        REQUIRE(w.GetNumOrgs() == 1);
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
    SymWorld w(random);
    config.SYM_LIMIT(4);
    w.Resize(10);

    //keep track of host organisms that are uninfected
    emp::DataMonitor<int>& uninfected_hosts_data_node = w.GetUninfectedHostsDataNode();
    REQUIRE(uninfected_hosts_data_node.GetTotal() == 0);
    REQUIRE(w.GetNumOrgs() == 0);

    WHEN("uninfected hosts are added"){
      size_t num_hosts = 10;

      for(size_t i = 0; i < num_hosts; i++){
        w.AddOrgAt(new Host(&random, &w, &config, int_val), i);
      }

      w.Update();
      THEN("they are tracked by the data node"){
        REQUIRE(w.GetNumOrgs() == num_hosts);
        REQUIRE(uninfected_hosts_data_node.GetTotal() == num_hosts);
      }

      WHEN("some hosts are infected "){
        size_t num_infections = 2;
        for(size_t i = 0; i < num_infections; i++){
          w.GetOrg(i).AddSymbiont(new Symbiont(&random, &w, &config, int_val));
        }
        w.Update();
        THEN("infected hosts are excluded from the uninfected host count"){
          REQUIRE(w.GetNumOrgs() == num_hosts);
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
    SymWorld w(random);
    w.Resize(4);
    w.SetFreeLivingSyms(1);
    config.SYM_LIMIT(3);
    size_t max_bin = 21;

    emp::DataMonitor<double,emp::data::Histogram>& sym_intval_node = w.GetSymIntValDataNode();
    REQUIRE(std::isnan(sym_intval_node.GetMean()));
    for(size_t i = 0; i < max_bin; i++){
      REQUIRE(sym_intval_node.GetHistCounts()[i] == 0);
    }
    REQUIRE(w.GetNumOrgs() == 0);

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

      Host *host = new Host(&random, &w, &config, int_val);
      w.AddOrgAt(host, 0);
      for(size_t i = 0; i < 3; i++){
        w.AddOrgAt(new Symbiont(&random, &w, &config, free_sym_int_vals[i]), emp::WorldPosition(0,i));
        host->AddSymbiont(new Symbiont(&random, &w, &config, hosted_sym_int_vals[i]));
      }
      w.Update();

      THEN("their average interaction values are tracked"){
        REQUIRE(sym_intval_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(sym_intval_node.GetMean() > (expected_av - 0.0001));
      }
      THEN("they were split into histogram bins correctly"){
        for(size_t i = 0; i < max_bin; i++){
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
    SymWorld w(random);
    w.Resize(4);
    w.SetFreeLivingSyms(1);
    config.SYM_LIMIT(3);
    size_t max_bin = 21;

    emp::DataMonitor<double,emp::data::Histogram>& free_sym_intval_node = w.GetFreeSymIntValDataNode();

    //initial checks for emptiness
    REQUIRE(std::isnan(free_sym_intval_node.GetMean()));
    REQUIRE(w.GetNumOrgs() == 0);
    for(size_t i = 0; i < max_bin; i++){
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

      Host *host = new Host(&random, &w, &config, int_val);
      w.AddOrgAt(host, 0);
      for(size_t i = 0; i < 3; i++){
        w.AddOrgAt(new Symbiont(&random, &w, &config, free_sym_int_vals[i]), emp::WorldPosition(0,i));
        host->AddSymbiont(new Symbiont(&random, &w, &config, hosted_sym_int_vals[i]));
      }
      w.Update();

      THEN("only free symbionts average interaction values are tracked"){
        REQUIRE(free_sym_intval_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(free_sym_intval_node.GetMean() > (expected_av - 0.0001));
      }
      THEN("only free symbionts were split into histogram bins correctly"){
        for(size_t i = 0; i < max_bin; i++){
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
    SymWorld w(random);
    w.Resize(4);
    w.SetFreeLivingSyms(1);
    config.SYM_LIMIT(3);
    size_t max_bin = 21;

    emp::DataMonitor<double,emp::data::Histogram>& hosted_sym_intval_node = w.GetHostedSymIntValDataNode();
    REQUIRE(std::isnan(hosted_sym_intval_node.GetMean()));
    REQUIRE(w.GetNumOrgs() == 0);
    for(size_t i = 0; i < max_bin; i++){
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

      Host *host = new Host(&random, &w, &config, int_val);
      w.AddOrgAt(host, 0);
      for(size_t i = 0; i < 3; i++){
        w.AddOrgAt(new Symbiont(&random, &w, &config, free_sym_int_vals[i]), emp::WorldPosition(0,i));
        host->AddSymbiont(new Symbiont(&random, &w, &config, hosted_sym_int_vals[i]));
      }
      w.Update();

      THEN("only hosted symbiont average interaction values are tracked"){
        REQUIRE(hosted_sym_intval_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(hosted_sym_intval_node.GetMean() > (expected_av - 0.0001));
      }
      THEN("only hosted symbionts were split into histogram bins correctly"){
        for(size_t i = 0; i < max_bin; i++){
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
    SymWorld w(random);
    w.Resize(4);
    w.SetFreeLivingSyms(1);
    config.SYM_LIMIT(3);
    size_t max_bin = 21;

    emp::DataMonitor<double,emp::data::Histogram>& host_intval_node = w.GetHostIntValDataNode();
    REQUIRE(std::isnan(host_intval_node.GetMean()));
    for(size_t i = 0; i < max_bin; i++){
      REQUIRE(host_intval_node.GetHistCounts()[i] == 0);
    }
    REQUIRE(w.GetNumOrgs() == 0);

    WHEN("hosts are added"){
      double int_vals[4] = {-1.0, -0.92, 0.38, 1.0};
      double expected_av = -0.135;

      emp::vector<long unsigned int> expected_hist_counts(21);
      std::fill(expected_hist_counts.begin(), expected_hist_counts.end(), 0);
      expected_hist_counts[0] = 2;
      expected_hist_counts[13] = 1;
      expected_hist_counts[19] = 1;

      for(size_t i = 0; i < 4; i++){
        w.AddOrgAt(new Host(&random, &w, &config, int_vals[i]), i);
      }
      w.Update();

      THEN("their average interaction values are tracked"){
        REQUIRE(host_intval_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(host_intval_node.GetMean() > (expected_av - 0.0001));
      }
      THEN("they were split into histogram bins correctly"){
        for(size_t i = 0; i < max_bin; i++){
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
    SymWorld w(random);
    w.Resize(4);
    w.SetFreeLivingSyms(1);
    config.SYM_LIMIT(3);
    size_t max_bin = 11;

    emp::DataMonitor<double,emp::data::Histogram>& sym_infectionchance_node = w.GetSymInfectChanceDataNode();
    REQUIRE(std::isnan(sym_infectionchance_node.GetMean()));
    for(size_t i = 0; i < max_bin; i++){
      REQUIRE(sym_infectionchance_node.GetHistCounts()[i] == 0);
    }
    REQUIRE(w.GetNumOrgs() == 0);

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

      Host *host = new Host(&random, &w, &config, int_val);
      w.AddOrgAt(host, 0);

      for(size_t i = 0; i < 3; i++){
        Symbiont *sym1 = new Symbiont(&random, &w, &config, int_val);
        Symbiont *sym2 = new Symbiont(&random, &w, &config, int_val);

        sym1->SetInfectionChance(free_sym_infection_chances[i]);
        sym2->SetInfectionChance(hosted_sym_infection_chances[i]);

        w.AddOrgAt(sym1, emp::WorldPosition(0,i));
        host->AddSymbiont(sym2);
      }
      w.Update();

      THEN("their average infection chance values are tracked"){
        REQUIRE(sym_infectionchance_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(sym_infectionchance_node.GetMean() > (expected_av - 0.0001));
      }
      THEN("they're split into histogram bins correctly"){
        for(size_t i = 0; i < max_bin; i++){
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
    SymWorld w(random);
    w.Resize(4);
    w.SetFreeLivingSyms(1);
    config.SYM_LIMIT(3);
    size_t max_bin = 11;

    emp::DataMonitor<double,emp::data::Histogram>& free_sym_infectionchance_node = w.GetFreeSymInfectChanceDataNode();
    REQUIRE(std::isnan(free_sym_infectionchance_node.GetMean()));
    for(size_t i = 0; i < max_bin; i++){
      REQUIRE(free_sym_infectionchance_node.GetHistCounts()[i] == 0);
    }
    REQUIRE(w.GetNumOrgs() == 0);

    WHEN("free and hosted syms are added"){
      double free_sym_infection_chances[3] = {0, 0.26, 0.73};
      double hosted_sym_infection_chances[3] = {0.32, 0.33, 1.0};

      double expected_av = 0.33;
      emp::vector<long unsigned int> expected_hist_counts(11);
      std::fill(expected_hist_counts.begin(), expected_hist_counts.end(), 0);
      expected_hist_counts[0] = 1;
      expected_hist_counts[2] = 1;
      expected_hist_counts[7] = 1;

      Host *host = new Host(&random, &w, &config, int_val);
      w.AddOrgAt(host, 0);

      for(size_t i = 0; i < 3; i++){
        Symbiont *sym1 = new Symbiont(&random, &w, &config, int_val);
        Symbiont *sym2 = new Symbiont(&random, &w, &config, int_val);

        sym1->SetInfectionChance(free_sym_infection_chances[i]);
        sym2->SetInfectionChance(hosted_sym_infection_chances[i]);

        w.AddOrgAt(sym1, emp::WorldPosition(0,i));
        host->AddSymbiont(sym2);
      }
      w.Update();

      THEN("only free symbiont average infection chances are tracked"){
        REQUIRE(free_sym_infectionchance_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(free_sym_infectionchance_node.GetMean() > (expected_av - 0.0001));
      }
      THEN("only free symbionts are split into histogram bins"){
        for(size_t i = 0; i < max_bin; i++){
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
    SymWorld w(random);
    w.Resize(4);
    w.SetFreeLivingSyms(1);
    config.SYM_LIMIT(3);
    size_t max_bin = 11;

    emp::DataMonitor<double,emp::data::Histogram>& hosted_sym_infectionchance_node = w.GetHostedSymInfectChanceDataNode();
    REQUIRE(std::isnan(hosted_sym_infectionchance_node.GetMean()));
    REQUIRE(w.GetNumOrgs() == 0);

    WHEN("free and hosted syms are added"){
      double free_sym_infection_chances[3] = {0, 0.26, 0.73};
      double hosted_sym_infection_chances[3] = {0.32, 0.33, 1.0};

      double expected_av = 0.55;
      emp::vector<long unsigned int> expected_hist_counts(11);
      std::fill(expected_hist_counts.begin(), expected_hist_counts.end(), 0);
      expected_hist_counts[3] = 2;
      expected_hist_counts[9] = 1;

      Host *host = new Host(&random, &w, &config, int_val);
      w.AddOrgAt(host, 0);

      for(size_t i = 0; i < 3; i++){
        Symbiont *sym1 = new Symbiont(&random, &w, &config, int_val);
        Symbiont *sym2 = new Symbiont(&random, &w, &config, int_val);

        sym1->SetInfectionChance(free_sym_infection_chances[i]);
        sym2->SetInfectionChance(hosted_sym_infection_chances[i]);

        w.AddOrgAt(sym1, emp::WorldPosition(0,i));
        host->AddSymbiont(sym2);
      }

      w.Update();
      //check they were tracked correctly
      THEN("only hosted symbiont infection chances are tracked"){
        REQUIRE(hosted_sym_infectionchance_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(hosted_sym_infectionchance_node.GetMean() > (expected_av - 0.0001));
      }
      THEN("only hosted symbionts are split into histogram bins"){
        for(size_t i = 0; i < max_bin; i++){
          REQUIRE(hosted_sym_infectionchance_node.GetHistCounts()[i] == expected_hist_counts[i]);
        }
      }
    }
  }
}
