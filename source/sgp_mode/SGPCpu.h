#ifndef SGPCPU_H
#define SGPCPU_H

#include "../default_mode/Host.h"
#include "AvidaSpec.h"
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

/**
 * Represents the virtual CPU and the program genome for an organism in the SGP
 * mode.
 */
class SGPCpu {
  using spec_t = AvidaSpec;

private:
  sgpl::Cpu<spec_t> cpu;
  sgpl::Program<spec_t> program;
  emp::Ptr<emp::Random> random;

public:
  AvidaPeripheral peripheral;

  SGPCpu(emp::Ptr<Organism> organism, emp::Ptr<SGPWorld> world,
         emp::Ptr<emp::Random> random)
      : program(100), random(random), peripheral(organism, world) {
    cpu.InitializeAnchors(program);
  }

  SGPCpu(emp::Ptr<Organism> organism, emp::Ptr<SGPWorld> world,
         emp::Ptr<emp::Random> random, SGPCpu &oldCpu)
      : program(oldCpu.program), random(random), peripheral(organism, world) {
    cpu.InitializeAnchors(program);
  }

  void runCpuStep(emp::WorldPosition location, size_t nCycles = 30) {
    // Generate random signals to launch available virtual cores
    while (cpu.TryLaunchCore(emp::BitSet<64>(random))) {
    }

    peripheral.location = location;

    // Execute up to 30 instructions
    sgpl::execute_cpu<spec_t>(nCycles, cpu, program, peripheral);
  }

  void Mutate() { program.ApplyPointMutations(0.01); }

  void PrintCode() {
    ::PrintCode(program, cpu.GetActiveCore().GetGlobalJumpTable());
  }
};

#endif