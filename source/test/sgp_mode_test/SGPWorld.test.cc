#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPWorldSetup.cc"

#include "../../catch/catch.hpp"

TEST_CASE("GetDominantInfo", "[sgp]") {
  emp::Random random(61);
  SymConfigBase config;
  config.DOMINANT_COUNT(10);

  SGPWorld world(random, &config, TaskSet{});

  SGPHost host1(&random, &world, &config);
  SGPHost host2(&random, &world, &config);
  // Make sure they have different genomes
  host1.Mutate();
  host2.Mutate();

  WHEN("The world contains 2 of one org and 1 of another") {
    // One copy of host1 and two of host2
    world.AddOrgAt(emp::NewPtr<SGPHost>(host1), 0);
    world.AddOrgAt(emp::NewPtr<SGPHost>(host2), 1);
    world.AddOrgAt(emp::NewPtr<SGPHost>(host2), 2);

    CHECK(world.GetNumOrgs() == 3);

    THEN("The first org is dominant") {
      auto dominant = world.GetDominantInfo();
      CHECK(dominant.size() == 2);

      CHECK(*dominant[0].first == host2);
      CHECK(*dominant[0].first != host1);
      CHECK(dominant[0].second == 2);

      CHECK(*dominant[1].first == host1);
      CHECK(*dominant[1].first != host2);
      CHECK(dominant[1].second == 1);
    }
  }
}

TEST_CASE("Free living sgp symbiont dynamics", "[sgp]") {
  GIVEN("A world and free living symbionts") {
    emp::Random random(61);
    SymConfigBase config;
    SGPWorld world(random, &config, SquareTasks);
    size_t world_size = 1000;
    world.Resize(world_size);

    config.SYM_HORIZ_TRANS_RES(10);
    config.FREE_LIVING_SYMS(1);

    for (size_t i = 0; i < world_size; i++) {
      world.AddOrgAt(emp::NewPtr<SGPHost>(&random, &world, &config), i);
      world.AddOrgAt(emp::NewPtr<SGPSymbiont>(&random, &world, &config, 0, 0), emp::WorldPosition(0, i));
    }

    emp::DataMonitor<int>& free_sym_count_node = world.GetCountFreeSymsDataNode();
    emp::DataMonitor<int>& hosted_sym_count_node = world.GetCountHostedSymsDataNode();
    SyncDataMonitor<double>& data_node_free_sym_donated = world.GetFreeSymDonatedDataNode();
    SyncDataMonitor<double>& data_node_hosted_sym_donated = world.GetHostedSymDonatedDataNode();

    WHEN("Endosymbiosis is permitted") {
      config.SYM_LIMIT(1);
      THEN("Symbionts eventually infect hosts") {
        world.Update();
        REQUIRE(free_sym_count_node.GetTotal() > 10);
        REQUIRE(hosted_sym_count_node.GetTotal() == 0);

        world.RunExperiment(false);
        REQUIRE(free_sym_count_node.GetTotal() > 10);
        REQUIRE(hosted_sym_count_node.GetTotal() > 0);
      }
      WHEN("Ectosymbiosis is not permitted") {
        config.ECTOSYMBIOSIS(0);
        world.Update();
        REQUIRE(data_node_free_sym_donated.UnsynchronizedGetMonitor().GetCount() == 0);
        REQUIRE(data_node_hosted_sym_donated.UnsynchronizedGetMonitor().GetCount() == 0);
        world.RunExperiment(false);
        THEN("Hosts only interact with hosted symbionts") {
          REQUIRE(hosted_sym_count_node.GetTotal() > 0);
          REQUIRE(data_node_free_sym_donated.UnsynchronizedGetMonitor().GetCount() == 0);
          REQUIRE(data_node_hosted_sym_donated.UnsynchronizedGetMonitor().GetCount() > 0);
        }
      }
      WHEN("Ectosymbiosis is permitted") {
        config.ECTOSYMBIOSIS(1);
        world.Update();
        REQUIRE(data_node_free_sym_donated.UnsynchronizedGetMonitor().GetCount() == 0);
        REQUIRE(data_node_hosted_sym_donated.UnsynchronizedGetMonitor().GetCount() == 0);
        world.RunExperiment(false);
        THEN("Hosts interact with both hosted and free living symbionts") {
          REQUIRE(data_node_free_sym_donated.UnsynchronizedGetMonitor().GetCount() > 0);
          REQUIRE(data_node_hosted_sym_donated.UnsynchronizedGetMonitor().GetCount() > 0);
        }
      }
    }
    WHEN("Endosymbiosis is not permitted") {
      config.SYM_LIMIT(0);
      THEN("Symbionts never infect hosts") {
        world.Update();
        REQUIRE(free_sym_count_node.GetTotal() > 10);
        REQUIRE(hosted_sym_count_node.GetTotal() == 0);

        world.RunExperiment(false);
        REQUIRE(free_sym_count_node.GetTotal() > 10);
        REQUIRE(hosted_sym_count_node.GetTotal() == 0);
      }
      WHEN("Ectosymbiosis is not permitted") {
        config.ECTOSYMBIOSIS(0);
        world.Update();
        REQUIRE(data_node_free_sym_donated.UnsynchronizedGetMonitor().GetCount() == 0);
        REQUIRE(data_node_hosted_sym_donated.UnsynchronizedGetMonitor().GetCount() == 0);
        world.RunExperiment(false);
        THEN("Hosts and symbionts never interact") {
          REQUIRE(data_node_free_sym_donated.UnsynchronizedGetMonitor().GetCount() == 0);
          REQUIRE(data_node_hosted_sym_donated.UnsynchronizedGetMonitor().GetCount() == 0);
        }
      }
      WHEN("Ectosymbiosis is permitted") {
        config.ECTOSYMBIOSIS(1);
        world.Update();
        REQUIRE(data_node_free_sym_donated.UnsynchronizedGetMonitor().GetCount() == 0);
        REQUIRE(data_node_hosted_sym_donated.UnsynchronizedGetMonitor().GetCount() == 0);
        world.RunExperiment(false);
        THEN("Hosts only interact with free living symbionts") {
          REQUIRE(data_node_free_sym_donated.UnsynchronizedGetMonitor().GetCount() > 0);
          REQUIRE(data_node_hosted_sym_donated.UnsynchronizedGetMonitor().GetCount() == 0);
        }
      }
    }
  }
}
