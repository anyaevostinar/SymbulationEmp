#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPSymbiont.h"

#include "../../catch/catch.hpp"

TEST_CASE("GetHostedSymDonatedDataNode", "[sgp]") {
  GIVEN("A world") {
    emp::Random random(44);
    SymConfigBase config;
    SGPWorld world(random, &config, SquareTasks);
    int side = 100;
    config.GRID_X(side);
    config.GRID_Y(side);
    config.SYM_HORIZ_TRANS_RES(10); 
    config.UPDATES(501);

    SyncDataMonitor<double>& data_node_hosted_sym_donated = world.GetHostedSymDonatedDataNode();
    emp::DataMonitor<int>& hosted_sym_count_node = world.GetCountHostedSymsDataNode();

    WHEN("There are no hosted symbionts in the world") {
      // run the test with ectosymbiotic free living symbionts to confirm that the hosted 
      // symbiont data node is not tracking free living symbionts
      config.FREE_LIVING_SYMS(1);
      config.ECTOSYMBIOSIS(1);
      config.SYM_LIMIT(0);
      world.Setup();
      world.RunExperiment(false);
      REQUIRE(hosted_sym_count_node.GetTotal() == 0);
      THEN("Hosted symbionts do not donate any points") {
        REQUIRE(data_node_hosted_sym_donated.UnsynchronizedGetMonitor().GetTotal() == 0);
      }
    }
    WHEN("There are hosted symbionts in the world") {
      world.Setup();
      world.RunExperiment(false);
      REQUIRE(hosted_sym_count_node.GetTotal() > 0);
      THEN("Hosted symbionts donate points") {
        REQUIRE(data_node_hosted_sym_donated.UnsynchronizedGetMonitor().GetTotal() > 0);
      }
    }
  }
}

TEST_CASE("GetHostedSymStolenDataNode", "[sgp]") {
  GIVEN("A world") {
    emp::Random random(44);
    SymConfigBase config;
    SGPWorld world(random, &config, SquareTasks);
    int side = 100;
    config.GRID_X(side);
    config.GRID_Y(side);
    config.SYM_HORIZ_TRANS_RES(10);
    config.UPDATES(501);

    SyncDataMonitor<double>& data_node_hosted_sym_stolen = world.GetHostedSymStolenDataNode();
    emp::DataMonitor<int>& hosted_sym_count_node = world.GetCountHostedSymsDataNode();
    
    WHEN("There are no hosted symbionts in the world") {
      config.FREE_LIVING_SYMS(1);
      config.ECTOSYMBIOSIS(1);
      config.SYM_LIMIT(0);
      world.Setup();
      world.RunExperiment(false);
      REQUIRE(hosted_sym_count_node.GetTotal() == 0);
      THEN("Hosted symbionts do not steal any points") {
        REQUIRE(data_node_hosted_sym_stolen.UnsynchronizedGetMonitor().GetTotal() == 0);
      }
    }
    WHEN("There are hosted symbionts in the world") {
      world.Setup();
      world.RunExperiment(false);
      REQUIRE(hosted_sym_count_node.GetTotal() > 0);
      THEN("Hosted symbionts steal points") {
        REQUIRE(data_node_hosted_sym_stolen.UnsynchronizedGetMonitor().GetTotal() > 0);
      }
    }
  }
}

TEST_CASE("GetHostedSymEarnedDataNode", "[sgp]") {
  GIVEN("A world") {
    emp::Random random(44);
    SymConfigBase config;
    SGPWorld world(random, &config, SquareTasks);
    int side = 10;
    config.GRID_X(side);
    config.GRID_Y(side);
    config.SYM_HORIZ_TRANS_RES(10);
    config.UPDATES(5);

    SyncDataMonitor<double>& data_node_hosted_sym_earned = world.GetHostedSymEarnedDataNode();
    emp::DataMonitor<int>& hosted_sym_count_node = world.GetCountHostedSymsDataNode();

    WHEN("There are only free living symbionts in the world") {
      config.FREE_LIVING_SYMS(1);
      config.SYM_LIMIT(0);
      world.Setup();
      world.RunExperiment(false);
      REQUIRE(hosted_sym_count_node.GetTotal() == 0);
      THEN("Hosted symbionts do not earn any points") {
        REQUIRE(data_node_hosted_sym_earned.UnsynchronizedGetMonitor().GetTotal() == 0);
      }
    }
    WHEN("There are no symbionts in the world") {
      config.START_MOI(0);
      world.Setup();
      world.RunExperiment(false);
      REQUIRE(hosted_sym_count_node.GetTotal() == 0);
      THEN("Hosted symbionts do not earn any points") {
        REQUIRE(data_node_hosted_sym_earned.UnsynchronizedGetMonitor().GetTotal() == 0);
      }
    }
    WHEN("There are hosted symbionts in the world") {
      config.UPDATES(101);
      world.Setup();
      world.RunExperiment(false);
      REQUIRE(hosted_sym_count_node.GetTotal() > 0);
      THEN("Hosted symbionts earn points") {
        REQUIRE(data_node_hosted_sym_earned.UnsynchronizedGetMonitor().GetTotal() > 0);
      }
    }
  }
}

