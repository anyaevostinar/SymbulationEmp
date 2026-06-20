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

// Wrapper class that exposes internal SymWorld functions exclusively for use
// in unit testing.
class SymWorld_Testing : public SymWorld {
public:
  SymWorld_Testing(emp::Random& _random, emp::Ptr<SymConfigBase> _config) :
    SymWorld(_random, _config)
  { }

  void SetupSpatialStructure() {
    SymWorld::SetupSpatialStructure();
  }
};

template<typename BASE_T, typename CONFIG_T>
class TestingWorldWrapper : public BASE_T {
public:
  using wrapped_world_t = BASE_T;
  using wrapped_config_t = CONFIG_T;

  TestingWorldWrapper(emp::Random& _random, emp::Ptr<wrapped_config_t> _config) :
    wrapped_world_t(_random, _config)
  { }

  void SetupSpatialStructure() {
    wrapped_world_t::SetupSpatialStructure();
  }
};

}