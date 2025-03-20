#include "../../sgp_mode/GenomeLibrary.h"
#include "../../sgp_mode/CPU.h"

#include "../../catch/catch.hpp"

void TestGenome(emp::Ptr<sgpmode::Task> task, void (sgpmode::ProgramBuilder::*method)()) {
  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.RANDOM_ANCESTOR(false);
  config.SYM_HORIZ_TRANS_RES(100);

  // Ensure we can actually complete the task
  task->MarkAlwaysPerformable();

  sgpmode::SGPWorld world(random, &config, sgpmode::TaskSet{task});

  // Mock Organism to check reproduction
  class TestOrg : public Organism {
  public:
    bool IsHost() override { return true; }
    void AddPoints(double p) override {}
    double GetPoints() override { return 0; }
  };

  TestOrg organism;
  sgpmode::ProgramBuilder builder;
  // Call the provided method reference
  (builder.*method)();
  sgpmode::CPU cpu(&organism, &world, builder.Build(100));

  cpu.RunCPUStep(0, 100);
  world.Update();

  REQUIRE((*world.GetTaskSet().begin()).n_succeeds_host > 0);

  cpu.state.shared_available_dependencies.Delete();
  cpu.state.used_resources.Delete();
  cpu.state.internal_environment.Delete();
}

TEST_CASE("Generate NOT program", "[sgp]") {
  TestGenome(emp::NewPtr<sgpmode::InputTask>(sgpmode::NOT), &sgpmode::ProgramBuilder::AddNot);
}

TEST_CASE("Generate NAND program", "[sgp]") {
  TestGenome(emp::NewPtr<sgpmode::InputTask>(sgpmode::NAND), &sgpmode::ProgramBuilder::AddNand);
}
TEST_CASE("Generate AND program", "[sgp]") {
  TestGenome(emp::NewPtr<sgpmode::InputTask>(sgpmode::AND), &sgpmode::ProgramBuilder::AddAnd);
}
TEST_CASE("Generate ORN program", "[sgp]") {
  TestGenome(emp::NewPtr<sgpmode::InputTask>(sgpmode::ORN), &sgpmode::ProgramBuilder::AddOrn);
}
TEST_CASE("Generate OR program", "[sgp]") {
  TestGenome(emp::NewPtr<sgpmode::InputTask>(sgpmode::OR), &sgpmode::ProgramBuilder::AddOr);
}
TEST_CASE("Generate ANDN program", "[sgp]") {
  TestGenome(emp::NewPtr<sgpmode::InputTask>(sgpmode::ANDN), &sgpmode::ProgramBuilder::AddAndn);
}
TEST_CASE("Generate NOR program", "[sgp]") {
  TestGenome(emp::NewPtr<sgpmode::InputTask>(sgpmode::NOR), &sgpmode::ProgramBuilder::AddNor);
}
TEST_CASE("Generate XOR program", "[sgp]") {
  TestGenome(emp::NewPtr<sgpmode::InputTask>(sgpmode::XOR), &sgpmode::ProgramBuilder::AddXor);
}
TEST_CASE("Generate EQU program", "[sgp]") {
  TestGenome(emp::NewPtr<sgpmode::InputTask>(sgpmode::EQU), &sgpmode::ProgramBuilder::AddEqu);
}

TEST_CASE("Empty ProgramBuilder can't do tasks", "[sgp]") {
  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.RANDOM_ANCESTOR(false);
  config.SYM_HORIZ_TRANS_RES(100);

  sgpmode::SGPWorld world(random, &config, sgpmode::LogicTasks);

  // Mock Organism to check reproduction
  class TestOrg : public Organism {
  public:
    bool IsHost() override { return true; }
    void AddPoints(double p) override {}
    double GetPoints() override { return 0; }
  };

  TestOrg organism;
  // Empty builder
  sgpmode::ProgramBuilder builder;
  sgpmode::CPU cpu(&organism, &world, builder.Build(100));

  cpu.RunCPUStep(0, 100);
  world.Update();

  for (auto data : world.GetTaskSet()) {
    REQUIRE(data.n_succeeds_host == 0);
  }

  cpu.state.shared_available_dependencies.Delete();
  cpu.state.used_resources.Delete();
  cpu.state.internal_environment.Delete();
}