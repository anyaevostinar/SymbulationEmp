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

  // // Threading-related member variables
  // emp::vector<emp::vector<size_t>> thread_batches; // Contains schedule indexes handled for each thread.
  // size_t thread_count; // How many threads?
  // bool threaded_mode = false;
  // emp::vector<std::thread> running_threads;
  // bool thread_pool_started = false;
  // emp::vector<size_t> thread_seeds; // To ensure replicates are independent, need to generate seeds using root rng

  // std::mutex ready_lock;
  // std::condition_variable ready_cv;
  // std::atomic<size_t> cur_update = 0;

  // std::mutex threads_done_lock;
  // std::condition_variable threads_done_cv;
  // size_t num_threads_done = 0;
  // std::atomic_bool finished = false;

  // // Helper function to get id to schedule w/thread batch info
  // size_t GetID(size_t batch_id, size_t idx) {
  //   emp_assert(batch_id < thread_batches.size());
  //   emp_assert(idx < thread_batches[batch_id].size());
  //   return schedule_order[thread_batches[batch_id][idx]];
  // }

  // TODO - re-evaluate use of threads; so much possibility for hard-to-detect bad behavior
  // Can't pass in world as a temporary reference
  // // TODO - benchmark this approach to scheduling on threads vs. dynamic batching
  // template<typename WORLD_T>
  // void RunThread(emp::Ptr<WORLD_T> world_ptr, size_t thread_id) {
  //   // Make sure each thread gets a different, deterministic, seed
  //   const size_t thread_seed = thread_seeds[thread_id];
  //   const auto& schedule_ids = thread_batches[thread_id];
  //   sgpl::tlrand.Get().ResetSeed(thread_seed);
  //   size_t last_update = std::numeric_limits<size_t>::max();
  //   while (true) {
  //     if (!finished && last_update == cur_update) {
  //       std::unique_lock<std::mutex> lock(ready_lock);
  //       ready_cv.wait(
  //         lock,
  //         [&]() { return finished || last_update != cur_update; }
  //       );
  //     }
  //     if (finished) return;
  //     last_update = cur_update;
  //     // Process assigned organisms
  //     for (size_t schedule_i : schedule_ids) {
  //       const size_t pop_id = GetID(thread_id, schedule_i);
  //       world_ptr->ProcessOrgsAt(pop_id);
  //     }

  //     {
  //       std::unique_lock<std::mutex> lock(threads_done_lock);
  //       num_threads_done++;
  //     }
  //     threads_done_cv.notify_all();
  //   }

  // }

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