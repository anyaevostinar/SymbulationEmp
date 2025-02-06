#ifndef CPU_STATE_H
#define CPU_STATE_H

#include "../Organism.h"
#include "../spec.h"
#include "../utils.h"
#include "IORingBuffer.h"
#include "Stacks.h"

#include "emp/Evolve/World_structure.hpp"
#include "emp/base/Ptr.hpp"
#include "emp/base/optional.hpp"
#include "emp/base/vector.hpp"
#include "emp/bits/Bits.hpp"
#include "emp/base/array.hpp"

#include <cstdint>

namespace sgpmode {


/**
 * The CPUState holds all state that can be accessed by instructions in the
 * organism's genomes. Each organism has its own CPUState.
 */
// TODO - write tests
class CPUState {
public:

protected:
  Stacks<uint32_t> stacks;
  IORingBuffer<uint32_t, 4> input_buf;

  // TODO - get rid of dynamic memory if possible
  // emp::BitSet<spec::NUM_TASKS> used_resources = emp::NewPtr<emp::BitSet<spec::NUM_TASKS>>();
  // emp::BitSet<spec::NUM_TASKS> tasks_performed = emp::NewPtr<emp::BitSet<spec::NUM_TASKS>>();
  // emp::BitSet<spec::NUM_TASKS> parent_tasks_performed = emp::NewPtr<emp::BitSet<spec::NUM_TASKS>>(true);
  // TODO - will need to resize / reset
  size_t num_tasks = 0;
  emp::BitVector used_resources;          // TODO - document use
  emp::BitVector tasks_performed;
  emp::BitVector parent_tasks_performed;

  emp::vector<int> task_change_loss;    // Change in task performance (relative to parent)
  emp::vector<int> task_change_gain;    // Change in task performance (relative to parent)

  // TODO - shift to emp::array if possible
  // int task_change_lose[spec::NUM_TASKS] = { 0 };
  // int task_change_gain[spec::NUM_TASKS] = { 0 };

  emp::vector<int> task_toward_partner;
  emp::vector<int> task_from_partner;
  // int task_toward_partner[spec::NUM_TASKS] = { 0 };
  // int task_from_partner[spec::NUM_TASKS] = { 0 };

  double survival_resource = 0.0;

  // emp::vector<size_t> available_dependencies;
  // emp::Ptr<emp::vector<size_t>> shared_available_dependencies =
  //     emp::NewPtr<emp::vector<size_t>>();

  // If this organism is queued for reproduction, this stores its position in
  // the queue. When the organism dies, its queue slot will be invalidated.
  // int in_progress_repro = -1;
  bool repro_in_progress = false;
  size_t repro_queue_pos = 0;

  // emp::Ptr<emp::vector<uint32_t>> internal_environment =
  //     emp::NewPtr<emp::vector<uint32_t>>();
  emp::vector<size_t> jump_table;

  emp::Ptr<Organism> organism; // Unowned pointer to organism using this CPU.

  emp::WorldPosition location;

public:
  CPUState(
    emp::Ptr<Organism> organism,
    size_t task_cnt = 0
  ) :
    stacks(2),
    num_tasks(task_cnt),
    organism(organism)
  {
    Reset(num_tasks);
  }

  // Reset state values for given num_tasks.
  // NOTE - does not update/clear organism pointer.
  void Reset(size_t task_cnt);

  // Reset cpu state, but keep num_tasks the same.
  void Reset() {
    Reset(num_tasks);
  }

  size_t GetNumTasks() const { return num_tasks; }
  emp::vector<size_t>& GetJumpTable() { return jump_table; }
  const emp::vector<size_t>& GetJumpTable() const { return jump_table; }
  void SetLocation(const emp::WorldPosition& loc) {
    location = loc;
  }
  const emp::WorldPosition& GetLocation() const { return loc; }

  // TODO - accessors
  // stacks
  // input_buf
  // used_resources
  // tasks_performed
  // parent_tasks_performed
  // task_change_loss
  // task_change_gain
  // task_toward_partner
  // task_from_partner
  // survival_resource
  // repro_in_progress
  // repro_queue_pos
  // jump_table
  // organism
  // location


};

void CPUState::Reset(size_t task_cnt) {
  num_tasks = task_cnt;

  // Clear stacks
  stacks.ClearAll();

  // Reset the input buffer.
  input_buf.Reset(0);
  // Resize + 0-out
  utils::ResizeClear(used_resources, num_tasks);
  utils::ResizeClear(tasks_performed, num_tasks);
  utils::ResizeClear(parent_tasks_performed, num_tasks);

  utils::ResizeFill(task_change_loss, num_tasks, 0);
  utils::ResizeFill(task_change_gain, num_tasks, 0);
  utils::ResizeFill(task_toward_partner, num_tasks, 0);
  utils::ResizeFill(task_from_partner, num_tasks, 0);

  survival_resource = 0.0;

  repro_in_progress = false;
  repro_queue_pos = 0;

  jump_table.clear();

  location = emp::WorldPosition();
}

}

#endif