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
  program[length - 4].op_code = Library::GetOpCode("SharedIO");
  program[length - 3].op_code = Library::GetOpCode("Nand");
  program[length - 2].op_code = Library::GetOpCode("SharedIO");
  program[length - 1].op_code = Library::GetOpCode("Reproduce");
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
  program[length - 6].op_code = Library::GetOpCode("Pop");
  program[length - 5].op_code = Library::GetOpCode("Increment");
  program[length - 4].op_code = Library::GetOpCode("Add");
  program[length - 3].op_code = Library::GetOpCode("Add");
  program[length - 2].op_code = Library::GetOpCode("SharedIO");
  program[length - 1].op_code = Library::GetOpCode("Reproduce");
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

// Other logic tasks

sgpl::Program<Spec> CreateNandProgram(size_t length) {
  sgpl::Program<Spec> program;
  // Set everything to 0 - this makes them no-ops since that's the first
  // inst in the library
  program.resize(length);
  program[0].op_code = Library::GetOpCode("Global Anchor");
  program[0].tag = START_TAG;

  // sharedio   r0
  // sharedio   r1
  // nand       r0, r1, r0
  // sharedio   r0
  // reproduce
  program[length - 5].op_code = Library::GetOpCode("SharedIO");
  program[length - 4].op_code = Library::GetOpCode("SharedIO");
  program[length - 4].args[0] = 1;
  program[length - 3].op_code = Library::GetOpCode("Nand");
  program[length - 3].args[1] = 1;
  program[length - 2].op_code = Library::GetOpCode("SharedIO");
  program[length - 1].op_code = Library::GetOpCode("Reproduce");
  return program;
}

sgpl::Program<Spec> CreateAndProgram(size_t length) {
  sgpl::Program<Spec> program;
  // Set everything to 0 - this makes them no-ops since that's the first
  // inst in the library
  program.resize(length);
  program[0].op_code = Library::GetOpCode("Global Anchor");
  program[0].tag = START_TAG;

  // ~(a nand b)
  // sharedio   r0
  // sharedio   r1
  // nand       r0, r1, r0
  // nand       r0, r0, r0
  // sharedio   r0
  // reproduce
  program[length - 6].op_code = Library::GetOpCode("SharedIO");
  program[length - 5].op_code = Library::GetOpCode("SharedIO");
  program[length - 5].args[0] = 1;
  program[length - 4].op_code = Library::GetOpCode("Nand");
  program[length - 4].args[1] = 1;
  program[length - 3].op_code = Library::GetOpCode("Nand");
  program[length - 2].op_code = Library::GetOpCode("SharedIO");
  program[length - 1].op_code = Library::GetOpCode("Reproduce");
  return program;
}

sgpl::Program<Spec> CreateOrnProgram(size_t length) {
  sgpl::Program<Spec> program;
  // Set everything to 0 - this makes them no-ops since that's the first
  // inst in the library
  program.resize(length);
  program[0].op_code = Library::GetOpCode("Global Anchor");
  program[0].tag = START_TAG;

  // (~a) nand b
  // sharedio   r0
  // sharedio   r1
  // nand       r0, r0, r0
  // nand       r0, r1, r0
  // sharedio   r0
  // reproduce
  program[length - 6].op_code = Library::GetOpCode("SharedIO");
  program[length - 5].op_code = Library::GetOpCode("SharedIO");
  program[length - 5].args[0] = 1;
  program[length - 4].op_code = Library::GetOpCode("Nand");
  program[length - 3].op_code = Library::GetOpCode("Nand");
  program[length - 3].args[1] = 1;
  program[length - 2].op_code = Library::GetOpCode("SharedIO");
  program[length - 1].op_code = Library::GetOpCode("Reproduce");
  return program;
}

sgpl::Program<Spec> CreateOrProgram(size_t length) {
  sgpl::Program<Spec> program;
  // Set everything to 0 - this makes them no-ops since that's the first
  // inst in the library
  program.resize(length);
  program[0].op_code = Library::GetOpCode("Global Anchor");
  program[0].tag = START_TAG;

  // (~a) nand (~b)
  // sharedio   r0
  // sharedio   r1
  // nand       r0, r0, r0
  // nand       r1, r1, r1
  // nand       r0, r1, r0
  // sharedio   r0
  // reproduce
  program[length - 7].op_code = Library::GetOpCode("SharedIO");
  program[length - 6].op_code = Library::GetOpCode("SharedIO");
  program[length - 6].args[0] = 1;
  program[length - 5].op_code = Library::GetOpCode("Nand");
  program[length - 4].op_code = Library::GetOpCode("Nand");
  program[length - 4].args = {1, 1, 1};
  program[length - 3].op_code = Library::GetOpCode("Nand");
  program[length - 3].args[1] = 1;
  program[length - 2].op_code = Library::GetOpCode("SharedIO");
  program[length - 1].op_code = Library::GetOpCode("Reproduce");
  return program;
}

