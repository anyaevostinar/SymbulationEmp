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
 * `CPUState &state`.
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
    core.JumpToIndex(state.jump_table[core.GetProgramCounter()]);
  }
});
INST(JumpIfLess, {
  if (*a < *b) {
    core.JumpToIndex(state.jump_table[core.GetProgramCounter()]);
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
INST(Reproduce, {
  // Only one reproduction is allowed per update
  if (state.in_progress_repro != -1 || !state.location.IsValid())
    return;

  // check that enough cycles have been executed
  size_t cycles_required_for_repro = state.organism->IsHost()
                      ? state.world->GetConfig()->HOST_MIN_CYCLES_BEFORE_REPRO()
                      : state.world->GetConfig()->SYM_MIN_CYCLES_BEFORE_REPRO();
  if(state.cpu_cycles_since_repro < cycles_required_for_repro) return;

  double points = state.organism->IsHost()
                      ? state.world->GetConfig()->HOST_REPRO_RES()
                      : state.world->GetConfig()->SYM_HORIZ_TRANS_RES();
  if (state.organism->GetPoints() > points) {
    state.organism->AddPoints(-points);
    state.in_progress_repro = state.world->to_reproduce.size();
    state.world->to_reproduce.push_back(state.organism);
    state.cpu_cycles_since_repro = 0;
  }
});

void AddNewInput(CPUState &state, uint32_t *output){
  uint32_t next;
  next = sgpl::tlrand.Get().GetUInt();
 
  *output = next;
  state.input_buf.push(next);
}

// Set output to value of register and set register to new input
//TODO: change to just "IO" to not be confusing
INST(SharedIO, {
  if(state.organism->IsHost()){
    state.world->GetTaskSet().ProcessOutput(state, *a, state.world->GetConfig()->HOST_ONLY_FIRST_TASK_CREDIT());
  } else {
    state.world->GetTaskSet().ProcessOutput(state, *a, state.world->GetConfig()->SYM_ONLY_FIRST_TASK_CREDIT());
  }
  
  //TODO: Add helper method for adding new input
  AddNewInput(state, a);
});



INST(Donate, {
  if (state.world->GetConfig()->DONATION_STEAL_INST() && (state.world->GetConfig()->SYMBIONT_TYPE() == 0 || state.world->GetConfig()->ALLOW_TRANSITION_EVOLUTION() == 1)) {
    if (state.organism->IsHost()){
      return;
    }
    emp::Ptr<Organism> host = state.organism->GetHost();
    if(host->GetCyclesGiven() <= 0){
      host->CycleTransfer(1);
      state.world->GetDonateCount().AddDatum(1);
    }
    
    
  }
  
});
INST(Steal, {
  if (state.world->GetConfig()->DONATION_STEAL_INST() && (state.world->GetConfig()->SYMBIONT_TYPE() == 1 || state.world->GetConfig()->ALLOW_TRANSITION_EVOLUTION() == 1)) {
    if (state.organism->IsHost()){
      return;
    }
    emp::Ptr<Organism> host = state.organism->GetHost();
    if(host->GetCyclesGiven() >= 0){
        host->CycleTransfer(-1);
        state.world->GetStealCount().AddDatum(1);
      }
    }
   
  }
);

} // namespace inst

#endif