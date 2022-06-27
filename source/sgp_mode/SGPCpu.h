#ifndef SGPCPU_H
#define SGPCPU_H

#include "../default_mode/Host.h"
#include "SGPWorld.h"
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

// peripheral
template <typename T> struct Peripheral {
  T *organism;
  bool passed = false;
  bool encountered = false;
};

// operations
struct TryReproduce {

  template <typename Spec>
  static void run(sgpl::Core<Spec> &core, const sgpl::Instruction<Spec> &inst,
                  const sgpl::Program<Spec> &,
                  typename Spec::peripheral_t &peripheral) {
    float val = core.registers[inst.args[0]];
    float dist = abs(3.0 - val);
    if (dist <= 3.2 && !peripheral.passed) {
      if (dist < 3.0) {
        peripheral.passed = true;
      }
      peripheral.organism->maybeReproduce(pow(0.3 * (3.2 - dist), 4));
    }
    peripheral.encountered = true;
  }

  static std::string name() { return "TryReproduce"; }

  static size_t prevalence() { return 1; }

  template <typename Spec>
  static auto descriptors(const sgpl::Instruction<Spec> &inst) {

    return std::map<std::string, std::string>{
        {"argument a", std::to_string(static_cast<int>(inst.args[0]))},
        {"summary", "prints the value of a"},
    };
  }
};

/**
 * Represents the virtual CPU and the program genome for an organism in the SGP mode.
 */
template <typename T> class SGPCpu {
  // Library which doesn't include Terminal, the instruction to create a
  // constant That way the organism must evolve a mechanism for counting to
  // three
  using library_t = sgpl::internal::LibraryInstantiator<decltype(std::tuple_cat(
      // std::declval<sgpl::ControlFlowOpLibrary::parent_t>(),
      std::declval<sgpl::ArithmeticOpLibrary::parent_t>(),
      std::declval<std::tuple<TryReproduce, sgpl::Increment,
                              sgpl::global::Anchor>>()))>::type;
  // sgpl::OpLibraryCoupler<sgpl::ControlFlowOpLibrary,
  // sgpl::ArithmeticOpLibrary, PrintVal>;

  // custom compile-time configurator type
  using spec_t = sgpl::Spec<library_t, Peripheral<T>>;

private:
  sgpl::Cpu<spec_t> cpu;
  sgpl::Program<spec_t> program;
  Peripheral<T> peripheral;
  emp::Ptr<emp::Random> random;

  static inline int numPassed = 0;
  static inline int numEncountered = 0;
  static inline int totalRuns = 0;

public:
  SGPCpu(T *organism, emp::Ptr<emp::Random> random)
      : program(25), peripheral{organism}, random(random) {
    cpu.InitializeAnchors(program);
  }

  SGPCpu(T *organism, emp::Ptr<emp::Random> random, SGPCpu<T> oldCpu)
      : program(oldCpu.program), peripheral{organism}, random(random) {
    cpu.InitializeAnchors(program);
  }

  void runCpuStep() {
    // Generate random signals to launch available virtual cores
    while (cpu.TryLaunchCore(emp::BitSet<64>(random))) {
    }

    // Execute up to 100 instructions
    peripheral.passed = false;
    peripheral.encountered = false;
    sgpl::execute_cpu<spec_t>(100, cpu, program, peripheral);
    if (peripheral.passed) {
      numPassed++;
    }
    if (peripheral.encountered) {
      numEncountered++;
    }
    totalRuns++;
    if (totalRuns == 200000) {
      std::cout << typeid(T).name()
                << ": Found: "
                << (int)(100 * ((double)numEncountered / (double)totalRuns))
                << "%; passed: "
                << (int)(100 * ((double)numPassed / (double)numEncountered))
                << "%" << std::endl;
      totalRuns = 0;
      numEncountered = 0;
      numPassed = 0;
    }
  }

  bool containsReproduceInstruction() {
    for (auto i : program) {
      if (i.GetOpName() == TryReproduce::name()) {
        return true;
      }
    }
    return false;
  }

  void Mutate() { program.ApplyPointMutations(0.01); }

  void PrintCode() {
    std::cout << "--------" << std::endl;
    for (auto i : program) {
      std::cout << i.GetOpName() << "(";
      bool first = true;
      for (auto a : i.args) {
        if (!first) {
          std::cout << ", ";
        } else {
          first = false;
        }
        std::cout << (int)a;
      }
      std::cout << ")" << std::endl;
    }
  }
};

#endif