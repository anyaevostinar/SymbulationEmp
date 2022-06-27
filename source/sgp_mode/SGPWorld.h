#ifndef SGPWORLD_H
#define SGPWORLD_H

#include "../default_mode/SymWorld.h"
#include "../default_mode/DataNodes.h"

class SGPWorld : public SymWorld {
public:
    SGPWorld(emp::Random& r, emp::Ptr<SymConfigBase> _config) : SymWorld(r, _config)  {}
};

#endif