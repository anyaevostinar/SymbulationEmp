#ifndef SGPWORLD_H
#define SGPWORLD_H

#include "../default_mode/SymWorld.h"
#include "../default_mode/DataNodes.h"

class SGPWorld : public SymWorld {
public:
    SGPWorld(emp::Random& r) : SymWorld(r)  {}
};

#endif