#include "../../lysis_mode/Phage.h"
#include "../../lysis_mode/Bacterium.h"
#include "../../lysis_mode/LysisWorld.h"


TEST_CASE("GetCFUDataNode", "[lysis]"){
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

TEST_CASE("GetLysisChanceDataNode", "[lysis]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    LysisWorld w(random);
    w.Resize(4);
    w.SetFreeLivingSyms(1);
    config.SYM_LIMIT(3);

    size_t max_bin = 10;

    emp::DataMonitor<double,emp::data::Histogram>& lysis_chance_data_node = w.GetLysisChanceDataNode();
    lysis_chance_data_node.SetupBins(0, 1.0, max_bin);
    REQUIRE(std::isnan(lysis_chance_data_node.GetMean()));
    for(size_t i = 0; i < max_bin; i++){
      REQUIRE(lysis_chance_data_node.GetHistCounts()[i] == 0);
    }

    WHEN("syms are added"){
      //setup
      size_t num_syms = 6;
      double lysis_chances[6] = {0, 0.13, 0.26, 0.45, 0.89, 1.0};
      double expected_av = 0.455;

      //insert organisms
      for(size_t i = 0; i < (num_syms / 2); i++){
        Phage *p = new Phage(&random, &w, &config, int_val);
        p->SetLysisChance(lysis_chances[i]);
        w.AddOrgAt(p, emp::WorldPosition(0,i));
      }
      Bacterium *h = new Bacterium(&random, &w, &config, int_val);
      w.AddOrgAt(h, 0);
      for(size_t i = 0; i < (num_syms / 2); i++){
        Phage *p = new Phage(&random, &w, &config, int_val);
        p->SetLysisChance(lysis_chances[i+3]);
        h->AddSymbiont(p);
      }
      w.Update();
      //check they were tracked correctly
      THEN("their average lysis chances are tracked"){
        REQUIRE(lysis_chance_data_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(lysis_chance_data_node.GetMean() > (expected_av - 0.0001));
      }
    }
  }
}

TEST_CASE("GetInductionChanceDataNode", "[lysis]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    LysisWorld w(random);
    w.Resize(4);
    w.SetFreeLivingSyms(1);
    config.SYM_LIMIT(3);

    size_t max_bin = 10;

    emp::DataMonitor<double,emp::data::Histogram>& induction_chance_data_node = w.GetInductionChanceDataNode();
    induction_chance_data_node.SetupBins(0, 1.0, max_bin);
    REQUIRE(std::isnan(induction_chance_data_node.GetMean()));
    for(size_t i = 0; i < max_bin; i++){
      REQUIRE(induction_chance_data_node.GetHistCounts()[i] == 0);
    }

    WHEN("syms are added"){
      //setup
      size_t num_syms = 6;
      double induction_chances[6] = {0, 0.13, 0.26, 0.45, 0.89, 1.0};
      double expected_av = 0.455;

      //insert organisms
      for(size_t i = 0; i < (num_syms / 2); i++){
        Phage *p = new Phage(&random, &w, &config, int_val);
        p->SetInductionChance(induction_chances[i]);
        w.AddOrgAt(p, emp::WorldPosition(0,i));
      }
      Bacterium *h = new Bacterium(&random, &w, &config, int_val);
      w.AddOrgAt(h, 0);
      for(size_t i = 0; i < (num_syms / 2); i++){
        Phage *p = new Phage(&random, &w, &config, int_val);
        p->SetInductionChance(induction_chances[i+3]);
        h->AddSymbiont(p);
      }
      w.Update();
      //check they were tracked correctly
      THEN("their average induction chances are tracked"){
        REQUIRE(induction_chance_data_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(induction_chance_data_node.GetMean() > (expected_av - 0.0001));
      }
    }
  }
}

TEST_CASE("GetBurstSizeDataNode", "[lysis]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    size_t burst_time = 3;
    LysisWorld w(random);
    w.Resize(4);
    w.SetFreeLivingSyms(1);
    config.LYSIS(1);
    config.LYSIS_CHANCE(1);
    config.BURST_TIME(burst_time);

    emp::DataMonitor<double>& burst_size_data_node = w.GetBurstSizeDataNode();
    REQUIRE(std::isnan(burst_size_data_node.GetMean()));

    WHEN("bacteria lyse"){
      size_t burst_sizes[4] = {4,5,6,7};
      double expected_av = 5.5;
      for(int i = 0; i < 4; i++){ // populate world with 4 bacteria
        //each of which has a different burst size
        Bacterium *h = new Bacterium(&random, &w, &config, int_val);
        Phage *p = new Phage(&random, &w, &config, int_val);
        for(size_t j = 0; j < burst_sizes[i]; j++) h->AddReproSym(p->reproduce());
        p->SetBurstTimer(burst_time);
        h->AddSymbiont(p);
        w.AddOrgAt(h, i);
      }
      w.Update();

      THEN("the average burst size is tracked"){
        REQUIRE(burst_size_data_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(burst_size_data_node.GetMean() > (expected_av - 0.0001));
      }
    }
  }
}

TEST_CASE("GetBurstCountDataNode", "[lysis]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    size_t burst_time = 3;
    LysisWorld w(random);
    w.Resize(4);
    w.SetFreeLivingSyms(1);
    config.LYSIS(1);
    config.LYSIS_CHANCE(1);
    config.BURST_TIME(burst_time);

    emp::DataMonitor<int>& burst_count_data_node = w.GetBurstCountDataNode();
    REQUIRE(burst_count_data_node.GetTotal() == 0);

    WHEN("bacteria lyse"){
      int expected_total = 2;
      for(int i = 0; i < 4; i++){ // populate world with 4 bacteria
        //2 of which will lyse and the others won't
        Bacterium *h = new Bacterium(&random, &w, &config, int_val);
        Phage *p = new Phage(&random, &w, &config, int_val);
        for(size_t j = 0; j < 1; j++) h->AddReproSym(p->reproduce());
        h->AddSymbiont(p);
        w.AddOrgAt(h, i);
        if(i < 2) p->SetBurstTimer(burst_time);
      }
      w.Update();

      THEN("the burst count is tracked"){
        REQUIRE(burst_count_data_node.GetTotal() == expected_total);
      }
    }
  }
}

