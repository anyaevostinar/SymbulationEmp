#include "../../sgp_mode/GenomeLibrary.h"
#include "../../sgp_mode/CPU.h"

#include "../../catch/catch.hpp"

void TestGenome(emp::Ptr<Task> task, void (ProgramBuilder::*method)()) {
  emp::Random random(61);
  SymConfigSGP config;
  config.SYM_HORIZ_TRANS_RES(100);

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

  cpu.RunCPUStep(0, 100);
  world.Update();

  REQUIRE((*world.GetTaskSet().begin()).n_succeeds_host > 0);
}

TEST_CASE("Generate NOT program", "[sgp]") {
  TestGenome(emp::NewPtr<Task>(NOT), &ProgramBuilder::AddNot);
}
TEST_CASE("Generate NAND program", "[sgp]") {
  TestGenome(emp::NewPtr<Task>(NAND), &ProgramBuilder::AddNand);
}
TEST_CASE("Generate AND program", "[sgp]") {
  TestGenome(emp::NewPtr<Task>(AND), &ProgramBuilder::AddAnd);
}
TEST_CASE("Generate ORN program", "[sgp]") {
  TestGenome(emp::NewPtr<Task>(ORN), &ProgramBuilder::AddOrn);
}
TEST_CASE("Generate OR program", "[sgp]") {
  TestGenome(emp::NewPtr<Task>(OR), &ProgramBuilder::AddOr);
}
TEST_CASE("Generate ANDN program", "[sgp]") {
  TestGenome(emp::NewPtr<Task>(ANDN), &ProgramBuilder::AddAndn);
}
TEST_CASE("Generate NOR program", "[sgp]") {
  TestGenome(emp::NewPtr<Task>(NOR), &ProgramBuilder::AddNor);
}
TEST_CASE("Generate XOR program", "[sgp]") {
  TestGenome(emp::NewPtr<Task>(XOR), &ProgramBuilder::AddXor);
}
TEST_CASE("Generate EQU program", "[sgp]") {
  TestGenome(emp::NewPtr<Task>(EQU), &ProgramBuilder::AddEqu);
}

TEST_CASE("Empty ProgramBuilder can't do tasks", "[sgp]") {
  emp::Random random(61);
  SymConfigSGP config;
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

  cpu.RunCPUStep(0, 100);
  world.Update();

  for (auto data : world.GetTaskSet()) {
    REQUIRE(data.n_succeeds_host == 0);
  }
}

TEST_CASE("BuildNoRepro creates obligate mutualist program", "[dono]") {
  ProgramBuilder builder;
  size_t program_len = 100;

  sgpl::Program<Spec> program = builder.BuildNoRepro(program_len);

  REQUIRE(program.size() == program_len);

  for (size_t i = program.size() - 5; i < program.size(); ++i) {
    REQUIRE(program[i].op_code == Library::GetOpCode("Donate"));
  }

  for (auto &inst : program) {
    REQUIRE(inst.op_code != Library::GetOpCode("Reproduce"));
  }

  REQUIRE(program[0].op_code == Library::GetOpCode("Global Anchor"));
  REQUIRE(program[0].tag == START_TAG);

}