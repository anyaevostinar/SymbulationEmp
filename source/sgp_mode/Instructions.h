#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "CPUState.h"
#include "SGPWorld.h"
#include "Tasks.h"
#include "sgpl/hardware/Cpu.hpp"
#include "sgpl/operations/flow_global/Anchor.hpp"
#include "sgpl/program/Program.hpp"
#include "sgpl/spec/Spec.hpp"
#include "sgpl/utility/ThreadLocalRandom.hpp"
#include <mutex>

namespace inst {

/**
 * Macro to easily create an instruction:
 * `INST(MyInstruction, { *a = *b + 2;})`. In the code block, operand registers
 * are visible as `a`, `b`, and `c`, all of type `uint32_t *`. Instructions may
 * also access the `Core &core`, `Instruction &inst`, `Program &program`, and
 * `Avidastate &state`.
 */
#define INST(InstName, InstCode)                                               \
  struct InstName {                                                            \
    template <typename Spec>                                                   \
    static void run(sgpl::Core<Spec> &core,                                    \
                    const sgpl::Instruction<Spec> &inst,                       \
                    const sgpl::Program<Spec> &program, CPUState &state) {     \
      uint32_t *a = (uint32_t *)&core.registers[inst.args[0]],                 \
               *b = (uint32_t *)&core.registers[inst.args[1]],                 \
               *c = (uint32_t *)&core.registers[inst.args[2]];                 \
      /* avoid "unused variable" warnings */                                   \
      a = a, b = b, c = c;                                                     \
      InstCode                                                                 \
    }                                                                          \
    static size_t prevalence() { return 1; }                                   \
    static std::string name() { return #InstName; }                            \
  };

INST(JumpIfNEq, {
  // Even != works differently on floats because of NaNs
  if (*a != *b) {
    core.JumpToGlobalAnchorMatch(inst.tag);
  }
});
INST(JumpIfLess, {
  if (*a < *b) {
    core.JumpToGlobalAnchorMatch(inst.tag);
  }
});
INST(Increment, { ++*a; });
INST(Decrement, { --*a; });
// Unary shift (>>1 or <<1)
INST(ShiftLeft, { *a <<= 1; });
INST(ShiftRight, { *a >>= 1; });
INST(Add, { *a = *b + *c; });
INST(Subtract, { *a = *b - *c; });
INST(Nand, { *a = ~(*b & *c); });
INST(Push, {
  // Organism stacks cap out at 16 elements so they don't waste too much memory
  if (state.stack.size() < 16) {
    state.stack.push_back(*a);
  }
});
INST(Pop, {
  if (state.stack.empty()) {
    *a = 0;
  } else {
    *a = state.stack.back();
    state.stack.pop_back();
  }
});
INST(SwapStack, { std::swap(state.stack, state.stack2); });
INST(Swap, { std::swap(*a, *b); });
std::mutex reproduce_mutex;
INST(Reproduce, {
  double points = state.host->IsHost() ? 128.0 : 4.0;
  if (state.host->GetPoints() > points) {
    state.host->AddPoints(-points);
    // Add this organism to the queue to reproduce, using the mutex to avoid a
    // data race
    std::lock_guard<std::mutex> lock(reproduce_mutex);
    state.world->to_reproduce.push_back(std::pair(state.host, state.location));
  }
});
// Set output to value of register and set register to new input
INST(IO, {
  float score = state.world->GetTaskSet().CheckTasks(state, *a);
  if (score != 0.0) {
    state.host->AddPoints(pow(2, score));
    if (!state.host->IsHost()) {
      state.world->sym_points_earned += pow(2, score);
    }
  }
  uint32_t next = sgpl::tlrand.Get().GetBits50();
  *a = next;
  state.input_buf.push(next);
});
INST(Donate, {
  if (state.host->IsHost())
    return;
  if (emp::Ptr<Organism> host = state.host->GetHost()) {
    // Donate 20% of the total points of the symbiont-host system
    // This way, a sym can donate e.g. 40 or 60 percent of their points in a
    // couple of instructions
    double to_donate =
        fmin(state.host->GetPoints(),
             (state.host->GetPoints() + host->GetPoints()) * 0.20);
    state.world->sym_points_donated += to_donate;
    host->AddPoints(to_donate);
    state.host->AddPoints(-to_donate);
  }
});

} // namespace inst

#endif