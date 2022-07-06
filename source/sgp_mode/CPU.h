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
#include <string>

/**
 * Represents the virtual CPU and the program genome for an organism in the SGP
 * mode.
 */
class CPU {
  using Library =
      sgpl::OpLibrary<sgpl::Nop<>, inst::JumpIfNEq, inst::JumpIfLess,
                      // if-label doesn't make sense for SGP, same with *-head
                      // and set-flow but this is required
                      sgpl::global::Anchor,
                      // single argument math
                      inst::ShiftLeft, inst::ShiftRight, inst::Increment,
                      inst::Decrement,
                      // Stack manipulation
                      inst::Push, inst::Pop, inst::SwapStack, inst::Swap,
                      // double argument math
                      inst::Add, inst::Subtract, inst::Nand,
                      // biological operations
                      // no copy or alloc
                      inst::Reproduce, inst::IO,
                      // no h-search
                      inst::Donate>;

  using Spec = sgpl::Spec<Library, CPUState>;

  sgpl::Cpu<Spec> cpu;
  sgpl::Program<Spec> program;
  emp::Ptr<emp::Random> random;

public:
  CPUState state;

  CPU(emp::Ptr<Organism> organism, emp::Ptr<SGPWorld> world,
      emp::Ptr<emp::Random> random)
      : program(100), random(random), state(organism, world) {
    cpu.InitializeAnchors(program);
  }

  CPU(emp::Ptr<Organism> organism, emp::Ptr<SGPWorld> world,
      emp::Ptr<emp::Random> random, CPU &old_cpu)
      : program(old_cpu.program), random(random), state(organism, world) {
    cpu.InitializeAnchors(program);
  }

  void RunCPUStep(emp::WorldPosition location, size_t nCycles = 1) {
    // Generate random signals to launch available virtual cores
    while (cpu.TryLaunchCore(emp::BitSet<64>(random))) {
    }

    state.location = location;

    sgpl::execute_cpu<Spec>(nCycles, cpu, program, state);
  }

  void Mutate() { program.ApplyPointMutations(0.01); }

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
      if (match.size()) {
        size_t tag = match.front();
        std::string tag_name;
        tag_name += 'A' + tag / 26;
        tag_name += 'A' + tag % 26;
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
      } else {
        std::cout << "<illegal instruction tag>";
      }
    }
    std::cout << '\n';
  }

  void PrintCode() {
    emp::map<std::string, size_t> arities{
        std::pair("Nop-0", 0),      std::pair("ShiftLeft", 1),
        std::pair("ShiftRight", 1), std::pair("Increment", 1),
        std::pair("Decrement", 1),  std::pair("Push", 1),
        std::pair("Pop", 1),        std::pair("SwapStack", 0),
        std::pair("Swap", 2),       std::pair("Add", 3),
        std::pair("Subtract", 3),   std::pair("Nand", 3),
        std::pair("Reproduce", 0),  std::pair("IO", 1),
        std::pair("Donate", 0)};
    emp::map<size_t, std::string> labels;

    std::cout << "--------" << std::endl;
    for (auto i : program) {
      PrintOp(i, arities, labels, cpu.GetActiveCore().GetGlobalJumpTable());
    }
  }
};

#endif