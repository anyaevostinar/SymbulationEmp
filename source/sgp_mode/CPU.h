#ifndef SGPCPU_H
#define SGPCPU_H

#include "../default_mode/Host.h"
#include "CPUState.h"
#include "GenomeLibrary.h"
#include "Instructions.h"
#include "SGPWorld.h"
#include "Tasks.h"
#include "sgpl/algorithm/execute_cpu.hpp"
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
  emp::Ptr<emp::Random> random;

public:
  CPUState state;

  /**
   * Constructs a new CPU for an ancestor organism, with either a random genome
   * or a blank genome that knows how to do a simple task depending on the
   * config setting RANDOM_ANCESTOR.
   */
  CPU(emp::Ptr<Organism> organism, emp::Ptr<SGPWorld> world,
      emp::Ptr<emp::Random> random)
      : program(CreateStartProgram(world->GetConfig())), random(random),
        state(organism, world) {
    cpu.InitializeAnchors(program);
    state.self_completed.resize(world->GetTaskSet().NumTasks());
    state.shared_completed->resize(world->GetTaskSet().NumTasks());
  }

  /**
   * Constructs a new CPU with a copy of another CPU's genome.
   */
  CPU(emp::Ptr<Organism> organism, emp::Ptr<SGPWorld> world,
      emp::Ptr<emp::Random> random, const sgpl::Program<Spec> &program)
      : program(program), random(random), state(organism, world) {
    cpu.InitializeAnchors(program);
    state.self_completed.resize(world->GetTaskSet().NumTasks());
    state.shared_completed->resize(world->GetTaskSet().NumTasks());
  }

  void Reset() {
    cpu.Reset();
    cpu.InitializeAnchors(program);
    state = CPUState(state.host, state.world);
    state.self_completed.resize(state.world->GetTaskSet().NumTasks());
    state.shared_completed->resize(state.world->GetTaskSet().NumTasks());
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
  void RunCPUStep(emp::WorldPosition location, size_t nCycles) {
    if (!cpu.HasActiveCore()) {
      cpu.DoLaunchCore(START_TAG);
    }

    state.location = location;

    sgpl::execute_cpu<Spec>(nCycles, cpu, program, state);
  }
  
  sgpl::Cpu<Spec> Getcpu(){
    return cpu;
  }
  sgpl::Program<Spec> GetProgram(){
    return program;
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
    cpu.InitializeAnchors(program);
  }

  const sgpl::Program<Spec> &GetProgram() const { return program; }

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
               sgpl::JumpTable<Spec, Spec::global_matching_t> &table) const {
    const std::string &name = ins.GetOpName();
    if (arities.count(name)) {
      // Simple instruction
      std::cout << "    " << emp::to_lower(name);
      for (size_t i = 0; i < 12 - name.length(); i++) {
        std::cout << ' ';
      }
      size_t arity = arities.at(name);
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

public:
  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Prints out a human-readable representation of the program code of
   * the organism's genome to standard output.
   */
  void PrintCode() {
    emp::map<std::string, size_t> arities{
        {"Nop-0", 0},     {"ShiftLeft", 1}, {"ShiftRight", 1}, {"Increment", 1},
        {"Decrement", 1}, {"Push", 1},      {"Pop", 1},        {"SwapStack", 0},
        {"Swap", 2},      {"Add", 3},       {"Subtract", 3},   {"Nand", 3},
        {"Reproduce", 0}, {"PrivateIO", 1}, {"SharedIO", 1},   {"Donate", 0},
        {"Reuptake", 1}};

    std::cout << "--------" << std::endl;
    for (auto i : program) {
      PrintOp(i, arities, cpu.GetActiveCore().GetGlobalJumpTable());
    }
  }
};

#endif