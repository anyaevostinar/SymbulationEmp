#ifndef AVIDA_SPEC_H
#define AVIDA_SPEC_H

#include "../Organism.h"
#include "SGPWorld.h"
#include "emp/Evolve/World.hpp"
#include "sgpl/hardware/Cpu.hpp"
#include "sgpl/operations/flow_global/Anchor.hpp"
#include "sgpl/program/Program.hpp"
#include "sgpl/spec/Spec.hpp"
#include "sgpl/utility/ThreadLocalRandom.hpp"
#include <atomic>
#include <cmath>
#include <iostream>
#include <mutex>
#include <string>
#include <variant>

// Helper that keeps the last `len` inputs and discards the rest
template <const size_t len = 8> struct IORingBuffer {
  uint32_t buffer[len];
  size_t next = 0;

  IORingBuffer() {
    for (size_t i = 0; i < len; i++) {
      buffer[i] = 0;
    }
  }

  void push(uint32_t x) {
    buffer[next] = x;
    next = (next + 1) % len;
  }

  uint32_t operator[](size_t idx) { return buffer[idx % len]; }

  size_t size() { return len; }
};

struct AvidaPeripheral {
  emp::vector<uint32_t> stack;
  emp::vector<uint32_t> stack2;

  IORingBuffer<> input_buf;
  emp::optional<uint32_t> output;

  emp::Ptr<emp::BitSet<64>> usedResources = emp::NewPtr<emp::BitSet<64>>();

  emp::Ptr<Organism> host;
  emp::Ptr<SGPWorld> world;

  emp::WorldPosition location;

  AvidaPeripheral(emp::Ptr<Organism> host, emp::Ptr<SGPWorld> world)
      : host(host), world(world) {}
};

// An input task computes an expected output based on the inputs, and if the
// organism's output matches, it gives it a certain reward:
// `InputTask sum{ 2, [](auto &x) { return x[0] + x[1]; }, 1.0 };`
struct InputTask {
  size_t n_inputs;
  std::function<uint32_t(emp::vector<uint32_t> &)> taskFun;
  float value;
};

// An output task returns a reward based on the output the organism produced:
// `OutputTask is42{ [](uint32_t x) { return x == 42 ? 2.0 : 0.0; } };`
struct OutputTask {
  std::function<float(uint32_t)> taskFun;
};

struct Task {
  std::string name;
  std::variant<InputTask, OutputTask> kind;
  bool unlimited = true;

  std::atomic<size_t> *n_succeeds = new std::atomic<size_t>(0);
  std::atomic<size_t> *n_succeeds_sym = new std::atomic<size_t>(0);
};
// The 9 default logic tasks in Avida
// These are checked top-to-bottom and the reward is given for the first one
// that matches
emp::vector<Task> DefaultTasks{
    {"NOT", InputTask{1, [](auto &x) { return ~x[0]; }, 1.0}, false},
    {"NAND", InputTask{2, [](auto &x) { return ~(x[0] & x[1]); }, 1.0}, false},
    {"AND", InputTask{2, [](auto &x) { return x[0] & x[1]; }, 2.0}, false},
    {"ORN", InputTask{2, [](auto &x) { return x[0] | ~x[1]; }, 2.0}, false},
    {"OR", InputTask{2, [](auto &x) { return x[0] | x[1]; }, 3.0}, false},
    {"ANDN", InputTask{2, [](auto &x) { return x[0] & ~x[1]; }, 3.0}, false},
    {"NOR", InputTask{2, [](auto &x) { return ~(x[0] | x[1]); }, 4.0}, false},
    {"XOR", InputTask{2, [](auto &x) { return x[0] ^ x[1]; }, 4.0}, false},
    {"EQU", InputTask{2, [](auto &x) { return ~(x[0] ^ x[1]); }, 5.0}, false}};

