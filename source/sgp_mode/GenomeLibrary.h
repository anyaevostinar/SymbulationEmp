#ifndef GENOME_LIBRARY
#define GENOME_LIBRARY

#include "Instructions.h"
#include "sgpl/operations/flow_global/Anchor.hpp"
#include "sgpl/program/Program.hpp"
#include "sgpl/spec/Spec.hpp"
#include <cstddef>
#include <limits>

using Library = sgpl::OpLibrary<
    sgpl::Nop<>,
    // single argument math
    inst::ShiftLeft, inst::ShiftRight, inst::Increment, inst::Decrement,
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

// Instead of picking an anchor to start at randomly, start at the anchor that
// has the most bits set by matching with the maximum valued tag. This way
// organisms can evolve to designate a certain anchor as the entry.
const Spec::tag_t START_TAG(std::numeric_limits<uint64_t>::max());
const size_t PROGRAM_LENGTH = 100;

sgpl::Program<Spec> CreateRandomProgram(size_t length) {
  return sgpl::Program<Spec>(length);
}

sgpl::Program<Spec> CreateNotProgram(size_t length) {
  sgpl::Program<Spec> program;
  // Set everything to 0 - this makes them no-ops since that's the first
  // inst in the library
  program.resize(length);
  program[0].op_code = Library::GetOpCode("Global Anchor");
  program[0].tag = START_TAG;

  // sharedio   r0
  // nand       r0, r0, r0
  // sharedio   r0
  // reproduce
  program[length-4].op_code = Library::GetOpCode("SharedIO");
  program[length-3].op_code = Library::GetOpCode("Nand");
  program[length-2].op_code = Library::GetOpCode("SharedIO");
  program[length-1].op_code = Library::GetOpCode("Reproduce");
  return program;
}

sgpl::Program<Spec> CreateSquareProgram(size_t length) {
  sgpl::Program<Spec> program;
  // Set everything to 0 - this makes them no-ops since that's the first
  // inst in the library
  program.resize(length);
  program[0].op_code = Library::GetOpCode("Global Anchor");
  program[0].tag = START_TAG;

  // Always output 4:
  // pop        r0
  // increment  r0          -> 1
  // add        r0, r0, r0  -> 2
  // add        r0, r0, r0  -> 4
  // sharedio   r0
  // reproduce
  program[length-6].op_code = Library::GetOpCode("Pop");
  program[length-5].op_code = Library::GetOpCode("Increment");
  program[length-4].op_code = Library::GetOpCode("Add");
  program[length-3].op_code = Library::GetOpCode("Add");
  program[length-2].op_code = Library::GetOpCode("SharedIO");
  program[length-1].op_code = Library::GetOpCode("Reproduce");
  return program;
}

sgpl::Program<Spec> CreateStartProgram(emp::Ptr<SymConfigBase> config) {
  if (config->RANDOM_ANCESTOR()) {
    return CreateRandomProgram(PROGRAM_LENGTH);
  } else if (config->TASK_TYPE() == 1) {
    return CreateNotProgram(PROGRAM_LENGTH);
  } else {
    return CreateSquareProgram(PROGRAM_LENGTH);
  }
}

#endif