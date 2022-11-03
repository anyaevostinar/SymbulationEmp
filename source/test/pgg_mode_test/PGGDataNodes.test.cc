#include "../../pgg_mode/PGGHost.h"
#include "../../pgg_mode/PGGSymbiont.h"
#include "../../pgg_mode/PGGWorld.h"

TEST_CASE("GetPGGDataNode", "[pgg]"){
  GIVEN("a world"){
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    PGGWorld world(random, &config);
    world.Resize(4);
    
    config.FREE_LIVING_SYMS(1);
    config.SYM_INFECTION_CHANCE(0);
    config.SYM_LIMIT(3);
    size_t num_bins = 11;

    emp::DataMonitor<double, emp::data::Histogram>& sym_donation_node = world.GetPGGDataNode();
    REQUIRE(std::isnan(sym_donation_node.GetMean()));
    for(size_t i = 0; i < num_bins; i++){
      REQUIRE(sym_donation_node.GetHistCounts()[i] == 0);
    }

    WHEN("efficient symbionts are added to the world"){
      double free_sym_donation_vals[3] = {0, 0.31, 0.45};
      double hosted_sym_donation_vals[3] = {0.71, 0.77, 1.0};

      double expected_av = 0.54;
      emp::vector<long unsigned int> expected_hist_counts(11);
      std::fill(expected_hist_counts.begin(), expected_hist_counts.end(), 0);
      expected_hist_counts[0] = 1;
      expected_hist_counts[3] = 1;
      expected_hist_counts[4] = 1;
      expected_hist_counts[7] = 2;
      expected_hist_counts[9] = 1;

      emp::Ptr<Host> host = emp::NewPtr<PGGHost>(&random, &world, &config, int_val);
      world.AddOrgAt(host, 0);

      for(size_t i = 0; i < 3; i++){
        world.AddOrgAt(emp::NewPtr<PGGSymbiont>(&random, &world, &config, int_val, free_sym_donation_vals[i]), emp::WorldPosition(0, i));
        host->AddSymbiont(emp::NewPtr<PGGSymbiont>(&random, &world, &config, int_val, hosted_sym_donation_vals[i]));
      }

      world.Update();

      THEN("their average efficiency is tracked by a data node"){
        REQUIRE(sym_donation_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(sym_donation_node.GetMean() > (expected_av - 0.0001));
      }
      THEN("they are sorted into histrogram bins"){
        for(size_t i = 0; i < num_bins; i++){
          REQUIRE(sym_donation_node.GetHistCounts()[i] == expected_hist_counts[i]);
        }
      }
    }
  }
}