sgpl::Program<Spec> CreateAndnProgram(size_t length) {
  sgpl::Program<Spec> program;
  // Set everything to 0 - this makes them no-ops since that's the first
  // inst in the library
  program.resize(length);
  program[0].op_code = Library::GetOpCode("Global Anchor");
  program[0].tag = START_TAG;

  // ~(a nand (~b))
  // sharedio   r0
  // sharedio   r1
  // nand       r1, r1, r1
  // nand       r0, r1, r0
  // nand       r0, r0, r0
  // sharedio   r0
  // reproduce
  program[length - 7].op_code = Library::GetOpCode("SharedIO");
  program[length - 6].op_code = Library::GetOpCode("SharedIO");
  program[length - 6].args[0] = 1;
  program[length - 5].op_code = Library::GetOpCode("Nand");
  program[length - 5].args = {1, 1, 1};
  program[length - 4].op_code = Library::GetOpCode("Nand");
  program[length - 4].args[1] = 1;
  program[length - 3].op_code = Library::GetOpCode("Nand");
  program[length - 2].op_code = Library::GetOpCode("SharedIO");
  program[length - 1].op_code = Library::GetOpCode("Reproduce");
  return program;
}

sgpl::Program<Spec> CreateNorProgram(size_t length) {
  sgpl::Program<Spec> program;
  // Set everything to 0 - this makes them no-ops since that's the first
  // inst in the library
  program.resize(length);
  program[0].op_code = Library::GetOpCode("Global Anchor");
  program[0].tag = START_TAG;

  // ~((~a) nand (~b))
  // sharedio   r0
  // sharedio   r1
  // nand       r0, r0, r0
  // nand       r1, r1, r1
  // nand       r0, r1, r0
  // nand       r0, r0, r0
  // sharedio   r0
  // reproduce
  program[length - 8].op_code = Library::GetOpCode("SharedIO");
  program[length - 7].op_code = Library::GetOpCode("SharedIO");
  program[length - 7].args[0] = 1;
  program[length - 6].op_code = Library::GetOpCode("Nand");
  program[length - 5].op_code = Library::GetOpCode("Nand");
  program[length - 5].args = {1, 1, 1};
  program[length - 4].op_code = Library::GetOpCode("Nand");
  program[length - 4].args[1] = 1;
  program[length - 3].op_code = Library::GetOpCode("Nand");
  program[length - 2].op_code = Library::GetOpCode("SharedIO");
  program[length - 1].op_code = Library::GetOpCode("Reproduce");
  return program;
}

sgpl::Program<Spec> CreateXorProgram(size_t length) {
  sgpl::Program<Spec> program;
  // Set everything to 0 - this makes them no-ops since that's the first
  // inst in the library
  program.resize(length);
  program[0].op_code = Library::GetOpCode("Global Anchor");
  program[0].tag = START_TAG;

  // (a & ~b) | (~a & b) --> (a nand ~b) nand (~a nand b)
  // sharedio   r0
  // sharedio   r1
  //
  // nand       r3, r1, r1
  // nand       r3, r3, r0
  //
  // nand       r2, r0, r0
  // nand       r2, r2, r1
  //
  // nand       r0, r2, r3
  // sharedio   r0
  // reproduce
  program[length - 9].op_code = Library::GetOpCode("SharedIO");
  program[length - 8].op_code = Library::GetOpCode("SharedIO");
  program[length - 8].args[0] = 1;

  program[length - 7].op_code = Library::GetOpCode("Nand");
  program[length - 7].args = {3, 1, 1};
  program[length - 6].op_code = Library::GetOpCode("Nand");
  program[length - 6].args = {3, 3, 0};

  program[length - 5].op_code = Library::GetOpCode("Nand");
  program[length - 5].args = {2, 0, 0};
  program[length - 4].op_code = Library::GetOpCode("Nand");
  program[length - 4].args = {2, 2, 1};

  program[length - 3].op_code = Library::GetOpCode("Nand");
  program[length - 3].args = {0, 2, 3};
  program[length - 2].op_code = Library::GetOpCode("SharedIO");
  program[length - 1].op_code = Library::GetOpCode("Reproduce");
  return program;
}

sgpl::Program<Spec> CreateEquProgram(size_t length) {
  sgpl::Program<Spec> program;
  // Set everything to 0 - this makes them no-ops since that's the first
  // inst in the library
  program.resize(length);
  program[0].op_code = Library::GetOpCode("Global Anchor");
  program[0].tag = START_TAG;

  // ~(a ^ b)
  // sharedio   r0
  // sharedio   r1
  //
  // nand       r3, r1, r1
  // nand       r3, r3, r0
  //
  // nand       r2, r0, r0
  // nand       r2, r2, r1
  //
  // nand       r0, r2, r3
  // nand       r0, r0, r0
  // sharedio   r0
  // reproduce
  program[length - 10].op_code = Library::GetOpCode("SharedIO");
  program[length - 9].op_code = Library::GetOpCode("SharedIO");
  program[length - 9].args[0] = 1;

  program[length - 8].op_code = Library::GetOpCode("Nand");
  program[length - 8].args = {3, 1, 1};
  program[length - 7].op_code = Library::GetOpCode("Nand");
  program[length - 7].args = {3, 3, 0};

  program[length - 6].op_code = Library::GetOpCode("Nand");
  program[length - 6].args = {2, 0, 0};
  program[length - 5].op_code = Library::GetOpCode("Nand");
  program[length - 5].args = {2, 2, 1};

  program[length - 4].op_code = Library::GetOpCode("Nand");
  program[length - 4].args = {0, 2, 3};
  program[length - 3].op_code = Library::GetOpCode("Nand");
  program[length - 2].op_code = Library::GetOpCode("SharedIO");
  program[length - 1].op_code = Library::GetOpCode("Reproduce");
  return program;
}

#endif