float checkTasks(AvidaPeripheral &peripheral, emp::vector<Task> &tasks) {
  if (!peripheral.output.has_value()) {
    return 0.0;
  }
  uint32_t check = peripheral.output.value();
  peripheral.output.reset();
  // Check output tasks
  for (size_t i = 0; i < tasks.size(); i++) {
    Task &task = tasks[i];
    if (std::holds_alternative<OutputTask>(task.kind) &&
        !peripheral.usedResources->Get(i)) {
      peripheral.usedResources->Set(i, !task.unlimited);
      float score = std::get<OutputTask>(task.kind).taskFun(check);
      if (score > 0.0) {
        if (peripheral.host->IsHost())
          (*task.n_succeeds)++;
        else
          (*task.n_succeeds_sym)++;
        return score;
      }
    }
  }
  // Check input tasks
  // Special case so they can't cheat at e.g. NOR (0110, 1011 --> 0)
  if (check == 0 || check == 1) {
    return 0.0;
  }
  emp::vector<uint32_t> inputs;
  for (size_t i = 0; i < peripheral.input_buf.size(); i++) {
    if (peripheral.input_buf[i] == 0)
      continue;

    inputs = {peripheral.input_buf[i], peripheral.input_buf[i + 1]};
    for (size_t i = 0; i < tasks.size(); i++) {
      Task &task = tasks[i];
      if (std::holds_alternative<InputTask>(task.kind) &&
          !peripheral.usedResources->Get(i)) {
        InputTask &itask = std::get<InputTask>(task.kind);
        if (itask.n_inputs > 1 && inputs[1] == 0)
          continue;

        if (itask.taskFun(inputs) == check) {
          peripheral.usedResources->Set(i, !task.unlimited);
          if (peripheral.host->IsHost())
            (*task.n_succeeds)++;
          else
            (*task.n_succeeds_sym)++;
          return itask.value;
        }
      }
    }
  }
  return 0.0f;
}

void taskCheckpoint() {
  std::cout << "Host tasks completed since last checkpoint:\n";
  for (auto &task : DefaultTasks) {
    std::cout << "  \t" << task.name << ": " << *task.n_succeeds;
    task.n_succeeds->store(0);
  }
  std::cout << std::endl;
  std::cout << "Symbiont tasks completed since last checkpoint:\n";
  for (auto &task : DefaultTasks) {
    std::cout << "  \t" << task.name << ": " << *task.n_succeeds_sym;
    task.n_succeeds_sym->store(0);
  }
  std::cout << std::endl;
}

namespace ainst {

/**
 * Macro to easily create an instruction:
 * `INST(MyInstruction, { *a = *b + 2;})`. In the code block, operand registers
 * are visible as `a`, `b`, and `c`, all of type `uint32_t *`. Instructions may
 * also access the `Core &core`, `Instruction &inst`, `Program &program`, and
 * `AvidaPeripheral &peripheral`.
 */
#define INST(InstName, InstCode)                                               \
  struct InstName {                                                            \
    template <typename Spec>                                                   \
    static void                                                                \
    run(sgpl::Core<Spec> &core, const sgpl::Instruction<Spec> &inst,           \
        const sgpl::Program<Spec> &program, AvidaPeripheral &peripheral) {     \
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
  if (peripheral.stack.size() < 16) {
    peripheral.stack.push_back(*a);
  }
});
INST(Pop, {
  if (peripheral.stack.empty()) {
    *a = 0;
  } else {
    *a = peripheral.stack.back();
    peripheral.stack.pop_back();
  }
});
INST(SwapStack, { std::swap(peripheral.stack, peripheral.stack2); });
INST(Swap, { std::swap(*a, *b); });
std::mutex reproduce_mutex;
INST(Reproduce, {
  double points = peripheral.host->IsHost() ? 128.0 : 4.0;
  if (peripheral.host->GetPoints() > points) {
    peripheral.host->AddPoints(-points);
    // Add this organism to the queue to reproduce, using the mutex to avoid a
    // data race
    std::lock_guard<std::mutex> lock(reproduce_mutex);
    peripheral.world->toReproduce.push_back(
        std::pair(peripheral.host, peripheral.location));
  }
});
// Set output to value of register and set register to new input
INST(IO, {
  peripheral.output = *a;
  float score = checkTasks(peripheral, DefaultTasks);
  if (score != 0.0) {
    peripheral.host->AddPoints(pow(2, score));
    if (!peripheral.host->IsHost()) {
      peripheral.world->SymPointsEarned += pow(2, score);
    }
  }
  uint32_t next = sgpl::tlrand.Get().GetBits50();
  *a = next;
  peripheral.input_buf.push(next);
});
INST(Donate, {
  if (peripheral.host->IsHost())
    return;
  if (emp::Ptr<Organism> host = peripheral.host->GetHost()) {
    // Donate 20% of the total points of the symbiont-host system
    // This way, a sym can donate e.g. 40 or 60 percent of their points in a
    // couple of instructions
    double toDonate =
        fmin(peripheral.host->GetPoints(),
             (peripheral.host->GetPoints() + host->GetPoints()) * 0.20);
    peripheral.world->SymPointsDonated += toDonate;
    host->AddPoints(toDonate);
    peripheral.host->AddPoints(-toDonate);
  }
});

} // namespace ainst