TEST_CASE("GetIncorporationDifferenceDataNode", "[lysis]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    config.SYM_LIMIT(4);
    int int_val = 0;
    size_t max_bin = 10;
    LysisWorld w(random);
    w.Resize(4);

    emp::DataMonitor<double,emp::data::Histogram>& inc_dif_data_node = w.GetIncorporationDifferenceDataNode();
    inc_dif_data_node.SetupBins(0, 1.0, max_bin);
    REQUIRE(std::isnan(inc_dif_data_node.GetMean()));
    for(size_t i = 0; i < max_bin; i++){
      REQUIRE(inc_dif_data_node.GetHistCounts()[i] == 0);
    }

    WHEN("hosted symbionts exist"){
      double host_inc_vals[2] = {0, 0.5}; //three syms per host
      double sym_inc_vals[6] = {0, 0, 0, 0.2, 0.3, 0.4};
      double expected_av = 0.1;
      for(int i = 0; i < 2; i++){
        Bacterium *h = new Bacterium(&random, &w, &config, int_val);
        h->SetIncVal(host_inc_vals[i]);
        for(size_t j = 0; j < 3; j++){
          Phage *p = new Phage(&random, &w, &config, int_val);
          p->SetIncVal(sym_inc_vals[(i*3)+j]);
          h->AddSymbiont(p);
        }
        w.AddOrgAt(h, i);
      }
      w.Update();

      THEN("the diffence in incorporation value between hosts and their symbionts is tracked"){
        REQUIRE(inc_dif_data_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(inc_dif_data_node.GetMean() > (expected_av - 0.0001));
      }
    }
  }
}
