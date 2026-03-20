#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "../Organism.h"
#include "../default_mode/SymWorld.h"

#include "sgpl/utility/ThreadLocalRandom.hpp"
#include "emp/base/vector.hpp"
#include "emp/math/Random.hpp"
#include "emp/math/random_utils.hpp"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <thread>
#include <algorithm>
#include <limits>

namespace sgpmode {

// TODO - tests!
class Scheduler {
public:
  using fun_process_org_t = std::function<void(emp::WorldPosition, Organism&)>;

protected:
  emp::Random& random;
  emp::vector<size_t> schedule_order; // Order of pop ids to evaluate.

public:
  Scheduler(
    emp::Random& rand,
    size_t world_size=1
    // size_t num_threads=1
  ) :
    random(rand)
    // thread_count(num_threads),
    // threaded_mode(num_threads > 1)
  {
    // SetupScheduler(world_size, num_threads);
    SetupScheduler(world_size);
  }

  ~Scheduler() { }

  // Allow re-configuration of scheduler post-construction.
  void SetupScheduler(size_t world_size) {
    emp_assert(world_size > 0, "World size must be > 0");

    // Resize schedule order to world size
    schedule_order.resize(world_size, 0);
    // Fill schedule order with valid ids
    std::iota(
      schedule_order.begin(),
      schedule_order.end(),
      0
    );

  }

  size_t GetScheduleSize() const { return schedule_order.size(); }
  const emp::vector<size_t>& GetCurSchedule() const { return schedule_order; }

  // Update schedule order (uniform random)
  void UpdateSchedule() {
    emp::Shuffle(random, schedule_order);
  }

  // Process all orgs in world population in current schedule order (single-threaded).
  template<typename WORLD_T>
  void Run(WORLD_T& world) {
    for (size_t world_id : schedule_order) {
      emp_assert(world_id < world.GetSize());
      world.ProcessOrgsAt(world_id);
    }
  }

};

}

#endif