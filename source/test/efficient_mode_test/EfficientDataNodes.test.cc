#include "../../efficient_mode/EfficientHost.h"
#include "../../efficient_mode/EfficientSymbiont.h"
#include "../../efficient_mode/EfficientWorld.h"

TEST_CASE("GetEfficiencyDataNode", "[efficient]"){
  GIVEN("a world"){
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    double points = 10;
    EfficientWorld world(random, &config);
    world.Resize(4);
    world.SetFreeLivingSyms(1);
    config.SYM_LIMIT(3);

    emp::DataMonitor<double>& sym_efficiency_node = world.GetEfficiencyDataNode();
    REQUIRE(std::isnan(sym_efficiency_node.GetMean()));

    WHEN("efficient symbionts are added to the world"){
      size_t num_syms = 6;
      double free_sym_efficiencies[3] = {0, 0.31, 0.45};
      double hosted_sym_efficiencies[3] = {0.71, 0.77, 1.0};

      double expected_av = 0.54;

      emp::Ptr<Host> host = emp::NewPtr<EfficientHost>(&random, &world, &config, int_val);
      world.AddOrgAt(host, 0);

      for(size_t i = 0; i < (num_syms/2); i++){
        world.AddOrgAt(emp::NewPtr<EfficientSymbiont>(&random, &world, &config, int_val, points, free_sym_efficiencies[i]), emp::WorldPosition(0, i));
        host->AddSymbiont(emp::NewPtr<EfficientSymbiont>(&random, &world, &config, int_val, points, hosted_sym_efficiencies[i]));
      }

      world.Update();
      THEN("their efficiency is tracked by a data node"){
        REQUIRE(sym_efficiency_node.GetMean() < (expected_av + 0.0001));
        REQUIRE(sym_efficiency_node.GetMean() > (expected_av - 0.0001));
      }
    }
  }
}
