#include "../../sgp_mode/GenomeLibrary.h"
#include "../../sgp_mode/CPU.h"

#include "../../catch/catch.hpp"

void TestGenome(Task task, void (ProgramBuilder::*method)()) {
  emp::Random random(61);
  SymConfigBase config;
  config.RANDOM_ANCESTOR(false);
  config.SYM_HORIZ_TRANS_RES(100);

  // Ensure we can actually complete the task
  task.dependencies.clear();
  task.unlimited = true;

  TaskSet task_set{task};
  SGPWorld world(random, &config, task_set);

  // Mock Organism to check reproduction
  class TestOrg : public Organism {
  public:
    bool IsHost() override { return true; }
    void AddPoints(double p) override {}
    double GetPoints() override { return 0; }
    emp::Ptr<Organism> Reproduce() override { return emp::NewPtr<TestOrg>(); }
  };

  TestOrg organism;
  ProgramBuilder builder;
  // Call the provided method reference
  (builder.*method)();
  CPU cpu(&organism, &world, &random, builder.build(100));
  // cpu.PrintCode();
  cpu.state.shared_completed = emp::NewPtr<emp::vector<size_t>>();
  cpu.state.shared_completed->resize(LogicTasks.NumTasks());

  cpu.RunCPUStep(0, 100);
  world.Update();

  REQUIRE((*task_set.begin()).n_succeeds_host > 0);

  cpu.state.shared_completed.Delete();
  cpu.state.used_resources.Delete();
  cpu.state.internalEnvironment.Delete();
}

TEST_CASE("Generate NOT program", "[sgp]") {
  TestGenome(NOT, &ProgramBuilder::addNot);
}

TEST_CASE("Generate SQUARE program", "[sgp]") {
  TestGenome(SQU, &ProgramBuilder::addSquare);
}

TEST_CASE("Generate NAND program", "[sgp]") {
  TestGenome(NAND, &ProgramBuilder::addNand);
}
TEST_CASE("Generate AND program", "[sgp]") {
  TestGenome(AND, &ProgramBuilder::addAnd);
}
TEST_CASE("Generate ORN program", "[sgp]") {
  TestGenome(ORN, &ProgramBuilder::addOrn);
}
TEST_CASE("Generate OR program", "[sgp]") {
  TestGenome(OR, &ProgramBuilder::addOr);
}
TEST_CASE("Generate ANDN program", "[sgp]") {
  TestGenome(ANDN, &ProgramBuilder::addAndn);
}
TEST_CASE("Generate NOR program", "[sgp]") {
  TestGenome(NOR, &ProgramBuilder::addNor);
}
TEST_CASE("Generate XOR program", "[sgp]") {
  TestGenome(XOR, &ProgramBuilder::addXor);
}
TEST_CASE("Generate EQU program", "[sgp]") {
  TestGenome(EQU, &ProgramBuilder::addEqu);
}
