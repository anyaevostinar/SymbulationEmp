#include "../../source/sgp_mode/SGPHost.h"
#include "../../source/sgp_mode/SGPWorld.h"
#include "../../source/sgp_mode/Tasks.cc"
#include <map>

TEST_CASE("SquareTask::MarkPerformed adds a new key-value pair if empty",
          "[sgp]") {
  emp::Random random;
  SymConfigBase config;
  SquareTask task{"PLACEHOLDER", [](uint32_t x) { return 5; }};
  SGPWorld world(random, &config, TaskSet{&task});
  uint32_t test_output = 4;
  Host org;
  CPUState state(&org, &world);
  state.available_dependencies.resize(1);
  task.MarkPerformed(state, test_output, 0, false);

  REQUIRE(world.data_node_host_squares[test_output] == 1);
}

TEST_CASE("SquareTask::MarkPerformed adds new key-value pair to non-empty map",
          "[sgp]") {
  emp::Random random;
  SymConfigBase config;
  SquareTask task{"PLACEHOLDER", [](uint32_t x) { return 5; }};
  SGPWorld world(random, &config, TaskSet{&task});
  world.data_node_host_squares.insert({5, 3});
  uint32_t test_output = 4;
  Host org;
  CPUState state(&org, &world);
  state.available_dependencies.resize(1);
  task.MarkPerformed(state, test_output, 0, false);

  REQUIRE(world.data_node_host_squares[test_output] == 1);
}

TEST_CASE("SquareTask::MarkPerformed increments frequency if key is present",
          "[sgp]") {
  emp::Random random;
  SymConfigBase config;
  SquareTask task{"PLACEHOLDER", [](uint32_t x) { return 5; }};
  SGPWorld world(random, &config, TaskSet{&task});
  uint32_t test_output = 4;
  world.data_node_host_squares.insert({test_output, 1});
  Host org;
  CPUState state(&org, &world);
  state.available_dependencies.resize(1);
  task.MarkPerformed(state, test_output, 0, false);

  REQUIRE(world.data_node_host_squares[test_output] == 2);
}
