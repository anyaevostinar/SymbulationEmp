#include "../../lysis_mode/Phage.h"
#include "../../lysis_mode/Bacterium.h"
#include "../../lysis_mode/LysisWorld.h"


TEST_CASE("GetCFUDataNode", "[lysis]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    LysisWorld world(random, &config);
    config.SYM_LIMIT(4);
    world.Resize(10);

    //keep track of host organisms that are uninfected or infected with only lysogenic phage
    emp::DataMonitor<int>& cfu_data_node = world.GetCFUDataNode();
    REQUIRE(cfu_data_node.GetTotal() == 0);
    REQUIRE(world.GetNumOrgs() == 0);

    WHEN("uninfected hosts are added"){
      size_t num_hosts = 10;

      for(size_t i = 0; i < num_hosts; i++){
        world.AddOrgAt(emp::NewPtr<Bacterium>(&random, &world, &config, int_val), i);
      }

      world.Update();
      THEN("they are tracked by the data node"){
        REQUIRE(world.GetNumOrgs() == num_hosts);
        REQUIRE(cfu_data_node.GetTotal() == num_hosts);
      }

      WHEN("some hosts are infected with lytic phage"){
        size_t num_infections = 2;
        for(size_t i = 0; i < num_infections; i++){
          emp::Ptr<Phage> phage = emp::NewPtr<Phage>(&random, &world, &config, int_val);
          phage->SetLysisChance(1.0);
          world.GetOrg(i).AddSymbiont(phage);
        }
        world.Update();
        THEN("infected hosts are excluded from the cfu count"){
          REQUIRE(world.GetNumOrgs() == num_hosts);
          REQUIRE(cfu_data_node.GetTotal() == (num_hosts - num_infections));
        }
      }

      WHEN("hosts are infected with lysogenic phage"){
        size_t num_infections = 2;
        for(size_t i = 0; i < num_infections; i++){
          emp::Ptr<Phage> phage = emp::NewPtr<Phage>(&random, &world, &config, int_val);
          phage->SetLysisChance(0.0);
          world.GetOrg(i).AddSymbiont(phage);
        }
        world.Update();
        THEN("infected hosts are excluded from the cfu count"){
          REQUIRE(world.GetNumOrgs() == num_hosts);
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
    LysisWorld world(random, &config);
    world.Resize(4);
    world.SetFreeLivingSyms(1);
    config.SYM_LIMIT(3);
    size_t num_bins = 11;

    emp::DataMonitor<double,emp::data::Histogram>& lysis_chance_data_node = world.GetLysisChanceDataNode();
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

      emp::Ptr<Bacterium> bacterium = emp::NewPtr<Bacterium>(&random, &world, &config, int_val);
      world.AddOrgAt(bacterium, 0);
      for(size_t i = 0; i < 3; i++){
        emp::Ptr<Phage> free_phage = emp::NewPtr<Phage>(&random, &world, &config, int_val);
        free_phage->SetLysisChance(free_phage_lysis_chances[i]);
        world.AddOrgAt(free_phage, emp::WorldPosition(0,i));

        emp::Ptr<Phage> hosted_phage = emp::NewPtr<Phage>(&random, &world, &config, int_val);
        hosted_phage->SetLysisChance(hosted_phage_lysis_chances[i]);
        bacterium->AddSymbiont(hosted_phage);
      }

      world.Update();
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
    LysisWorld world(random, &config);
    world.Resize(4);
    world.SetFreeLivingSyms(1);
    config.SYM_LIMIT(3);
    size_t num_bins = 11;

    emp::DataMonitor<double,emp::data::Histogram>& induction_chance_data_node = world.GetInductionChanceDataNode();
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

      emp::Ptr<Bacterium> bacterium = emp::NewPtr<Bacterium>(&random, &world, &config, int_val);
      world.AddOrgAt(bacterium, 0);
      for(size_t i = 0; i < 3; i++){
        emp::Ptr<Phage> free_phage = emp::NewPtr<Phage>(&random, &world, &config, int_val);
        free_phage->SetInductionChance(free_phage_induction_chances[i]);
        world.AddOrgAt(free_phage, emp::WorldPosition(0,i));

        emp::Ptr<Phage> hosted_phage = emp::NewPtr<Phage>(&random, &world, &config, int_val);
        hosted_phage->SetInductionChance(hosted_phage_induction_chances[i]);
        bacterium->AddSymbiont(hosted_phage);
      }

      world.Update();
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
    LysisWorld world(random, &config);
    world.Resize(4);
    world.SetFreeLivingSyms(1);
    config.LYSIS(1);
    config.LYSIS_CHANCE(1);
    config.BURST_TIME(burst_time);

    emp::DataMonitor<double>& burst_size_data_node = world.GetBurstSizeDataNode();
    REQUIRE(std::isnan(burst_size_data_node.GetMean()));

    WHEN("bacteria lyse"){
      size_t burst_sizes[4] = {4,5,6,7};
      double expected_av = 5.5;
      for(int i = 0; i < 4; i++){ // populate world with 4 bacteria
        //each of which has a different burst size
        emp::Ptr<Bacterium> bacterium = emp::NewPtr<Bacterium>(&random, &world, &config, int_val);
        emp::Ptr<Phage> phage = emp::NewPtr<Phage>(&random, &world, &config, int_val);
        for(size_t j = 0; j < burst_sizes[i]; j++) {
          emp::Ptr<Organism> new_repro_phage = phage->Reproduce();
          bacterium->AddReproSym(new_repro_phage);
        }
        phage->SetBurstTimer(burst_time);
        bacterium->AddSymbiont(phage);
        world.AddOrgAt(bacterium, i);
      }
      world.Update();

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
    LysisWorld world(random, &config);
    world.Resize(4);
    world.SetFreeLivingSyms(1);
    config.LYSIS(1);
    config.LYSIS_CHANCE(1);
    config.BURST_TIME(burst_time);

    emp::DataMonitor<int>& burst_count_data_node = world.GetBurstCountDataNode();
    REQUIRE(burst_count_data_node.GetTotal() == 0);

    WHEN("bacteria lyse"){
      int expected_total = 2;
      for(int i = 0; i < 4; i++){ // populate world with 4 bacteria
        emp::Ptr<Bacterium> bacterium = emp::NewPtr<Bacterium>(&random, &world, &config, int_val);
        emp::Ptr<Phage> phage = emp::NewPtr<Phage>(&random, &world, &config, int_val);
        if(i < 2){ //ensure two of the bacteria will lyse
          phage->SetBurstTimer(burst_time);
        }
        bacterium->AddSymbiont(phage);

        emp::Ptr<Organism> new_repro_phage = phage->Reproduce();
        bacterium->AddReproSym(new_repro_phage);

        world.AddOrgAt(bacterium, i);
      }
      world.Update();

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
    LysisWorld world(random, &config);
    world.Resize(4);

    emp::DataMonitor<double,emp::data::Histogram>& inc_dif_data_node = world.GetIncorporationDifferenceDataNode();
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
      emp::Ptr<Bacterium> bacterium = emp::NewPtr<Bacterium>(&random, &world, &config, int_val);
      bacterium->SetIncVal(0);
      world.AddOrgAt(bacterium, 0);
      for(size_t j = 0; j < 3; j++){
        emp::Ptr<Phage> phage = emp::NewPtr<Phage>(&random, &world, &config, int_val);
        phage->SetIncVal(0);
        bacterium->AddSymbiont(phage);
      }

      // inc val differences of 0.36, 0.15, 0.15
      double sym_inc_vals[3] = {0.14, 0.35, 0.65};
      bacterium = emp::NewPtr<Bacterium>(&random, &world, &config, int_val);
      bacterium->SetIncVal(0.5);
      world.AddOrgAt(bacterium, 1);
      for(size_t j = 0; j < 3; j++){
        emp::Ptr<Phage> phage = emp::NewPtr<Phage>(&random, &world, &config, int_val);
        phage->SetIncVal(sym_inc_vals[j]);
        bacterium->AddSymbiont(phage);
      }

      world.Update();

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
