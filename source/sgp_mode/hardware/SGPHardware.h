#ifndef SGPHARDWARE_H
#define SGPHARDWARE_H

#include "CPUState.h"
#include "Instructions.h"
#include "../GenomeLibrary.h"
#include "../Tasks.h"
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
  using spec_t = HW_SPEC_T;
  using cpu_t = sgpl::Cpu<spec_t>;
  using program_t = sgpl::Program<spec_t>;
  using inst_t = sgpl::Instruction<spec_t>;
  using jump_table_t = sgpl::JumpTable<spec_t, spec_t::global_matching_t>;
  using cpu_state_t = CPUState<spec_t>; // <WORLD_T>;
  // using world_t = WORLD_T;

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
    const jump_table_t& table,
    std::ostream& out = std::cout
  ) const;

  // Internal helper function for initializing local jump table used by
  // symbulation jump instructions.
  void InitializeLocalJumpTable() {
    // Get global jump table in sgplite cpu
    const auto& table = cpu.GetActiveCore().GetGlobalJumpTable();
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
  void InitializeState() {
    cpu.InitializeAnchors(program);

    // If CPU has no active cores, launch a core.
    if (!cpu.HasActiveCore()) {
      // TODO - move START_TAG definition into spec
      cpu.DoLaunchCore(START_TAG);
    }

    // Initialize local jump table for program.
    InitializeLocalJumpTable();

  }

public:

  /**
   * Constructs a new CPU for an ancestor organism, with a blank genome.
   */
  SGPHardware(
    emp::Ptr<Organism> organism,
    size_t task_cnt
  ) :
    program(),
    state(organism, task_cnt)
  {
    // State constructor (above) will reset cpu state.
    // InitializeState (below) will configure the local jump table using program.
    InitializeState();
  }

  /**
   * Constructs a new CPU with a copy of another CPU's genome.
   */
  SGPHardware(
    emp::Ptr<Organism> organism,
    size_t task_cnt,
    const sgpl::Program<Spec>& program,
  ) :
    program(program),
    state(organism, task_cnt)
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

  void Reset(size_t task_cnt) {
    cpu.Reset();
    state.Reset(task_cnt);
    InitializeState();
  }

  void SetProgram(const sgpl::Program<Spec>& new_program) {
    program = new_program;
    Reset();
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
  void RunCPUStep(const emp::WorldPosition& location, size_t n_cycles) {
    if (!cpu.HasActiveCore()) {
      cpu.DoLaunchCore(START_TAG);
    }

    state.SetLocation(location);

    sgpl::execute_cpu_n_cycles<Spec>(n_cycles, cpu, program, state);
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Mutates the genome code stored in the CPU.
   */
  // TODO - move out of hardware?
  // NOTE - for now, just pass in the mutation rate
  void Mutate(double mut_rate) {
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

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Prints out a human-readable representation of the program code of
   * the organism's genome to the given output stream or standard output.
   */
  void PrintCode(std::ostream& out = std::cout) {
    // TODO - refactor internal/external dependencies of these functions
    //        could also consider shifting this functionality outside of this
    //        class and into a utilities file.
    for (const auto& i : program) {
      PrintOp(
        i,
        lib_info::arities,
        cpu.GetActiveCore().GetGlobalJumpTable(),
        out
      );
    }
  }

};

void CPU::PrintOp(
  const sgpl::Instruction<Spec>& ins,
  const emp::map<std::string, size_t>& arities,
  sgpl::JumpTable<Spec, Spec::global_matching_t>& table,
  std::ostream& out = std::cout
) const {
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
