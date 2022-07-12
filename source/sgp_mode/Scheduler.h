#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "../Organism.h"
#include "../default_mode/SymWorld.h"
#include "sgpl/utility/ThreadLocalRandom.hpp"
#include <atomic>
#include <condition_variable>
#include <emp/base/vector.hpp>
#include <functional>
#include <thread>

class Scheduler {
  const size_t BATCH_SIZE = 64;

  SymWorld &world;
  size_t thread_count;

  bool thread_pool_started = false;
  std::function<void(emp::WorldPosition, Organism &)> callback;
  emp::vector<std::thread> running_threads;

  std::mutex ready_lock;
  std::condition_variable ready_cv;
  std::atomic<size_t> next_id = 0;
  std::atomic<size_t> update = -1;

  std::mutex done_lock;
  std::condition_variable done_cv;
  size_t n_done;
  std::atomic_bool finished = false;

  void RunThread(size_t i) {
    // Make sure each thread gets a different, deterministic, seed
    sgpl::tlrand.Get().ResetSeed(i);
    size_t last_update = -1;
    while (true) {
      if (!finished && last_update == update) {
        std::unique_lock<std::mutex> lock(ready_lock);
        ready_cv.wait(lock,
                      [&]() { return finished || last_update != update; });
      }
      if (finished)
        return;
      last_update = update;

      while (true) {
        // Process the next BATCH_SIZE organisms
        size_t start = next_id.fetch_add(BATCH_SIZE);
        if (start > world.GetSize())
          break;

        size_t end = start + BATCH_SIZE;

        for (size_t id = start; id < end; id++) {
          if (world.IsOccupied(id)) {
            callback(id, world.GetOrg(id));
          }
        }
      }

      {
        std::unique_lock<std::mutex> lock(done_lock);
        n_done++;
      }
      done_cv.notify_all();
    }
  }

public:
  Scheduler(SymWorld &world, size_t thread_count)
      : world(world), thread_count(thread_count) {}

  ~Scheduler() {
    {
      std::unique_lock<std::mutex> lock(ready_lock);
      finished = true;
    }
    ready_cv.notify_all();
    for (auto &thread : running_threads) {
      thread.join();
    }
  }

  void ProcessOrgsSync(
      std::function<void(emp::WorldPosition, Organism &)> callback) {
    for (size_t id = 0; id < world.GetSize(); id++) {
      if (world.IsOccupied(id)) {
        callback(id, world.GetOrg(id));
      }
    }
  }

  void
  ProcessOrgs(std::function<void(emp::WorldPosition, Organism &)> callback) {
    // Special case so we don't start any threads when they're not needed
    if (thread_count == 1)
      return ProcessOrgsSync(callback);

    this->callback = callback;

    if (!thread_pool_started) {
      thread_pool_started = true;
      for (size_t i = 0; i < thread_count; i++) {
        running_threads.push_back(std::thread(&Scheduler::RunThread, this, i));
      }
    }

    // Notify threads
    next_id = 0;
    n_done = 0;
    {
      std::unique_lock<std::mutex> lock(ready_lock);
      update = world.GetUpdate();
    }
    ready_cv.notify_all();

    // Wait for threads to finish
    {
      std::unique_lock<std::mutex> lock(done_lock);
      done_cv.wait(lock, [&]() { return n_done == thread_count; });
    }
  }
};

#endif