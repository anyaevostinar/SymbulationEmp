#ifndef GENOME_LIBRARY
#define GENOME_LIBRARY

#include "Instructions.h"

#include "sgpl/operations/flow_global/Anchor.hpp"
#include "sgpl/program/Instruction.hpp"
#include "sgpl/program/Program.hpp"

#include <cstddef>
#include <limits>
#include <unordered_set>

namespace sgpmode {

// TODO - re-add all instructions
using Library = sgpl::OpLibrary<
  sgpl::Nop<>,
  inst::Increment,
  inst::Decrement,
  sgpl::global::Anchor
>;

namespace lib_info {
  const std::unordered_set<uint8_t> jump_opcodes = {
    // Library::GetOpCode("JumpIfNEq"),
    // Library::GetOpCode("JumpIfLess")
    // Library::GetOpCode("JumpEq")
  };

  const emp::map<std::string, size_t> arities {
    {"Nop-0", 0},     {"ShiftLeft", 1}, {"ShiftRight", 1}, {"Increment", 1},
    {"Decrement", 1}, {"Push", 1},      {"Pop", 1},        {"SwapStack", 0},
    {"Swap", 2},      {"Add", 3},       {"Subtract", 3},   {"Nand", 3},
    {"Reproduce", 0}, {"PrivateIO", 1}, {"SharedIO", 1},   {"Donate", 0},
    {"Reuptake", 1},  {"Steal", 0},     {"Infect", 0}, {"DynamicInst", 3}
  };
}

// Unused instructions:
//  inst::PrivateIO,
//  inst::Donate,
//  inst::Reuptake,
//  inst::Infect,
//  inst::DynamicInst
//  inst::Steal

}

#endif