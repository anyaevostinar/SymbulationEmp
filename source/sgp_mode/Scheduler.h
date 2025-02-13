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

namespace sgpmode {

// TODO - tests!
class Scheduler {
public:
  using fun_process_org_t = std::function<void(emp::WorldPosition, Organism&)>;

protected:
emp::Random& random;
  emp::vector<size_t> schedule_order; // Order of pop ids to evaluate.
  emp::vector<emp::vector<size_t>> thread_batches; // Contains schedule indexes handled for each thread.
  size_t thread_count; // How many threads?
  bool threaded_mode = false;
  emp::vector<std::thread> running_threads;
  bool thread_pool_started = false;

  // Helper function to get id to schedule w/thread batch info
  size_t GetID(size_t batch_id, size_t idx) {
    emp_assert(batch_id < thread_batches.size());
    emp_assert(idx < thread_batches[batch_id].size());
    return schedule_order[thread_batches[batch_id][idx]];
  }

  template<typename WORLD_T>
  void RunThread(WORLD_T& world, size_t thread_id) {
    // TODO
  }

public:
  Scheduler(
    emp::Random& rand,
    size_t world_size=1,
    size_t num_threads=1
  ) :
    random(rand),
    thread_count(num_threads),
    threaded_mode(num_threads > 1)
  {
    SetupScheduler(world_size, num_threads);
  }

  // Allow re-configuration of scheduler post-construction.
  void SetupScheduler(size_t world_size, size_t num_threads) {
    emp_assert(num_threads > 0, "Thread count cannot be 0");
    emp_assert(world_size > 0, "World size must be > 0");
    schedule_order.clear();
    thread_batches.clear();
    running_threads.clear(); // TODO - do I need to wait?
    thread_pool_started = false;
    // If num threads is bigger than world size, set thread count to world size
    thread_count = (num_threads > world_size) ? world_size : num_threads;
    threaded_mode = num_threads > 1;

    // Resize schedule order to world size
    schedule_order.resize(world_size, 0);
    // Fill schedule order with valid ids
    std::iota(
      schedule_order.begin(),
      schedule_order.end(),
      0
    );
    // TODO - test that thread batches are as expected

    if (threaded_mode) {
      // If multiple threads, configure thread batches.
      // The following code will work regardless of number of threads
      const size_t base_batch_size = (size_t)(world_size / thread_count);

      // world size might not evenly divide into given batch sizes,
      // so spread the "leftovers" evenly over batches.
      size_t leftover_cnt = world_size - (base_batch_size * thread_count);
      thread_batches.resize(thread_count);
      size_t schedule_i = 0;
      for (size_t batch_i = 0; batch_i < thread_batches.size(); ++batch_i) {
        size_t batch_size = base_batch_size;
        if (leftover_cnt > 0) {
          ++batch_size;
          --leftover_cnt;
        }
        thread_batches[batch_i].resize(batch_size, 0);
        std::iota(
          thread_batches[batch_i].begin(),
          thread_batches[batch_i].end(),
          schedule_i
        );
        schedule_i += batch_size;
      }
      emp_assert(schedule_i == world_size);

    }

  }

  size_t GetScheduleSize() const { return schedule_order.size(); }
  const emp::vector<size_t>& GetCurSchedule() const { return schedule_order; }

  // Update schedule order (uniform random)
  void UpdateSchedule() {
    emp::Shuffle(random, schedule_order);
  }

  // TODO - process
  // Process all orgs in world population in current schedule order (single-threaded).
  template<typename WORLD_T>
  void RunSync(WORLD_T& world) {
    for (size_t world_id : schedule_order) {
      emp_assert(world_id < world.GetSize());
      world.ProcessOrgAt(world_id);  // TODO - implement process org at
    }
  }

  template<typename WORLD_T>
  void Run(WORLD_T& world) {
    // thread batches should work for sync or threaded mode
    if (!threaded_mode) {
      RunSync(world);
      return;
    }

    // TODO - better way to init threads?
    if (!thread_pool_started) {
      // Init thread pool
      for (size_t thread_i = 0; thread_i < thread_count; ++thread_i) {
        running_threads.emplace_back(
          std::thread(&Scheduler::RunThread<WORLD_T>, this, world, thread_i)
        );
      }
    }

    // TODO
    // -- bookmark --
  }

};

}

#endif