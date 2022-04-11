#include "../../lysis_mode/Phage.h"
#include "../../lysis_mode/Bacterium.h"
#include "../../lysis_mode/LysisWorld.h"


TEST_CASE("GetCFUDataNode", "[lysis]"){
  GIVEN( "a world" ) {
    emp::Random random(17);
    SymConfigBase config;
    int int_val = 0;
    LysisWorld w(random);
    config.SYM_LIMIT(4);
    w.Resize(10);

    //keep track of host organisms that are uninfected or infected with only lysogenic phage
    emp::DataMonitor<int>& cfu_data_node = w.GetCFUDataNode();
    REQUIRE(cfu_data_node.GetTotal() == 0);
    REQUIRE(w.GetNumOrgs() == 0);

    WHEN("uninfected hosts are added"){
      size_t num_hosts = 10;

      for(size_t i = 0; i < num_hosts; i++){
        w.AddOrgAt(new Bacterium(&random, &w, &config, int_val), i);
      }

      w.Update();
      THEN("they are tracked by the data node"){
        REQUIRE(w.GetNumOrgs() == num_hosts);
        REQUIRE(cfu_data_node.GetTotal() == num_hosts);
      }

      WHEN("some hosts are infected with lytic phage"){
        size_t num_infections = 2;
        for(size_t i = 0; i < num_infections; i++){
          Phage *p = new Phage(&random, &w, &config, int_val);
          p->SetLysisChance(1.0);
          w.GetOrg(i).AddSymbiont(p);
        }
        w.Update();
        THEN("infected hosts are excluded from the cfu count"){
          REQUIRE(w.GetNumOrgs() == num_hosts);
          REQUIRE(cfu_data_node.GetTotal() == (num_hosts - num_infections));
        }
      }

      WHEN("hosts are infected with lysogenic phage"){
        size_t num_infections = 2;
        for(size_t i = 0; i < num_infections; i++){
          Phage *p = new Phage(&random, &w, &config, int_val);
          p->SetLysisChance(0.0);
          w.GetOrg(i).AddSymbiont(p);
        }
        w.Update();
        THEN("infected hosts are excluded from the cfu count"){
          REQUIRE(w.GetNumOrgs() == num_hosts);
          REQUIRE(cfu_data_node.GetTotal() == num_hosts);
        }
      }
    }
  }
}