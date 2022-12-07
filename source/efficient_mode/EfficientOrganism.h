#ifndef EFF_ORG
#define EFF_ORG

#include "../Organism.h"

enum class TransmissionMode { Vertical, Horizontal };

class EfficientOrganism : public virtual Organism {
protected:
  double efficiency;

public:
  EfficientOrganism(double _efficiency) : efficiency(_efficiency) {}

  /**
   * Input: Efficiency value
   *
   * Output: None
   *
   * Purpose: Setting an efficient symbiont's efficiency value.
   */
  void SetEfficiency(double _in) {
    if (_in > 1 || _in < 0)
      throw "Invalid efficiency chance. Must be between 0 and 1 (inclusive)";
    efficiency = _in;
  }

  /**
   * Input: None
   *
   * Output: A double representing the symbiont's efficiency.
   *
   * Purpose: Getting an efficient symbiont's efficiency value.
   */
  double GetEfficiency() { return efficiency; }
};

#endif