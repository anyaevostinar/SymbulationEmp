#include "../../sgp_mode/GenomeLibrary.h"
#include "../../sgp_mode/CPU.h"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPHost.cc"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPWorldSetup.cc"
#include "../../sgp_mode/SGPDataNodes.h"

#include "../../catch/catch.hpp"

TEST_CASE("Organisms, without mutation can only do NOT operations", "[sgp]") {
     
  emp::Random random(1);
  SymConfigSGP config;
  config.SEED(2);
  config.INTERACTION_MECHANISM(HEALTH);
  config.SYMBIONT_TYPE(PARASITE);
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

  // NOT builder
  ProgramBuilder builder;
  (builder.AddNot)();
  CPU cpu(&organism, &world, builder.Build(100));
  
  
  
  cpu.RunCPUStep(0, 100);
  
  //The result of a AND bitwise operations when one of the inputs, in binary, is all ones will be the other input
  long all_ones_binary = 4294967295;
  cpu.state.input_buf.push(all_ones_binary);
  cpu.RunCPUStep(0, 100);
  world.Update();

  //Checks both that NOT is being done and no other operations are being done
  for (auto data : world.GetTaskSet()) {
    
      if(data.task.name != "NOT"){
    
       REQUIRE(data.n_succeeds_host == 0);
      }
      else{
        REQUIRE(data.n_succeeds_host > 0);
      }
  }
}
