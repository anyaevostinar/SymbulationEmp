#include "../../sgp_mode/GenomeLibrary.h"
#include "../../sgp_mode/CPU.h"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPHost.cc"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPWorldSetup.cc"
#include "../../sgp_mode/SGPDataNodes.h"

#include "../../catch/catch.hpp"


TEST_CASE("When ONLY_FIRST_TASK_CREDIT is 1, Symbiotes can only do 1 task at a time", "[1CRED]"){
 
  std::cout << "When ONLY_FIRST_TASK_CREDIT is 1, Symbiotes can only do 1 task at a time" << std::endl;
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

  SGPWorld world(random, &config, LogicTasks);

  //Builds program that does both NOT and NAND operations
  ProgramBuilder program;
  program.AddNot();
  program.AddNand();

  //Creates a host that only does NOT operations
  emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));
  
  //Creates a symbiote that does both Not and Nand operations
  emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, program.Build(100));

  //Adds host to world and sym to host.
  world.AddOrgAt(host, 0);
  host->AddSymbiont(sym);


  
  for (int i = 0; i < 250; i++) {
          
          world.Update();
        }

  //Grabs symbiote from host, ensure that sym did not die in process.
  emp::vector<emp::Ptr<Organism>> &syms = host->GetSymbionts();
  emp::Ptr<Organism> Cur_Org = syms[0];
  emp::Ptr<SGPSymbiont> Cur_Sym = Cur_Org.DynamicCast<SGPSymbiont>();
  
  //Checks to see how many tasks have been completed, only 1 should be set to 1. 
  int tasksCompleted = 0;
  for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
    std::cout << Cur_Sym->GetCPU().state.tasks_performed->Get(i) << std::endl;
    tasksCompleted += Cur_Sym->GetCPU().state.tasks_performed->Get(i);
  }
  REQUIRE(tasksCompleted == 1);


}

TEST_CASE("When ONLY_FIRST_TASK_CREDIT is 0, Symbiotes can do more then 1 task", "[1CRED]"){
  std::cout << "When ONLY_FIRST_TASK_CREDIT is 0, Symbiotes can do more then 1 task" << std::endl;
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
  config.ONLY_FIRST_TASK_CREDIT(0);

  SGPWorld world(random, &config, LogicTasks);

  
   //Builds program that does both NOT and NAND operations
  ProgramBuilder program;
  program.AddNot();
  program.AddNand();

  //Creates a host that only does NOT operations
  emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));
  
  //Creates a symbiote that does both Not and Nand operations
  emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, program.Build(100));

  //Adds host to world and sym to host.
  world.AddOrgAt(host, 0);
  host->AddSymbiont(sym);


  
  
  for (int i = 0; i < 250; i++) {
          
          world.Update();
        }
 //Grabs symbiote from host, ensure that sym did not die in process.
  emp::vector<emp::Ptr<Organism>> &syms = host->GetSymbionts();
  emp::Ptr<Organism> Cur_Org = syms[0];
  emp::Ptr<SGPSymbiont> Cur_Sym = Cur_Org.DynamicCast<SGPSymbiont>();
  
  //Checks to see how many tasks have been completed, there should be more then 1. 
  int tasksCompleted = 0;
  for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
    std::cout << Cur_Sym->GetCPU().state.tasks_performed->Get(i) << std::endl;
    tasksCompleted += Cur_Sym->GetCPU().state.tasks_performed->Get(i);
  }
  REQUIRE(tasksCompleted > 1);


}


