#pragma once

#include "../ConfigSetup.h"

#include "../default_mode/SymWorld.h"

#include "../../Empirical/include/emp/Evolve/World.hpp"
#include "../../Empirical/include/emp/math/random_utils.hpp"
#include "../../Empirical/include/emp/math/Random.hpp"

namespace test_utils {

void SetEmptyWellMixed(SymConfigBase& cfg) {
  // Set spatial structure mode to well-mixed
  cfg.SPATIAL_STRUCT_MODE("well-mixed");
  // Set size 0 environment with 0 initial population size
  cfg.WORLD_WIDTH(0);
  cfg.WORLD_HEIGHT(0);
  cfg.INIT_POP_SIZE(0);
}

// Wrapper for Symbumlation world classes intended to expose protected
// functions as public for testing.
// WARNING: protected functions were likely protected for a reason! Use at your
//          own risk. protected functions need testing, too ;)
// Example USAGE:
//    using world_t = test_utils::TestingWorldWrapper<LysisWorld,SymConfigLysis>;
template<typename WORLD_T, typename CONFIG_T=SymConfigBase>
class TestingWorldWrapper : public WORLD_T {
public:
  using wrapped_world_t = WORLD_T;
  using wrapped_config_t = CONFIG_T;

  TestingWorldWrapper(emp::Random& _random, emp::Ptr<wrapped_config_t> _config) :
    wrapped_world_t(_random, _config)
  { }

  void SetupSpatialStructure() {
    wrapped_world_t::SetupSpatialStructure();
  }

  void SetupHosts(long unsigned int* POP_SIZE) override {
    wrapped_world_t::SetupHosts(POP_SIZE);
  }

  void SetupSymbionts(long unsigned int* total_syms) override {
    wrapped_world_t::SetupSymbionts(total_syms);
  }

};

}