#include <filesystem>
#include <fstream>
#include <string>

TEST_CASE("GetStressEscapeeOffspringAttemptCount", "[sgp]") {
  GIVEN("Stress is on, parasites are present, and an extinction event occurs") {
    emp::Random random(32);
    SymConfigSGP config;

    size_t stress_escapee_offspring_count = 3;
    config.EXTINCTION_FREQUENCY(1);
    config.PARASITE_NUM_OFFSPRING_ON_STRESS_INTERACTION(stress_escapee_offspring_count);
    config.TRACK_PARENT_TASKS(1);
    config.INTERACTION_MECHANISM(STRESS);
    config.SYMBIONT_TYPE(1);
    config.PARASITE_DEATH_CHANCE(1);

    SGPWorld world(random, &config, LogicTasks);

    emp::Ptr<StressHost> host = emp::NewPtr<StressHost>(&random, &world, &config);
    emp::Ptr<SGPSymbiont> matching_symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config);

    host->AddSymbiont(matching_symbiont);

    host->GetCPU().state.parent_tasks_performed->Set(1);
    matching_symbiont->GetCPU().state.parent_tasks_performed->Set(1); 

    world.AddOrgAt(host, 0);

    WHEN("Parasite offspring escape") {
      world.Update();
      REQUIRE(world.IsOccupied(0) == false);

      emp::DataMonitor<size_t>& escapee_attempts = world.GetStressEscapeeOffspringAttemptCount();
      THEN("The number of attempted stress escapees is recorded"){
        REQUIRE(escapee_attempts.GetTotal() == stress_escapee_offspring_count);
      }
    }
  }
}

TEST_CASE("GetStressEscapeeOffspringSuccessCount", "[sgp]") {
  GIVEN("Stress is on, parasites are present, and an extinction event occurs") {
    emp::Random random(32);
    SymConfigSGP config;

    size_t stress_escapee_offspring_count = 3;
    config.EXTINCTION_FREQUENCY(1);
    config.PARASITE_NUM_OFFSPRING_ON_STRESS_INTERACTION(stress_escapee_offspring_count);
    config.TRACK_PARENT_TASKS(1);
    config.INTERACTION_MECHANISM(STRESS);
    config.SYMBIONT_TYPE(1);
    config.BASE_DEATH_CHANCE(0);
    config.PARASITE_DEATH_CHANCE(1);
    config.GRID(1);

    SGPWorld world(random, &config, LogicTasks);

    emp::Ptr<StressHost> host = emp::NewPtr<StressHost>(&random, &world, &config);
    emp::Ptr<SGPSymbiont> matching_symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config);

    emp::Ptr<StressHost> vulnerable_host = emp::NewPtr<StressHost>(&random, &world, &config);

    host->AddSymbiont(matching_symbiont);

    host->GetCPU().state.parent_tasks_performed->Set(1);
    matching_symbiont->GetCPU().state.parent_tasks_performed->Set(1);
    matching_symbiont->GetCPU().state.tasks_performed->Set(0);
    vulnerable_host->GetCPU().state.parent_tasks_performed->Set(1);

    world.AddOrgAt(host, 0);
    world.AddOrgAt(vulnerable_host, 1);

    WHEN("Parasite offspring escape") {
      world.Update();
      REQUIRE(world.IsOccupied(0) == false);
      REQUIRE(world.IsOccupied(1) == true);
      REQUIRE(vulnerable_host->HasSym());
      REQUIRE(vulnerable_host->GetSymbionts().at(0).DynamicCast<SGPSymbiont>()->GetCPU().state.parent_tasks_performed->Get(0) == 1);
      emp::DataMonitor<size_t>& escapee_successes = world.GetStressEscapeeOffspringSuccessCount();
      THEN("The number of successful stress escapees is recorded") {
        REQUIRE(escapee_successes.GetTotal() == 1);
      }
    }
  }
}