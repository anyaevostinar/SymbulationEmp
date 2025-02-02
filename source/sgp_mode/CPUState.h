#ifndef CPU_STATE_H
#define CPU_STATE_H

#include "../Organism.h"

#include "emp/Evolve/World_structure.hpp"
#include "emp/base/Ptr.hpp"
#include "emp/base/optional.hpp"
#include "emp/base/vector.hpp"
#include "emp/bits/Bits.hpp"
#include "spec.h"

#include <cstdint>

namespace sgpmode {

class SGPWorld;

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

/**
 * The CPUState holds all state that can be accessed by instructions in the
 * organism's genomes. Each organism has its own CPUState.
 */
// TODO - promote to full class
struct CPUState {
  emp::vector<uint32_t> stack;
  emp::vector<uint32_t> stack2;

  IORingBuffer<4> input_buf;

  // TODO - get rid of dynamic memory if possible
  emp::Ptr<emp::BitSet<spec::NUM_TASKS>> used_resources = emp::NewPtr<emp::BitSet<spec::NUM_TASKS>>();
  emp::Ptr<emp::BitSet<spec::NUM_TASKS>> tasks_performed = emp::NewPtr<emp::BitSet<spec::NUM_TASKS>>();
  emp::Ptr<emp::BitSet<spec::NUM_TASKS>> parent_tasks_performed = emp::NewPtr<emp::BitSet<spec::NUM_TASKS>>(true);
  // TODO - shift to emp::array if possible
  int task_change_lose[spec::NUM_TASKS] = { 0 };
  int task_change_gain[spec::NUM_TASKS] = { 0 };

  int task_toward_partner[spec::NUM_TASKS] = { 0 };
  int task_from_partner[spec::NUM_TASKS] = { 0 };

  double survivial_resource = 0.0;

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

}

#endif