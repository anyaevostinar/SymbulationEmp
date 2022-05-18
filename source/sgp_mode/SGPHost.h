#ifndef SGPHOST_H
#define SGPHOST_H

#include "../default_mode/Host.h"
#include "SGPWorld.h"
#include "sgpl/algorithm/execute_cpu.hpp"
#include "sgpl/hardware/Cpu.hpp"
#include "sgpl/library/OpLibraryCoupler.hpp"
#include "sgpl/library/prefab/CompleteOpLibrary.hpp"
#include "sgpl/program/Program.hpp"
#include "sgpl/spec/Spec.hpp"
#include "sgpl/utility/ThreadLocalRandom.hpp"
#include <cmath>
#include <iostream>
#include <string>

class SGPHost;

struct Peripheral {
  SGPHost *self;
};

// custom CPU operation
struct PrintVal {

  template <typename Spec>
  static void run(sgpl::Core<Spec> &core, const sgpl::Instruction<Spec> &inst,
                  const sgpl::Program<Spec> &,
                  typename Spec::peripheral_t &peripheral);

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

// extends prefab ControlFlowOpLibrary with SayHello operation
using library_t = sgpl::OpLibraryCoupler<sgpl::CompleteOpLibrary, PrintVal>;

// custom compile-time configurator type
using spec_t = sgpl::Spec<library_t, Peripheral>;

class SGPHost : public Host {
private:
  sgpl::Cpu<spec_t> cpu;
  sgpl::Program<spec_t> program;
  Peripheral peripheral;
  emp::Ptr<SGPWorld> my_world;

public:
  SGPHost(emp::Ptr<emp::Random> _random, emp::Ptr<SGPWorld> _world,
          emp::Ptr<SymConfigBase> _config, double _intval = 0.0,
          emp::vector<emp::Ptr<Organism>> _syms = {},
          emp::vector<emp::Ptr<Organism>> _repro_syms = {},
          std::set<int> _set = std::set<int>(), double _points = 0.0)
      : Host(_random, _world, _config, _intval, _syms, _repro_syms, _set,
             _points),
        program(25), peripheral{this} {
    my_world = _world;
    cpu.InitializeAnchors(program);
  }

  bool passed;
  bool encountered;
  inline static int numPassed;
  inline static int numEncountered;
  inline static int totalRuns;
  emp::WorldPosition lastPos;

  void Process(emp::WorldPosition pos) {
    // Run cpu step
    lastPos = pos;

    // Generate random signals to launch available virtual cores
    while (cpu.TryLaunchCore(emp::BitSet<64>(random))) {
    }

    // Execute up to 100 instructions
    passed = false;
    encountered = false;
    sgpl::execute_cpu<spec_t>(100, cpu, program, peripheral);
    if (passed) {
      numPassed++;
    }
    if (encountered) {
      numEncountered++;
    }
    totalRuns++;
    if ((totalRuns % 200000) == 0) {
      std::cout
        << "Found: "
        << (int) (100 * ((double) numEncountered / (double) totalRuns))
        << "%; passed: "
        << (int) (100 * ((double) numPassed / (double) numEncountered))
        << "%"
        << std::endl;
    }

    // Instead of calling Host::Process, do the important stuff here
    // Symbiotes are ignored for the time being
    // Our instruction handles reproduction
    if (GetDead()) {
      return;
    }
    GrowOlder();
  }

  void maybeReproduce(float chance) {
    if (random->P(chance)) {
      auto p = reproduce();
      my_world->DoBirth(p, lastPos.GetIndex());
    }
  }

  bool containsReproduceInstruction() {
    for (auto i : program) {
      if (i.GetOpName() == PrintVal::name()) {
        return true;
      }
    }
    return false;
  }

  emp::Ptr<Organism> makeNew() {
    emp::Ptr<SGPHost> host_baby =
        emp::NewPtr<SGPHost>(random, my_world, my_config, GetIntVal());
    host_baby->program = program;
    return host_baby;
  }

  void mutate() {
    Host::mutate();
    program.ApplyPointMutations(0.01);
  }

  void PrintCode() {
    std::cout << "--------" << std::endl;
    for (auto i : program) {
      std::cout << i.GetOpName() << "(";
      for (auto a : i.args) {
        std::cout << (int)a << " ";
      }
      std::cout << ")" << std::endl;
    }
  }
};

template <typename Spec>
void PrintVal::run(sgpl::Core<Spec> &core, const sgpl::Instruction<Spec> &inst,
                   const sgpl::Program<Spec> &,
                   typename Spec::peripheral_t &peripheral) {
  float val = core.registers[inst.args[0]];
  float dist = abs(val);
  if (dist < 1.0 && !peripheral.self->passed) {
    peripheral.self->passed = true;
    peripheral.self->maybeReproduce(0.2 * (1.0 - dist));
  }
  peripheral.self->encountered = true;
}

#endif