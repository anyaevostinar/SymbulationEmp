#include "../../lysis_mode/Phage.h"
#include "../../lysis_mode/LysisWorld.h"

TEST_CASE("Lysis mode Update()", "[lysis]") {
  emp::Random random(17);
  SymConfigLysis config;
  int int_val = 0;
  int world_size = 4;
  int res_per_update = 10;
  int num_updates = 5;
  int burst_time = 2;

  LysisWorld world(random, &config);
  world.Resize(world_size);
  
  config.LYSIS(1);
  config.LYSIS_CHANCE(1);
  config.RES_DISTRIBUTE(res_per_update);
  config.BURST_TIME(burst_time);
  config.FREE_LIVING_SYMS(1);

  emp::Ptr<Organism> phage = emp::NewPtr<Phage>(&random, &world, &config, int_val);

  WHEN("there are no hosts"){
    THEN("phage don't reproduce or get points on update"){
      world.AddOrgAt(phage, emp::WorldPosition(0, 0));

      int orig_num_orgs = world.GetNumOrgs();
      int orig_points = phage->GetPoints();

      for(int i = 0; i < num_updates; i ++){
        world.Update();
      }

      int new_num_orgs = world.GetNumOrgs();
      int new_points = phage->GetPoints();

      REQUIRE(new_num_orgs == orig_num_orgs);
      REQUIRE(new_points == orig_points);
    }
  }

  WHEN("there are hosts"){
    emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
    THEN("phage and hosts mingle in the world"){
      world.AddOrgAt(host, 0);
      world.AddOrgAt(phage, emp::WorldPosition(0,1));

      for(int i = 0; i < num_updates; i++){
        world.Update();
      }

      REQUIRE(world.GetNumOrgs() == 2);
    }
  }
}

TEST_CASE("Lysis SetupSymbionts", "[lysis]") {
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigLysis config;
    LysisWorld world(random, &config);

    size_t world_size = 6;
    world.Resize(world_size);
    config.FREE_LIVING_SYMS(1);

    WHEN("SetupSymbionts is called") {
      size_t num_to_add = 2;
      world.SetupSymbionts(&num_to_add);

      THEN("The specified number of phage are added to the world") {
        size_t num_added = world.GetNumOrgs();
        REQUIRE(num_added == num_to_add);

        emp::Ptr<Organism> symbiont;
        int prev_burst_timer = -6;
        for (size_t i = 0; i < world_size; i++) {
          symbiont = world.GetSymAt(i);
          if (symbiont) {
            int sym_burst_timer = symbiont->GetBurstTimer();
            REQUIRE(sym_burst_timer >= -5);
            REQUIRE(sym_burst_timer <= 5);
            REQUIRE(sym_burst_timer != prev_burst_timer);
            prev_burst_timer = sym_burst_timer;
            REQUIRE(symbiont->GetName() == "Phage");
          }
        }
      }
    }
  }
}

TEST_CASE("Lysis SetupHosts", "[lysis]") {
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigLysis config;
    LysisWorld world(random, &config);

    WHEN("SetupHosts is called") {
      size_t num_to_add = 5;
      world.SetupHosts(&num_to_add);

      THEN("The specified number of bacteria are added to the world") {
        size_t num_added = world.GetNumOrgs();
        REQUIRE(num_added == num_to_add);

        emp::Ptr<Organism> host = world.GetPop()[0];
        REQUIRE(host != nullptr);
        REQUIRE(host->GetName() == "Bacterium");
      }
    }
  }
}
