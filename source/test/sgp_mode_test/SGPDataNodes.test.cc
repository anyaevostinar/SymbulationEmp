#include <filesystem>
#include <fstream>
#include <string>

TEST_CASE("Correct data files are created", "[sgp]") {
  
  GIVEN("A World with no mutation"){
    emp::Random random(1);
    SymConfigSGP config;
    config.SEED(2);
    config.INTERACTION_MECHANISM(0);
    config.SYMBIONT_TYPE(1);
    config.MUTATION_RATE(0.0);
    config.MUTATION_SIZE(0.000);
    config.FILE_NAME("DataTest");
    config.CPU_TRANSFER_CHANCE(0.8);
    config.SYM_HORIZ_TRANS_RES(0);



    WHEN("The World is ran for 200 Updates with 1 host"){
    SGPWorld world(random, &config, LogicTasks);

    //Creates a host that only does NOT operations
    emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));

    //Adds host to world
    world.AddOrgAt(host, 0);
    world.CreateDataFiles();
    for (int i = 0; i < 200; i++){
      world.Update();
    }
    
    world.WriteTaskCombinationsFile("EndingTaskCombinationsDataTest_SEED2.data");
    world.WriteOrgReproHistFile("OrgReproHistDataTest_SEED2.data");
    THEN("All correct data files are created"){
      REQUIRE(std::filesystem::exists("OrganismCountsDataTest_SEED2.data"));
      REQUIRE(std::filesystem::exists("TasksDataTest_SEED2.data"));
      REQUIRE(std::filesystem::exists("TransmissionRatesDataTest_SEED2.data"));

      REQUIRE(std::filesystem::exists("EndingTaskCombinationsDataTest_SEED2.data"));
      REQUIRE(std::filesystem::exists("OrgReproHistDataTest_SEED2.data"));
      REQUIRE(std::filesystem::exists("SymInstCountDataTest_SEED2.data"));
    }
    // Create a text string, which is used to output the text file
      std::ifstream file("OrganismCountsDataTest_SEED2.data");
      std::string str; 
      THEN("The OrganismCount file should contain 3 lines"){
        std::getline(file, str);
        THEN("The first should be a header"){
          REQUIRE(str == "update,count,hosted_syms");
        }
        std::getline(file, str);
        THEN("The second should be 1 host alive at update 0"){
          REQUIRE(str == "0,1,0");
        }
        std::getline(file, str);
        THEN("The third should be 1 host alive at update 100"){
          REQUIRE(str == "100,1,0");
        }
      }
    

    std::ifstream file3("TasksDataTest_SEED2.data");
    std::string str3; 
    THEN("The Task File should contain 3 lines"){
      std::getline(file3, str3);
        THEN("The first should be a header"){
          REQUIRE(str3 == "update,host_task_NOT,sym_task_NOT,host_task_NAND,sym_task_NAND,host_task_AND,sym_task_AND,host_task_ORN,sym_task_ORN,host_task_OR,sym_task_OR,host_task_ANDN,sym_task_ANDN,host_task_NOR,sym_task_NOR,host_task_XOR,sym_task_XOR,host_task_EQU,sym_task_EQU");
        }
        std::getline(file3, str3);
        THEN("The second should be all zeroes because at 0 updates in no tasks should have been completed"){
          REQUIRE(str3 == "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
        }
        std::getline(file3, str3);
        THEN("The third should contain 4 NOT host tasks because in 100 updates 1 host can perform their genome 4 times"){
          REQUIRE(str3 == "100,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
        }
    }

    std::ifstream file4("TransmissionRatesDataTest_SEED2.data");
    std::string str4; 
    THEN("The TransmissionRates File should contain 3 lines"){
      std::getline(file4, str4);
        THEN("The first should be a header"){
          REQUIRE(str4 == "update,attempts_horiztrans,successes_horiztrans,attempts_verttrans,successes_verttrans");
        }
        std::getline(file4, str4);
        THEN("The second should be all zeroes because there are no symbionts"){
          REQUIRE(str4 == "0,0,0,0,0");
        }
        std::getline(file4, str4);
        THEN("The third should be at 100 updates and the rest be zero because there are no symbionts"){
          REQUIRE(str4 == "100,0,0,0,0");
        }
    }

    std::ifstream file5("EndingTaskCombinationsDataTest_SEED2.data");
    std::string str5; 
    THEN("The EndingTaskCombinations File should contain 2 lines"){
      std::getline(file5, str5);
        THEN("The first should be a header"){
          REQUIRE(str5 == "task_profile,host_count,symbiont_count,can_inf_hosts,can_inf_symbionts");
        }
        std::getline(file5, str5);
        THEN("The second should be 1 organism completing solely the NOT task"){
          REQUIRE(str5 == "000000001,1,0,0,0");
        }
        
    }
  }

    WHEN("The World is ran for 200 Updates with 2 hosts and 1 symbiont"){
      SGPWorld world(random, &config, LogicTasks);

      //Creates a host and sym that only do NOT operations
      emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));
      emp::Ptr<SGPHost> host2 = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));

      emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(100));

      //Adds host to world and sym to host.
      world.AddOrgAt(host, 0);
      world.AddOrgAt(host2, 1);

      host->AddSymbiont(sym);

      world.CreateDataFiles();
      for (int i = 0; i < 200; i++){
        world.Update();
      }
      
      
      world.WriteTaskCombinationsFile("EndingTaskCombinationsDataTest_SEED2.data");
      world.WriteOrgReproHistFile("OrgReproHistDataTest_SEED2.data");
      THEN("All correct data files are created"){
        REQUIRE(std::filesystem::exists("OrganismCountsDataTest_SEED2.data"));
        REQUIRE(std::filesystem::exists("TasksDataTest_SEED2.data"));
        REQUIRE(std::filesystem::exists("TransmissionRatesDataTest_SEED2.data"));

        REQUIRE(std::filesystem::exists("EndingTaskCombinationsDataTest_SEED2.data"));
        REQUIRE(std::filesystem::exists("OrgReproHistDataTest_SEED2.data"));
      }
      // Create a text string, which is used to output the text file
        std::ifstream file("OrganismCountsDataTest_SEED2.data");
        std::string str; 
        THEN("The OrganismCount file should contain 3 lines"){
          std::getline(file, str);
          THEN("The first should be a header"){
            REQUIRE(str == "update,count,hosted_syms");
          }
          std::getline(file, str);
          THEN("The second should be 2 hosts and 1 sym alive at update 0"){
            REQUIRE(str == "0,2,1");
          }
          std::getline(file, str);
          THEN("The third should be 2 hosts and 2 sym alive at update 100"){
            REQUIRE(str == "100,2,2");
          }
        }
    

      std::ifstream file3("TasksDataTest_SEED2.data");
      std::string str3; 
      THEN("The Task File should contain 3 lines"){
        std::getline(file3, str3);
          THEN("The first should be a header"){
            REQUIRE(str3 == "update,host_task_NOT,sym_task_NOT,host_task_NAND,sym_task_NAND,host_task_AND,sym_task_AND,host_task_ORN,sym_task_ORN,host_task_OR,sym_task_OR,host_task_ANDN,sym_task_ANDN,host_task_NOR,sym_task_NOR,host_task_XOR,sym_task_XOR,host_task_EQU,sym_task_EQU");
          }
          std::getline(file3, str3);
          THEN("The second should be all zeroes because at 0 updates in no tasks should have been completed"){
            REQUIRE(str3 == "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
          }
          std::getline(file3, str3);
          THEN("The third should contain 8 NOT host tasks and 7 NOT sym tasks"){
            REQUIRE(str3 == "100,8,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
          }
      }

      std::ifstream file4("TransmissionRatesDataTest_SEED2.data");
      std::string str4; 
      THEN("The TransmissionRates File should contain 3 lines"){
        std::getline(file4, str4);
          THEN("The first should be a header"){
            REQUIRE(str4 == "update,attempts_horiztrans,successes_horiztrans,attempts_verttrans,successes_verttrans");
          }
          std::getline(file4, str4);
          THEN("The second should be all zeroes because the syms have been unable to attempt transmissions yet"){
            REQUIRE(str4 == "0,0,0,0,0");
          }
          std::getline(file4, str4);
          THEN("The third should be at 100 updates with 7 attempts and 1 success"){
            REQUIRE(str4 == "100,7,1,0,0");
          }
      }

      std::ifstream file5("EndingTaskCombinationsDataTest_SEED2.data");
      std::string str5; 
      THEN("The EndingTaskCombinations File should contain 2 lines"){
      std::getline(file5, str5);
        THEN("The first should be a header"){
          REQUIRE(str5 == "task_profile,host_count,symbiont_count,can_inf_hosts,can_inf_symbionts");
        }
        std::getline(file5, str5);
        THEN("The second should be 4 organisms completing solely the NOT task, 2 hosts and 2 symbionts"){
          REQUIRE(str5 == "000000001,2,2,0,0");
        }
      }


      std::filesystem::remove("OrganismCountsDataTest_SEED2.data");
      std::filesystem::remove("TasksDataTest_SEED2.data");
      std::filesystem::remove("TransmissionRatesDataTest_SEED2.data");

      std::filesystem::remove("EndingTaskCombinationsDataTest_SEED2.data");
      std::filesystem::remove("OrgReproHistDataTest_SEED2.data");
      std::filesystem::remove("SymInstCountDataTest_SEED2.data");

      THEN("All previously created data files have been removed"){
        REQUIRE(!std::filesystem::exists("OrganismCountsDataTest_SEED2.data"));
        REQUIRE(!std::filesystem::exists("TasksDataTest_SEED2.data"));
        REQUIRE(!std::filesystem::exists("TransmissionRatesDataTest_SEED2.data"));

        REQUIRE(!std::filesystem::exists("EndingTaskCombinationsDataTest_SEED2.data"));
        REQUIRE(!std::filesystem::exists("OrgReproHistDataTest_SEED2.data"));
        REQUIRE(!std::filesystem::exists("SymInstCountDataTest_SEED2.data"));
      }

    }

  }
}
