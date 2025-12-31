#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"

#include "../../../catch/catch.hpp"

/**
 * This file is dedicated to unit tests for SGPHost
 */

TEST_CASE("SGPHost destructor cleans up shared pointers and in-progress reproduction", "[sgp][sgp-unit]") {
    GIVEN("A host"){
        emp::Random random(31);
        SymConfigSGP config;
        SGPWorld world(random, &config, LogicTasks);
        emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));
        host->SetLocation(emp::WorldPosition(1, 2));
        host->GetCPU().state.in_progress_repro = 3;
        world.to_reproduce.resize(5); 

        WHEN("The host is destroyed") {
            host.Delete(); 
            
            THEN("Reproduction queue is invalidated after symbiont is destroyed") {
            REQUIRE(world.to_reproduce[3] == nullptr);
            }
        }
    }
}


TEST_CASE("Host == operators", "[sgp][sgp-unit]") {
    GIVEN("A host"){
        emp::Random random(31);
        SymConfigSGP config;
        SGPWorld world(random, &config, LogicTasks);
        
        emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));
        WHEN("2 clones of the original host are created"){
            emp::Ptr<SGPHost> clone1 = emp::NewPtr<SGPHost>(*host);
            emp::Ptr<SGPHost> clone2 = emp::NewPtr<SGPHost>(*host);

            THEN("host is equal to first clone"){
                REQUIRE(*host == *clone1);
            }
            THEN("The first clone is equal to the second clone"){
                REQUIRE(*clone1 == *clone2);
            }
        }
        
        WHEN("A host that is different to the original is created"){
             emp::Ptr<SGPHost> different = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(99)); // For comparing
             THEN("The host is not equal to the different host"){
                REQUIRE_FALSE(*host == *different);
            }
        }
  }
}

TEST_CASE("Host > & < operators", "[sgp][sgp-unit]") {
    GIVEN("two different hosts"){
        emp::Random random(31);
        SymConfigSGP config;
        SGPWorld world(random, &config, LogicTasks);
        
        emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));
        emp::Ptr<SGPHost> different = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(99)); // For comparing
        WHEN("The two hosts are compared"){
            THEN("One host is less then the other host"){
                // Can't assert true/false without knowing bitcode ordering,
                // assert that bitcode ordering is well-defined
                bool lt = *host < *different || *different < *host;
                REQUIRE(lt);      
            }
        }
    }
}


TEST_CASE("MakeNew returns identical host", "[sgp][sgp-unit]"){
    GIVEN("A host"){
        emp::Random random(31);
        SymConfigSGP config;
        SGPWorld world(random, &config, LogicTasks);
        
        emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));

        WHEN("A new host is created using the MakeNew function of the first host"){
            emp::Ptr<Organism> host_remade = host->MakeNew();
            THEN("MakeNew produces an identical host"){
                REQUIRE(*host_remade == *host);
            }
        }
    }
}

TEST_CASE("SetReproCount & GetReproCount","[sgp][sgp-unit]"){
    GIVEN("An SGPWorld and a host"){
        emp::Random random(31);
        SymConfigSGP config;
        SGPWorld world(random, &config, LogicTasks);
        emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));
        WHEN("The host is added to the world"){
            world.AddOrgAt(host, 0);
            
            THEN("Repro count of the host is 0"){
                REQUIRE(host->GetReproCount() == 0);
            }
            WHEN("The host's repro count is increased by 1"){
                host->SetReproCount(1);
                THEN("Repro count of the host is 1"){
                    REQUIRE(host->GetReproCount() == 1);
                }
            }
        }
    }
}
