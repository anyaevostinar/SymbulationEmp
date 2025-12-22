#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"

#include "../../../catch/catch.hpp"

/**
 * This file is dedicated to unit tests for SGPHost
 */

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
    GIVEN("A world and a host"){
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
        WHEN("2 hosts that are clones of original host and one host that is different are created"){
            emp::Ptr<SGPHost> clone1 = emp::NewPtr<SGPHost>(*host);
            emp::Ptr<SGPHost> clone2 = emp::NewPtr<SGPHost>(*host);
            emp::Ptr<SGPHost> different = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(99)); // For comparing

            THEN("host is equal to first clone"){
                REQUIRE(*host == *clone1);
            }
            THEN("The first clone is equal to the second clone"){
                REQUIRE(*clone1 == *clone2);
            }

            THEN("The first clone is not equal to the different host"){
                REQUIRE_FALSE(*host == *different);
            }

            
            WHEN("A host is created using the MakeNew function of SGPHost"){
                emp::Ptr<Organism> host_remade = host->MakeNew();
                THEN("MakeNew produces an identical host"){
                    REQUIRE(*host_remade == *host);
                }
            }
            // Can't assert true/false without knowing bitcode ordering,
            // assert that bitcode ordering is well-defined
            bool lt = *host < *different || *different < *host;
            REQUIRE(lt);
            
            clone1.Delete();
            clone2.Delete();
            different.Delete();
            
        }
  }
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
