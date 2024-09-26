#ifndef SGP_CONFIG_H
#define SGP_CONFIG_H
#include "../../Empirical/include/emp/config/config.hpp"
#include "../ConfigSetup.h"

EMP_EXTEND_CONFIG(SymConfigSGP, SymConfigBase,
  GROUP(SGP, "Complex Genomes Settings"),
  VALUE(CYCLES_PER_UPDATE, size_t, 4, "Number of CPU cycles that organisms run every update"),
  VALUE(THREAD_COUNT, size_t, 12, "Number of threads used to process organisms in parallel"),
  VALUE(RANDOM_ANCESTOR, bool, false, "Randomize ancestor genomes instead of using the blank genome with just NOT and reproduction"),
  VALUE(TASK_TYPE, bool, 1, "If random ancestor off, 1 for NOT + repro starting genome, 0 for repro starting program"),
  VALUE(DONATION_STEAL_INST, bool, 1, "1 if you want donate and steal instructions in the instruction set, 0 if not"),
  VALUE(RANDOM_IO_INPUT, bool, true, "1 to give organisms random input when they IO, 0 to give them only ones"),

  VALUE(LIMITED_TASK_RESET_INTERVAL, size_t, 8, "Number of updates before an org is allowed to complete a non-unlimited task again"),
  VALUE(STEAL_PENALTY, double, 0.10, "Proportion of resources that are lost when stealing from a host"),
  VALUE(DONATE_PENALTY, double, 0.10, "Proportion of resources that are lost when donating to a host")
)

#endif