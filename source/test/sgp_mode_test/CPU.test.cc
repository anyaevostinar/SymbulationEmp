#include "../../sgp_mode/CPU.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPDataNodes.h"

TEST_CASE("Ancestor CPU can reproduce", "[sgp]") {
  emp::Random random(61);
  SymConfigBase config;
  config.RANDOM_ANCESTOR(false);

  // Make the NOT task guaranteed to provide enough resources for reproduction after one completion
  TaskSet task_set{
      {"NOT", InputTask{1, [](auto &a) { return ~a[0]; }, 256.0}, true}};
  SGPWorld world(random, &config, task_set);

  // Mock Organism to check reproduction
  class TestOrg : public Organism {
  public:
    int reproduce_count = 0;
    double points = 0.0;
    bool IsHost() override { return false; }
    void AddPoints(double p) override { points += p; }
    double GetPoints() override { return points; }
    emp::Ptr<Organism> Reproduce() override {
      reproduce_count++;
      return emp::NewPtr<TestOrg>();
    }
  };

  TestOrg organism;
  CPU cpu(&organism, &world, &random);
  cpu.state.shared_completed = emp::NewPtr<emp::vector<size_t>>();
  cpu.state.shared_completed->resize(LogicTasks.NumTasks());

  // It should reproduce at the end of its program, which has length 100
  cpu.RunCPUStep(0, 100);
  world.Update();

  REQUIRE(organism.reproduce_count == 1);

  cpu.state.shared_completed.Delete();
  cpu.state.used_resources.Delete();
  cpu.state.internalEnvironment.Delete();
}