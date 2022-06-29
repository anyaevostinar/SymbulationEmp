#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <emp/base/vector.hpp>
#include "SGPWorld.h"
#include "SGPHost.h"
#include "SGPSymbiont.h"
#include <thread>
#include <atomic>

const size_t THREAD_COUNT = 16;
const size_t BATCH_SIZE = 64;

void thread_fun(SymWorld &world, std::atomic<size_t> &next_id) {
  while(true) {
    // Process CPUs for the next BATCH_SIZE organisms
    size_t start = next_id.fetch_add(BATCH_SIZE);
    if (start > world.GetSize()) return;
    size_t end = start + BATCH_SIZE;

    for (size_t id = start; id < end; id++) {
      if (world.IsOccupied(id)) {
        SGPHost &host = (SGPHost&) world.GetOrg(id);
        host.getCpu().runCpuStep(id);

        for (auto sym : host.GetSymbionts()) {
          emp::Ptr<SGPSymbiont> s_sym = sym.DynamicCast<SGPSymbiont>();
          s_sym->getCpu().runCpuStep(id);
        }
      }
    }
  }
}

void runCpus(SymWorld &world) {
  // TODO use a persistent thread pool instead of spawning new threads every update
  emp::vector<std::thread> threads;
  std::atomic<size_t> next_id = 0;
  for (size_t i = 0; i < THREAD_COUNT-1; i++) {
    threads.push_back(std::thread(thread_fun, std::ref(world), std::ref(next_id)));
  }
  // Last thread is the main thread
  thread_fun(world, next_id);
  // Wait for others to finish
  for (auto &thread : threads) {
    thread.join();
  }
}

#endif