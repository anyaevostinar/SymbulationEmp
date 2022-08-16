#include "../../sgp_mode/GenomeLibrary.h"
#include "../../sgp_mode/CPU.h"

#include "../../catch/catch.hpp"

void TestGenome(emp::Ptr<Task> task, void (ProgramBuilder::*method)()) {
  emp::Random random(61);
  SymConfigBase config;
  config.RANDOM_ANCESTOR(false);
  config.SYM_HORIZ_TRANS_RES(100);

  // Ensure we can actually complete the task
  task->MarkAlwaysPerformable();

  SGPWorld world(random, &config, TaskSet{task});

  // Mock Organism to check reproduction
  class TestOrg : public Organism {
  public:
    bool IsHost() override { return true; }
    void AddPoints(double p) override {}
    double GetPoints() override { return 0; }
  };

  TestOrg organism;
  ProgramBuilder builder;
  // Call the provided method reference
  (builder.*method)();
  CPU cpu(&organism, &world, builder.Build(100));
  // cpu.PrintCode();
  cpu.state.shared_completed = emp::NewPtr<emp::vector<size_t>>();
  cpu.state.shared_completed->resize(LogicTasks.NumTasks());

  cpu.RunCPUStep(0, 100);
  world.Update();

  REQUIRE((*world.GetTaskSet().begin()).n_succeeds_host > 0);

  cpu.state.shared_completed.Delete();
  cpu.state.used_resources.Delete();
  cpu.state.internalEnvironment.Delete();
}

TEST_CASE("Generate NOT program", "[sgp]") {
  TestGenome(emp::NewPtr<InputTask>(NOT), &ProgramBuilder::AddNot);
}

TEST_CASE("Generate SQUARE program", "[sgp]") {
  TestGenome(emp::NewPtr<SquareTask>(SQU), &ProgramBuilder::AddSquare);
}

TEST_CASE("Generate NAND program", "[sgp]") {
  TestGenome(emp::NewPtr<InputTask>(NAND), &ProgramBuilder::AddNand);
}
TEST_CASE("Generate AND program", "[sgp]") {
  TestGenome(emp::NewPtr<InputTask>(AND), &ProgramBuilder::AddAnd);
}
TEST_CASE("Generate ORN program", "[sgp]") {
  TestGenome(emp::NewPtr<InputTask>(ORN), &ProgramBuilder::AddOrn);
}
TEST_CASE("Generate OR program", "[sgp]") {
  TestGenome(emp::NewPtr<InputTask>(OR), &ProgramBuilder::AddOr);
}
TEST_CASE("Generate ANDN program", "[sgp]") {
  TestGenome(emp::NewPtr<InputTask>(ANDN), &ProgramBuilder::AddAndn);
}
TEST_CASE("Generate NOR program", "[sgp]") {
  TestGenome(emp::NewPtr<InputTask>(NOR), &ProgramBuilder::AddNor);
}
TEST_CASE("Generate XOR program", "[sgp]") {
  TestGenome(emp::NewPtr<InputTask>(XOR), &ProgramBuilder::AddXor);
}
TEST_CASE("Generate EQU program", "[sgp]") {
  TestGenome(emp::NewPtr<InputTask>(EQU), &ProgramBuilder::AddEqu);
}

TEST_CASE("Empty ProgramBuilder can't do tasks", "[sgp]") {
  emp::Random random(61);
  SymConfigBase config;
  config.RANDOM_ANCESTOR(false);
  config.SYM_HORIZ_TRANS_RES(100);

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
  CPU cpu(&organism, &world, builder.Build(100));
  // cpu.PrintCode();
  cpu.state.shared_completed = emp::NewPtr<emp::vector<size_t>>();
  cpu.state.shared_completed->resize(LogicTasks.NumTasks());

  cpu.RunCPUStep(0, 100);
  world.Update();

  for (auto data : world.GetTaskSet()) {
    REQUIRE(data.n_succeeds_host == 0);
  }

  cpu.state.shared_completed.Delete();
  cpu.state.used_resources.Delete();
  cpu.state.internalEnvironment.Delete();
}