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

  static std::string name() { return "PrintVal"; }

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
        program(100), peripheral{this} {
    my_world = _world;
    cpu.InitializeAnchors(program);
  }

  bool happened;
  bool added;

  void Process(emp::WorldPosition pos) {
    // run cpu step

    // generate random signals to launch available virtual cores
    while (cpu.TryLaunchCore(emp::BitSet<64>(random)))
      ;

    // execute up to one thousand instructions
    happened = false;
    added = false;
    sgpl::execute_cpu<spec_t>(100, cpu, program, peripheral);

    Host::Process(pos);
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
        std::cout << (int) a << " ";
      }
      std::cout << ")" << std::endl;
    }
  }
};

long h_total = 0;
long h_got = 0;

template <typename Spec>
void PrintVal::run(sgpl::Core<Spec> &core, const sgpl::Instruction<Spec> &inst,
                   const sgpl::Program<Spec> &,
                   typename Spec::peripheral_t &peripheral) {
  float val = core.registers[inst.args[0]];
  float dist = abs(val);
  if (dist < 1.0 && !peripheral.self->happened) {
    peripheral.self->happened = true;
    h_got++;
    peripheral.self->AddPoints(-std::min(peripheral.self->GetPoints(), 5000.0));
  } else {
    peripheral.self->AddPoints(5000);
  }
  if (!peripheral.self->added) {
    peripheral.self->added = true;
    h_total++;
    if ((h_total & 0b111111111111111) == 0) {
      std::cout << "percent: " <<  ((double) h_got) /  ((double) h_total) << std::endl;
    }
  }
}

#endif