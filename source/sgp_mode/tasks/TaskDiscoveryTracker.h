#pragma once
#ifndef TASK_DISCOVERY_TRACKER_H
#define TASK_DISCOVERY_TRACKER_H

#include "emp/bits/Bits.hpp"
#include "emp/base/vector.hpp"
#include "emp/data/DataFile.hpp"
#include "emp/base/assert.hpp"

namespace sgpmode {

/**
 * Purpose: Consolidate information about individual task discoveries.
 */
struct DiscoveryInfo {
  /**
   * Purpose: Whether or not the task has been discovered.
   */
  bool discovered;

  /**
   * Purpose: Update of task discovery.
   */
  size_t update;

  /**
   * Purpose: Location in population/world of task discovery.
   */
  size_t location;

  /**
   * Purpose: Default constructor.
   */
  DiscoveryInfo() = default;

  /**
   * Purpose: Construct DiscoveryInfo object with discovery information.
   *
   * Input:
   *  - in_discovered: Boolean indicating whether task is discovered
   *  - in_update: update of task discovery
   *  - in_location: location of task discovery
   */
  DiscoveryInfo(
    bool in_discovered,
    size_t in_update,
    size_t in_location
  ) :
    discovered(in_discovered),
    update(in_update),
    location(in_location)
  { }

  /**
   * Purpose: Mark this task as discovered.
   *
   * Input:
   *  - disc_update: update of task discovery
   *  - disc_location: location of task discovery
   * Output: None.
   */
  void MarkDiscovered(size_t disc_update, size_t disc_location) {
    discovered = true;
    update = disc_update;
    location = disc_location;
  }

};

class TaskDiscoveryTracker {
public:
protected:
  /**
   * Purpose: Tracks tasks "discovered" by individual hosts.
   *          To discover a task, a host must get credit for performing that
   *          task.
   */
  emp::vector<DiscoveryInfo> host_task_discoveries;

  /**
   * Purpose: Tracks tasks "discovered" by a host parent-offspring pair.
   *          I.e., a host and its parent must both perform the task for it
   *          to be added here.
   */
  emp::vector<DiscoveryInfo> host_parent_offspring_task_discoveries;

  /**
   * Purpose: Tracks tasks "discovered" by symbionts.
   *          To discover a task, a symbiont get credit for performing that
   *          task.
   */
  emp::vector<DiscoveryInfo> sym_task_discoveries;

  /**
   * Purpose: Tracks tasks "discovered" by a symbiont parent-offspring pair.
   *          I.e., a symbiont and its parent must both perform the task for it
   *          to be added here.
   */
  emp::vector<DiscoveryInfo> sym_parent_offspring_task_discoveries;

  emp::Ptr<emp::DataFile> task_discovery_data_file = nullptr;
  bool data_file_setup = false;
  size_t out_update = 0;
  size_t out_task_id = 0;
  std::string out_task_name = "";

  void MarkDiscovered(
    emp::vector<DiscoveryInfo>& discoveries,
    size_t task_id,
    size_t update,
    size_t location
  ) {
    emp_assert(task_id < discoveries.size());
    discoveries[task_id].MarkDiscovered(update, location);
  }

public:
  TaskDiscoveryTracker() = default;

  ~TaskDiscoveryTracker() {
    if (data_file_setup) {
      task_discovery_data_file.Delete();
    }
  }

  void Reset(size_t num_tasks) {
    host_task_discoveries.clear();
    host_parent_offspring_task_discoveries.clear();
    sym_task_discoveries.clear();
    sym_parent_offspring_task_discoveries.clear();

    host_task_discoveries.resize(num_tasks, {false, 0, 0});
    host_parent_offspring_task_discoveries.resize(num_tasks, {false, 0, 0});
    sym_task_discoveries.resize(num_tasks, {false, 0, 0});
    sym_parent_offspring_task_discoveries.resize(num_tasks, {false, 0, 0});
  }

  // todo move below class def to improve readability
  void SetupOutputFile(const std::string& filepath);

  template<typename WORLD_T>
  void UpdateOutputFile(WORLD_T& world) {
    emp_assert(data_file_setup);
    out_update = world.GetUpdate();
    // Write one line per task
    auto& task_set = world.GetTaskEnv().GetTaskSet();
    const size_t num_tasks = host_task_discoveries.size();
    for (size_t task_i = 0; task_i < num_tasks; ++task_i) {
      emp_assert(task_i < task_set.GetSize());
      out_task_id = task_i;
      out_task_name = task_set.GetName(task_i);
      task_discovery_data_file->Update();
    }
  }

  bool IsHostTaskDiscovered(size_t task_id) const {
    emp_assert(task_id < host_task_discoveries.size());
    return host_task_discoveries[task_id].discovered;
  }

  bool IsHostParentOffspringTaskDiscovered(size_t task_id) const {
    emp_assert(task_id < host_parent_offspring_task_discoveries.size());
    return host_parent_offspring_task_discoveries[task_id].discovered;
  }

