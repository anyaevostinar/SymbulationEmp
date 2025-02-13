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

#include <functional>
#include <mutex>

namespace sgpmode::inst {

// template<typename WORLD_T>
// class InstructionLib {


// };

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
    static void run(                                                           \
      sgpl::Core<Spec>& core,                                                  \
      const sgpl::Instruction<Spec>& inst,                                     \
      const sgpl::Program<Spec>& program,                                      \
      CPUState<Spec::world_t>& state                                                 \
    ) {                                                                        \
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

INST(Increment, { ++*a; });
INST(Decrement, { --*a; });

// Unary shift (>>1 or <<1)
INST(ShiftLeft, { *a <<= 1; });
INST(ShiftRight, { *a >>= 1; });

INST(Add, { *a = *b + *c; });
INST(Subtract, { *a = *b - *c; });

INST(Nand, { *a = ~(*b & *c); });

INST(Push, {
  state.GetStacks().Push(*a);
});

INST(Pop, {
  if (auto val = state.GetStacks().Pop()) {
    *a = val;
  } else {
    *a = 0;
  }
});

INST(SwapStack, {
  state.GetStacks().ChangeActive();
});

INST(Swap, { std::swap(*a, *b); });

std::mutex reproduce_mutex;
INST(Reproduce, {
  const auto& org_loc = state.GetLocation();

  // If reproduction already in progress or organism has invalid location,
  //  this reproduction attempt fails.
  const bool invalid_attempt = state.ReproInProgress() || !org_loc.IsValid() || state.ReproAttempt();
  if (invalid_attempt) {
    return;
  }

  // Repro attempt is valid, flag attempt for world to handle.
  state.FlagReproAttempt();

  // // TODO - move this out of the instruction
  // double points = state.organism->IsHost()
  //                     ? state.world->GetConfig()->HOST_REPRO_RES()
  //                     : state.world->GetConfig()->SYM_HORIZ_TRANS_RES();
  // if (state.organism->GetPoints() > points) {
  //   state.organism->AddPoints(-points);
  //   // Add this organism to the queue to reproduce, using the mutex to avoid a
  //   // data race
  //   std::lock_guard<std::mutex> lock(reproduce_mutex);
  //   state.in_progress_repro = state.world->to_reproduce.size();
  //   state.world->to_reproduce.push_back(
  //       std::pair(state.organism, state.location));
  // }
});

// Set output to value of register and set register to new input
INST(PrivateIO, {
  //---------------------------
  // TODO - allow world to handle points update
  // float score = state.world->GetTaskSet().CheckTasks(state, *a, false);
  // if (score != 0.0) {
  //   if (!state.organism->IsHost()) {
  //     state.world->GetSymEarnedDataNode().WithMonitor(
  //         [=](auto &m) { m.AddDatum(score); });
  //   } else {
  //     // A host loses 25% of points when performing private IO operations
  //     score *= 1.0; //turning off penalty for now
  //   }
  //   state.organism->AddPoints(score);
  // }
  //---------------------------

  uint32_t next;
  if (state.world->GetConfig()->RANDOM_IO_INPUT()) {
    next = sgpl::tlrand.Get().GetUInt();
  } else {
    next = 1;
  }
  *a = next;
  state.input_buf.push(next);
});

void AddOrganismPoints(CPUState state, uint32_t output) {
  float score = state.world->GetTaskSet().CheckTasks(state, output, true);
  if (score != 0.0) {
    state.organism->AddPoints(score);
    if (!state.organism->IsHost()) {
      state.world->GetSymEarnedDataNode().WithMonitor(
          [=](auto &m) { m.AddDatum(score); });
    }
  }
}
// Set output to value of register and set register to new input
INST(SharedIO, {
  AddOrganismPoints(state, *a);
  uint32_t next;
  if (state.world->GetConfig()->RANDOM_IO_INPUT()) {
    next = sgpl::tlrand.Get().GetUInt();
  } else {
    next = 1;
  }
  *a = next;
  state.input_buf.push(next);
});
INST(Donate, {
  if (state.world->GetConfig()->DONATION_STEAL_INST()) {
    if (state.organism->IsHost() || state.organism->GetHost() == nullptr)
      return;
    if (emp::Ptr<Organism> host = state.organism->GetHost()) {
      // Donate 20% of the total points of the symbiont-host system
      // This way, a sym can donate e.g. 40 or 60 percent of their points in a
      // couple of instructions
      double to_donate =
          fmin(state.organism->GetPoints(),
               (state.organism->GetPoints() + host->GetPoints()) * 0.20);
      state.world->GetSymDonatedDataNode().WithMonitor(
          [=](auto &m) { m.AddDatum(to_donate); });
      host->AddPoints(to_donate *
                      (1.0 - state.world->GetConfig()->DONATE_PENALTY()));
      state.organism->AddPoints(-to_donate);
    }
  }
});
INST(Steal, {
  if (state.world->GetConfig()->DONATION_STEAL_INST()) {
    if (state.organism->IsHost() || state.organism->GetHost() == nullptr)
      return;
    if (emp::Ptr<Organism> host = state.organism->GetHost()) {
      // Steal 20% of the total points of the symbiont-host system
      // This way, a sym can steal e.g. 40 or 60 percent of the host's points in
      // a couple of instructions
      double to_steal =
          fmin(host->GetPoints(),
               (state.organism->GetPoints() + host->GetPoints()) * 0.20);
      state.world->GetSymStolenDataNode().WithMonitor(
          [=](auto &m) { m.AddDatum(to_steal); });
      host->AddPoints(-to_steal);
      // 10% of the stolen resources are lost
      state.organism->AddPoints(
          to_steal * (1.0 - state.world->GetConfig()->STEAL_PENALTY()));
    }
  }
});

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

INST(Infect, {
  if (state.world->GetConfig()->FREE_LIVING_SYMS()) {
    // check that it is neither a host nor a hosted sym
    if (state.organism->IsHost() || state.organism->GetHost() != nullptr) return;
    int pop_index = state.location.GetPopID();
    // check that there's an available host
    if (state.world->IsOccupied(pop_index)) {
      //check that there's enough space for infection
      int syms_size = state.world->GetPop()[pop_index]->GetSymbionts().size();
      if (syms_size < state.world->GetConfig()->SYM_LIMIT()) {
        // extract the symbiont from the fls vector and decrement the free living org count, then
        // add the sym to the host's sym list
        state.world->GetPop()[pop_index]->AddSymbiont(state.world->ExtractSym(pop_index));
        // change the location
        state.location = emp::WorldPosition(pop_index, syms_size);
      }
      else {
        state.organism->SetDead(); // infection failed, set it dead and do deletion next update
      }
    }
  }
});

// NOTE - Prev Jump instructions used global anchors
// TODO - Check if should cast register values to uint32_t or leave as floats?
/**
 * Jumps to a local anchor that matches the instruction tag if `reg[arg_0]` ==
 * `reg[arg_1]`.
 * Adapted from sgplite JumpIf instruction
 */
struct JumpIfEq {

  template<typename Spec>
  static void run(
    sgpl::Core<Spec>& core,
    const sgpl::Instruction<Spec>& inst,
    const sgpl::Program<Spec>& prog,
    CPUState& state
  ) noexcept {
    if ( (uint32_t)(core.registers[inst.args[0]]) == (uint32_t)(core.registers[inst.args[0]])) {
      core.JumpToLocalAnchorMatch( inst.tag );
    }
  }

  static std::string name() { return "Local Jump If Equal"; }

  static size_t prevalence() { return 1; }

  template<typename Spec>
  static auto descriptors( const sgpl::Instruction<Spec>& inst ) {

    using tag_t = typename Spec::tag_t;

    return std::map<std::string, std::string>{
      { "argument a", uit_emp::to_string( static_cast<int>( inst.args[0] ) ) },
      { "argument b", uit_emp::to_string( static_cast<int>( inst.args[1] ) ) },
      { "tag bits", uit_emp::to_string( inst.tag ) },
      { "tag moniker", uit_emp::hash_namify( std::hash< tag_t >{}( inst.tag ) ) },
      { "summary", "if a == b, goto tag match local anchor" },
    };
  }

  template<typename Spec>
  static std::set<std::string> categories(const sgpl::Instruction<Spec>&) {
    return {
      "flow",
      "local flow",
      "intrinsic",
      "op",
    };
  }

};

// TODO - Check if should cast register values to uint32_t or leave as floats?
/**
 * Jumps to a local anchor that matches the instruction tag if `reg[arg_0]` !=
 * `reg[arg_1]`.
 * Adapted from sgplite JumpIf instruction
 */
struct JumpIfNEq {

  template<typename Spec>
  static void run(
    sgpl::Core<Spec>& core,
    const sgpl::Instruction<Spec>& inst,
    const sgpl::Program<Spec>&,
    typename Spec::peripheral_t&
  ) noexcept {
    if ( (uint32_t)(core.registers[ inst.args[0] ]) != (uint32_t)(core.registers[ inst.args[0] ])) {
      core.JumpToLocalAnchorMatch( inst.tag );
    }
  }

  static std::string name() { return "Local Jump If Not Equal"; }

  static size_t prevalence() { return 1; }

  template<typename Spec>
  static auto descriptors( const sgpl::Instruction<Spec>& inst ) {

    using tag_t = typename Spec::tag_t;

    return std::map<std::string, std::string>{
      { "argument a", uit_emp::to_string( static_cast<int>( inst.args[0] ) ) },
      { "argument b", uit_emp::to_string( static_cast<int>( inst.args[1] ) ) },
      { "tag bits", uit_emp::to_string( inst.tag ) },
      { "tag moniker", uit_emp::hash_namify( std::hash< tag_t >{}( inst.tag ) ) },
      { "summary", "if a != b, goto tag match local anchor" },
    };
  }

  template<typename Spec>
  static std::set<std::string> categories(const sgpl::Instruction<Spec>&) {
    return {
      "flow",
      "local flow",
      "intrinsic",
      "op",
    };
  }

};

// TODO - Check if should cast register values to uint32_t or leave as floats?
/**
 * Jumps to a local anchor that matches the instruction tag if `reg[arg_0]` is
 * less than `reg[arg_1]`.
 * Adapted from sgplite JumpIf instruction
 */
struct JumpIfLess {

  template<typename Spec>
  static void run(
    sgpl::Core<Spec>& core,
    const sgpl::Instruction<Spec>& inst,
    const sgpl::Program<Spec>&,
    CPUState& state
  ) noexcept {
    if ( (uint32_t)(core.registers[inst.args[0]]) < ((uint32_t)core.registers[inst.args[1]]) ) {
      core.JumpToLocalAnchorMatch( inst.tag );
    }
  }

  static std::string name() { return "Local Jump If Less"; }

  static size_t prevalence() { return 1; }

  template<typename Spec>
  static auto descriptors( const sgpl::Instruction<Spec>& inst ) {

    using tag_t = typename Spec::tag_t;

    return std::map<std::string, std::string>{
      { "argument a", uit_emp::to_string( static_cast<int>( inst.args[0] ) ) },
      { "argument b", uit_emp::to_string( static_cast<int>( inst.args[1] ) ) },
      { "tag bits", uit_emp::to_string( inst.tag ) },
      { "tag moniker", uit_emp::hash_namify( std::hash< tag_t >{}( inst.tag ) ) },
      { "summary", "if a < b, goto tag match local anchor" },
    };
  }

  template<typename Spec>
  static std::set<std::string> categories(const sgpl::Instruction<Spec>&) {
    return {
      "flow",
      "local flow",
      "intrinsic",
      "op",
    };
  }

};


// Trigger interaction
struct Interact {

  template <typename Spec>
  static void run(
    sgpl::Core<Spec> &core,
    const sgpl::Instruction<Spec> &inst,
    const sgpl::Program<Spec> &program,
    CPUState &state
  ) {
    uint32_t *a = (uint32_t *)&core.registers[inst.args[0]],
              *b = (uint32_t *)&core.registers[inst.args[1]],
              *c = (uint32_t *)&core.registers[inst.args[2]];
    /* avoid "unused variable" warnings */
    a = a, b = b, c = c;
    // InstCode
  }

  static size_t prevalence() { return 1; }
  static std::string name() { return "Interact"; }
};




} // namespace inst

#endif