#ifndef ANTIBIOTIC_CONFIG_H
#define ANTIBIOTIC_CONFIG_H
#include "../../Empirical/include/emp/config/config.hpp"
#include "../ConfigSetup.h"
#include "../sgp_mode/SGPConfigSetup.h"

namespace antibioticmode{

/*Notes: can we set different defaults for values in this config?*/
EMP_EXTEND_CONFIG(AntibioticConfig, sgpmode::SymConfigSGP,
  GROUP(ANTI, "Settings for antibiotic resistance"),
  VALUE(NUM_HUMANS, size_t, 10, "Number of humans in the simulation")
)
}

#endif
