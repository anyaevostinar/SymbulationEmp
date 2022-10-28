#include "../../efficient_mode/EfficientWorld.h"

TEST_CASE("Efficient SetupSymbionts", "[efficient]") {
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigBase config;
    EfficientWorld world(random, &config);

    size_t world_size = 6;
    world.Resize(world_size);
    config.FREE_LIVING_SYMS(1);

    WHEN("SetupSymbionts is called") {
      size_t num_to_add = 2;
      world.SetupSymbionts(&num_to_add);

      THEN("The specified number of efficient symbionts are added to the world") {
        size_t num_added = world.GetNumOrgs();
        REQUIRE(num_added == num_to_add);

        emp::Ptr<Organism> symbiont;
        for (size_t i = 0; i < world_size; i++) {
          symbiont = world.GetSymAt(i);
          if (symbiont) {
            REQUIRE(symbiont->GetEfficiency() == 1);
            REQUIRE(symbiont->GetName() == "EfficientSymbiont");
          }
        }
      }
    }
  }
}

TEST_CASE("Efficient SetupHosts", "[efficient]") {
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigBase config;
    EfficientWorld world(random, &config);

    WHEN("SetupHosts is called") {
      size_t num_to_add = 5;
      world.SetupHosts(&num_to_add);

      THEN("The specified number of efficient hosts are added to the world") {
        size_t num_added = world.GetNumOrgs();
        REQUIRE(num_added == num_to_add);

        emp::Ptr<Organism> host = world.GetPop()[0];
        REQUIRE(host != nullptr);
        REQUIRE(host->GetName() == "EfficientHost");
      }
    }
  }
}

TEST_CASE("Efficient Setup", "[efficient]") {
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigBase config;
    EfficientWorld world(random, &config);
    config.GRID_X(1);
    config.GRID_Y(1);

    double eff_mut_rate;
    double horiz_mut_rate = 1;
    config.HORIZ_MUTATION_RATE(horiz_mut_rate);

    WHEN("The config option for the efficiency mutation rate is -1") {
      eff_mut_rate = -1;
      config.EFFICIENCY_MUT_RATE(eff_mut_rate);
      world.Setup();
      THEN("The horizontal mutation rate is used") {
        REQUIRE(config.EFFICIENCY_MUT_RATE() == horiz_mut_rate);
      }
    }
    WHEN("The config option for the efficiency mutation rate is not -1") {
      eff_mut_rate = 0.2;
      config.EFFICIENCY_MUT_RATE(eff_mut_rate);
      world.Setup();
      THEN("The efficiency mutation rate is used") {
        REQUIRE(config.EFFICIENCY_MUT_RATE() == eff_mut_rate);
      }
    }
  }
  
}