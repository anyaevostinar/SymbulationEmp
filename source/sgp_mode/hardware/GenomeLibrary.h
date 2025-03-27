#ifndef GENOME_LIBRARY
#define GENOME_LIBRARY

#include "Instructions.h"

#include "sgpl/program/Instruction.hpp"
#include "sgpl/program/Program.hpp"
#include "sgpl/library/OpLibrary.hpp"
#include "sgpl/operations/operations.hpp"

#include <cstddef>
#include <limits>
#include <unordered_set>
#include <set>

namespace sgpmode {

// NOTE - Discuss what instructions that we'd like to include
using Library = sgpl::OpLibrary<
  sgpl::Nop<>,
  inst::Increment,
  inst::Decrement,
  sgpl::BitwiseShift, // NOTE - replaced ShiftLeft/ShiftRight with BitwiseShift instruction
  sgpl::Add,
  sgpl::Subtract,
  // inst::Add,
  // inst::Subtract,
  inst::Nand,
  inst::Push,
  inst::Pop,
  inst::SwapStack,
  inst::Swap,
  inst::Reproduce,
  inst::IO,
  inst::JumpIfNEq,
  inst::JumpIfLess,
  inst::JumpIfEq,
  inst::Donate,
  inst::Steal,
  inst::Infect,
  sgpl::global::Anchor
>;

namespace lib_info {
  const emp::map<std::string, size_t> arities {
    {"Nop-0", 0},     {"ShiftLeft", 1}, {"ShiftRight", 1}, {"Increment", 1},
    {"Decrement", 1}, {"Push", 1},      {"Pop", 1},        {"SwapStack", 0},
    {"Swap", 2},      {"Add", 3},       {"Subtract", 3},   {"Nand", 3},
    {"Reproduce", 0}, {"PrivateIO", 1}, {"SharedIO", 1},   {"Donate", 0},
    {"Reuptake", 1},  {"Steal", 0},     {"Infect", 0}, {"DynamicInst", 3},
    {"IO", 1}, {"JumpIfNEq", 2}, {"JumpIfEq", 2}, {"JumpIfLess", 2}
  };
}

// Unused instructions:
//  inst::PrivateIO,
//  inst::Donate,
//  inst::Reuptake,
//  inst::Infect,
//  inst::DynamicInst
//  inst::Steal
template<typename Iter>
void del_inst(
  Iter begin,
  Iter end,
  const unsigned char inst,
  const unsigned char num_inst
) {
  unsigned char current = 0;
  for (; begin != end; ++begin) {
    if (*begin == inst) {
      if (current == inst)
        current = (current + 1) % num_inst;
      *begin = current;
      current = (current + 1) % num_inst;
    }
  }
}

}

#endif