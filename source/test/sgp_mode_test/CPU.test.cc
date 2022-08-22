#include "../../sgp_mode/CPU.h"
#include "../../sgp_mode/SGPDataNodes.h"
#include "../../sgp_mode/SGPWorld.h"

TEST_CASE("Ancestor CPU can reproduce", "[sgp]") {
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

  emp::Random random(61);
  SymConfigBase config;
  config.RANDOM_ANCESTOR(false);
  config.SYM_HORIZ_TRANS_RES(1);
  WHEN("logic tasks are used") {
    // Make the ancestor genome do NOT instead of SQUARE
    config.TASK_TYPE(1);

    TaskSet task_set{emp::NewPtr<InputTask>(NOT)};
    SGPWorld world(random, &config, task_set);

    TestOrg organism;
    CPU cpu(&organism, &world);
    cpu.state.shared_completed = emp::NewPtr<emp::vector<size_t>>();
    cpu.state.shared_completed->resize(1);

    // It should reproduce at the end of its program, which has length 100
    cpu.RunCPUStep(0, 100);
    world.Update();

    REQUIRE(organism.reproduce_count == 1);

    cpu.state.shared_completed.Delete();
    cpu.state.used_resources.Delete();
    cpu.state.internalEnvironment.Delete();
  }
  WHEN("square task is used") {
    config.TASK_TYPE(0);

    TaskSet task_set{emp::NewPtr<SquareTask>(SQU)};
    SGPWorld world(random, &config, task_set);

    TestOrg organism;
    CPU cpu(&organism, &world);
    cpu.state.shared_completed = emp::NewPtr<emp::vector<size_t>>();
    cpu.state.shared_completed->resize(1);

    // It should reproduce at the end of its program, which has length 100
    cpu.RunCPUStep(0, 100);
    world.Update();

    REQUIRE(organism.reproduce_count == 1);

    cpu.state.shared_completed.Delete();
    cpu.state.used_resources.Delete();
    cpu.state.internalEnvironment.Delete();
  }
}