#ifndef SGPCPU_H
#define SGPCPU_H

#include "../default_mode/Host.h"
#include "CPUState.h"
#include "Instructions.h"
#include "SGPWorld.h"
#include "Tasks.h"
#include "sgpl/algorithm/execute_cpu.hpp"
#include "sgpl/hardware/Cpu.hpp"
#include "sgpl/library/OpLibraryCoupler.hpp"
#include "sgpl/library/prefab/ArithmeticOpLibrary.hpp"
#include "sgpl/library/prefab/ControlFlowOpLibrary.hpp"
#include "sgpl/operations/flow_global/Anchor.hpp"
#include "sgpl/operations/unary/Increment.hpp"
#include "sgpl/operations/unary/Terminal.hpp"
#include "sgpl/program/Program.hpp"
#include "sgpl/spec/Spec.hpp"
#include "sgpl/utility/ThreadLocalRandom.hpp"
#include <cmath>
#include <iostream>
#include <limits>
#include <string>

/**
 * Represents the virtual CPU and the program genome for an organism in the SGP
 * mode.
 */
class CPU {
  using Library =
      sgpl::OpLibrary<sgpl::Nop<>,
                      // single argument math
                      inst::ShiftLeft, inst::ShiftRight, inst::Increment,
                      inst::Decrement,
                      // biological operations
                      // no copy or alloc
                      inst::Reproduce, inst::PrivateIO, inst::SharedIO,
                      // double argument math
                      inst::Add, inst::Subtract, inst::Nand,
                      // Stack manipulation
                      inst::Push, inst::Pop, inst::SwapStack, inst::Swap,
                      // no h-search
                      inst::Donate, inst::JumpIfNEq, inst::JumpIfLess, inst::Reuptake,
                      // if-label doesn't make sense for SGP, same with *-head
                      // and set-flow but this is required
                      sgpl::global::Anchor>;

  using Spec = sgpl::Spec<Library, CPUState>;

  sgpl::Cpu<Spec> cpu;
  sgpl::Program<Spec> program;
  emp::Ptr<emp::Random> random;
  // Instead of picking an anchor to start at randomly, start at the anchor that
  // has the most bits set by matching with the maximum valued tag. This way
  // organisms can evolve to designate a certain anchor as the entry.
  Spec::tag_t start_tag;

public:
  CPUState state;

  CPU(emp::Ptr<Organism> organism, emp::Ptr<SGPWorld> world,
      emp::Ptr<emp::Random> random)
      : random(random), start_tag(std::numeric_limits<uint64_t>::max()),
        state(organism, world) {
    if (world->GetConfig()->RANDOM_ANCESTOR()) {
      program = sgpl::Program<Spec>(100);
    } else {
      // Set everything to 0 - this makes them no-ops since that's the first
      // inst in the library
      program.resize(100);
      program[0].op_code = Library::GetOpCode("Global Anchor");
      program[0].tag = start_tag;

      // sharedio   r0
      // nand       r0, r0, r0
      // sharedio   r0
      // reproduce
      program[96].op_code = Library::GetOpCode("SharedIO");
      program[97].op_code = Library::GetOpCode("Nand");
      program[98].op_code = Library::GetOpCode("SharedIO");
      program[99].op_code = Library::GetOpCode("Reproduce");
    }

    Mutate();
    state.self_completed.resize(world->GetTaskSet().NumTasks());
  }

  CPU(emp::Ptr<Organism> organism, emp::Ptr<SGPWorld> world,
      emp::Ptr<emp::Random> random, CPU &old_cpu)
      : program(old_cpu.program), random(random), state(organism, world) {
    cpu.InitializeAnchors(program);
    state.self_completed.resize(world->GetTaskSet().NumTasks());
  }

  sgpl::Program<Spec> &GetProgram() { return program; }
  
  void SetProgram(sgpl::Program<Spec> &new_program){
      this->program = new_program;
  };

  CPUState GetState(){return  state;}

  void RunCPUStep(emp::WorldPosition location, size_t nCycles) {
    if (!cpu.HasActiveCore()) {
      cpu.DoLaunchCore(start_tag);
    }

    state.location = location;

    sgpl::execute_cpu<Spec>(nCycles, cpu, program, state);
  }

  void Mutate() {
    program.ApplyPointMutations(0.03);
    cpu.InitializeAnchors(program);
  }
  


  void PrintOp(sgpl::Instruction<Spec> &ins,
               emp::map<std::string, size_t> &arities,
               emp::map<size_t, std::string> &labels,
               sgpl::JumpTable<Spec, Spec::global_matching_t> &table) {
    const std::string &name = ins.GetOpName();
    if (arities.count(name)) {
      // Simple instruction
      std::cout << "    " << emp::to_lower(name);
      for (size_t i = 0; i < 12 - name.length(); i++) {
        std::cout << ' ';
      }
      size_t arity = arities[name];
      bool first = true;
      for (size_t i = 0; i < arity; i++) {
        if (!first) {
          std::cout << ", ";
        }
        first = false;
        std::cout << 'r' << (int)ins.args[i];
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
        std::cout << "    " << emp::to_lower(name);
        for (size_t i = 0; i < 12 - name.length(); i++) {
          std::cout << ' ';
        }
        std::cout << 'r' << (int)ins.args[0] << ", r" << (int)ins.args[1]
                  << ", " << tag_name;
      } else if (name == "Global Anchor") {
        std::cout << tag_name << ':';
      } else {
        std::cout << "<unknown " << name << ">";
      }
    }
    std::cout << '\n';
  }

  void PrintCode() {
    emp::map<std::string, size_t> arities{
        {"Nop-0", 0},     {"ShiftLeft", 1}, {"ShiftRight", 1}, {"Increment", 1},
        {"Decrement", 1}, {"Push", 1},      {"Pop", 1},        {"SwapStack", 0},
        {"Swap", 2},      {"Add", 3},       {"Subtract", 3},   {"Nand", 3},
        {"Reproduce", 0}, {"PrivateIO", 1},    {"SharedIO", 1},   {"Donate", 0}, {"Reuptake", 1}};
    emp::map<size_t, std::string> labels;

    std::cout << "--------" << std::endl;
    for (auto i : program) {
      PrintOp(i, arities, labels, cpu.GetActiveCore().GetGlobalJumpTable());
    }
  }
};

#endif