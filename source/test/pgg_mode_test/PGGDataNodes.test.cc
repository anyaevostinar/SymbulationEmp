#include "../../pgg_mode/PGGHost.h"
#include "../../pgg_mode/PGGSymbiont.h"
#include "../../pgg_mode/PGGWorld.h"

TEST_CASE("GetPGGDataNode", "[pgg]"){
  GIVEN("a world"){
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    PGGWorld w(random);
    w.Resize(4);
    w.SetFreeLivingSyms(1);
    config.SYM_LIMIT(3);

    emp::DataMonitor<double, emp::data::Histogram>& sym_donation_node = w.GetPGGDataNode();
    REQUIRE(std::isnan(sym_donation_node.GetMean()));

    WHEN("efficient symbionts are added to the world"){
      size_t num_syms = 6;
      double donation_vals[6] = {0, 0.3, 0.45, 0.6, 0.77, 1.0};
      double expected_av = 0.52;

      Host *h = new PGGHost(&random, &w, &config, int_val);
      w.AddOrgAt(h, 0);

      for(size_t i = 0; i < (num_syms/2); i++){
        w.AddOrgAt(new PGGSymbiont(&random, &w, &config, int_val, donation_vals[i]), emp::WorldPosition(0, i));
        h->AddSymbiont(new PGGSymbiont(&random, &w, &config, int_val, donation_vals[i+3]));
      }

      w.Update();
      THEN("their efficiency is tracked by a data node"){
        REQUIRE(sym_donation_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(sym_donation_node.GetMean() > (expected_av - 0.0001));
      }
    }
  }
}
