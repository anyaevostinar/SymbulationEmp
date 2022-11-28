#ifndef EFF_ORG
#define EFF_ORG

#include "../Organism.h"

class EfficientOrganism : public virtual Organism {
public:
  virtual double GetEfficiency() = 0;
};

#endif