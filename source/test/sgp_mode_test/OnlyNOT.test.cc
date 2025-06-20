#include "../../sgp_mode/GenomeLibrary.h"
#include "../../sgp_mode/CPU.h"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPHost.cc"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPWorldSetup.cc"
#include "../../sgp_mode/SGPDataNodes.h"

#include "../../catch/catch.hpp"

TEST_CASE("Organisms, without mutation can only do NOT operations", "[EliasHE]") {
     
  emp::Random random(1);
  SymConfigSGP config;
  config.RANDOM_ANCESTOR(false);
  config.SEED(2);
  config.ORGANISM_TYPE(HEALTH);
  config.STRESS_TYPE(PARASITE);
  config.MUTATION_RATE(0.0);
  config.MUTATION_SIZE(0.000);
  SGPWorld world(random, &config, LogicTasks);

  // Mock Organism to check reproduction
  class TestOrg : public Organism {
  public:
    bool IsHost() override { return true; }
    void AddPoints(double p) override {}
    double GetPoints() override { return 0; }
  };

  TestOrg organism;
  // Empty builder
  ProgramBuilder builder;
  (builder.AddNot)();
  CPU cpu(&organism, &world, builder.Build(100));
  
  
  
  cpu.RunCPUStep(0, 100);
  
  int all_ones_binary = 4294967295;
  cpu.state.input_buf.push(all_ones_binary);
  cpu.RunCPUStep(0, 100);
  world.Update();

  for (auto data : world.GetTaskSet()) {
    
      std::cout << data.task.name << "  "<<data.n_succeeds_host<< std::endl;
      if(data.task.name != "NOT"){
        //std::cout << "I am not NOT I am "<< data.task.name << std::endl;
       REQUIRE(data.n_succeeds_host == 0);
      }
      else{
        REQUIRE(data.n_succeeds_host > 0);
      }
    
  }

  cpu.state.shared_available_dependencies.Delete();
  cpu.state.used_resources.Delete();
  cpu.state.internal_environment.Delete();


}