#include "../../sgp_mode/GenomeLibrary.h"
#include "../../sgp_mode/CPU.h"

#include "../../catch/catch.hpp"

void TestGenome(Task task, sgpl::Program<Spec> program) {
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
  CPU cpu(&organism, &world, &random, program);
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
  TestGenome(NOT, CreateNotProgram(100));
}

TEST_CASE("Generate SQUARE program", "[sgp]") {
  TestGenome(SQU, CreateSquareProgram(100));
}

TEST_CASE("Generate NAND program", "[sgp]") {
  TestGenome(NAND, CreateNandProgram(100));
}
TEST_CASE("Generate AND program", "[sgp]") {
  TestGenome(AND, CreateAndProgram(100));
}
TEST_CASE("Generate ORN program", "[sgp]") {
  TestGenome(ORN, CreateOrnProgram(100));
}
TEST_CASE("Generate OR program", "[sgp]") {
  TestGenome(OR, CreateOrProgram(100));
}
TEST_CASE("Generate ANDN program", "[sgp]") {
  TestGenome(ANDN, CreateAndnProgram(100));
}
TEST_CASE("Generate NOR program", "[sgp]") {
  TestGenome(NOR, CreateNorProgram(100));
}
TEST_CASE("Generate XOR program", "[sgp]") {
  TestGenome(XOR, CreateXorProgram(100));
}
TEST_CASE("Generate EQU program", "[sgp]") {
  TestGenome(EQU, CreateEquProgram(100));
}
