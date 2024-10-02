#include "../../source/sgp_mode/Tasks.cc"
#include "../../sgp_mode/CPU.h"
#include "../../sgp_mode/SGPDataNodes.h"
#include "../../sgp_mode/SGPWorld.h"

TEST_CASE("Ancestor CPU can reproduce", "[sgp]") {
  // Mock Organism to check reproduction
  class TestOrg : public Organism {
  public:
    int reproduce_count = 0;
    double points = 0.0;
    emp::vector<emp::Ptr<Organism>> empty_vector = emp::vector<emp::Ptr<Organism>>();
    bool IsHost() override { return true; }
    void AddPoints(double p) override { points += p; }
    double GetPoints() override { return points; }
    bool GetDead() override { return false; }
    emp::vector<emp::Ptr<Organism>>& GetSymbionts() override { return empty_vector; }
    emp::Ptr<Organism> Reproduce() override {
      reproduce_count++;
      return emp::NewPtr<TestOrg>();
    }
  };

  emp::Random random(61);
  SymConfigSGP config;
  config.RANDOM_ANCESTOR(false);
  config.HOST_REPRO_RES(1);
  WHEN("logic tasks are used") {
    // Make the ancestor genome do NOT
    config.TASK_TYPE(1);

    TaskSet task_set{ emp::NewPtr<InputTask>(NOT) };
    SGPWorld world(random, &config, task_set);

    TestOrg organism;
    CPU cpu(&organism, &world);

    // It should reproduce at the end of its program, which has length 100
    cpu.RunCPUStep(0, 100);
    world.Update();

    REQUIRE(organism.reproduce_count == 1);

    cpu.state.shared_available_dependencies.Delete();
    cpu.state.used_resources.Delete();
    cpu.state.internal_environment.Delete();
    cpu.state.tasks_performed.Delete();
  }
}