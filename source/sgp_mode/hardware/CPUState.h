#ifndef HARDWARE_CPU_STATE_H
#define HARDWARE_CPU_STATE_H

#include "RingBuffer.h"
#include "Stacks.h"
#include "../org_type_info.h"
#include "../../utils.h"
#include "../../Organism.h"

#include "emp/Evolve/World_structure.hpp"
#include "emp/base/Ptr.hpp"
#include "emp/base/optional.hpp"
#include "emp/base/vector.hpp"
#include "emp/bits/Bits.hpp"
#include "emp/base/array.hpp"
#include "emp/math/math.hpp"

#include <cstdint>

namespace sgpmode {

// NOTE - should this be repro "in progress" or "queued"?
//        I think this is only used to manage queued reproductions?
enum class ReproState { NONE=0, ATTEMPTING, IN_PROGRESS };

/**
 * The CPUState holds all state that can be accessed by instructions in the
 * organism's genomes. Each organism has its own CPUState.
 * @tparam WORLD_T: The type representing to the world the simulation is in
 */
// TODO - write tests
// TODO - cleanup member variables (reduce down to only what we're using)
template<typename WORLD_T>
class CPUState {
public:
  // using spec_t = HW_SPEC_T;
  using world_t = WORLD_T;
  using reg_val_t = typename world_t::hw_spec_t::register_value_t;
  using input_buf_t = RingBuffer<uint32_t>;
  using output_buf_t = emp::vector<uint32_t>;

  struct ReproInfo {
    ReproState state = ReproState::NONE;
    size_t queue_pos = 0;
  };

protected:
  Stacks<uint32_t> stacks;
  input_buf_t input_buf;
  output_buf_t output_buffer;
  size_t task_env_id = 0; // Tracks current task ID environment used by this organism
  size_t num_tasks = 0;
  // NOTE - should this be in the CPU state? Or, move into organism class as "phenotype" information?
  emp::BitVector tasks_performed;
  emp::vector<size_t> tasks_performance_count;

  emp::BitVector first_task_performed;
  // size_t first_task_performed_id = (size_t)-1;

  // Track which outputs for each task have been credited.
  // - Only give credit for repeats after all pairs have been used
  // task outputs credited
  emp::vector< std::set<uint32_t> > task_outputs_credited;

  emp::BitVector parent_tasks_performed;
  emp::BitVector parent_first_task_performed;

  // NOTE - should this be tracked by the systematics instead?
  // NOTE - shifted int to size_t, looked like these were only ever positive numbers
  // NOTE - Manage all of this with a struct that contains relevant logic?
  emp::vector<size_t> lineage_task_change_loss;    // Change in task performance (relative to parent)
  emp::vector<size_t> lineage_task_change_gain;    // Change in task performance (relative to parent)
  // NOTE - shifted int to size_t, looked like these were only ever positive numbers
  emp::vector<size_t> lineage_task_converge_partner;
  emp::vector<size_t> lineage_task_diverge_partner;

  double survival_resource = 0.0; // TODO - move this out of CPUState
  size_t cpu_cycles_to_exec = 0;  // Used by world to adjust per-update cpu cycle allotment.

  // If this organism is queued for reproduction, this stores its position in
  // the queue. When the organism dies, its queue slot will be invalidated.
  ReproInfo repro_info;
  size_t cpu_cycles_since_repro = 0;

  emp::vector<size_t> jump_table;

  emp::Ptr<Organism> organism; // Unowned pointer to organism using this CPU.
  emp::Ptr<world_t> world_ptr; // Unowned pointer to the world using this CPU.


public:
  CPUState(
    emp::Ptr<world_t> world,
    emp::Ptr<Organism> organism,
    size_t task_count = 0,
    size_t stack_limit = org_info::DEFAULT_STACK_SIZE_LIMIT
  ) :
    stacks(2),
    num_tasks(task_count),
    organism(organism),
    world_ptr(world)
  {
    Reset(num_tasks);
    stacks.SetStackLimit(stack_limit);
  }

