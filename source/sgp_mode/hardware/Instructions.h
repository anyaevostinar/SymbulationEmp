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
      CPUState<typename HW_SPEC_T::world_t>& state                                  \
    ) {                                                                        \
      uint32_t* a_ptr = (uint32_t*)&core.registers[inst.args[0]];                 \
      uint32_t* b_ptr = (uint32_t*)&core.registers[inst.args[1]];                 \
      uint32_t* c_ptr = (uint32_t*)&core.registers[inst.args[2]];                 \
      uint32_t& a = *a_ptr; \
      uint32_t& b = *b_ptr; \
      uint32_t& c = *c_ptr; \
      /* avoid "unused variable" warnings */                                   \
      a = a, b = b, c = c;                                                     \
      InstCode                                                                 \
    }                                                                          \
    static size_t prevalence() { return 1; }                                   \
    static std::string name() { return #InstName; }                            \
  };

// NOTE - Discuss weirdness with casting behavior as-is
INST(Increment, {
  core.registers[inst.args[0]] += 1;
});

INST(Decrement, {
  core.registers[inst.args[0]] -= 1;
});

// Unary shift (>>1 or <<1)
INST(ShiftLeft, { a <<= 1; });
INST(ShiftRight, { a >>= 1; });

INST(Add, { a = b + c; });
INST(Subtract, { a = b - c; });

INST(Nand, { a = ~(b & c); });

INST(Push, {
  // Push value in register a to active stack.
  state.GetStacks().Push(a);
});

INST(Pop, {
  if (auto val = state.GetStacks().Pop()) {
    a = val.value();
  } else {
    a = 0;
  }
});

INST(SwapStack, {
  state.GetStacks().ChangeActive();
});

INST(Swap, { std::swap(a, b); });

INST(Reproduce, {
  const emp::WorldPosition& org_loc = state.GetLocation();
  // std::cout << "Repro inst!" << std::endl;
  // Check whether this attempt at reproduction is allowed.
  const bool invalid_attempt = state.ReproInProgress() || !org_loc.IsValid() || state.ReproAttempt();
  if (invalid_attempt) {
    return;
  }
  // std::cout << "  Mark repro attempt!" << std::endl;
  state.MarkReproAttempt();
});

// NOTE - what is the intended difference between SharedIO and PrivateIO?
INST(IO, {
  // (1) Add output to output buffer
  state.GetOutputBuffer().emplace_back(a);
  // (2) Read next value from input buffer (advancing buffer read ptr)
  a = state.GetInputBuffer().read();
});

// NOTE - Discuss whether we want to be using custom jump table vs. using signalgp's
//        module infrastructure.
INST(JumpIfNEq, {
  if (a != b) {
    core.JumpToIndex(state.GetJumpDest(core.GetProgramCounter()));
  }
});

INST(JumpIfLess, {
  if (a < b) {
    core.JumpToIndex(state.GetJumpDest(core.GetProgramCounter()));
  }
});

INST(JumpIfEq, {
  if (a == b) {
    core.JumpToIndex(state.GetJumpDest(core.GetProgramCounter()));
  }
});

// BOOKMARK
// TODO - Donate / Steal instructions
INST(Donate, {
  // This instruction does nothing if executed by a host or if this is a symbiont
  // without a host.
  if (state.IsHost() || !state.HasHost()) {
    return;
  }
  // If we're here, we know that we have a symbiont with a host.
  state.GetWorld().SymDonateToHost(state.GetOrg(), state.GetHost());
});

INST(Steal, {
  // This instruction does nothing if executed by a
  if (state.IsHost() || !state.HasHost()) {
    return;
  }
  state.GetWorld().SymStealFromHost(state.GetOrg(), state.GetHost());
});

// Only active if free living sym mode turned on
INST(Infect, {
  // Check that this is neither a host or a hosted symbiont
  if (state.IsHost() || state.HasHost()) {
    return;
  }
  state.GetWorld().FreeLivingSymDoInfect(state.GetOrg());
});

// NOTE - Discuss following old instructions that were unused (and whether we still want them)
/*
INST(Reuptake, {
  uint32_t next;
  AddOrganismPoints(state, *a);
  // Only get resources if the organism has values in their internal environment
  if (state.internal_environment->size() > 0) {
    // Take a resource from back of internal environment vector
    next = state.internal_environment->back();
    // Clear out the selected resource from Internal Environment
    state.internal_environment->pop_back();
    *a = next;
    state.input_buf.push(next);
  } else {
    // Otherwise, reset the register to 0
    *a = 0;
  }
});
*/

} // namespace inst


#endif