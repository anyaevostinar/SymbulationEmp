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
    sym_intval_node.SetupBins(-1.0, 1.1, max_bin);
    REQUIRE(std::isnan(sym_intval_node.GetMean()));
    for(size_t i = 0; i < max_bin; i++){
      REQUIRE(sym_intval_node.GetHistCounts()[i] == 0);
    }
    REQUIRE(w.GetNumOrgs() == 0);

    WHEN("free and hosted syms are added"){
      //setup
      size_t num_syms = 6;

      //note: can't use variable in array initialization because some
      //compilers don't allow variable-sized arrays
      double int_vals[6] = {-1.0, -0.43, 0, 0.71, 0.75, 1.0};

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
        REQUIRE(sym_intval_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(sym_intval_node.GetMean() > (expected_av - 0.0001));
      }
      THEN("they were split into histogram bins correctly"){
        //int vals: -1.0, -0.43, 0, 0.71, 0.75, 1.0
        //exp bins: 0, 5, 10, 17, 17, 19
        REQUIRE(sym_intval_node.GetHistCounts()[0] == 1);
        REQUIRE(sym_intval_node.GetHistCounts()[5] == 1);
        REQUIRE(sym_intval_node.GetHistCounts()[9] == 1);
        REQUIRE(sym_intval_node.GetHistCounts()[17] == 2);
        REQUIRE(sym_intval_node.GetHistCounts()[19] == 1);
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
    free_sym_intval_node.SetupBins(-1.0, 1.1, max_bin);

    //initial checks for emptiness
    REQUIRE(std::isnan(free_sym_intval_node.GetMean()));
    REQUIRE(w.GetNumOrgs() == 0);
    for(size_t i = 0; i < max_bin; i++){
      REQUIRE(free_sym_intval_node.GetHistCounts()[i] == 0);
    }

    WHEN("free and hosted syms are added"){
      size_t num_syms = 6;

      //note: can't use variable in array initialization because some
      //compilers don't allow variable-sized arrays
      double int_vals[6] = {-1.0, -0.43, 0, 0.71, 0.75, 1.0};
      double expected_av = 0;
      for(size_t i = 0; i < (num_syms/2); i++){
        expected_av += int_vals[i];
      } expected_av = expected_av / (num_syms/2);

      //insert organisms
      Host *host = new Host(&random, &w, &config, int_val);
      w.AddOrgAt(host, 0);
      for(size_t i = 0; i < (num_syms / 2); i++){
        w.AddOrgAt(new Symbiont(&random, &w, &config, int_vals[i]), emp::WorldPosition(0,i));
        host->AddSymbiont(new Symbiont(&random, &w, &config, int_vals[i+3]));
      }
      w.Update();
      //check they were tracked correctly
      THEN("only free symbionts average interaction values are tracked"){
        REQUIRE(free_sym_intval_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(free_sym_intval_node.GetMean() > (expected_av - 0.0001));
      }
      THEN("only free symbionts were split into histogram bins correctly"){
        REQUIRE(free_sym_intval_node.GetHistCounts()[0] == 1);
        REQUIRE(free_sym_intval_node.GetHistCounts()[5] == 1);
        REQUIRE(free_sym_intval_node.GetHistCounts()[9] == 1);
        REQUIRE(free_sym_intval_node.GetHistCounts()[17] == 0);
        REQUIRE(free_sym_intval_node.GetHistCounts()[19] == 0);
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
    hosted_sym_intval_node.SetupBins(-1.0, 1.1, max_bin);
    REQUIRE(std::isnan(hosted_sym_intval_node.GetMean()));
    REQUIRE(w.GetNumOrgs() == 0);
    for(size_t i = 0; i < max_bin; i++){
      REQUIRE(hosted_sym_intval_node.GetHistCounts()[i] == 0);
    }

    WHEN("free and hosted syms are added"){
      //setup
      size_t num_syms = 6;

      //note: can't use variable in array initialization because some
      //compilers don't allow variable-sized arrays
      double int_vals[6] = {-1.0, -0.43, 0, 0.71, 0.75, 1.0};
      double expected_av = 0;
      for(size_t i = (num_syms/2); i < num_syms; i++){ //hosted syms are later
        expected_av += int_vals[i];
      } expected_av = expected_av / (num_syms/2);

      //insert organisms
      Host *host = new Host(&random, &w, &config, int_val);
      w.AddOrgAt(host, 0);
      for(size_t i = 0; i < (num_syms / 2); i++){
        w.AddOrgAt(new Symbiont(&random, &w, &config, int_vals[i]), emp::WorldPosition(0,i));
        host->AddSymbiont(new Symbiont(&random, &w, &config, int_vals[i+3]));
      }

      w.Update();
      //check they were tracked correctly
      THEN("only hosted symbiont average interaction values are tracked"){
        REQUIRE(hosted_sym_intval_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(hosted_sym_intval_node.GetMean() > (expected_av - 0.0001));
      }
      THEN("only hosted symbionts were split into histogram bins correctly"){
        REQUIRE(hosted_sym_intval_node.GetHistCounts()[0] == 0);
        REQUIRE(hosted_sym_intval_node.GetHistCounts()[5] == 0);
        REQUIRE(hosted_sym_intval_node.GetHistCounts()[9] == 0);
        REQUIRE(hosted_sym_intval_node.GetHistCounts()[17] == 2);
        REQUIRE(hosted_sym_intval_node.GetHistCounts()[19] == 1);
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
    host_intval_node.SetupBins(-1.0, 1.1, max_bin);
    REQUIRE(std::isnan(host_intval_node.GetMean()));
    for(size_t i = 0; i < max_bin; i++){
      REQUIRE(host_intval_node.GetHistCounts()[i] == 0);
    }
    REQUIRE(w.GetNumOrgs() == 0);

    WHEN("hosts are added"){
      //setup
      size_t num_hosts = 4;

      //note: can't use variable in array initialization because some
      //compilers don't allow variable-sized arrays
      double int_vals[4] = {-1.0, -0.5, 0.38, 1.0};
      double expected_av = -0.03;
      for(size_t i = 0; i < num_hosts; i++){
        w.AddOrgAt(new Host(&random, &w, &config, int_vals[i]), i);
      }
      w.Update();
      //check they were tracked correctly
      THEN("their average interaction values are tracked"){
        REQUIRE(host_intval_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(host_intval_node.GetMean() > (expected_av - 0.0001));
      }
      THEN("they were split into histogram bins correctly"){
        REQUIRE(host_intval_node.GetHistCounts()[0] == 1);
        REQUIRE(host_intval_node.GetHistCounts()[4] == 1);
        REQUIRE(host_intval_node.GetHistCounts()[13] == 1);
        REQUIRE(host_intval_node.GetHistCounts()[19] == 1);
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
    sym_infectionchance_node.SetupBins(0, 1.1, max_bin);
    REQUIRE(std::isnan(sym_infectionchance_node.GetMean()));
    for(size_t i = 0; i < max_bin; i++){
      REQUIRE(sym_infectionchance_node.GetHistCounts()[i] == 0);
    }
    REQUIRE(w.GetNumOrgs() == 0);

    WHEN("free and hosted syms are added"){
      //setup
      size_t num_syms = 6;
      double infection_chances[6] = {0, 0.2, 0.34, 0.77, 0.33, 1.0};
      double expected_av = 0.44;

      //insert organisms
      Host *host = new Host(&random, &w, &config, int_val);
      w.AddOrgAt(host, 0);

      for(size_t i = 0; i < (num_syms / 2); i++){
        Symbiont *sym1 = new Symbiont(&random, &w, &config, int_val);
        Symbiont *sym2 = new Symbiont(&random, &w, &config, int_val);

        sym1->SetInfectionChance(infection_chances[i]);
        sym2->SetInfectionChance(infection_chances[i+3]);

        w.AddOrgAt(sym1, emp::WorldPosition(0,i));
        host->AddSymbiont(sym2);
      }

      w.Update();
      //check they were tracked correctly
      THEN("their average infection chance values are tracked"){
        REQUIRE(sym_infectionchance_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(sym_infectionchance_node.GetMean() > (expected_av - 0.0001));
      }
      THEN("they're split into histogram bins correctly"){
        REQUIRE(sym_infectionchance_node.GetHistCounts()[0] == 1);
        REQUIRE(sym_infectionchance_node.GetHistCounts()[2] == 1);
        REQUIRE(sym_infectionchance_node.GetHistCounts()[3] == 2);
        REQUIRE(sym_infectionchance_node.GetHistCounts()[7] == 1);
        REQUIRE(sym_infectionchance_node.GetHistCounts()[9] == 1);
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
    free_sym_infectionchance_node.SetupBins(0, 1.1, max_bin);
    REQUIRE(std::isnan(free_sym_infectionchance_node.GetMean()));
    REQUIRE(w.GetNumOrgs() == 0);

    WHEN("free and hosted syms are added"){
      size_t num_syms = 6;

      //note: can't use variable in array initialization because some
      //compilers don't allow variable-sized arrays
      double infection_chances[6] = {0, 0.2, 0.34, 0.77, 0.33, 1.0};
      double expected_av = 0.18;

      //insert organisms
      Host *host = new Host(&random, &w, &config, int_val);
      w.AddOrgAt(host, 0);

      for(size_t i = 0; i < (num_syms / 2); i++){
        Symbiont *sym1 = new Symbiont(&random, &w, &config, int_val);
        Symbiont *sym2 = new Symbiont(&random, &w, &config, int_val);

        sym1->SetInfectionChance(infection_chances[i]);
        sym2->SetInfectionChance(infection_chances[i+3]);

        w.AddOrgAt(sym1, emp::WorldPosition(0,i));
        host->AddSymbiont(sym2);
      }

      w.Update();

      //check they were tracked correctly
      THEN("only free symbiont average infection chances are tracked"){
        REQUIRE(free_sym_infectionchance_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(free_sym_infectionchance_node.GetMean() > (expected_av - 0.0001));
      }
      THEN("only free symbionts are split into histogram bins"){
        REQUIRE(free_sym_infectionchance_node.GetHistCounts()[0] == 1);
        REQUIRE(free_sym_infectionchance_node.GetHistCounts()[2] == 1);
        REQUIRE(free_sym_infectionchance_node.GetHistCounts()[3] == 1);
        REQUIRE(free_sym_infectionchance_node.GetHistCounts()[7] == 0);
        REQUIRE(free_sym_infectionchance_node.GetHistCounts()[9] == 0);
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
    hosted_sym_infectionchance_node.SetupBins(0, 1.1, max_bin);
    REQUIRE(std::isnan(hosted_sym_infectionchance_node.GetMean()));
    REQUIRE(w.GetNumOrgs() == 0);

    WHEN("free and hosted syms are added"){
      //setup
      size_t num_syms = 6;

      //note: can't use variable in array initialization because some
      //compilers don't allow variable-sized arrays
      double infection_chances[6] = {0, 0.2, 0.34, 0.77, 0.33, 1.0};
      double expected_av = 0.7;

      //insert organisms
      Host *host = new Host(&random, &w, &config, int_val);
      w.AddOrgAt(host, 0);

      for(size_t i = 0; i < (num_syms / 2); i++){
        Symbiont *sym1 = new Symbiont(&random, &w, &config, int_val);
        Symbiont *sym2 = new Symbiont(&random, &w, &config, int_val);

        sym1->SetInfectionChance(infection_chances[i]);
        sym2->SetInfectionChance(infection_chances[i+3]);

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
        REQUIRE(hosted_sym_infectionchance_node.GetHistCounts()[0] == 0);
        REQUIRE(hosted_sym_infectionchance_node.GetHistCounts()[2] == 0);
        REQUIRE(hosted_sym_infectionchance_node.GetHistCounts()[3] == 1);
        REQUIRE(hosted_sym_infectionchance_node.GetHistCounts()[7] == 1);
        REQUIRE(hosted_sym_infectionchance_node.GetHistCounts()[9] == 1);
      }
    }
  }
}