TEST_CASE("When ONLY_FIRST_TASK_CREDIT is 1, Symbiotes are allowed to do no tasks", "[1CRED]"){
 
  std::cout << "When ONLY_FIRST_TASK_CREDIT is 1, Symbiotes are allowed to do no tasks" << std::endl;
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

  SGPWorld world(random, &config, LogicTasks);

  
  //Builds program that does no tasks
  ProgramBuilder program;

  //Creates a host that only does NOT operations
  emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));
  
  //Creates a symbiote that does not do any tasks
  emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, program.Build(100));

  //Adds host to world and sym to host.
  world.AddOrgAt(host, 0);
  host->AddSymbiont(sym);


  for (int i = 0; i < 250; i++) {
          
          world.Update();
        }
  //Grabs symbiote from host, ensure that sym did not die in process.
  emp::vector<emp::Ptr<Organism>> &syms = host->GetSymbionts();
  emp::Ptr<Organism> Cur_Org = syms[0];
  emp::Ptr<SGPSymbiont> Cur_Sym = Cur_Org.DynamicCast<SGPSymbiont>();
  
  //Checks to see how many tasks have been completed, there should be none. 
  int tasksCompleted = 0;
  for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
    std::cout << Cur_Sym->GetCPU().state.tasks_performed->Get(i) << std::endl;
    tasksCompleted += Cur_Sym->GetCPU().state.tasks_performed->Get(i);
  }
  REQUIRE(tasksCompleted == 0);


}

TEST_CASE("When ONLY_FIRST_TASK_CREDIT is 0, Symbiotes are allowed to do no tasks", "[1CRED]"){
  std::cout << "When ONLY_FIRST_TASK_CREDIT is 0, Symbiotes are allowed to do no tasks" << std::endl;
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
  config.ONLY_FIRST_TASK_CREDIT(0);

  SGPWorld world(random, &config, LogicTasks);

  
  //Builds program that does no tasks
  ProgramBuilder program;

  //Creates a host that only does NOT operations
  emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));
  
  //Creates a symbiote that does not do any tasks
  emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, program.Build(100));

  //Adds host to world and sym to host.
  world.AddOrgAt(host, 0);
  host->AddSymbiont(sym);


  
  
  for (int i = 0; i < 250; i++) {
          
          world.Update();
        }
  //Grabs symbiote from host, ensure that sym did not die in process.
  emp::vector<emp::Ptr<Organism>> &syms = host->GetSymbionts();
  emp::Ptr<Organism> Cur_Org = syms[0];
  emp::Ptr<SGPSymbiont> Cur_Sym = Cur_Org.DynamicCast<SGPSymbiont>();
  
  //Checks to see how many tasks have been completed, there should be none. 
  int tasksCompleted = 0;
  for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
    std::cout << Cur_Sym->GetCPU().state.tasks_performed->Get(i) << std::endl;
    tasksCompleted += Cur_Sym->GetCPU().state.tasks_performed->Get(i);
  }
  REQUIRE(tasksCompleted == 0);


}

TEST_CASE("When ONLY_FIRST_TASK_CREDIT is 1, Hosts can do more then 1 task", "[1CRED]"){
  
  
  std::cout << "When ONLY_FIRST_TASK_CREDIT is 1, Hosts can do more then 1 task" << std::endl;
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

  SGPWorld world(random, &config, LogicTasks);

  
  //Builds program that does both NOT and NAND operations
  ProgramBuilder program;
  program.AddNot();
  program.AddNand();

  //Creates a host that does both Not and Nand operations
  emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, program.Build(100));
  
  //Creates a symbiote that does both Not and Nand operations
  emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, program.Build(100));

  //Adds host to world and sym to host.
  world.AddOrgAt(host, 0);
  host->AddSymbiont(sym);
  
  
  for (int i = 0; i < 250; i++) {
          
          world.Update();
        }

  //Checks to see how many tasks the host complete, 2 should be set to 1. 
  int tasksCompleted = 0;
  for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
    std::cout << host->GetCPU().state.tasks_performed->Get(i) << std::endl;
    tasksCompleted += host->GetCPU().state.tasks_performed->Get(i);
  }
  REQUIRE(tasksCompleted > 1);


}

TEST_CASE("When ONLY_FIRST_TASK_CREDIT is 0, Hosts can do more then 1 task", "[1CRED]"){
 
  std::cout << "When ONLY_FIRST_TASK_CREDIT is 0, Hosts can do more then 1 task" << std::endl;
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
  config.ONLY_FIRST_TASK_CREDIT(0);

  SGPWorld world(random, &config, LogicTasks);

  
  //Builds program that does both NOT and NAND operations
  ProgramBuilder program;
  program.AddNot();
  program.AddNand();

  //Creates a host that does both Not and Nand operations
  emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, program.Build(100));
  
  //Creates a symbiote that does both Not and Nand operations
  emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, program.Build(100));

  //Adds host to world and sym to host.
  world.AddOrgAt(host, 0);
  host->AddSymbiont(sym);

  
  for (int i = 0; i < 250; i++) {
          
          world.Update();
        }
 
  //Checks to see how many tasks the host complete, 2 should be set to 1. 
  int tasksCompleted = 0;
  for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
    std::cout << host->GetCPU().state.tasks_performed->Get(i) << std::endl;
    tasksCompleted += host->GetCPU().state.tasks_performed->Get(i);
  }
  REQUIRE(tasksCompleted > 1);


}

