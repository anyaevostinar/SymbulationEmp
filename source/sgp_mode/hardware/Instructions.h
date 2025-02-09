#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "CPUState.h"
// #include "SGPWorld.h"
// #include "Tasks.h"

#include "sgpl/hardware/Cpu.hpp"
#include "sgpl/operations/flow_global/Anchor.hpp"
#include "sgpl/program/Program.hpp"
#include "sgpl/utility/ThreadLocalRandom.hpp"

#include <functional>
#include <mutex>

namespace sgpmode::inst {

// template<typename WORLD_T>
// class InstructionLib {


// };

// TODO - test switch from pionters to references

/**
 * Macro to easily create an instruction:
 * `INST(MyInstruction, { *a = *b + 2;})`. In the code block, operand registers
 * are visible as `a`, `b`, and `c`, all of type `uint32_t *`. Instructions may
 * also access the `Core &core`, `Instruction &inst`, `Program &program`, and
 * `CPUState &state`.
 */
#define INST(InstName, InstCode)                                               \
  struct InstName {                                                            \
    template <typename HW_SPEC_T>                                                   \
    static void run(                                                           \
      sgpl::Core<HW_SPEC_T>& core,                                                  \
      const sgpl::Instruction<HW_SPEC_T>& inst,                                     \
      const sgpl::Program<HW_SPEC_T>& program,                                      \
      CPUState<typename HW_SPEC_T::world_t>& state                                                 \
    ) {                                                                        \
      uint32_t& a = core.registers[inst.args[0]];                 \
      uint32_t& b = core.registers[inst.args[1]],                 \
      uint32_t& c = core.registers[inst.args[2]];                 \
      /* avoid "unused variable" warnings */                                   \
      a = a, b = b, c = c;                                                     \
      InstCode                                                                 \
    }                                                                          \
    static size_t prevalence() { return 1; }                                   \
    static std::string name() { return #InstName; }                            \
  };

// TODO - remove commented out code if new versions work as intended.
// INST(JumpIfNEq, {
//   // Even != works differently on floats because of NaNs
//   if (*a != *b) {
//     core.JumpToIndex(state.jump_table[core.GetProgramCounter()]);
//   }
// });
// INST(JumpIfLess, {
//   if (*a < *b) {
//     core.JumpToIndex(state.jump_table[core.GetProgramCounter()]);
//   }
// });

INST(Increment, { ++a; });
INST(Decrement, { --a; });


} // namespace inst

#endif