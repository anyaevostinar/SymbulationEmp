#include "../../sgp_mode/GenomeLibrary.h"
#include "../../sgp_mode/CPU.h"

#include "../../catch/catch.hpp"

void TestGenome(Task task, sgpl::Program<Spec> program) {
  emp::Random random(61);
  SymConfigBase config;
  config.RANDOM_ANCESTOR(false);
  config.SYM_HORIZ_TRANS_RES(100);

  // Make the NOT task guaranteed to provide enough resources for reproduction
  // after one completion
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
  TestGenome({"NOT", InputTask{1, [](auto &a) { return ~a[0]; }, 256.0}, true},
             CreateNotProgram(100));
}

TEST_CASE("Generate SQUARE program", "[sgp]") {
  TestGenome({"SQU", OutputTask{[](uint32_t x) {
                return sqrt(x) - floor(sqrt(x)) == 0 ? 40.0 : 0.0;
              }}},
             CreateSquareProgram(100));
}