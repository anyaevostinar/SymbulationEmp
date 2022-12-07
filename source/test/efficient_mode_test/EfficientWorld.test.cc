#include "../../efficient_mode/EfficientWorld.h"

TEST_CASE("Efficient GetNewSym", "[efficient]") {
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigBase config;
    EfficientWorld world(random, &config);

    WHEN("GetNewSym is called") {
      THEN("It returns an object of type EfficientSymbiont") {
        REQUIRE(symbiont->GetEfficiency() == 1);
        REQUIRE(world.GetNewSym()->GetName() == "EfficientSymbiont");
      }
    }
  }
}

TEST_CASE("Efficient GetNewHost", "[efficient]") {
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigBase config;
    EfficientWorld world(random, &config);

    WHEN("GetNewHost is called") {
      THEN("It returns an object of type EfficientHost") {
        REQUIRE(world.GetNewHost()->GetName() == "EfficientHost");
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