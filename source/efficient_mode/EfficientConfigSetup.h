#ifndef EFFICIENT_CONFIG_H
#define EFFICIENT_CONFIG_H
#include "../../Empirical/include/emp/config/config.hpp"
#include "../ConfigSetup.h"

EMP_EXTEND_CONFIG(SymConfigEfficient, SymConfigBase,
  GROUP(DTH, "Settings for the Dirty Transmission Hypothesis"),
  VALUE(EFFICIENT_SYM, bool, 0, "Do you want symbionts that also have an efficiency value that evolves"),
  VALUE(COMPETITION_MODE, bool, 0, "Should a competition between two types be conducted? (Which is specified in the code)")
)

#endif
