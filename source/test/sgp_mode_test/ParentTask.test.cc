#include "../../sgp_mode/GenomeLibrary.h"
#include "../../sgp_mode/CPU.h"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPHost.cc"
#include "../../sgp_mode/HealthHost.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPWorldSetup.cc"
#include "../../sgp_mode/SGPDataNodes.h"

#include "../../catch/catch.hpp"


TEST_CASE("If a symbiote reproduces, its child's parent_tasks_perforemd will be equivalent to the parents tasks", "[TaskTest]"){
 
  std::cout << "If a symbiote reproduces, its child's parent_tasks_perforemd will be equivalent to the parents tasks" << std::endl;
  emp::Random random(1);
  SymConfigSGP config;
  config.RANDOM_ANCESTOR(false);
  config.SEED(2);
  config.ORGANISM_TYPE(HEALTH);
  config.STRESS_TYPE(PARASITE);
  config.MUTATION_RATE(0.0);
  config.MUTATION_SIZE(0.002);
  config.TRACK_PARENT_TASKS(1);
  config.VT_TASK_MATCH(1);
  config.ONLY_FIRST_TASK_CREDIT(1);
  config.HOST_REPRO_RES(10000);

  SGPWorld world(random, &config, LogicTasks);

  //Builds program that does both NOT and NAND operations
  ProgramBuilder program;
  program.AddNot();

  //Creates a host that only does NOT operations
  emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));
  emp::Ptr<SGPHost> host2 = emp::NewPtr<SGPHost>(&random, &world, &config, CreateEQUProgram(100));
  //Creates a symbiote that does both Not and Nand operations
  emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, program.Build(100));

  //Adds host to world and sym to host.
  world.AddOrgAt(host, 0);
  world.AddOrgAt(host2, 1);
  host->AddSymbiont(sym);


  
  for (int i = 0; i < 1000; i++) {
          
          world.Update();
        }

  
  REQUIRE(!host2->HasSym());
  
  emp::vector<emp::Ptr<Organism>> &syms = host->GetSymbionts();
  emp::Ptr<Organism> Cur_Org = syms[0];
  emp::Ptr<SGPSymbiont> Cur_Sym = Cur_Org.DynamicCast<SGPSymbiont>();

  
  //Checks to see how many tasks the syms parents have completed, there should be none. 
  int tasksCompleted = 0;
  for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
    std::cout << Cur_Sym->GetCPU().state.parent_tasks_performed->Get(i) << std::endl;
    tasksCompleted += Cur_Sym->GetCPU().state.parent_tasks_performed->Get(i);
  }
  REQUIRE(tasksCompleted == 0);
  


}