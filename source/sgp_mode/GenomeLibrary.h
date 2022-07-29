#ifndef GENOME_LIBRARY
#define GENOME_LIBRARY

#include "Instructions.h"
#include "sgpl/operations/flow_global/Anchor.hpp"
#include "sgpl/program/Instruction.hpp"
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

/**
 * Allows building up a program without knowing the final size.
 * When it's done and `build()` is called, the instructions added to the builder will be
 * located at the end of the generated program, right before `reproduce`.
 */
class ProgramBuilder : emp::vector<sgpl::Instruction<Spec>> {
public:
  void add(const std::string op_name, uint8_t arg0 = 0, uint8_t arg1 = 0,
           uint8_t arg2 = 0) {
    sgpl::Instruction<Spec> inst;
    inst.op_code = Library::GetOpCode(op_name);
    inst.args = {arg0, arg1, arg2};
    push_back(inst);
  }

  sgpl::Program<Spec> build(size_t length) {
    add("Reproduce");

    sgpl::Program<Spec> program;
    // Set everything to 0 - this makes them no-ops since that's the first
    // inst in the library
    program.resize(length - size());
    program[0].op_code = Library::GetOpCode("Global Anchor");
    program[0].tag = START_TAG;

    program.insert(program.end(), begin(), end());

    return program;
  }

  void addNot() {
    // sharedio   r0
    // nand       r0, r0, r0
    // sharedio   r0
    add("SharedIO");
    add("Nand");
    add("SharedIO");
  }

  void addSquare() {
    // Always output 4:
    // pop        r0
    // increment  r0          -> 1
    // add        r0, r0, r0  -> 2
    // add        r0, r0, r0  -> 4
    // sharedio   r0
    add("Pop");
    add("Increment");
    add("Add");
    add("Add");
    add("SharedIO");
    add("Reproduce");
  }

  void addNand() {
    // sharedio   r0
    // sharedio   r1
    // nand       r0, r1, r0
    // sharedio   r0
    add("SharedIO");
    add("SharedIO", 1);
    add("Nand", 0, 1, 0);
    add("SharedIO");
  }

  void addAnd() {
    // ~(a nand b)
    // sharedio   r0
    // sharedio   r1
    // nand       r0, r1, r0
    // nand       r0, r0, r0
    // sharedio   r0
    add("SharedIO");
    add("SharedIO", 1);
    add("Nand", 0, 1, 0);
    add("Nand");
    add("SharedIO");
  }

  void addOrn() {
    // (~a) nand b
    // sharedio   r0
    // sharedio   r1
    // nand       r0, r0, r0
    // nand       r0, r1, r0
    // sharedio   r0
    add("SharedIO");
    add("SharedIO", 1);
    add("Nand");
    add("Nand", 0, 1, 0);
    add("SharedIO");
  }

  void addOr() {
    // (~a) nand (~b)
    // sharedio   r0
    // sharedio   r1
    // nand       r0, r0, r0
    // nand       r1, r1, r1
    // nand       r0, r1, r0
    // sharedio   r0
    add("SharedIO");
    add("SharedIO", 1);
    add("Nand", 0, 0, 0);
    add("Nand", 1, 1, 1);
    add("Nand", 0, 1, 0);
    add("SharedIO");
  }

  void addAndn() {
    // ~(a nand (~b))
    // sharedio   r0
    // sharedio   r1
    // nand       r1, r1, r1
    // nand       r0, r1, r0
    // nand       r0, r0, r0
    // sharedio   r0
    add("SharedIO");
    add("SharedIO", 1);
    add("Nand", 1, 1, 1);
    add("Nand", 0, 1, 0);
    add("Nand", 0, 0, 0);
    add("SharedIO");
  }

  void addNor() {
    // ~((~a) nand (~b))
    // sharedio   r0
    // sharedio   r1
    // nand       r0, r0, r0
    // nand       r1, r1, r1
    // nand       r0, r1, r0
    // nand       r0, r0, r0
    // sharedio   r0
    add("SharedIO");
    add("SharedIO", 1);
    add("Nand", 0, 0, 0);
    add("Nand", 1, 1, 1);
    add("Nand", 0, 1, 0);
    add("Nand", 0, 0, 0);
    add("SharedIO");
  }

  void addXor() {
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
    add("SharedIO");
    add("SharedIO", 1);

    add("Nand", 3, 1, 1);
    add("Nand", 3, 3, 0);

    add("Nand", 2, 0, 0);
    add("Nand", 2, 2, 1);

    add("Nand", 0, 2, 3);
    add("SharedIO");
  }

  void addEqu() {
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
    add("SharedIO");
    add("SharedIO", 1);

    add("Nand", 3, 1, 1);
    add("Nand", 3, 3, 0);

    add("Nand", 2, 0, 0);
    add("Nand", 2, 2, 1);

    add("Nand", 0, 2, 3);
    add("Nand", 0, 0, 0);
    add("SharedIO");
  }
};

sgpl::Program<Spec> CreateRandomProgram(size_t length) {
  return sgpl::Program<Spec>(length);
}

sgpl::Program<Spec> CreateNotProgram(size_t length) {
  ProgramBuilder program;
  program.addNot();
  return program.build(length);
}

sgpl::Program<Spec> CreateSquareProgram(size_t length) {
  ProgramBuilder program;
  program.addSquare();
  return program.build(length);
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