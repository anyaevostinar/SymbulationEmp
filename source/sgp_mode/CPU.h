#ifndef SGPCPU_H
#define SGPCPU_H

#include "../default_mode/Host.h"
#include "CPUState.h"
#include "GenomeLibrary.h"
#include "Instructions.h"
#include "SGPWorld.h"
#include "Tasks.h"
#include "sgpl/algorithm/execute_cpu_n_cycles.hpp"
#include "sgpl/hardware/Cpu.hpp"
#include "sgpl/program/Program.hpp"
#include "sgpl/spec/Spec.hpp"
#include "sgpl/utility/ThreadLocalRandom.hpp"
#include <iostream>
#include <string>

/**
 * Represents the virtual CPU and the program genome for an organism in the SGP
 * mode.
 */
class CPU {
  sgpl::Cpu<Spec> cpu;
  sgpl::Program<Spec> program;

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

    
    uint8_t GlobalJumpNot = Library::GetOpCode("Global Jump If Not");
    
    //uint8_t JumpLess = Library::GetOpCode("JumpIfLess");
    if (!cpu.HasActiveCore()) {
      cpu.DoLaunchCore(START_TAG);
    }
    auto &table = cpu.GetActiveCore().GetGlobalJumpTable();
    size_t idx = 0;
    state.jump_table.resize(100);
    for (auto &i : program) {
      if (i.op_code == GlobalJumpNot) {
        auto entry = table.MatchRegulated(i.tag);
        state.jump_table[idx] =
            entry.size() > 0 ? table.GetVal(entry.front()) : idx + 1;
      }
      idx++;
    }
  }

public:
  CPUState state;

  sgpl::Cpu<Spec> GetCPUPointer() {
    return cpu;
  }

  /**
   * Constructs a new CPU for an ancestor organism, with either a random genome
   * or a blank genome that knows how to do a simple task depending on the
   * config setting RANDOM_ANCESTOR.
   */
  CPU(emp::Ptr<Organism> organism, emp::Ptr<SGPWorld> world)
      : program(CreateStartProgram(world->GetConfig())),
        state(organism, world) {
    InitializeState();
  }

  /**
   * Constructs a new CPU with a copy of another CPU's genome.
   */
  CPU(emp::Ptr<Organism> organism, emp::Ptr<SGPWorld> world,
      const sgpl::Program<Spec> &program)
      : program(program), state(organism, world) {
    InitializeState();
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To destruct the objects belonging to CPU.
   */
  ~CPU() {
    state.parent_tasks_performed.Delete();
    state.tasks_performed.Delete();
    state.parent_or_current_tasks_performed.Delete();
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Resets the CPU to its initial state.
   */
  void Reset() {
    cpu.Reset();
    state = CPUState(state.organism, state.world);
    InitializeState();
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
  void RunCPUStep(emp::WorldPosition location, size_t n_cycles) {
    if (!cpu.HasActiveCore()) {
      cpu.DoLaunchCore(START_TAG);
    }

    state.location = location;

    
    sgpl::execute_cpu_n_cycles<Spec>(n_cycles, cpu, program, state);

    state.cpu_cycles_since_repro += n_cycles;
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Mutates the genome code stored in the CPU.
   */
  void Mutate() {
    program.ApplyPointMutations(state.world->GetConfig()->MUTATION_SIZE() *
                                15.0);
    InitializeState();
  }

  /**
   * Input: None
   *
   * Output: Returns the CPU's program
   *
   * Purpose: To Get the Program of an Organism from its CPU
   */
  const sgpl::Program<Spec> &GetProgram() const { return program; }

  /**
   * Input: None
   *
   * Output: A length 64 emp bitset which describes the phenotype of organism
   * such that the ith bit in the bitset marks the completion of task i.
   *
   * Purpose: Get the phenotype of an organism
   */
  // emp::BitSet<CPU_BITSET_LENGTH> TasksPerformable() const {
  //   // Make a temporary copy of this CPU so that its state isn't clobbered
  //   CPU org_cpu = *this;
  //   org_cpu.Reset();
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

private:
  /**
   * Input: The instruction to print, and the context needed to print it.
   *
   * Output: None
   *
   * Purpose: Prints out the human-readable representation of a single
   * instruction.
   */
  void PrintOp(const sgpl::Instruction<Spec> &ins,
               const emp::map<std::string, size_t> &arities,
               sgpl::JumpTable<Spec, Spec::global_matching_t> &table,
               std::ostream &out = std::cout) const {
    const std::string &name = ins.GetOpName();
    if (arities.count(name)) {
      // Simple instruction
      out << "    " << emp::to_lower(name);
      for (size_t i = 0; i < 17 - name.length(); i++) {
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

      if (name == "Global Jump If Not" || name == "Local Jump If Not") {
        std::string small_name = "";
        if(name == "Global Jump If Not"){
          small_name = "global jumpifnot";
        }
        else{
          small_name = "local jumpifnot";
        }
        
        out << "    " << emp::to_lower(small_name);
        for (size_t i = 0; i < 17 - small_name.length(); i++) {
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

public:
  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Prints out a human-readable representation of the program code of
   * the organism's genome to the given output stream or standard output.
   */
  void PrintCode(std::ostream &out = std::cout) {
    emp::map<std::string, size_t> arities{
        {"Nop-0", 0},{"Bitwise Shift", 1}, {"Increment", 1},
        {"Decrement", 1}, {"Add", 3}, {"Subtract", 3}, 
        {"Nand", 3},{"Reproduce", 0}, {"SharedIO", 1},{"Donate",0},{"Steal",0}};
      

    for (auto i : program) {
      PrintOp(i, arities, cpu.GetActiveCore().GetGlobalJumpTable(), out);
    }
  }
};

#endif
