#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"

#include "../../../catch/catch.hpp"


TEST_CASE("SGPHost destructor cleans up shared pointers and in-progress reproduction", "[sgp][sgp-unit]") {
     emp::Random random(31);
	SymConfigSGP config;
    SGPWorld world(random, &config, LogicTasks);
    emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));
    host->SetLocation(emp::WorldPosition(1, 2));
    host->GetCPU().state.in_progress_repro = 3;
    world.to_reproduce.resize(5); 

    WHEN("Host is destroyed") {
        host.Delete(); 
        
        THEN("Reproduction queue is invalidated after symbiont is destroyed") {
        REQUIRE(world.to_reproduce[3] == nullptr);
        }
    }
}


TEST_CASE("Host comparison operators", "[sgp][sgp-unit]") {
    emp::Random random(31);
	SymConfigSGP config;
	SGPWorld world(random, &config, LogicTasks);
	

    emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));
    config.TRACK_PARENT_TASKS(1);
    world.AddOrgAt(host, 0);
   

    //Make sure the host is still equal even after having lived a while
    for (int i = 0; i < 25; i++) {
      world.Update();
    }
    emp::Ptr<SGPHost> clone1 = emp::NewPtr<SGPHost>(*host);
    emp::Ptr<SGPHost> clone2 = emp::NewPtr<SGPHost>(*host);
    emp::Ptr<SGPHost> different = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(99)); // For comparing

    REQUIRE(*host == *clone1);
    REQUIRE(*clone1 == *clone2);

    REQUIRE_FALSE(*host == *different);

    
    emp::Ptr<Organism> host_remade = host->MakeNew();
    REQUIRE(*host_remade == *host);

    // Can't assert true/false without knowing bitcode ordering,
    // assert that bitcode ordering is well-defined
    bool lt = *host < *different || *different < *host;
    REQUIRE(lt);
    
    clone1.Delete();
    clone2.Delete();
    different.Delete();
  }

TEST_CASE("SetReproCount & GetReproCount","[sgp][sgp-unit]"){
    emp::Random random(31);
	SymConfigSGP config;
	SGPWorld world(random, &config, LogicTasks);
    emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));
    world.AddOrgAt(host, 0);

    WHEN("A host is added to the world"){
        THEN("Repro count of Host is 0"){
            REQUIRE(host->GetReproCount() == 0);
        }
    }
    WHEN("Host's repro count is increased by 1"){
        host->SetReproCount(1);
        THEN("Repro count of Host is 1"){
            REQUIRE(host->GetReproCount() == 1);
        }
    }
}
