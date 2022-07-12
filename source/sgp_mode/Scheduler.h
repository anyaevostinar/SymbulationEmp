#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "../Organism.h"
#include "../default_mode/SymWorld.h"
#include "sgpl/utility/ThreadLocalRandom.hpp"
#include <atomic>
#include <emp/base/vector.hpp>
#include <functional>
#include <thread>

class Scheduler {
  const size_t THREAD_COUNT = 12;
  const size_t BATCH_SIZE = 48;

  void RunThread(SymWorld &world, std::atomic<size_t> &next_id, size_t i,
                 std::function<void(emp::WorldPosition, Organism &)> &callback) {
    // Make sure each thread gets a different, deterministic, seed
    sgpl::tlrand.Get().ResetSeed(world.GetUpdate() * THREAD_COUNT + i);
    while (true) {
      // Process CPUs for the next BATCH_SIZE organisms
      size_t start = next_id.fetch_add(BATCH_SIZE);
      if (start > world.GetSize())
        return;
      size_t end = start + BATCH_SIZE;

      for (size_t id = start; id < end; id++) {
        if (world.IsOccupied(id)) {
          callback(id, world.GetOrg(id));
        }
      }
    }
  }

public:
  void ProcessOrgs(SymWorld &world, std::function<void(emp::WorldPosition, Organism &)> callback) {
    // TODO use a persistent thread pool instead of spawning new threads every
    // update
    emp::vector<std::thread> threads;
    std::atomic<size_t> next_id = 0;
    for (size_t i = 0; i < THREAD_COUNT - 1; i++) {
      threads.push_back(std::thread(&Scheduler::RunThread, this,
                                    std::ref(world), std::ref(next_id), i,
                                    std::ref(callback)));
    }
    // Last thread is the main thread
    RunThread(world, next_id, THREAD_COUNT - 1, callback);
    // Wait for others to finish
    for (auto &thread : threads) {
      thread.join();
    }
  }
};

#endif