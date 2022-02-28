#include "../default_mode/DataNodes.h"
#include "../default_mode/Symbiont.h"
#include "../default_mode/Host.h"

TEST_CASE("GetHostCountDataNode"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    SymWorld w(random);
    w.Resize(4);

    emp::DataMonitor<int>& host_count_node = w.GetHostCountDataNode();
    REQUIRE(host_count_node.GetTotal() == 0);
    REQUIRE(w.GetNumOrgs() == 0);

    WHEN("a host is added"){
      w.AddOrgAt(new Host(&random, &w, &config, int_val), 0);
      w.Update();
      host_count_node = w.GetHostCountDataNode();
      THEN("the host is tracked by the data node"){
        REQUIRE(host_count_node.GetTotal() == 1);
        REQUIRE(w.GetNumOrgs() == 1);
      }
    }
  }
}

TEST_CASE("GetSymCountDataNode"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    SymWorld w(random);
    w.Resize(4);

    emp::DataMonitor<int>& sym_count_node = w.GetSymCountDataNode();
    REQUIRE(sym_count_node.GetTotal() == 0);
    REQUIRE(w.GetNumOrgs() == 0);

    WHEN("a sym is added"){
      w.AddOrgAt(new Symbiont(&random, &w, &config, int_val), 0);
      w.Update();
      sym_count_node = w.GetSymCountDataNode();
      THEN("the sym is tracked by the data node"){
        REQUIRE(sym_count_node.GetTotal() == 1);
        REQUIRE(w.GetNumOrgs() == 1);
      }
    }
  }
}
