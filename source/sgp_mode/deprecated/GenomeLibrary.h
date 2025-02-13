#ifndef GENOME_LIBRARY
#define GENOME_LIBRARY

#include "Instructions.h"
#include "SGPHardwareSpec.h"

#include "sgpl/operations/flow_global/Anchor.hpp"
#include "sgpl/program/Instruction.hpp"
#include "sgpl/program/Program.hpp"

#include <cstddef>
#include <limits>
#include <unordered_set>

namespace sgpmode {

using Library = sgpl::OpLibrary<
  sgpl::Nop<>,
  // single argument math
  inst::ShiftLeft,
  inst::ShiftRight,
  inst::Increment,
  inst::Decrement,
  // biological operations
  // no copy or alloc
  // no h-search
  inst::Reproduce,
  inst::SharedIO,
  // double argument math
  inst::Add,
  inst::Subtract,
  inst::Nand,
  // Stack manipulation
  inst::Push,
  inst::Pop,
  inst::SwapStack,
  inst::Swap,
  // Control flow
  // TODO - add back in JumpIfEq
  // inst::JumpIfEq, Leave off for now; minimal changes before compilation works
  inst::JumpIfNEq,
  inst::JumpIfLess,
  //fls basics
  // if-label doesn't make sense for SGP, same with *-head
  // and set-flow but this is required
  sgpl::global::Anchor
>;

namespace lib_info {
  const std::unordered_set<uint8_t> jump_opcodes = {
    Library::GetOpCode("JumpIfNEq"),
    Library::GetOpCode("JumpIfLess")
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

using Spec = sgpl::Spec<Library, CPUState>;

// Instead of picking an anchor to start at randomly, start at the anchor that
// has the most bits set by matching with the maximum valued tag. This way
// organisms can evolve to designate a certain anchor as the entry.
const Spec::tag_t START_TAG(std::numeric_limits<uint64_t>::max());

const size_t PROGRAM_LENGTH = 100;

/**
 * Allows building up a program without knowing the final size.
 * When it's done and `build()` is called, the instructions added to the builder
 * will be located at the end of the generated program, right before
 * `reproduce`.
 */
class ProgramBuilder : emp::vector<sgpl::Instruction<Spec>> {
public:
  void Add(const std::string op_name, uint8_t arg0 = 0, uint8_t arg1 = 0,
           uint8_t arg2 = 0) {
    sgpl::Instruction<Spec> inst;
    inst.op_code = Library::GetOpCode(op_name);
    inst.args = {arg0, arg1, arg2};
    push_back(inst);
  }

  sgpl::Program<Spec> Build(size_t length) {
    Add("Reproduce");

    sgpl::Program<Spec> program;
    // Set everything to 0 - this makes them no-ops since that's the first
    // inst in the library
    program.resize(length - size());
    program[0].op_code = Library::GetOpCode("Global Anchor");
    program[0].tag = START_TAG;

    program.insert(program.end(), begin(), end());

    return program;
  }

  sgpl::Program<Spec> BuildNoRepro(size_t length) {
    //For making an obligate mutualist
    Add("Donate");
    Add("Donate");
    Add("Donate");
    Add("Donate");
    Add("Donate");
    sgpl::Program<Spec> program;
    // Set everything to 0 - this makes them no-ops since that's the first
    // inst in the library
    program.resize(length - size());
    program[0].op_code = Library::GetOpCode("Global Anchor");
    program[0].tag = START_TAG;

    program.insert(program.end(), begin(), end());

    return program;
  }

  void AddNot() {
    // sharedio   r0
    // nand       r0, r0, r0
    // sharedio   r0
    Add("SharedIO");
    Add("Nand");
    Add("SharedIO");
  }

  void AddPrivateNot() {
    // privateio   r0
    // nand       r0, r0, r0
    // privateio   r0
    Add("PrivateIO");
    Add("Nand");
    Add("PrivateIO");
  }

  void AddNand() {
    // sharedio   r0
    // sharedio   r1
    // nand       r0, r1, r0
    // sharedio   r0
    Add("SharedIO");
    Add("SharedIO", 1);
    Add("Nand", 0, 1, 0);
    Add("SharedIO");
  }

  void AddPrivateNand() {
    // sharedio   r0
    // sharedio   r1
    // nand       r0, r1, r0
    // sharedio   r0
    Add("PrivateIO");
    Add("PrivateIO", 1);
    Add("Nand", 0, 1, 0);
    Add("PrivateIO");
  }

  void AddAnd() {
    // ~(a nand b)
    // sharedio   r0
    // sharedio   r1
    // nand       r0, r1, r0
    // nand       r0, r0, r0
    // sharedio   r0
    Add("SharedIO");
    Add("SharedIO", 1);
    Add("Nand", 0, 1, 0);
    Add("Nand");
    Add("SharedIO");
  }

  void AddOrn() {
    // (~a) nand b
    // sharedio   r0
    // sharedio   r1
    // nand       r0, r0, r0
    // nand       r0, r1, r0
    // sharedio   r0
    Add("SharedIO");
    Add("SharedIO", 1);
    Add("Nand");
    Add("Nand", 0, 1, 0);
    Add("SharedIO");
  }

  void AddOr() {
    // (~a) nand (~b)
    // sharedio   r0
    // sharedio   r1
    // nand       r0, r0, r0
    // nand       r1, r1, r1
    // nand       r0, r1, r0
    // sharedio   r0
    Add("SharedIO");
    Add("SharedIO", 1);
    Add("Nand", 0, 0, 0);
    Add("Nand", 1, 1, 1);
    Add("Nand", 0, 1, 0);
    Add("SharedIO");
  }

  void AddAndn() {
    // ~(a nand (~b))
    // sharedio   r0
    // sharedio   r1
    // nand       r1, r1, r1
    // nand       r0, r1, r0
    // nand       r0, r0, r0
    // sharedio   r0
    Add("SharedIO");
    Add("SharedIO", 1);
    Add("Nand", 1, 1, 1);
    Add("Nand", 0, 1, 0);
    Add("Nand", 0, 0, 0);
    Add("SharedIO");
  }

  void AddNor() {
    // ~((~a) nand (~b))
    // sharedio   r0
    // sharedio   r1
    // nand       r0, r0, r0
    // nand       r1, r1, r1
    // nand       r0, r1, r0
    // nand       r0, r0, r0
    // sharedio   r0
    Add("SharedIO");
    Add("SharedIO", 1);
    Add("Nand", 0, 0, 0);
    Add("Nand", 1, 1, 1);
    Add("Nand", 0, 1, 0);
    Add("Nand", 0, 0, 0);
    Add("SharedIO");
  }

  void AddXor() {
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
    Add("SharedIO");
    Add("SharedIO", 1);

    Add("Nand", 3, 1, 1);
    Add("Nand", 3, 3, 0);

    Add("Nand", 2, 0, 0);
    Add("Nand", 2, 2, 1);

    Add("Nand", 0, 2, 3);
    Add("SharedIO");
  }

  void AddEqu() {
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
    Add("SharedIO");
    Add("SharedIO", 1);

    Add("Nand", 3, 1, 1);
    Add("Nand", 3, 3, 0);

    Add("Nand", 2, 0, 0);
    Add("Nand", 2, 2, 1);

    Add("Nand", 0, 2, 3);
    Add("Nand", 0, 0, 0);
    Add("SharedIO");
  }
};

sgpl::Program<Spec> CreateRandomProgram(size_t length) {
  return sgpl::Program<Spec>(length);
}

sgpl::Program<Spec> CreateReproProgram(size_t length) {
  ProgramBuilder program;
  return program.Build(length);
}

sgpl::Program<Spec> CreateNotProgram(size_t length) {
  ProgramBuilder program;
  program.AddNot();
  return program.Build(length);
}


sgpl::Program<Spec> CreatePrivateNotProgram(size_t length) {
  ProgramBuilder program;
  program.AddPrivateNot();
  return program.Build(length);
}

sgpl::Program<Spec> CreatePrivateNotNandProgram(size_t length) {
  ProgramBuilder program;
  program.AddPrivateNot();
  program.AddPrivateNand();
  return program.Build(length);
}

sgpl::Program<Spec> CreateMutualistStart(size_t length) {
  ProgramBuilder program;
  program.AddNand();
  return program.BuildNoRepro(length);
}

/**
 * Picks what type of starting program should be created based on the config and
 * creates it.
 */
sgpl::Program<Spec> CreateStartProgram(emp::Ptr<SymConfigSGP> config) {
  if (config->RANDOM_ANCESTOR()) {
    return CreateRandomProgram(PROGRAM_LENGTH);
  } else if (config->TASK_TYPE() == 1) {
    return CreateNotProgram(PROGRAM_LENGTH);
  } else {
    return CreateReproProgram(PROGRAM_LENGTH);
  }
}

}

#endif