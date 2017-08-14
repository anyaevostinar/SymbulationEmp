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
    // changing 0.5 to 1.0 and 0.0 to 1.0 to see if I understand where these numbers go
    world.Insert( Host(1.0, Symbiont(), std::set<int>(), 1.0), 5 );
    world.Print(PrintOrg);
  }


};
