#ifndef SGPHARDWARE_H
#define SGPHARDWARE_H

#include "CPUState.h"
#include "Instructions.h"
#include "GenomeLibrary.h"
// #include "../Tasks.h"
#include "../spec.h"
#include "../../default_mode/Host.h"

#include "sgpl/algorithm/execute_cpu_n_cycles.hpp"
#include "sgpl/hardware/Cpu.hpp"
#include "sgpl/program/Program.hpp"
#include "sgpl/spec/Spec.hpp"
#include "sgpl/utility/ThreadLocalRandom.hpp"
#include "emp/datastructs/set_utils.hpp"

#include <iostream>
#include <string>

namespace sgpmode {


/**
 * Represents the virtual CPU and the program genome for an organism in the SGP
 * mode.
 */
template<typename HW_SPEC_T>
class SGPHardware {
public:
  using this_t = SGPHardware<HW_SPEC_T>;
  using spec_t = HW_SPEC_T;
  using cpu_t = sgpl::Cpu<spec_t>;
  using program_t = sgpl::Program<spec_t>;
  using inst_t = sgpl::Instruction<spec_t>;
  using jump_table_t = sgpl::JumpTable<spec_t, typename spec_t::global_matching_t>;
  using world_t = typename spec_t::world_t;
  using cpu_state_t = CPUState<world_t>;
  using tag_t = typename spec_t::tag_t;

protected:
  cpu_t cpu;
  program_t program;
  cpu_state_t state;       // cpu_t Peripheral

  /**
   * Input: The instruction to print, and the context needed to print it.
   *
   * Output: None
   *
   * Purpose: Prints out the human-readable representation of a single
   * instruction.
   */
  void PrintOp(
    const inst_t& ins,
    const emp::map<std::string, size_t>& arities,
    jump_table_t& table,
    std::ostream& out = std::cout

    // const sgpl::Instruction<HW_SPEC_T>& ins,
    // const emp::map<std::string, size_t>& arities,
    // sgpl::JumpTable<HW_SPEC_T, typename HW_SPEC_T::global_matching_t>& table,
    // std::ostream& out = std::cout
  ) ;

