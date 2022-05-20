#include "../../lysis_mode/Phage.h"
#include "../../lysis_mode/LysisWorld.h"

TEST_CASE("Lysis mode Update()", "lysis"){
  emp::Random random(17);
  SymConfigBase config;
  int int_val = 0;
  int world_size = 4;
  int res_per_update = 10;
  int num_updates = 5;
  int burst_time = 2;

  LysisWorld world(random, &config);
  world.Resize(world_size);
  world.SetFreeLivingSyms(1);
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