  /**
   * Input: Number of tasks.
   *
   * Output: None.
   *
   * Purpose: Reset state values for given number of tasks.
   */
  // NOTE - does not update/clear organism pointer or world pointer.
  void Reset(size_t task_count)  {
    num_tasks = task_count;
    // Clear stacks
    stacks.ClearAll();
    stacks.SetActive(0);

    // Reset the input buffer.
    input_buf.Reset(0, 0);
    task_env_id = 0;

    // Reset output buffer
    output_buffer.clear();

    // Reset tasks credited
    task_outputs_credited.clear();
    task_outputs_credited.resize(task_count, {});

    // Resize + 0-out
    // utils::ResizeClear(used_resources, num_tasks);
    utils::ResizeClear(tasks_performed, num_tasks);
    utils::ResizeClear(parent_tasks_performed, num_tasks);
    // first_task_performed_id = (size_t)-1;
    utils::ResizeClear(first_task_performed, num_tasks);
    utils::ResizeClear(parent_first_task_performed, num_tasks);

    utils::ResizeFill(tasks_performance_count, num_tasks, 0);
    utils::ResizeFill(lineage_task_change_loss, num_tasks, 0);
    utils::ResizeFill(lineage_task_change_gain, num_tasks, 0);
    utils::ResizeFill(lineage_task_converge_partner, num_tasks, 0);
    utils::ResizeFill(lineage_task_diverge_partner, num_tasks, 0);

    survival_resource = 0.0;

    cpu_cycles_to_exec = 0.0;
    // cpu_cycles_gained = 0.0;
    // cpu_cycles_lost = 0.0;

    ResetReproState();
    cpu_cycles_since_repro = 0;

    jump_table.clear();

  }

  /**
   * Input: None.
   *
   * Output: None.
   *
   * Purpose: Resets cpu state, but keeps number of tasks the same.
   */
  void Reset() {
    Reset(num_tasks);
  }

  /**
   * Input: None.
   *
   * Output: None.
   *
   * Purpose: Returns the number of tasks.
   */
  size_t GetNumTasks() const { return num_tasks; }

  /**
   * Input: None.
   *
   * Output: None.
   *
   * Purpose: Returns the jump table.
   */
  emp::vector<size_t>& GetJumpTable() { return jump_table; }
  const emp::vector<size_t>& GetJumpTable() const { return jump_table; }
  
  /**
   * Input: Index of jump.
   *
   * Output: Instruction id of where the jump, jumps to.
   *
   * Purpose: Access where the input jump leads to.
   */
  size_t GetJumpDest(size_t pc) const {
    emp_assert(pc < jump_table.size());
    return jump_table[pc];
  }

  /**
   * Input: New location of host.
   *
   * Output: None.
   *
   * Purpose: Sets a new location of the host.
   */
  void SetLocation(const emp::WorldPosition& loc) {
    GetOrgPtr()->SetLocation(loc);
  }

  /**
   * Input: None.
   *
   * Output: The location of the host.
   *
   * Purpose: Access the location of the host.
   */
  const emp::WorldPosition& GetLocation() const { return GetOrg().GetLocation(); }

  /**
   * Input: Number of cycles.
   *
   * Output: None.
   *
   * Purpose: Sets the number of instructions that will be executed this update.
   */
  void SetCPUCyclesToExec(size_t num) {
    cpu_cycles_to_exec = num;
  }

  /**
   * Input: None.
   *
   * Output: Number of CPU cycles to be excuted.
   *
   * Purpose: Gets the number of instructions that will be executed this update.
   */
  size_t GetCPUCyclesToExec() const {
    return cpu_cycles_to_exec;
  }

  /**
   * Input: Number of cycles to add.
   *
   * Output: None.
   *
   * Purpose: Increase the number of instructions that will be executed this update.
   */
  void GainCPUCycles(size_t gain) {
    cpu_cycles_to_exec += gain;
  }

  /**
   * Input: Number of cycles to lose.
   *
   * Output: None.
   *
   * Purpose: Decrease the number of instructions that will be executed this update.
   */
  void LoseCPUCycles(size_t loss) {
    emp_assert(loss <= cpu_cycles_to_exec);
    // NOTE - assert sufficient, or do we want to ensure loss is not bigger than
    //        exec
    cpu_cycles_to_exec -= loss;
  }

