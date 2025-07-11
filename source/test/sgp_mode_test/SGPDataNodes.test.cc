#include <filesystem>

TEST_CASE("Checks for correct data files", "[DS]") {
     
  emp::Random random(1);
  SymConfigSGP config;
  config.SEED(2);
  config.ORGANISM_TYPE(0);
  config.STRESS_TYPE(1);
  config.MUTATION_RATE(0.0);
  config.MUTATION_SIZE(0.000);
  config.FILE_NAME("DataTest");


  SGPWorld world(random, &config, LogicTasks);


  WHEN("World is ran for 200 Updates"){
  
  world.CreateDataFiles();
  for (int i = 0; i < 200; i++){
   
    world.Update();
  }
  
  

  THEN("All correct data files are created"){
    REQUIRE(std::filesystem::exists("OrganismCountsDataTest_SEED2.data"));
    REQUIRE(std::filesystem::exists("SymDonatedDataTest_SEED2.data"));
    REQUIRE(std::filesystem::exists("TasksDataTest_SEED2.data"));
    REQUIRE(std::filesystem::exists("TransmissionRatesDataTest_SEED2.data"));
  }
  std::filesystem::remove("OrganismCountsDataTest_SEED2.data");
  std::filesystem::remove("SymDonatedDataTest_SEED2.data");
  std::filesystem::remove("TasksDataTest_SEED2.data");
  std::filesystem::remove("TransmissionRatesDataTest_SEED2.data");
  }
}
