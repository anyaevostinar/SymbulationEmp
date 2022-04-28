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
    size_t num_bins = 11;

    emp::DataMonitor<double,emp::data::Histogram>& lysis_chance_data_node = w.GetLysisChanceDataNode();
    REQUIRE(std::isnan(lysis_chance_data_node.GetMean()));
    for(size_t i = 0; i < num_bins; i++){
      REQUIRE(lysis_chance_data_node.GetHistCounts()[i] == 0);
    }

    WHEN("syms are added"){
      double free_phage_lysis_chances[3] = {0, 0.43, 0.47};
      double hosted_phage_lysis_chances[3] = {0.45, 0.89, 1.0};

      double expected_av = 0.54;
      emp::vector<long unsigned int> expected_hist_counts(11);
      std::fill(expected_hist_counts.begin(), expected_hist_counts.end(), 0);
      expected_hist_counts[0] = 1;
      expected_hist_counts[4] = 3;
      expected_hist_counts[8] = 1;
      expected_hist_counts[9] = 1;

      Bacterium *bacterium = new Bacterium(&random, &w, &config, int_val);
      w.AddOrgAt(bacterium, 0);
      for(size_t i = 0; i < 3; i++){
        Phage *free_phage = new Phage(&random, &w, &config, int_val);
        free_phage->SetLysisChance(free_phage_lysis_chances[i]);
        w.AddOrgAt(free_phage, emp::WorldPosition(0,i));

        Phage *hosted_phage = new Phage(&random, &w, &config, int_val);
        hosted_phage->SetLysisChance(hosted_phage_lysis_chances[i]);
        bacterium->AddSymbiont(hosted_phage);
      }

      w.Update();
      //check they were tracked correctly
      THEN("their average lysis chances are tracked"){
        REQUIRE(lysis_chance_data_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(lysis_chance_data_node.GetMean() > (expected_av - 0.0001));
      }
      THEN("they were sorted into the correct histogram bins"){
        for(size_t i = 0; i < num_bins; i++){
          REQUIRE(lysis_chance_data_node.GetHistCounts()[i] == expected_hist_counts[i]);
        }
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
    size_t num_bins = 11;

    emp::DataMonitor<double,emp::data::Histogram>& induction_chance_data_node = w.GetInductionChanceDataNode();
    REQUIRE(std::isnan(induction_chance_data_node.GetMean()));
    for(size_t i = 0; i < num_bins; i++){
      REQUIRE(induction_chance_data_node.GetHistCounts()[i] == 0);
    }

    WHEN("syms are added"){
      double free_phage_induction_chances[3] = {0, 0.43, 0.47};
      double hosted_phage_induction_chances[3] = {0.45, 0.89, 1.0};

      double expected_av = 0.54;
      emp::vector<long unsigned int> expected_hist_counts(11);
      std::fill(expected_hist_counts.begin(), expected_hist_counts.end(), 0);
      expected_hist_counts[0] = 1;
      expected_hist_counts[4] = 3;
      expected_hist_counts[8] = 1;
      expected_hist_counts[9] = 1;

      Bacterium *bacterium = new Bacterium(&random, &w, &config, int_val);
      w.AddOrgAt(bacterium, 0);
      for(size_t i = 0; i < 3; i++){
        Phage *free_phage = new Phage(&random, &w, &config, int_val);
        free_phage->SetInductionChance(free_phage_induction_chances[i]);
        w.AddOrgAt(free_phage, emp::WorldPosition(0,i));

        Phage *hosted_phage = new Phage(&random, &w, &config, int_val);
        hosted_phage->SetInductionChance(hosted_phage_induction_chances[i]);
        bacterium->AddSymbiont(hosted_phage);
      }

      w.Update();
      //check they were tracked correctly
      THEN("their average induction chances are tracked"){
        REQUIRE(induction_chance_data_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(induction_chance_data_node.GetMean() > (expected_av - 0.0001));
      }
      THEN("they were sorted into the correct histogram bins"){
        for(size_t i = 0; i < num_bins; i++){
          REQUIRE(induction_chance_data_node.GetHistCounts()[i] == expected_hist_counts[i]);
        }
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
        for(size_t j = 0; j < burst_sizes[i]; j++) {
          Organism *new_repro_phage = p->reproduce();
          h->AddReproSym(new_repro_phage);
        }
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
        Bacterium *bacterium = new Bacterium(&random, &w, &config, int_val);
        Phage *phage = new Phage(&random, &w, &config, int_val);
        if(i < 2){ //ensure two of the bacteria will lyse
          phage->SetBurstTimer(burst_time);
        }
        bacterium->AddSymbiont(phage);

        Organism *new_repro_phage = phage->reproduce();
        bacterium->AddReproSym(new_repro_phage);

        w.AddOrgAt(bacterium, i);
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
    size_t num_bins = 11;
    LysisWorld w(random);
    w.Resize(4);

    emp::DataMonitor<double,emp::data::Histogram>& inc_dif_data_node = w.GetIncorporationDifferenceDataNode();
    REQUIRE(std::isnan(inc_dif_data_node.GetMean()));
    for(size_t i = 0; i < num_bins; i++){
      REQUIRE(inc_dif_data_node.GetHistCounts()[i] == 0);
    }

    double expected_av = 0.11;
    emp::vector<long unsigned int> expected_hist_counts(11);
    std::fill(expected_hist_counts.begin(), expected_hist_counts.end(), 0);
    expected_hist_counts[0] = 3;
    expected_hist_counts[1] = 2;
    expected_hist_counts[3] = 1;

    WHEN("hosted symbionts exist"){
      // no inc val difference
      Bacterium *bacterium = new Bacterium(&random, &w, &config, int_val);
      bacterium->SetIncVal(0);
      w.AddOrgAt(bacterium, 0);
      for(size_t j = 0; j < 3; j++){
        Phage *phage = new Phage(&random, &w, &config, int_val);
        phage->SetIncVal(0);
        bacterium->AddSymbiont(phage);
      }

      // inc val differences of 0.36, 0.15, 0.15
      double sym_inc_vals[3] = {0.14, 0.35, 0.65};
      bacterium = new Bacterium(&random, &w, &config, int_val);
      bacterium->SetIncVal(0.5);
      w.AddOrgAt(bacterium, 1);
      for(size_t j = 0; j < 3; j++){
        Phage *phage = new Phage(&random, &w, &config, int_val);
        phage->SetIncVal(sym_inc_vals[j]);
        bacterium->AddSymbiont(phage);
      }

      w.Update();

      THEN("the diffence in incorporation value between hosts and their symbionts is tracked"){
        REQUIRE(inc_dif_data_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(inc_dif_data_node.GetMean() > (expected_av - 0.0001));
      }
      THEN("incorporation difference values are sorted into histogram bins"){
        for(size_t i = 0; i < num_bins; i++){
          REQUIRE(inc_dif_data_node.GetHistCounts()[i] == expected_hist_counts[i]);
        }
      }
    }
  }
}