  bool IsSymTaskDiscovered(size_t task_id) const {
    emp_assert(task_id < sym_task_discoveries.size());
    return sym_task_discoveries[task_id].discovered;
  }

  bool IsSymParentOffspringTaskDiscovered(size_t task_id) const {
    emp_assert(task_id < sym_parent_offspring_task_discoveries.size());
    return sym_parent_offspring_task_discoveries[task_id].discovered;
  }

  void MarkHostTaskDiscovery(
    size_t task_id,
    size_t discovery_update,
    size_t discovery_location
  ) {
    MarkDiscovered(
      host_task_discoveries,
      task_id,
      discovery_update,
      discovery_location
    );
  }

  void MarkHostParentOffspringTaskDiscovery(
    size_t task_id,
    size_t discovery_update,
    size_t discovery_location
  ) {
    MarkDiscovered(
      host_parent_offspring_task_discoveries,
      task_id,
      discovery_update,
      discovery_location
    );
  }

  void MarkSymTaskDiscovery(
    size_t task_id,
    size_t discovery_update,
    size_t discovery_location
  ) {
    MarkDiscovered(
      sym_task_discoveries,
      task_id,
      discovery_update,
      discovery_location
    );
  }

  void MarkSymParentOffspringTaskDiscovery(
    size_t task_id,
    size_t discovery_update,
    size_t discovery_location
  ) {
    MarkDiscovered(
      sym_parent_offspring_task_discoveries,
      task_id,
      discovery_update,
      discovery_location
    );
  }

};


void TaskDiscoveryTracker::SetupOutputFile(const std::string& filepath) {
  if (data_file_setup) {
    task_discovery_data_file.Delete();
  }
  task_discovery_data_file = emp::NewPtr<emp::DataFile>(filepath);

  // Configure file columns:
  task_discovery_data_file->AddFun<size_t>(
    [this]() -> size_t { return out_update; },
    "output_update"
  );
  task_discovery_data_file->AddFun<size_t>(
    [this]() -> size_t { return out_task_id; },
    "task_id"
  );
  task_discovery_data_file->AddFun<std::string>(
    [this]() -> std::string { return out_task_name; },
    "task_name"
  );

  // -- host task discovery columns --
  task_discovery_data_file->AddFun<bool>(
    [this]() -> bool {
      return host_task_discoveries[out_task_id].discovered;
    },
    "host_discovered"
  );
  task_discovery_data_file->AddFun<size_t>(
    [this]() -> size_t {
      return host_task_discoveries[out_task_id].update;
    },
    "host_discovered_update"
  );
  task_discovery_data_file->AddFun<size_t>(
    [this]() -> size_t {
      return host_task_discoveries[out_task_id].location;
    },
    "host_discovered_location"
  );

  // -- host parent-offspring discovery columns --
  task_discovery_data_file->AddFun<bool>(
    [this]() -> bool {
      return host_parent_offspring_task_discoveries[out_task_id].discovered;
    },
    "host_parent_offspring_discovered"
  );
  task_discovery_data_file->AddFun<size_t>(
    [this]() -> size_t {
      return host_parent_offspring_task_discoveries[out_task_id].update;
    },
    "host_parent_offspring_discovered_update"
  );
  task_discovery_data_file->AddFun<size_t>(
    [this]() -> size_t {
      return host_parent_offspring_task_discoveries[out_task_id].location;
    },
    "host_parent_offspring_discovered_location"
  );

  // -- symbiont task discovery columns --
  task_discovery_data_file->AddFun<bool>(
    [this]() -> bool {
      return sym_task_discoveries[out_task_id].discovered;
    },
    "sym_discovered"
  );
  task_discovery_data_file->AddFun<size_t>(
    [this]() -> size_t {
      return sym_task_discoveries[out_task_id].update;
    },
    "sym_discovered_update"
  );
  task_discovery_data_file->AddFun<size_t>(
    [this]() -> size_t {
      return sym_task_discoveries[out_task_id].location;
    },
    "sym_discovered_location"
  );

  // -- symbiont parent-offspring task discovery columns --
  task_discovery_data_file->AddFun<bool>(
    [this]() -> bool {
      return sym_parent_offspring_task_discoveries[out_task_id].discovered;
    },
    "sym_parent_offspring_discovered"
  );
  task_discovery_data_file->AddFun<size_t>(
    [this]() -> size_t {
      return sym_parent_offspring_task_discoveries[out_task_id].update;
    },
    "sym_parent_offspring_discovered_update"
  );
  task_discovery_data_file->AddFun<size_t>(
    [this]() -> size_t {
      return sym_parent_offspring_task_discoveries[out_task_id].location;
    },
    "sym_parent_offspring_discovered_location"
  );

  task_discovery_data_file->PrintHeaderKeys();
  data_file_setup = true;
}

}

#endif