TEST_CASE("GetFreeSymDonatedDataNode", "[sgp]") {
  GIVEN("A world") {
    emp::Random random(44);
    SymConfigBase config;
    SGPWorld world(random, &config, SquareTasks);
    int side = 100;
    config.GRID_X(side);
    config.GRID_Y(side);
    config.SYM_HORIZ_TRANS_RES(10);
    config.UPDATES(501);

    SyncDataMonitor<double>& data_node_free_sym_donated = world.GetFreeSymDonatedDataNode();
    emp::DataMonitor<int>& free_sym_count_node = world.GetCountFreeSymsDataNode();

    WHEN("There are no free living symbionts in the world") {
      // run the test with hosted symbionts to confirm that the free 
      // symbiont data node is not tracking hosted symbionts
      config.FREE_LIVING_SYMS(0);
      world.Setup();
      world.RunExperiment(false);
      REQUIRE(free_sym_count_node.GetTotal() == 0);
      THEN("Free living symbionts do not donate any points") {
        REQUIRE(data_node_free_sym_donated.UnsynchronizedGetMonitor().GetTotal() == 0);
      }
    }
    WHEN("There are free symbionts in the world and ectosymbiosis is permitted") {
      config.FREE_LIVING_SYMS(1);
      config.ECTOSYMBIOSIS(1);
      config.SYM_INFECTION_CHANCE(0);
      world.Setup();
      world.RunExperiment(false);
      REQUIRE(free_sym_count_node.GetTotal() > 0);
      THEN("Free living symbionts donate points") {
        REQUIRE(data_node_free_sym_donated.UnsynchronizedGetMonitor().GetTotal() > 0);
      }
    }
  }
}

TEST_CASE("GetFreeSymStolenDataNode", "[sgp]") {
  GIVEN("A world") {
    emp::Random random(44);
    SymConfigBase config;
    SGPWorld world(random, &config, SquareTasks);
    int side = 100;
    config.GRID_X(side);
    config.GRID_Y(side);
    config.SYM_HORIZ_TRANS_RES(10);
    config.UPDATES(501);

    SyncDataMonitor<double>& data_node_free_sym_stolen = world.GetFreeSymStolenDataNode();
    emp::DataMonitor<int>& free_sym_count_node = world.GetCountFreeSymsDataNode();

    WHEN("There are no free living symbionts in the world") {
      // run the test with hosted symbionts to confirm that the free 
      // symbiont data node is not tracking hosted symbionts
      config.FREE_LIVING_SYMS(0);
      world.Setup();
      world.RunExperiment(false);
      REQUIRE(free_sym_count_node.GetTotal() == 0);
      THEN("Free living symbionts do not steal any points") {
        REQUIRE(data_node_free_sym_stolen.UnsynchronizedGetMonitor().GetTotal() == 0);
      }
    }
    WHEN("There are free living symbionts in the world and ectosymbiosis is permitted") {
      config.FREE_LIVING_SYMS(1);
      config.ECTOSYMBIOSIS(1);
      world.Setup();
      world.RunExperiment(false);
      REQUIRE(free_sym_count_node.GetTotal() > 0);
      THEN("Free living symbionts steal points") {
        REQUIRE(data_node_free_sym_stolen.UnsynchronizedGetMonitor().GetTotal() > 0);
      }
    }
  }
}

TEST_CASE("GetFreeSymEarnedDataNode", "[sgp]") {
  GIVEN("A world") {
    emp::Random random(44);
    SymConfigBase config;
    SGPWorld world(random, &config, SquareTasks);
    int side = 10;
    config.GRID_X(side);
    config.GRID_Y(side);
    config.SYM_HORIZ_TRANS_RES(10);
    config.UPDATES(5);

    SyncDataMonitor<double>& data_node_free_sym_earned = world.GetFreeSymEarnedDataNode();
    emp::DataMonitor<int>& free_sym_count_node = world.GetCountFreeSymsDataNode();

    WHEN("There are no free living symbionts in the world") {
      world.Setup();
      world.RunExperiment(false);
      REQUIRE(free_sym_count_node.GetTotal() == 0);
      THEN("Free living symbionts do not earn any points") {
        REQUIRE(data_node_free_sym_earned.UnsynchronizedGetMonitor().GetTotal() == 0);
      }
    }
    WHEN("There are free living symbionts in the world") {
      config.UPDATES(101);
      config.FREE_LIVING_SYMS(1);
      config.SYM_INFECTION_CHANCE(0);
      world.Setup();
      world.RunExperiment(false);
      REQUIRE(free_sym_count_node.GetTotal() > 0);
      THEN("Free living symbionts earn points") {
        REQUIRE(data_node_free_sym_earned.UnsynchronizedGetMonitor().GetTotal() > 0);
      }
    }
  }
}

TEST_CASE("GetHostEarnedDataNode", "[sgp]") {
  GIVEN("A world") {
    emp::Random random(44);
    SymConfigBase config;
    SGPWorld world(random, &config, SquareTasks);
    int side = 10;
    config.GRID_X(side);
    config.GRID_Y(side);
    config.SYM_HORIZ_TRANS_RES(10);
    config.UPDATES(5);
    
    SyncDataMonitor<double>& data_node_host_earned = world.GetHostEarnedDataNode();
    
    WHEN("There are no hosts in the world") {
      config.POP_SIZE(0);
      world.Setup();
      world.RunExperiment(false);
      REQUIRE(world.GetNumOrgs() == 0);
      THEN("Hosts do not earn any points"){
        REQUIRE(data_node_host_earned.UnsynchronizedGetMonitor().GetTotal() == 0);
      }
    }
    WHEN("There are hosts in the world") {
      config.UPDATES(101);
      config.POP_SIZE(-1);
      world.Setup();
      world.RunExperiment(false);
      REQUIRE(world.GetNumOrgs() > 0);
      THEN("Hosts earn points"){
        REQUIRE(data_node_host_earned.UnsynchronizedGetMonitor().GetTotal() > 0);
      }
    }
  }
}