TEST_CASE("When ONLY_FIRST_TASK_CREDIT is 1, Symbiotes can only do 1 task at a time, using ALL tasks", "[1CRED]"){
 
  std::cout << "When ONLY_FIRST_TASK_CREDIT is 1, Symbiotes can only do 1 task at a time, using ALL tasks" << std::endl;
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

  SGPWorld world(random, &config, LogicTasks);

  //Creates a program that can complete all tasks
  ProgramBuilder program;
  program.AddNot();
  program.AddNand();
  program.AddAnd();
  program.AddOrn();
  program.AddOr();
  program.AddAndn();
  program.AddNor();
  program.AddXor();
  program.AddEqu();

  //Creates a host that only does NOT operations
  emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));

  //Creates a symbiote that completes all tasks
  emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, program.Build(100));

  //Adds host to world and sym to host.
  world.AddOrgAt(host, 0);
  host->AddSymbiont(sym);
  
  
  for (int i = 0; i < 250; i++) {
          
          world.Update();
        }
  //Grabs symbiote from host, ensure that sym did not die in process.
  emp::vector<emp::Ptr<Organism>> &syms = host->GetSymbionts();
  emp::Ptr<Organism> Cur_Org = syms[0];
  emp::Ptr<SGPSymbiont> Cur_Sym = Cur_Org.DynamicCast<SGPSymbiont>();
  
  //Checks to see how many tasks have been completed, only 1 should be set to 1. 
  int tasksCompleted = 0;
  for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
    std::cout << Cur_Sym->GetCPU().state.tasks_performed->Get(i) << std::endl;
    tasksCompleted += Cur_Sym->GetCPU().state.tasks_performed->Get(i);
  }
  REQUIRE(tasksCompleted <= 1);


}

TEST_CASE("When ONLY_FIRST_TASK_CREDIT is 0, Symbiotes can do all tasks", "[1CRED]"){
 
  std::cout << "When ONLY_FIRST_TASK_CREDIT is 0, Symbiotes can do all tasks" << std::endl;
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
  config.ONLY_FIRST_TASK_CREDIT(0);

  SGPWorld world(random, &config, LogicTasks);

  
  //Creates a program that can complete all tasks
  ProgramBuilder program;
  program.AddNot();
  program.AddNand();
  program.AddAnd();
  program.AddOrn();
  program.AddOr();
  program.AddAndn();
  program.AddNor();
  program.AddXor();
  program.AddEqu();

  //Creates a host that only does NOT operations
  emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));

  //Creates a symbiote that completes all tasks
  emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, program.Build(100));

  //Adds host to world and sym to host.
  world.AddOrgAt(host, 0);
  host->AddSymbiont(sym);

  
  for (int i = 0; i < 250; i++) {
          
          world.Update();
        }
  //Grabs symbiote from host, ensure that sym did not die in process.
  emp::vector<emp::Ptr<Organism>> &syms = host->GetSymbionts();
  emp::Ptr<Organism> Cur_Org = syms[0];
  emp::Ptr<SGPSymbiont> Cur_Sym = Cur_Org.DynamicCast<SGPSymbiont>();
  
  //Checks to see how many tasks have been completed, all of them should be set to 1. 
  int tasksCompleted = 0;
  for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
    std::cout << Cur_Sym->GetCPU().state.tasks_performed->Get(i) << std::endl;
    tasksCompleted += Cur_Sym->GetCPU().state.tasks_performed->Get(i);
  }
  REQUIRE(tasksCompleted == CPU_BITSET_LENGTH);


}

