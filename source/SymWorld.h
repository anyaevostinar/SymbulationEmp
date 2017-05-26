#include "evo/World.h"
#include "tools/Random.h"
#include <set>
#include "SymOrg.h"

class SymWorld {
 private:
  emp::Random random;
  emp::evo::GridWorld<Host> world;

 public:
  SymWorld() {
    world.ConfigPop(20,20);
    world.Insert( Host(0.5, Symbiont(), std::set<int>(), 0.0), 5 );
    world.Print(PrintOrg);
  }


};
