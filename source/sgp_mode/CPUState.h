#ifndef CPU_STATE_H
#define CPU_STATE_H

#include "../Organism.h"
#include "emp/Evolve/World_structure.hpp"
#include "emp/base/Ptr.hpp"
#include "emp/base/optional.hpp"
#include "emp/base/vector.hpp"
#include "emp/bits/BitSet.hpp"
#include <cstdint>

/// A helper class for a ring buffer that keeps the latest `len` inputs and
/// discards the rest.
template <const size_t len> class IORingBuffer {
  uint32_t buffer[len];
  size_t next = 0;

public:
  IORingBuffer() {
    for (size_t i = 0; i < len; i++) {
      buffer[i] = 0;
    }
  }

  void push(uint32_t x) {
    buffer[next] = x;
    next = (next + 1) % len;
  }

  uint32_t operator[](size_t idx) const { return buffer[idx % len]; }

  size_t size() const { return len; }
};

// CPUState has a pointer to the SGPWorld, but it can't include it
class SGPWorld;

/**
 * The CPUState holds all state that can be accessed by instructions in the
 * organism's genomes. Each organism has its own CPUState.
 */
struct CPUState {
  emp::vector<uint32_t> stack;
  emp::vector<uint32_t> stack2;

  IORingBuffer<4> input_buf;

  emp::Ptr<emp::BitSet<64>> used_resources = emp::NewPtr<emp::BitSet<64>>();
  emp::Ptr<emp::BitSet<64>> tasks_performed = emp::NewPtr<emp::BitSet<64>>();
  emp::vector<size_t> available_dependencies;
  emp::Ptr<emp::vector<size_t>> shared_available_dependencies =
      emp::NewPtr<emp::vector<size_t>>();
  // If this organism is queued for reproduction, this stores its position in
  // the queue. When the organism dies, its queue slot will be invalidated.
  int in_progress_repro = -1;
  emp::Ptr<emp::vector<uint32_t>> internal_environment =
      emp::NewPtr<emp::vector<uint32_t>>();
  emp::vector<size_t> jump_table;

  emp::Ptr<Organism> organism;
  emp::Ptr<SGPWorld> world;
  emp::WorldPosition location;

  CPUState(emp::Ptr<Organism> organism, emp::Ptr<SGPWorld> world)
      : organism(organism), world(world) {}
};

#endif