  /**
   * Input: None.
   *
   * Output: The number of cycles.
   *
   * Purpose: Used to access the number of cycles that it will execute and then set it back to 0.
   */
  size_t ExtractCPUCycles() {
    const size_t cycles = cpu_cycles_to_exec;
    cpu_cycles_to_exec = 0;
    return cycles;
  }

  /**
   * Input: New task enviroment Id.
   *
   * Output: None.
   *
   * Purpose: Sets the id of the task enviroment to the input.
   */
  void SetTaskEnvID(size_t id) { task_env_id = id; }

  /**
   * Input: None.
   *
   * Output: The task enviroment ID.
   *
   * Purpose: Access the ID of the task enviroment.
   */
  size_t GetTaskEnvID() const { return task_env_id; }

  /**
   * Input: None.
   *
   * Output: The input buffer
   *
   * Purpose: Access the current input buffer of the CPUState
   */
  input_buf_t& GetInputBuffer() { return input_buf; }
  const input_buf_t& GetInputBuffer() const { return input_buf; }

  /**
   * Input: None.
   *
   * Output: The output buffer
   *
   * Purpose: Access the current output buffer of the CPUState
   */
  output_buf_t& GetOutputBuffer() { return output_buffer; }
  const output_buf_t& GetOutputBuffer() const { return output_buffer; }

  /**
   * Input: A vector of integers containing all of the inputs.
   *
   * Output: None.
   *
   * Purpose: Setting the input buffer to a list of new inputs.
   */
  void SetInputs(const emp::vector<uint32_t>& inputs) {
    input_buf.SetBuffer(inputs);
    emp_assert(input_buf.size() == inputs.size());
  }

  /**
   * Input: A vector of integers containing all of the output.
   *
   * Output: None.
   *
   * Purpose: Setting the output buffer to a list of new outputs.
   */
  void SetOutputs(const emp::vector<uint32_t>& outputs) {
    for (size_t i = 0; i < outputs.size(); i++) {
      if (i < output_buffer.size()) {
        output_buffer[i] = outputs[i];
      }
      else {
        output_buffer.emplace_back(outputs[i]);
      }
    }
  }

  /**
   * Input: An Organism pointer.
   *
   * Output: None.
   *
   * Purpose: Sets which organism this CPU state corresponds with
   */
  void SetOrganism(emp::Ptr<Organism> org_ptr) {
    organism = org_ptr;
  }

  /**
   * Input: None.
   *
   * Output: Organism Pointer.
   *
   * Purpose: Gets a pointer to the organism
   */
  emp::Ptr<Organism> GetOrgPtr() { return organism; }

  /**
   * Input: None.
   *
   * Output: Organism.
   *
   * Purpose: Gets a reference to the organism
   */
  Organism& GetOrg() { return *organism; }
  const Organism& GetOrg() const { return *organism; }

  /**
   * Input: None.
   *
   * Output: Bool, whether organism is a host or not.
   *
   * Purpose: Checks if the organism is a host
   */
  bool IsHost() const { return organism->IsHost(); }

  /**
   * Input: None.
   *
   * Output: Bool, whether the organism is a symbiont or not.
   *
   * Purpose: Checks if the organism is a symbiont
   */
  bool IsSym() const { return !(organism->IsHost()); }

  /**
   * Input: None.
   *
   * Output: Bool, whether the organism has a host.
   *
   * Purpose: Checks if the organism has an associated host
   used for checking if symbiont has a host before we access it.
   */
  bool HasHost() const { return organism->GetHost() != nullptr; }

  /**
   * Input: None.
   *
   * Output: Organism, The host of the organism.
   *
   * Purpose: Used to access the host of the organism for interactions.
   */
  Organism& GetHost() {
    emp_assert(HasHost());
    return *(organism->GetHost());
  }
  const Organism& GetHost() const {
    emp_assert(HasHost());
    return *(organism->GetHost());
  }

  /**
   * Input: World pointer.
   *
   * Output: None.
   *
   * Purpose: Sets the world of the CPU state to the new input world
   */
  void SetWorld(emp::Ptr<world_t> w_ptr) { world_ptr = w_ptr; }

