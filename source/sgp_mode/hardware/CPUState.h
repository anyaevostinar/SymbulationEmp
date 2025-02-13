#ifndef CPU_STATE_H
#define CPU_STATE_H

#include "IORingBuffer.h"
#include "Stacks.h"
#include "../spec.h"
#include "../utils.h"
#include "../../Organism.h"

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
template<typename WORLD_T>
class CPUState {
public:
  // using spec_t = HW_SPEC_T;
  using world_t = WORLD_T;

protected:
  Stacks<uint32_t> stacks;
  IORingBuffer<uint32_t, 4> input_buf;
  emp::vector<size_t> output_buffer;

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
  bool repro_attempt = false;     // Flags whether organism has attempted reproduction.
  bool repro_in_progress = false;
  size_t repro_queue_pos = 0;

  // emp::Ptr<emp::vector<uint32_t>> internal_environment =
  //     emp::NewPtr<emp::vector<uint32_t>>();
  emp::vector<size_t> jump_table;

  emp::Ptr<Organism> organism; // Unowned pointer to organism using this CPU.
  emp::Ptr<world_t> world_ptr; // Unowned pointer to the world using this CPU.

  emp::WorldPosition location;

public:
  CPUState(
    emp::Ptr<world_t> world,
    emp::Ptr<Organism> organism,
    size_t task_cnt = 0,
    size_t stack_limit = spec::DEFAULT_STACK_SIZE_LIMIT
  ) :
    stacks(2),
    num_tasks(task_cnt),
    organism(organism),
    world_ptr(world)
  {
    Reset(num_tasks);
    stacks.SetStackLimit(stack_limit);
  }

  // Reset state values for given num_tasks.
  // NOTE - does not update/clear organism pointer or world pointer.
  void Reset(size_t task_cnt)  {
    num_tasks = task_cnt;

    // Clear stacks
    stacks.ClearAll();
    stacks.SetActive(0);

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
  const emp::WorldPosition& GetLocation() const { return location; }

  // const emp::WorldPosition& GetLocation() const { return loc; }

  void SetOrganism(emp::Ptr<Organism> org_ptr) {
    organism = org_ptr;
  }

  emp::Ptr<Organism> GetOrgPtr() { return organism; }
  Organism& GetOrg() { return *organism; }
  const Organism& GetOrg() const { return *organism; }

  void SetWorld(emp::Ptr<world_t> w_ptr) { world_ptr = w_ptr; }
  emp::Ptr<world_t> GetWorldPtr() { return world_ptr; }
  world_t& GetWorld() { return *world_ptr; }
  const world_t& GetWorld() const { return *world_ptr; }

  Stacks<uint32_t>& GetStacks() { return stacks; }
  const Stacks<uint32_t>& GetStacks() const { return stacks; }

  void SetReproInProgress(bool val) { repro_in_progress = val; }
  void SetReproAttempt(bool val) { repro_attempt = val; }

  bool ReproInProgress() const { return repro_in_progress; }
  bool ReproAttempt() const { return repro_attempt; }

  size_t GetReproQueuePos() const { return repro_queue_pos; }
  void SetReproQueuePos(size_t pos) { repro_queue_pos = pos; }

   void ResetReproState() {
    repro_queue_pos = 0;
    repro_attempt = false;
    repro_in_progress = false;
  }

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
  // repro_queue_pos
  // jump_table
  // organism
  // location


};

}

#endif