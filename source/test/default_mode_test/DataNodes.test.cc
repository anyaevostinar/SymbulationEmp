#include "../../default_mode/DataNodes.h"
#include "../../default_mode/Symbiont.h"
#include "../../default_mode/Host.h"
#include "../../lysis_mode/Phage.h"
#include "../../lysis_mode/Bacterium.h"
#include "../../lysis_mode/LysisWorld.h"

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

TEST_CASE("GetCFUDataNode", "[default]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    LysisWorld w(random);
    config.SYM_LIMIT(4);
    w.Resize(10);

    //keep track of host organisms that are uninfected or infected with only lysogenic phage
    emp::DataMonitor<int>& cfu_data_node = w.GetCFUDataNode();
    REQUIRE(cfu_data_node.GetTotal() == 0);
    REQUIRE(w.GetNumOrgs() == 0);

    WHEN("uninfected hosts are added"){
      size_t num_hosts = 10;

      for(size_t i = 0; i < num_hosts; i++){
        w.AddOrgAt(new Bacterium(&random, &w, &config, int_val), i);
      }

      w.Update();
      THEN("they are tracked by the data node"){
        REQUIRE(w.GetNumOrgs() == num_hosts);
        REQUIRE(cfu_data_node.GetTotal() == num_hosts);
      }

      WHEN("some hosts are infected with lytic phage"){
        size_t num_infections = 2;
        for(size_t i = 0; i < num_infections; i++){
          Phage *p = new Phage(&random, &w, &config, int_val);
          p->SetLysisChance(1.0);
          w.GetOrg(i).AddSymbiont(p);
        }
        w.Update();
        THEN("infected hosts are excluded from the cfu count"){
          REQUIRE(w.GetNumOrgs() == num_hosts);
          REQUIRE(cfu_data_node.GetTotal() == (num_hosts - num_infections));
        }
      }

      WHEN("hosts are infected with lysogenic phage"){
        size_t num_infections = 2;
        for(size_t i = 0; i < num_infections; i++){
          Phage *p = new Phage(&random, &w, &config, int_val);
          p->SetLysisChance(0.0);
          w.GetOrg(i).AddSymbiont(p);
        }
        w.Update();
        THEN("infected hosts are excluded from the cfu count"){
          REQUIRE(w.GetNumOrgs() == num_hosts);
          REQUIRE(cfu_data_node.GetTotal() == num_hosts);
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

    size_t max_bin = 20;

    emp::DataMonitor<double,emp::data::Histogram>& sym_intval_node = w.GetSymIntValDataNode();
    sym_intval_node.SetupBins(-1.0, 1.0, max_bin);
    REQUIRE(std::isnan(sym_intval_node.GetMean()));
    for(size_t i = 0; i < max_bin; i++){
      REQUIRE(sym_intval_node.GetHistCounts()[i] == 0);
    }
    REQUIRE(w.GetNumOrgs() == 0);

    WHEN("free and hosted syms are added"){
      //setup
      size_t num_syms = 6;
      double int_vals[num_syms] = {-1.0, -0.43, 0, 0.71, 0.75, 1.0};
      double expected_av = 0;
      for(size_t i = 0; i < num_syms; i++){
        expected_av += int_vals[i];
      } expected_av = expected_av / num_syms;

      //insert organisms
      for(size_t i = 0; i < (num_syms / 2); i++){
        w.AddOrgAt(new Symbiont(&random, &w, &config, int_vals[i]), emp::WorldPosition(0,i));
      }
      Host *h = new Host(&random, &w, &config, int_val);
      w.AddOrgAt(h, 0);
      for(size_t i = 0; i < (num_syms / 2); i++){
        h->AddSymbiont(new Symbiont(&random, &w, &config, int_vals[i+3]));
      }
      w.Update();
      //check they were tracked correctly
      THEN("their average interaction values are tracked"){
        REQUIRE(sym_intval_node.GetMean() < (expected_av + 0.01));
        REQUIRE(sym_intval_node.GetMean() > (expected_av - 0.01));
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

    emp::DataMonitor<double,emp::data::Histogram>& free_sym_intval_node = w.GetFreeSymIntValDataNode();
    REQUIRE(std::isnan(free_sym_intval_node.GetMean()));
    REQUIRE(w.GetNumOrgs() == 0);

    WHEN("free and hosted syms are added"){
      size_t num_syms = 6;
      double int_vals[num_syms] = {-0.5, -0.4, 0, 0.7, 0.75, 0.78};
      double expected_av = 0;
      for(size_t i = 0; i < (num_syms/2); i++){
        expected_av += int_vals[i];
      } expected_av = expected_av / (num_syms/2);

      //insert organisms
      for(size_t i = 0; i < (num_syms / 2); i++){
        w.AddOrgAt(new Symbiont(&random, &w, &config, int_vals[i]), emp::WorldPosition(0,i));
      }
      Host *h = new Host(&random, &w, &config, int_val);
      w.AddOrgAt(h, 0);
      for(size_t i = 0; i < (num_syms / 2); i++){
        h->AddSymbiont(new Symbiont(&random, &w, &config, int_vals[i+3]));
      }
      w.Update();
      //check they were tracked correctly
      THEN("only free sym average interaction values are tracked"){
        REQUIRE(free_sym_intval_node.GetMean() < (expected_av + 0.01));
        REQUIRE(free_sym_intval_node.GetMean() > (expected_av - 0.01));
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

    emp::DataMonitor<double,emp::data::Histogram>& hosted_sym_intval_node = w.GetHostedSymIntValDataNode();
    REQUIRE(std::isnan(hosted_sym_intval_node.GetMean()));
    REQUIRE(w.GetNumOrgs() == 0);

    WHEN("free and hosted syms are added"){
      //setup
      size_t num_syms = 6;
      double int_vals[num_syms] = {-0.5, -0.4, 0, 0.7, 0.75, 0.78};
      double expected_av = 0;
      for(size_t i = (num_syms/2); i < num_syms; i++){ //hosted syms are later
        expected_av += int_vals[i];
      } expected_av = expected_av / (num_syms/2);

      //insert organisms
      for(size_t i = 0; i < (num_syms / 2); i++){
        w.AddOrgAt(new Symbiont(&random, &w, &config, int_vals[i]), emp::WorldPosition(0,i));
      }
      Host *h = new Host(&random, &w, &config, int_val);
      w.AddOrgAt(h, 0);
      for(size_t i = 0; i < (num_syms / 2); i++){
        h->AddSymbiont(new Symbiont(&random, &w, &config, int_vals[i+3]));
      }
      w.Update();
      //check they were tracked correctly
      THEN("only free sym average interaction values are tracked"){
        REQUIRE(hosted_sym_intval_node.GetMean() < (expected_av + 0.01));
        REQUIRE(hosted_sym_intval_node.GetMean() > (expected_av - 0.01));
      }
    }
  }
}