  /**
   * Input: None.
   *
   * Output: World pointer
   *
   * Purpose: Gets a pointer to the current world
   */
  emp::Ptr<world_t> GetWorldPtr() { return world_ptr; }

  /**
   * Input: None.
   *
   * Output: World
   *
   * Purpose: Gets a reference to the current world
   */
  world_t& GetWorld() { return *world_ptr; }
  const world_t& GetWorld() const { return *world_ptr; }

  /**
   * Input: None.
   *
   * Output: Stacks of integers
   *
   * Purpose: Returns the current stack of integers
   */
  Stacks<uint32_t>& GetStacks() { return stacks; }
  const Stacks<uint32_t>& GetStacks() const { return stacks; }

  /**
   * Input: None.
   *
   * Output: None.
   *
   * Purpose: Marks the CPU state as currently attempting to reproduce
   */
  void MarkReproAttempt() { repro_info.state = ReproState::ATTEMPTING; }

  /**
   * Input: None.
   *
   * Output: None.
   *
   * Purpose: Marks the CPU state as currently attempting to reproduce
   */
  void MarkReproInProgress(size_t queue_pos) {
    repro_info.state = ReproState::IN_PROGRESS;
    repro_info.queue_pos = queue_pos;
  }

  /**
   * Input: None.
   *
   * Output: Bool, if repro state is in progress.
   *
   * Purpose: Used to check if the organism is currently in the process of reproducing
   */
  bool ReproInProgress() const {
    return repro_info.state == ReproState::IN_PROGRESS;
  }

  /**
   * Input: None.
   *
   * Output: Bool, if repro state is in attempting.
   *
   * Purpose: Used to check if the organism is currently attempting to reproduce
   */
  bool ReproAttempt() const {
    return repro_info.state == ReproState::ATTEMPTING;
  }

  /**
   * Input: None.
   *
   * Output: Bool, if repro state is not reproducing.
   *
   * Purpose: Used to check if the organism is currently not attempting to or in the process of reproducing
   */
  bool NotReproducing() const {
    return repro_info.state == ReproState::NONE;
  }

  /**
   * Input: None.
   *
   * Output: Size_t, location in repro queue
   *
   * Purpose: Gets the location of the organisms reproduction event in the reproduciton queue
   */
  size_t GetReproQueuePos() const {
    emp_assert(ReproInProgress(), "Queue position valid only if repro is in progress");
    return repro_info.queue_pos;
  }

  /**
   * Input: None.
   *
   * Output: None.
   *
   * Purpose: Resets the repro state, readying it for reproduction again and reset queue position
   */
   void ResetReproState() {
    repro_info.state = ReproState::NONE;
    repro_info.queue_pos = 0;
  }

   /**
   * Input: None.
   *
   * Output: Size_t, number of CPU cycles since the last reproduction event
   *
   * Purpose: Used to track how long it has been since last reproduction to enforce a minimum period between reproductions
   */
  size_t GetCPUCyclesSinceRepro() const { return cpu_cycles_since_repro; }
  
  /**
   * Input: Size_t, number of CPU cycles that will be added
   *
   * Output: None.
   *
   * Purpose: Increases the number of cycles that occured since the last reproduction
   */
  void IncCPUCyclesSinceRepro(size_t inc_amount = 1) {
    cpu_cycles_since_repro += inc_amount;
  }

  /**
   * Input: Size_t, the number that the cycles since repro will be set to
   *
   * Output: None.
   *
   * Purpose: Sets the number of cycles that occured since the last reproduction
   * generally used to reset it to 0 when reproduction occurs.
   */
  void SetCPUCyclesSinceRepro(size_t value) {
    cpu_cycles_since_repro = value;
  }

  /**
   * Input: None.
   *
   * Output: BitVector, A vector containing a 0 or 1 for each task if they are completed or not
   *
   * Purpose: To access what tasks the organsism has/has not performed
   */
  const emp::BitVector& GetTasksPerformed() const { return tasks_performed; }
  emp::BitVector& GetTasksPerformed() { return tasks_performed; }

