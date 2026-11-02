#include "../../../catch/catch.hpp"

#include "../../../sgp_mode/tasks/TaskDiscoveryTracker.h"

TEST_CASE("TaskDiscoveryTracker tracks discoveries as intended", "[sgp]") {
  using tracker_t = sgpmode::TaskDiscoveryTracker;
  WHEN("TaskDiscoveryTracker is initialized to track 4 tasks") {
    const size_t num_tasks = 4;
    tracker_t tracker(num_tasks);
    THEN("Discovery tracking vectors should be the correct length") {
      REQUIRE(tracker.GetHostTaskDiscoveries().size() == 4);
      REQUIRE(tracker.GetHostParentOffspringTaskDiscoveries().size() == 4);
      REQUIRE(tracker.GetSymTaskDiscoveries().size() == 4);
      REQUIRE(tracker.GetSymParentOffspringTaskDiscoveries().size() == 4);
    }
    THEN("All tasks should be initially undiscovered") {
      for (size_t task_i = 0; task_i < num_tasks; ++task_i) {
        REQUIRE(!tracker.IsHostTaskDiscovered(task_i));
        REQUIRE(!tracker.IsHostParentOffspringTaskDiscovered(task_i));
        REQUIRE(!tracker.IsSymTaskDiscovered(task_i));
        REQUIRE(!tracker.IsSymParentOffspringTaskDiscovered(task_i));
      }
    }
    WHEN("Host task is marked as discovered") {
      const size_t discovery_update = 10;
      const size_t discovery_location = 100;
      tracker.MarkHostTaskDiscovery(1, discovery_update, discovery_location);
      for (size_t task_i = 0; task_i < num_tasks; ++task_i) {
        if (task_i == 1) {
          REQUIRE(tracker.IsHostTaskDiscovered(task_i));
          const auto& discovery_info = tracker.GetHostTaskDiscoveries().at(task_i);
          REQUIRE(discovery_info.location == discovery_location);
          REQUIRE(discovery_info.update == discovery_update);
        } else {
          REQUIRE(!tracker.IsHostTaskDiscovered(task_i));
        }
        REQUIRE(!tracker.IsHostParentOffspringTaskDiscovered(task_i));
        REQUIRE(!tracker.IsSymTaskDiscovered(task_i));
        REQUIRE(!tracker.IsSymParentOffspringTaskDiscovered(task_i));
      }
    }
    WHEN("Host parent-offspring task is marked as discovered") {
      const size_t discovery_update = 10;
      const size_t discovery_location = 100;
      tracker.MarkHostParentOffspringTaskDiscovery(1, discovery_update, discovery_location);
      for (size_t task_i = 0; task_i < num_tasks; ++task_i) {
        if (task_i == 1) {
          REQUIRE(tracker.IsHostParentOffspringTaskDiscovered(task_i));
          const auto& discovery_info = tracker.GetHostParentOffspringTaskDiscoveries().at(task_i);
          REQUIRE(discovery_info.location == discovery_location);
          REQUIRE(discovery_info.update == discovery_update);
        } else {
          REQUIRE(!tracker.IsHostParentOffspringTaskDiscovered(task_i));
        }
        REQUIRE(!tracker.IsHostTaskDiscovered(task_i));
        REQUIRE(!tracker.IsSymTaskDiscovered(task_i));
        REQUIRE(!tracker.IsSymParentOffspringTaskDiscovered(task_i));
      }
    }
    WHEN("Symbiont task is marked as discovered") {
      const size_t discovery_update = 10;
      const size_t discovery_location = 100;
      tracker.MarkSymTaskDiscovery(1, discovery_update, discovery_location);
      for (size_t task_i = 0; task_i < num_tasks; ++task_i) {
        if (task_i == 1) {
          REQUIRE(tracker.IsSymTaskDiscovered(task_i));
          const auto& discovery_info = tracker.GetSymTaskDiscoveries().at(task_i);
          REQUIRE(discovery_info.location == discovery_location);
          REQUIRE(discovery_info.update == discovery_update);
        } else {
          REQUIRE(!tracker.IsSymTaskDiscovered(task_i));
        }
        REQUIRE(!tracker.IsHostParentOffspringTaskDiscovered(task_i));
        REQUIRE(!tracker.IsHostTaskDiscovered(task_i));
        REQUIRE(!tracker.IsSymParentOffspringTaskDiscovered(task_i));
      }
    }
    WHEN("Symbiont parent-offspring task is marked as discovered") {
      const size_t discovery_update = 10;
      const size_t discovery_location = 100;
      tracker.MarkSymParentOffspringTaskDiscovery(1, discovery_update, discovery_location);
      for (size_t task_i = 0; task_i < num_tasks; ++task_i) {
        if (task_i == 1) {
          REQUIRE(tracker.IsSymParentOffspringTaskDiscovered(task_i));
          const auto& discovery_info = tracker.GetSymParentOffspringTaskDiscoveries().at(task_i);
          REQUIRE(discovery_info.location == discovery_location);
          REQUIRE(discovery_info.update == discovery_update);
        } else {
          REQUIRE(!tracker.IsSymParentOffspringTaskDiscovered(task_i));
        }
        REQUIRE(!tracker.IsSymTaskDiscovered(task_i));
        REQUIRE(!tracker.IsHostParentOffspringTaskDiscovered(task_i));
        REQUIRE(!tracker.IsHostTaskDiscovered(task_i));
      }
    }
  }

}