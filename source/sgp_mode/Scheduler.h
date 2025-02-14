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

  // Threading-related member variables
  emp::vector<emp::vector<size_t>> thread_batches; // Contains schedule indexes handled for each thread.
  size_t thread_count; // How many threads?
  bool threaded_mode = false;
  emp::vector<std::thread> running_threads;
  bool thread_pool_started = false;
  emp::vector<size_t> thread_seeds; // To ensure replicates are independent, need to generate seeds using root rng

  std::mutex ready_lock;
  std::condition_variable ready_cv;
  std::atomic<size_t> cur_update = 0;

  std::mutex threads_done_lock;
  std::condition_variable threads_done_cv;
  size_t num_threads_done = 0;
  std::atomic_bool finished = false;

  // Helper function to get id to schedule w/thread batch info
  size_t GetID(size_t batch_id, size_t idx) {
    emp_assert(batch_id < thread_batches.size());
    emp_assert(idx < thread_batches[batch_id].size());
    return schedule_order[thread_batches[batch_id][idx]];
  }

  // TODO - re-evaluate use of threads; so much possibility for hard-to-detect bad behavior
  // Can't pass in world as a temporary reference
  // TODO - benchmark this approach to scheduling on threads vs. dynamic batching
  template<typename WORLD_T>
  void RunThread(emp::Ptr<WORLD_T> world_ptr, size_t thread_id) {
    // Make sure each thread gets a different, deterministic, seed
    const size_t thread_seed = thread_seeds[thread_id];
    const auto& schedule_ids = thread_batches[thread_id];
    sgpl::tlrand.Get().ResetSeed(thread_seed);
    size_t last_update = std::numeric_limits<size_t>::max();
    while (true) {
      if (!finished && last_update == cur_update) {
        std::unique_lock<std::mutex> lock(ready_lock);
        ready_cv.wait(
          lock,
          [&]() { return finished || last_update != cur_update; }
        );
      }
      if (finished) return;
      last_update = cur_update;
      // Process assigned organisms
      for (size_t schedule_i : schedule_ids) {
        const size_t pop_id = GetID(thread_id, schedule_i);
        world_ptr->ProcessOrgAt(pop_id);
      }

      {
        std::unique_lock<std::mutex> lock(threads_done_lock);
        num_threads_done++;
      }
      threads_done_cv.notify_all();
    }

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

  ~Scheduler() {
    {
      std::unique_lock<std::mutex> lock(ready_lock);
      finished = true;
    }
    ready_cv.notify_all();
    for (auto& thread : running_threads) {
      thread.join();
    }
  }

  // Allow re-configuration of scheduler post-construction.
  void SetupScheduler(size_t world_size, size_t num_threads) {
    emp_assert(num_threads > 0, "Thread count cannot be 0");
    emp_assert(world_size > 0, "World size must be > 0");

    // TODO - kill threads if necessary
    {
      std::unique_lock<std::mutex> lock(ready_lock);
      finished = true;
    }
    ready_cv.notify_all();
    for (auto& thread : running_threads) {
      thread.join();
    }
    finished = false;

    schedule_order.clear();
    thread_batches.clear();
    running_threads.clear(); // TODO - do I need to wait?
    thread_seeds.clear();
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

      // Assign each thread a randomly generated seed
      thread_seeds.resize(thread_count);
      std::generate(
        thread_seeds.begin(),
        thread_seeds.end(),
        [this]() { return random.GetUInt(); }
      );

    }

  }

  size_t GetScheduleSize() const { return schedule_order.size(); }
  const emp::vector<size_t>& GetCurSchedule() const { return schedule_order; }

  // Update schedule order (uniform random)
  void UpdateSchedule() {
    emp::Shuffle(random, schedule_order);
  }

  // Process all orgs in world population in current schedule order (single-threaded).
  template<typename WORLD_T>
  void RunSync(WORLD_T& world) {
    for (size_t world_id : schedule_order) {
      emp_assert(world_id < world.GetSize());
      world.ProcessOrgAt(world_id);  // TODO - implement process org at
    }
  }

  // TODO - review threading implementation
  template<typename WORLD_T>
  void RunThreaded(WORLD_T& world) {
    emp_assert(threaded_mode);

    // TODO - better way to init threads?
    if (!thread_pool_started) {
      // Init thread pool
      thread_pool_started = true;
      for (size_t thread_i = 0; thread_i < thread_count; ++thread_i) {
        running_threads.emplace_back(
          // TODO - test that this way of passing world into thread works as expected
          std::thread(&Scheduler::RunThread<WORLD_T>, this, &world, thread_i)
        );
      }
    }

    num_threads_done = 0;
    {
      std::unique_lock<std::mutex> lock(ready_lock);
      cur_update = world.GetUpdate();
    }
    ready_cv.notify_all();

    // Wait for threads to finish
    {
      std::unique_lock<std::mutex> lock(threads_done_lock);
      threads_done_cv.wait(lock, [&]() { return num_threads_done == thread_count; });
    }

  }

  template<typename WORLD_T>
  void Run(WORLD_T& world) {
    // thread batches should work for sync or threaded mode
    if (!threaded_mode) {
      RunSync(world);
    } else {
      RunThreaded(world);
    }
  }

};

}

#endif