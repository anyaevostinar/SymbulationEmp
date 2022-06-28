#ifndef AVIDA_SPEC_H
#define AVIDA_SPEC_H

#include "../Organism.h"
#include "emp/Evolve/World.hpp"
#include "sgpl/algorithm/execute_cpu.hpp"
#include "sgpl/hardware/Cpu.hpp"
#include "sgpl/library/OpLibraryCoupler.hpp"
#include "sgpl/library/prefab/ArithmeticOpLibrary.hpp"
#include "sgpl/library/prefab/ControlFlowOpLibrary.hpp"
#include "sgpl/operations/flow_global/Anchor.hpp"
#include "sgpl/operations/unary/Increment.hpp"
#include "sgpl/operations/unary/Terminal.hpp"
#include "sgpl/program/Program.hpp"
#include "sgpl/spec/Spec.hpp"
#include "sgpl/utility/ThreadLocalRandom.hpp"
#include <cmath>
#include <iostream>
#include <string>

template <const size_t len = 8> struct IORingBuffer {
  uint32_t buffer[len];
  size_t next = 0;

  void push(uint32_t x) {
    buffer[next] = x;
    next = (next + 1) % len;
  }

  float any_pair(std::function<float(uint32_t, uint32_t)> func) {
    for (size_t i = 0; i < len; i++) {
      if (buffer[i] == 0 || buffer[(i + 1) % len] == 0)
        continue;
      float score = func(buffer[i], buffer[(i + 1) % len]);
      if (score != 0.0) {
        return score;
      }
    }
    return 0.0;
  }
};

struct AvidaPeripheral {
  std::vector<uint32_t> stack;
  std::vector<uint32_t> stack2;

  IORingBuffer<> input_buf;
  std::optional<uint32_t> output;

  std::bitset<64> done_tasks;

  emp::Ptr<Organism> host;

  emp::WorldPosition location;

  float merit = 0.0;

  AvidaPeripheral(emp::Ptr<Organism> host) : host(host) {}
};