using AvidaLibrary =
    sgpl::OpLibrary<sgpl::Nop<>, ainst::JumpIfNEq, ainst::JumpIfLess,
                    // if-label doesn't make sense for SGP, same with *-head and
                    // set-flow but this is required
                    sgpl::global::Anchor,
                    // single argument math
                    ainst::ShiftLeft, ainst::ShiftRight, ainst::Increment,
                    ainst::Decrement,
                    // Stack manipulation
                    ainst::Push, ainst::Pop, ainst::SwapStack, ainst::Swap,
                    // double argument math
                    ainst::Add, ainst::Subtract, ainst::Nand,
                    // biological operations
                    // no copy or alloc
                    ainst::Reproduce, ainst::IO,
                    // no h-search
                    ainst::Donate>;

using AvidaSpec = sgpl::Spec<AvidaLibrary, AvidaPeripheral>;

void PrintOp(sgpl::Instruction<AvidaSpec> &ins,
             emp::map<std::string, size_t> &arities,
             emp::map<size_t, std::string> &labels,
             sgpl::Program<AvidaSpec> &program,
             sgpl::JumpTable<AvidaSpec, AvidaSpec::global_matching_t> &table) {
  const std::string &name = ins.GetOpName();
  if (arities.count(name)) {
    // Simple instruction
    std::cout << "    " << emp::to_lower(name);
    for (size_t i = 0; i < 12 - name.length(); i++) {
      std::cout << ' ';
    }
    size_t arity = arities[name];
    bool first = true;
    for (size_t i = 0; i < arity; i++) {
      if (!first) {
        std::cout << ", ";
      }
      first = false;
      std::cout << 'r' << (int)ins.args[i];
    }
  } else {
    // Jump or anchor with a tag
    // Match the tag to the correct global anchor, then print it out as a
    // 2-letter code AA, AB, etc.
    auto match = table.MatchRegulated(ins.tag);
    if (match.size()) {
      size_t tag = match.front();
      std::string tag_name;
      tag_name += 'A' + tag / 26;
      tag_name += 'A' + tag % 26;
      if (name == "JumpIfNEq" || name == "JumpIfLess") {
        std::cout << "    " << emp::to_lower(name);
        for (size_t i = 0; i < 12 - name.length(); i++) {
          std::cout << ' ';
        }
        std::cout << 'r' << (int)ins.args[0] << ", r" << (int)ins.args[1]
                  << ", " << tag_name;
      } else if (name == "Global Anchor") {
        std::cout << tag_name << ':';
      } else {
        std::cout << "<unknown " << name << ">";
      }
    } else {
      std::cout << "<illegal instruction tag>";
    }
  }
  std::cout << '\n';
}

void PrintCode(
    sgpl::Program<AvidaSpec> &program,
    sgpl::JumpTable<AvidaSpec, AvidaSpec::global_matching_t> &table) {
  emp::map<std::string, size_t> arities{
      std::pair("Nop-0", 0),      std::pair("ShiftLeft", 1),
      std::pair("ShiftRight", 1), std::pair("Increment", 1),
      std::pair("Decrement", 1),  std::pair("Push", 1),
      std::pair("Pop", 1),        std::pair("SwapStack", 0),
      std::pair("Swap", 2),       std::pair("Add", 3),
      std::pair("Subtract", 3),   std::pair("Nand", 3),
      std::pair("Reproduce", 0),  std::pair("IO", 1),
      std::pair("Donate", 0)};
  emp::map<size_t, std::string> labels;

  std::cout << "--------" << std::endl;
  for (auto i : program) {
    PrintOp(i, arities, labels, program, table);
  }
}

#endif
