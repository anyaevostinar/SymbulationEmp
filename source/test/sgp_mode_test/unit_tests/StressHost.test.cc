#include "../../../sgp_mode/StressHost.h"

TEST_CASE("Symbiont interaction-value-based extinction probabilities", "[sgp][sgp-unit]") {
  emp::Random random(62);
  SymConfigSGP config;
  config.INTERACTION_MECHANISM(STRESS);
  config.EXTINCTION_FREQUENCY(1);
  config.GRID_X(10);
  config.GRID_Y(10);

  double parasite_death_chance = 0.75;
  double mutualist_death_chance = 0.25;
  double base_death_chance = 0.5;
  config.PARASITE_DEATH_CHANCE(parasite_death_chance);
  config.MUTUALIST_DEATH_CHANCE(mutualist_death_chance);
  config.BASE_DEATH_CHANCE(base_death_chance);
  config.ALLOW_TRANSITION_EVOLUTION(1);

  SGPWorld world(random, &config, LogicTasks);

  emp::Ptr<StressHost> host = emp::NewPtr<StressHost>(&random, &world, &config, CreateNotProgram(PROGRAM_LENGTH));
  world.AddOrgAt(host, 0);
  REQUIRE(world.GetNumOrgs() == 1);

  WHEN("A host has a symbiont") {
    emp::Ptr<SGPSymbiont> symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(PROGRAM_LENGTH));
    host->AddSymbiont(symbiont);

    WHEN("The symbiont has non-matching tasks") {
      host->GetCPU().state.tasks_performed->Set(0);
      symbiont->GetCPU().state.tasks_performed->Set(8);

      THEN("The host dies with base death chance probability") {
        REQUIRE(host->GetDeathChance() == base_death_chance);
      }
    }

    WHEN("The symbiont has matching tasks") {
      host->GetCPU().state.tasks_performed->Set(8);
      symbiont->GetCPU().state.tasks_performed->Set(8);

      THEN("Host base death chance is calculated according to symbiont interaction value") {
        symbiont->SetIntVal(-1);
        REQUIRE(host->GetDeathChance() > base_death_chance);
        REQUIRE(host->GetDeathChance() == 0.75);

        symbiont->SetIntVal(-0.9);
        REQUIRE(host->GetDeathChance() > base_death_chance);
        REQUIRE(host->GetDeathChance() == 0.725);

        symbiont->SetIntVal(0);
        REQUIRE(host->GetDeathChance() == base_death_chance);
        REQUIRE(host->GetDeathChance() == 0.5);

        symbiont->SetIntVal(0.01);
        REQUIRE(host->GetDeathChance() < base_death_chance);
        REQUIRE(host->GetDeathChance() == 0.4975);

        symbiont->SetIntVal(1);
        REQUIRE(host->GetDeathChance() < base_death_chance);
        REQUIRE(host->GetDeathChance() == 0.25);
      }
    }
  }

  WHEN("A host does not have a symbiont") {
    THEN("The host dies with base death chance probability") {
      REQUIRE(host->GetDeathChance() == base_death_chance);
    }
  }
}