  /**
   * Input: size_t task id.
   *
   * Output: Bool, whether that task has been performed or not
   *
   * Purpose: To check whether a specific task has been completed
   */
  bool GetTaskPerformed(size_t task_id) const { return tasks_performed.Get(task_id); }

  /**
   * Input: None.
   *
   * Output: A BitVector containing all tasks, 0 if they are not performed or not 
   * the first task to be performed, 1 if they were the first task performed.
   *
   * Purpose: To check what the first task the organism performed was.
   */
  const emp::BitVector& GetFirstTaskPerformed() const { return first_task_performed; }
  emp::BitVector& GetFirstTaskPerformed() { return first_task_performed; }


   /**
   * Input: None.
   *
   * Output: BitVector, A vector containing a 0 or 1 based on if
   * the organism's parent performed the task
   *
   * Purpose: To access what tasks the organsism's parent has/has not performed
   */
  const emp::BitVector& GetParentTasksPerformed() const { return parent_tasks_performed; }
  emp::BitVector& GetParentTasksPerformed() { return parent_tasks_performed; }

  /**
   * Input: size_t task id.
   *
   * Output: Bool, whether that task has been performed or not by the organism's parent
   *
   * Purpose: To check whether a specific task has been completed by the organism's parent
   */
  bool GetParentTaskPerformed(size_t task_id) const { return parent_tasks_performed.Get(task_id); }

  /**
   * Input: BitVector of parents tasks.
   *
   * Output: None.
   *
   * Purpose: Sets up the parent tasks Vector to be equivalent to the organism's parent's tasks.
   */
  void SetParentTasksPerformed(const emp::BitVector& parent_tasks) {
    parent_tasks_performed.Import(parent_tasks);
  }

  /**
   * Input: size_t task id, bool whether that task has been perforemd.
   *
   * Output: None.
   *
   * Purpose: Sets that whether the parent has performed the task corresponding to
   * task_id to the value of performed
   */
  void SetParentTaskPerformed(size_t task_id, bool performed=true) {
    parent_tasks_performed.Set(task_id, performed);
  }

  /**
   * Input: None.
   *
   * Output: A BitVector containing all tasks, 0 if they were not performed by the parent or not 
   * the first task the parent performed, 1 if they were the first task the parent performed.
   *
   * Purpose: To check what the first task the organism's parent performed was.
   */
  const emp::BitVector& GetParentFirstTaskPerformed() const { return parent_first_task_performed; }
  emp::BitVector& GetParentFirstTaskPerformed() { return parent_first_task_performed; }

  /**
   * Input: BitVector of parents tasks.
   *
   * Output: None.
   *
   * Purpose: Sets up the parent first tasks Vector to be equivalent to the organism's parent's first completed task.
   */
  void SetParentFirstTaskPerformed(const emp::BitVector& parent_first_task) {
    parent_first_task_performed.Import(parent_first_task);
  }

   /**
   * Input: size_t task id, bool whether that task has been perforemd.
   *
   * Output: None.
   *
   * Purpose: Sets that whether the parent the task corresponding to
   * task_id was the first task performed to the value of performed
   */
  void SetParentFirstTaskPerformed(size_t task_id, bool performed=true) {
    parent_first_task_performed.Clear();
    parent_first_task_performed.Set(task_id, performed);
  }


  /**
   * Input: None.
   *
   * Output: A vector of size_t, containing the amount of each task the organism has performed
   *
   * Purpose: Used to see how many of each task an organism has performed
   */
  const emp::vector<size_t>& GetTaskPerformanceCounts() const { return tasks_performance_count; }
  emp::vector<size_t>& GetTaskPerformanceCounts() { return tasks_performance_count; }
  
  /**
   * Input: size_t task id.
   *
   * Output: size_t, number of times the given task was completed
   *
   * Purpose: Used to check how many times the provided task was performed by this organism
   */
  size_t GetTaskPerformanceCount(size_t task_id) const {
    emp_assert(task_id < tasks_performance_count.size());
    return tasks_performance_count[task_id];
  }

