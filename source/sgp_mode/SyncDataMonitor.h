#ifndef SYNCDATAMONITOR_H
#define SYNCDATAMONITOR_H

#include <mutex>
#include "emp/data/DataNode.hpp"

namespace sgpmode {

/// Helper which synchronizes access to the DataMonitor with a mutex
template <typename T, emp::data... MODS> class SyncDataMonitor {
  std::mutex mutex;
  emp::DataMonitor<T, MODS...> monitor;

public:
  /**
   * Input: None
   *
   * Output: Reference to the underlying DataMonitor, without synchronization.
   *
   * Purpose: Accesses the underlying monitor without synchronization, should
   * only be used when you're sure there's no multithreading going on.
   */
  emp::DataMonitor<T, MODS...> &UnsynchronizedGetMonitor() { return monitor; }

  /**
   * Input: An action to perform with the DataMonitor.
   *
   * Output: None
   *
   * Purpose: Calls the provided callback with the DataMonitor while holding the
   * mutex, releasing it when it returns.
   */
  template <typename F> void WithMonitor(F f) {
    std::lock_guard lock(mutex);
    f(monitor);
  }
};
}

#endif