namespace ainst {

#define INST(InstName, InstCode)                                               \
  struct InstName {                                                            \
    template <typename Spec>                                                   \
    static void                                                                \
    run(sgpl::Core<Spec> &core, const sgpl::Instruction<Spec> &inst,           \
        const sgpl::Program<Spec> &program, AvidaPeripheral &peripheral) {     \
      InstCode                                                                 \
    }                                                                          \
    static size_t prevalence() { return 1; }                                   \
    static std::string name() { return #InstName; }                            \
  };

INST(JumpIfNEq, {
  // Even != works differently on floats because of NaNs
  if (*(uint32_t *)&core.registers[inst.args[0]] !=
      *(uint32_t *)&core.registers[inst.args[1]]) {
    core.JumpToGlobalAnchorMatch(inst.tag);
  }
});
INST(JumpIfLess, {
  if (*(uint32_t *)&core.registers[inst.args[0]] <
      *(uint32_t *)&core.registers[inst.args[1]]) {
    core.JumpToGlobalAnchorMatch(inst.tag);
  }
});
INST(Increment, { ++*(uint32_t *)&core.registers[inst.args[0]]; });
INST(Decrement, { --*(uint32_t *)&core.registers[inst.args[0]]; });
// Unary shift (>>1 or <<1)
INST(ShiftLeft, { *(uint32_t *)&core.registers[inst.args[0]] <<= 1; });
INST(ShiftRight, { *(uint32_t *)&core.registers[inst.args[0]] >>= 1; });
INST(Add, {
  *(uint32_t *)&core.registers[inst.args[0]] =
      *(uint32_t *)&core.registers[inst.args[1]] +
      *(uint32_t *)&core.registers[inst.args[2]];
});
INST(Subtract, {
  *(uint32_t *)&core.registers[inst.args[0]] =
      *(uint32_t *)&core.registers[inst.args[1]] -
      *(uint32_t *)&core.registers[inst.args[2]];
});
INST(Nand, {
  *(uint32_t *)&core.registers[inst.args[0]] =
      ~(*(uint32_t *)&core.registers[inst.args[1]] &
        *(uint32_t *)&core.registers[inst.args[2]]);
});
INST(Push, {
  peripheral.stack.push_back(*(uint32_t *)&core.registers[inst.args[0]]);
});
INST(Pop, {
  if (peripheral.stack.empty()) {
    *(uint32_t *)&core.registers[inst.args[0]] = 0;
  } else {
    *(uint32_t *)&core.registers[inst.args[0]] = peripheral.stack.back();
    peripheral.stack.pop_back();
  }
});
INST(SwapStack, { std::swap(peripheral.stack, peripheral.stack2); });
INST(Swap, {
  std::swap(core.registers[inst.args[0]], core.registers[inst.args[1]]);
});
INST(Reproduce, {
  // TODO when should this work?
  float prob = peripheral.merit / 256.0;
  if (sgpl::tlrand.Get().P(prob)) {
    peripheral.merit = 0;
    peripheral.done_tasks.reset();
    peripheral.host->ReproduceAndSpawn(peripheral.location);
  }
});
// Set output to value of register and set register to new input
INST(IO, {
  uint32_t next = sgpl::tlrand.Get().GetBits50();
  peripheral.input_buf.push(next);
  peripheral.output = *(uint32_t *)&core.registers[inst.args[0]];
  *(uint32_t *)&core.registers[inst.args[0]] = next;
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
                    ainst::Reproduce, ainst::IO
                    // no h-search
                    >;

using AvidaSpec = sgpl::Spec<AvidaLibrary, AvidaPeripheral>;

void PrintOp(sgpl::Instruction<AvidaSpec> &ins,
             emp::map<std::string, size_t> &arities,
             emp::map<AvidaSpec::tag_t, std::string> &labels,
             sgpl::Program<AvidaSpec> &program) {
  const std::string &name = ins.GetOpName();
  if (arities.count(name)) {
    // Simple instruction
    std::cout << "    " << emp::to_lower(name);
    for (size_t i = 0; i < 12 - name.length(); i++) {
      std::cout << ' ';
    }
    size_t arity = arities[name];
    for (size_t i = 0; i < arity; i++) {
      std::cout << "r" << (int)ins.args[i] << ' ';
    }
  } else {
    std::string tag_name;
    if (labels.count(ins.tag)) {
      tag_name = labels[ins.tag];
    } else {
      size_t offset = labels.size();
      tag_name = 'A' + (char) (offset / 26);
      tag_name += 'A' + (char) (offset % 26);
      labels.insert(std::pair(ins.tag, tag_name));
      // program.j
    }
    if (name == "JumpIfNEq" || name == "JumpIfLess") {
      std::cout << "    " << emp::to_lower(name);
      for (size_t i = 0; i < 12 - name.length(); i++) {
        std::cout << ' ';
      }
      std::cout << tag_name;
    } else if (name == "Global Anchor") {
      std::cout << tag_name << ':';
    } else {
      std::cout << "<unknown " << name << ">";
    }
  }
  std::cout << '\n';
}

void PrintCode(sgpl::Program<AvidaSpec> &program) {
  emp::map<std::string, size_t> arities{
      std::pair("Nop-0", 0),      std::pair("ShiftLeft", 1),
      std::pair("ShiftRight", 1), std::pair("Increment", 1),
      std::pair("Decrement", 1),  std::pair("Push", 1),
      std::pair("Pop", 1),        std::pair("SwapStack", 0),
      std::pair("Swap", 3),       std::pair("Add", 3),
      std::pair("Subtract", 3),   std::pair("Nand", 3),
      std::pair("Reproduce", 0),  std::pair("IO", 1)};
  emp::map<AvidaSpec::tag_t, std::string> labels;

  std::cout << "--------" << std::endl;
  for (auto i : program) {
    PrintOp(i, arities, labels, program);
  }
}

float avidaCheckDefaultTasks(AvidaPeripheral &peripheral) {
  if (!peripheral.output.has_value()) {
    return false;
  }
  uint32_t check = peripheral.output.value();
  peripheral.output.reset();
  // The 9 default logic tasks in Avida
  return peripheral.input_buf.any_pair([&](uint32_t x, uint32_t y) {
    // NOT
    if (!peripheral.done_tasks[0] && check == ~x) {
      peripheral.done_tasks.set(0);
      return 1.0;
    }
    // NAND
    if (!peripheral.done_tasks[1] && check == ~(x & y)) {
      peripheral.done_tasks.set(1);
      return 1.0;
    }
    // AND
    if (!peripheral.done_tasks[2] && check == (x & y)) {
      peripheral.done_tasks.set(2);
      return 2.0;
    }
    // ORN
    // I'm pretty sure this is what ORN and ANDN are doing
    if (!peripheral.done_tasks[3] && check == (x | ~y)) {
      peripheral.done_tasks.set(3);
      return 2.0;
    }
    // OR
    if (!peripheral.done_tasks[4] && check == (x | y)) {
      peripheral.done_tasks.set(4);
      return 3.0;
    }
    // ANDN
    if (!peripheral.done_tasks[5] && check == (x & ~y)) {
      peripheral.done_tasks.set(5);
      return 3.0;
    }
    // NOR
    if (!peripheral.done_tasks[6] && check == ~(x | y)) {
      std::cout << "NOR" << std::endl;
      peripheral.done_tasks.set(6);
      return 4.0;
    }
    // XOR
    if (!peripheral.done_tasks[7] && check == (x ^ y)) {
      std::cout << "XOR" << std::endl;
      peripheral.done_tasks.set(7);
      return 4.0;
    }
    // EQU (=NXOR)
    if (!peripheral.done_tasks[8] && check == ~(x ^ y)) {
      std::cout << "EQU!" << std::endl;
      peripheral.done_tasks.set(8);
      return 5.0;
    }
    return 0.0;
  });
}

/* TASKS

REACTION  NOT  not   process:value=1.0:type=pow  requisite:max_count=1
REACTION  NAND nand  process:value=1.0:type=pow  requisite:max_count=1
REACTION  AND  and   process:value=2.0:type=pow  requisite:max_count=1
REACTION  ORN  orn   process:value=2.0:type=pow  requisite:max_count=1
REACTION  OR   or    process:value=3.0:type=pow  requisite:max_count=1
REACTION  ANDN andn  process:value=3.0:type=pow  requisite:max_count=1
REACTION  NOR  nor   process:value=4.0:type=pow  requisite:max_count=1
REACTION  XOR  xor   process:value=4.0:type=pow  requisite:max_count=1
REACTION  EQU  equ   process:value=5.0:type=pow  requisite:max_count=1
*/

/* INSTRUCTIONS

# No-ops
INST nop-A         # a
INST nop-B         # b
INST nop-C         # c

# Flow control operations
INST if-n-equ      # d
INST if-less       # e
INST if-label      # f
INST mov-head      # g
INST jmp-head      # h
INST get-head      # i
INST set-flow      # j

# Single Argument Math
INST shift-r       # k
INST shift-l       # l
INST inc           # m
INST dec           # n
INST push          # o
INST pop           # p
INST swap-stk      # q
INST swap          # r

# Double Argument Math
INST add           # s
INST sub           # t
INST nand          # u

# Biological Operations
INST h-copy        # v
INST h-alloc       # w
INST h-divide      # x

# I/O and Sensory
INST IO            # y
INST h-search      # z
*/

#endif