  /**
   * Input: size_t task id.
   *
   * Output: None.
   *
   * Purpose: Used to reset both whether the task was and the number of times the task was completed. 
   */
  void ResetTaskPerformance(size_t task_id) {
    emp_assert(task_id < tasks_performance_count.size());
    tasks_performance_count[task_id] = 0;
    tasks_performed.Set(task_id, false);
    task_outputs_credited[task_id].clear();
    first_task_performed.Set(task_id, false);
  }

  // NOTE - could move these into protected, then write a single wrapper function
  //  Upside: less management
  //  Downside: locked into credit checking
  /**
   * Input: size_t task id.
   *
   * Output: None.
   *
   * Purpose: Marks the CPU state has having completed the task and increments the performance counter
   */
  void MarkTaskPerformed(size_t task_id) {
    emp_assert(task_id < tasks_performed.GetSize());
    emp_assert(task_id < tasks_performance_count.size());
    if (!tasks_performed.Any()) {
      // first_task_performed_id = task_id;
      first_task_performed.Set(task_id, true);
    }
    tasks_performed.Set(task_id, true);
    ++(tasks_performance_count[task_id]);

  }

   /**
   * Input: size_t task id, uint32_t output value
   *
   * Output: Whether the output value has been used before to perform that task.
   *
   * Purpose: Checks whether this is a new way to complete the task that the organism has not done before
   */
  // Has this output value been credited for given task id?
  bool OutputCredited(size_t task_id, uint32_t output_val) const {
    emp_assert(task_id < task_outputs_credited.size());
    return emp::Has(task_outputs_credited[task_id], output_val);
  }

  /**
   * Input: size_t task id 
   *
   * Output: A set containing all outputs that have been credited for the given task
   *
   * Purpose: Used to access all outputs that the organism has been used to perform the task
   */
  const std::set<uint32_t>& GetOutputsCredited(size_t task_id) const {
    emp_assert(task_id < task_outputs_credited.size());
    return task_outputs_credited[task_id];
  }

  // Credit the output value
  /**
   * Input: size_t task id, uint32_t output value
   *
   * Output: None.
   *
   * Purpose: Marks the given output value as being used to perform the given task
   */
  void CreditOutputValue(size_t task_id, uint32_t output_val) {
    emp_assert(task_id < task_outputs_credited.size());
    task_outputs_credited[task_id].emplace(output_val);
  }

   /**
   * Input: size_t task id
   *
   * Output: None.
   *
   * Purpose: Resets the given tasks credited outputs to be empty
   */
  void ResetCreditedOutputs(size_t task_id) {
    task_outputs_credited[task_id].clear();
  }

  /**
   * Input: None.
   *
   * Output: None.
   *
   * Purpose: Resets all tasks credited outputs to be empty
   */
  void ResetCreditedOutputs() {
    for (auto& outputs : task_outputs_credited) {
      outputs.clear();
    }
  }


  size_t GetLineageTaskLossCount(size_t task_id) const {
    return lineage_task_change_loss[task_id];
  }

  const emp::vector<size_t>& GetLineageTaskLoss() const {
    return lineage_task_change_loss;
  }

  size_t GetLineageTaskGainCount(size_t task_id) const {
    return lineage_task_change_gain[task_id];
  }

  const emp::vector<size_t>& GetLineageTaskGain() const {
    return lineage_task_change_gain;
  }

  void SetLineageTaskLossCount(size_t task_id, size_t count) {
    lineage_task_change_loss[task_id] = count;
  }
  void SetLineageTaskGainCount(size_t task_id, size_t count) {
    lineage_task_change_gain[task_id] = count;
  }

  size_t GetLineageTaskConvergeToPartner(size_t task_id) const {
    return lineage_task_converge_partner[task_id];
  }

  size_t GetLineageTaskDivergeFromPartner(size_t task_id) const {
    return lineage_task_diverge_partner[task_id];
  }

  void SetLineageTaskConvergeToPartner(size_t task_id, size_t count) {
    lineage_task_converge_partner[task_id] = count;
  }
  void SetLineageTaskDivergeFromPartner(size_t task_id, size_t count) {
    lineage_task_diverge_partner[task_id] = count;
  }

};

}

#endif