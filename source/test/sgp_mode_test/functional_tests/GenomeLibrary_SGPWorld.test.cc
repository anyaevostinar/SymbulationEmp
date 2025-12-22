#include "../../../sgp_mode/GenomeLibrary.h"
#include "../../../sgp_mode/CPU.h"

#include "../../../catch/catch.hpp"

void TestGenome(emp::Ptr<Task> task, void (ProgramBuilder::*method)()) {
  GIVEN("An SGPWorld and an organism"){
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
    WHEN("The host has the specified program"){
      ProgramBuilder builder;
      // Call the provided method reference
      (builder.*method)();
      CPU cpu(&organism, &world, builder.Build(100));

      cpu.RunCPUStep(0, 100);
      world.Update();

      THEN("Host is able to complete the specified task"){
        REQUIRE((*world.GetTaskSet().begin()).n_succeeds_host > 0);
      }
    }
  }
}

TEST_CASE("Generate NOT program", "[sgp][sgp-functional]") {
  TestGenome(emp::NewPtr<Task>(NOT), &ProgramBuilder::AddNot);
}
TEST_CASE("Generate NAND program", "[sgp][sgp-functional]") {
  TestGenome(emp::NewPtr<Task>(NAND), &ProgramBuilder::AddNand);
}
TEST_CASE("Generate AND program", "[sgp][sgp-functional]") {
  TestGenome(emp::NewPtr<Task>(AND), &ProgramBuilder::AddAnd);
}
TEST_CASE("Generate ORN program", "[sgp][sgp-functional]") {
  TestGenome(emp::NewPtr<Task>(ORN), &ProgramBuilder::AddOrn);
}
TEST_CASE("Generate OR program", "[sgp][sgp-functional]") {
  TestGenome(emp::NewPtr<Task>(OR), &ProgramBuilder::AddOr);
}
TEST_CASE("Generate ANDN program", "[sgp][sgp-functional]") {
  TestGenome(emp::NewPtr<Task>(ANDN), &ProgramBuilder::AddAndn);
}
TEST_CASE("Generate NOR program", "[sgp][sgp-functional]") {
  TestGenome(emp::NewPtr<Task>(NOR), &ProgramBuilder::AddNor);
}
TEST_CASE("Generate XOR program", "[sgp][sgp-functional]") {
  TestGenome(emp::NewPtr<Task>(XOR), &ProgramBuilder::AddXor);
}
TEST_CASE("Generate EQU program", "[sgp][sgp-functional]") {
  TestGenome(emp::NewPtr<Task>(EQU), &ProgramBuilder::AddEqu);
}

TEST_CASE("Empty ProgramBuilder can't do tasks", "[sgp][sgp-functional]") {
  GIVEN("A World and an Organism"){
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
    WHEN("The Organism has an empty program"){
      ProgramBuilder builder;
      CPU cpu(&organism, &world, builder.Build(100));

      cpu.RunCPUStep(0, 100);
      world.Update();
      THEN("The Organism cannot complete any tasks"){
        for (auto data : world.GetTaskSet()) {
          REQUIRE(data.n_succeeds_host == 0);
        }
      }
    }
}
}