  // Internal helper function for initializing local jump table used by
  // symbulation jump instructions.
  void InitializeLocalJumpTable() {
    // Get global jump table in sgplite cpu
    auto& table = cpu.GetActiveCore().GetGlobalJumpTable();
    auto& state_jump_table = state.GetJumpTable();
    // NOTE - jump table was previously size 100. Seemed like that was because
    //        program size is 100?
    state_jump_table.resize(program.size(), 0);
    size_t idx = 0;
    for (auto& inst : program) {
      const uint8_t inst_opcode = inst.op_code;
      if (emp::Has(lib_info::jump_opcodes, inst_opcode)) {
        const auto entry{table.MatchRegulated(inst.tag)};
        state_jump_table[idx] = (entry.size() > 0) ?
          table.GetVal(entry.front()) :
          idx + 1;
      }
      ++idx;
    }
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Initializes the jump table and task information in the CPUState.
   * Should be called when a new CPU is created or the program is changed.
   */
  // TODO - should this be launching cores? At the moment, it needs to.
  void InitializeState() {
    cpu.InitializeAnchors(program);

    LaunchCPU(state.GetWorld().START_TAG);

    // NOTE - this is awkward: it requires that a CPU core be launched to run.
    //        This means that we need the start tag for any operation that would reset the CPU.
    // Initialize local jump table for program.
    InitializeLocalJumpTable();
  }

public:

  /**
   * Constructs a new CPU for an ancestor organism, with a blank genome.
   */
  SGPHardware(
    emp::Ptr<world_t> world_ptr,
    emp::Ptr<Organism> organism
  ) :
    program(),
    state(
      world_ptr,
      organism,
      world_ptr->GetTaskCount()
    )
  {
    // State constructor (above) will reset cpu state.
    // InitializeState (below) will configure the local jump table using program.
    InitializeState();
  }

  /**
   * Constructs a new CPU with a copy of another CPU's genome.
   */
  SGPHardware(
    emp::Ptr<world_t> world_ptr,
    emp::Ptr<Organism> organism,
    const program_t& program
  ) :
    program(program),
    state(
      world_ptr,
      organism,
      world_ptr->GetTaskCount()
    )
  {
    // State constructor (above) will reset cpu state.
    // InitializeState (below) will configure the local jump table using program.
    InitializeState();
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To destruct the objects belonging to CPU.
   */
  ~SGPHardware() { }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Resets the CPU to its initial state.
   */
  void Reset() {
    Reset(state.GetNumTasks());
  }

  // TODO - is there a reason we might want to support different start tags?
  void Reset(size_t task_cnt) {
    cpu.Reset();
    state.Reset(task_cnt);
    InitializeState();
  }

  void SetProgram(const program_t& new_program) {
    program = new_program;
    Reset();
  }

  // Start a CPU core if none have been started
  void LaunchCPU(const tag_t& start_tag, bool force_launch=false) {
    // If CPU has no active cores or force is true, launch a core.
    if (force_launch || !cpu.HasActiveCore()) {
      cpu.DoLaunchCore(start_tag);
    }
  }

  /**
   * Input: The location of the organism (used for reproduction), and the number
   * of CPU cycles to run. If the organism shouldn't be allowed to reproduce,
   * then the location should be `emp::WorldPosition::invalid_id`.
   *
   * Output: None
   *
   * Purpose: Steps the CPU forward a certain number of cycles.
   */
  void RunCPUStep(const emp::WorldPosition& location, size_t n_cycles=1) {

    // TODO / NOTE - Why set location on every CPU step?
    // -> Moved into ProcessOrg
    // state.SetLocation(location);
    std::cout << "RunCPUStep" << std::endl;
    std::cout << "  Has active core? " << cpu.HasActiveCore() << std::endl;
    sgpl::execute_cpu_n_cycles<spec_t>(n_cycles, cpu, program, state);
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Mutates the genome code stored in the CPU.
   */
  // TODO - move out of hardware?
  // TODO - implement more (configurable) mutation operators
  // NOTE - for now, just pass in the mutation rate
  void Mutate(double mut_rate) {
    std::cout << "Mutate!" << std::endl;
    // TODO - get rid of or define magic number somewhere
    program.ApplyPointMutations(mut_rate * 15.0);
    // InitializeState();
    Reset(); // NOTE - this function was previously just Initializing state,
             // which didn't reset the cpu. I think we want to reset the CPU here also?
  }

  /**
   * Input: None
   *
   * Output: Returns the CPU's program
   *
   * Purpose: To Get the Program of an Organism from its CPU
   */
  const program_t& GetProgram() const { return program; }

  const cpu_state_t& GetCPUState() const { return state; }
  cpu_state_t& GetCPUState() { return state; }

  cpu_t& GetCPU() { return cpu; }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Prints out a human-readable representation of the program code of
   * the organism's genome to the given output stream or standard output.
   */
  // TODO - clean up printing
  void PrintCode(std::ostream& out = std::cout) {
    // TODO - refactor internal/external dependencies of these functions
    //        could also consider shifting this functionality outside of this
    //        class and into a utilities file.
    for (auto i : program) {
      PrintOp(
        i,
        lib_info::arities,
        cpu.GetActiveCore().GetGlobalJumpTable(),
        out
      );
    }
  }

};

template<typename HW_SPEC_T>
void SGPHardware<HW_SPEC_T>::PrintOp(
  const sgpl::Instruction<HW_SPEC_T>& ins,
  const emp::map<std::string, size_t>& arities,
  jump_table_t& table,
  std::ostream& out
) {
  const std::string& name = ins.GetOpName();
  if (arities.count(name)) {
    // Simple instruction
    out << "    " << emp::to_lower(name);
    for (size_t i = 0; i < 12 - name.length(); i++) {
      out << ' ';
    }
    size_t arity = arities.at(name);
    bool first = true;
    for (size_t i = 0; i < arity; i++) {
      if (!first) {
        out << ", ";
      }
      first = false;
      out << 'r' << (int)ins.args[i];
    }
  } else {
    // Jump or anchor with a tag
    // Match the tag to the correct global anchor, then print it out as a
    // 2-letter code AA, AB, etc.
    auto match = table.MatchRegulated(ins.tag);
    std::string tag_name;
    if (match.size()) {
      size_t tag = match.front();
      tag_name += 'A' + tag / 26;
      tag_name += 'A' + tag % 26;
    } else {
      tag_name = "<nowhere>";
    }

    if (name == "JumpIfNEq" || name == "JumpIfLess") {
      out << "    " << emp::to_lower(name);
      for (size_t i = 0; i < 12 - name.length(); i++) {
        out << ' ';
      }
      out << 'r' << (int)ins.args[0] << ", r" << (int)ins.args[1] << ", "
          << tag_name;
    } else if (name == "Global Anchor") {
      out << tag_name << ':';
    } else {
      out << "<unknown " << name << ">";
    }
  }
  out << '\n';
}

/* ------- The following functions weren't being used. --------
TODO - either delete, re-incoporate into hardware, or relocate implementations
*/
/**
 * Input: None
 *
 * Output: A length 64 emp bitset which describes the phenotype of organism
 * such that the ith bit in the bitset marks the completion of task i.
 *
 * Purpose: Get the phenotype of an organism
 */
// emp::BitSet<spec::NUM_TASKS> TasksPerformable() const {
//   // Make a temporary copy of this CPU so that its state isn't clobbered
//   CPU org_cpu = *this;
//   org_cpu.Reset();
//   org_cpu.state.available_dependencies = {1, 1, 1, 1, 1, 1, 1, 1, 1};
//   // Turn off limited resources for this method
//   int old_lim_res = org_cpu.state.world->GetConfig()->LIMITED_RES_TOTAL();
//   org_cpu.state.world->GetConfig()->LIMITED_RES_TOTAL(-1);

//   org_cpu.RunCPUStep(emp::WorldPosition::invalid_id, 400);

//   // and then reset it to the previous value
//   org_cpu.state.world->GetConfig()->LIMITED_RES_TOTAL(old_lim_res);
//   return *org_cpu.state.used_resources;
// }

/*
  * Input: The identifier for a specific task
  *
  * Output: a boolean representing a program's ability to do a specific task
  *
  * Purpose: To return whether or not the organism can perform the given task
  */
// bool CanPerformTask(size_t task_id) const {
//   return TasksPerformable().Get(task_id);
// }



}

#endif
