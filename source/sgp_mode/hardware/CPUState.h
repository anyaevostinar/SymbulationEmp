#ifndef HARDWARE_CPU_STATE_H
#define HARDWARE_CPU_STATE_H

#include "RingBuffer.h"
#include "Stacks.h"
#include "../org_type_info.h"
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

// NOTE - should this be repro "in progress" or "queued"?
//        I think this is only used to manage queued reproductions?
enum class ReproState { NONE=0, ATTEMPTING, IN_PROGRESS };

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
  using input_buf_t = RingBuffer<uint32_t>;

  struct ReproInfo {
    ReproState state = ReproState::NONE;
    size_t queue_pos = 0;
  };
protected:
  Stacks<uint32_t> stacks;
  input_buf_t input_buf;
  emp::vector<uint32_t> output_buffer;
  size_t task_env_id = 0; // Tracks current task ID environment used by this organism

  // TODO - get rid of dynamic memory if possible
  // emp::BitSet<spec::NUM_TASKS> used_resources = emp::NewPtr<emp::BitSet<spec::NUM_TASKS>>();
  // emp::BitSet<spec::NUM_TASKS> tasks_performed = emp::NewPtr<emp::BitSet<spec::NUM_TASKS>>();
  // emp::BitSet<spec::NUM_TASKS> parent_tasks_performed = emp::NewPtr<emp::BitSet<spec::NUM_TASKS>>(true);
  // TODO - will need to resize / reset
  size_t num_tasks = 0;
  // emp::BitVector used_resources;          // TODO - document use
  // NOTE - should this be in the CPU state? Or, move into organism class as "phenotype" information?
  emp::BitVector tasks_performed;
  emp::vector<size_t> tasks_performance_cnt;

  emp::BitVector parent_tasks_performed;  // TODO - Configure parent tasks performed

  emp::vector<int> task_change_loss;    // Change in task performance (relative to parent)
  emp::vector<int> task_change_gain;    // Change in task performance (relative to parent)

  // TODO - shift to emp::array if possible
  // int task_change_lose[spec::NUM_TASKS] = { 0 };
  // int task_change_gain[spec::NUM_TASKS] = { 0 };

  emp::vector<int> task_toward_partner;
  emp::vector<int> task_from_partner;
  // int task_toward_partner[spec::NUM_TASKS] = { 0 };
  // int task_from_partner[spec::NUM_TASKS] = { 0 };

  double survival_resource = 0.0; // TODO - move this out of CPUState

  // emp::vector<size_t> available_dependencies;
  // emp::Ptr<emp::vector<size_t>> shared_available_dependencies =
  //     emp::NewPtr<emp::vector<size_t>>();

  // If this organism is queued for reproduction, this stores its position in
  // the queue. When the organism dies, its queue slot will be invalidated.
  // int in_progress_repro = -1;
  // bool repro_attempt = false;     // Flags whether organism has attempted reproduction.
  // bool repro_in_progress = false;
  // size_t repro_queue_pos = 0;
  ReproInfo repro_info;

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
    size_t stack_limit = org_info::DEFAULT_STACK_SIZE_LIMIT
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
    input_buf.Reset(0, 0);
    task_env_id = 0;

    // Reset output buffer
    output_buffer.clear();

    // Resize + 0-out
    // utils::ResizeClear(used_resources, num_tasks);
    utils::ResizeClear(tasks_performed, num_tasks);
    utils::ResizeClear(parent_tasks_performed, num_tasks);

    utils::ResizeFill(tasks_performance_cnt, num_tasks, 0);
    utils::ResizeFill(task_change_loss, num_tasks, 0);
    utils::ResizeFill(task_change_gain, num_tasks, 0);
    utils::ResizeFill(task_toward_partner, num_tasks, 0);
    utils::ResizeFill(task_from_partner, num_tasks, 0);

    survival_resource = 0.0;

    ResetReproState();

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
  size_t GetJumpDest(size_t pc) const {
    emp_assert(pc < jump_table.size());
    return jump_table[pc];
  }

  void SetLocation(const emp::WorldPosition& loc) {
    location = loc;
  }
  const emp::WorldPosition& GetLocation() const { return location; }

  void SetTaskEnvID(size_t id) { task_env_id = id; }
  size_t GetTaskEnvID() const { return task_env_id; }

  input_buf_t& GetInputBuffer() { return input_buf; }
  const input_buf_t& GetInputBuffer() const { return input_buf; }

  emp::vector<uint32_t>& GetOutputBuffer() { return output_buffer; }
  const emp::vector<uint32_t>& GetOutputBuffer() const { return output_buffer; }

  void SetInputs(const emp::vector<uint32_t>& inputs) {
    input_buf.SetBuffer(inputs);
    emp_assert(input_buf.size() == inputs.size());
  }

  // const emp::WorldPosition& GetLocation() const { return loc; }

  void SetOrganism(emp::Ptr<Organism> org_ptr) {
    organism = org_ptr;
  }

  emp::Ptr<Organism> GetOrgPtr() { return organism; }
  Organism& GetOrg() { return *organism; }
  const Organism& GetOrg() const { return *organism; }
  bool IsHost() const { return organism->IsHost(); }
  bool IsSym() const { return !(organism->IsHost()); }
  bool HasHost() const { return organism->GetHost() != nullptr; }
  Organism& GetHost() {
    emp_assert(HasHost());
    return *(organism->GetHost());
  }
  const Organism& GetHost() const {
    emp_assert(HasHost());
    return *(organism->GetHost());
  }

  void SetWorld(emp::Ptr<world_t> w_ptr) { world_ptr = w_ptr; }
  emp::Ptr<world_t> GetWorldPtr() { return world_ptr; }
  world_t& GetWorld() { return *world_ptr; }
  const world_t& GetWorld() const { return *world_ptr; }

  Stacks<uint32_t>& GetStacks() { return stacks; }
  const Stacks<uint32_t>& GetStacks() const { return stacks; }

  void MarkReproAttempt() { repro_info.state = ReproState::ATTEMPTING; }
  void MarkReproInProgress(size_t queue_pos) {
    repro_info.state = ReproState::IN_PROGRESS;
    repro_info.queue_pos = queue_pos;
  }
  bool ReproInProgress() const {
    return repro_info.state == ReproState::IN_PROGRESS;
  }
  bool ReproAttempt() const {
    return repro_info.state == ReproState::ATTEMPTING;
  }
  size_t GetReproQueuePos() const {
    emp_assert(ReproInProgress(), "Queue position valid only if repro is in progress");
    return repro_info.queue_pos;
  }

   void ResetReproState() {
    // repro_queue_pos = 0;
    // repro_attempt = false;
    // repro_in_progress = false;
    repro_info.state = ReproState::NONE;
    repro_info.queue_pos = 0;
  }

  const emp::BitVector& GetTasksPerformed() const { return tasks_performed; }
  emp::BitVector& GetTasksPerformed() { return tasks_performed; }
  bool GetTaskPerformed(size_t task_id) const { return tasks_performed.Get(task_id); }

  const emp::BitVector& GetParentTasksPerformed() const { return parent_tasks_performed; }
  emp::BitVector& GetParentTasksPerformed() { return parent_tasks_performed; }
  bool GetParentTaskPerformed(size_t task_id) const { return parent_tasks_performed.Get(task_id); }
  void SetParentTasksPerformed(const emp::BitVector& parent_tasks) {
    parent_tasks_performed.Import(parent_tasks);
  }

  const emp::vector<size_t>& GetTaskPerformanceCounts() const { return tasks_performance_cnt; }
  emp::vector<size_t>& GetTaskPerformanceCounts() { return tasks_performance_cnt; }
  size_t GetTaskPerformanceCount(size_t task_id) const {
    emp_assert(task_id < tasks_performance_cnt.size());
    return tasks_performance_cnt[task_id];
  }

  void MarkTaskPerformed(size_t task_id) {
    emp_assert(task_id < tasks_performed.GetSize());
    emp_assert(task_id < tasks_performance_cnt.size());
    tasks_performed.Set(task_id, true);
    ++(tasks_performance_cnt[task_id]);
  }

};

}

#endif