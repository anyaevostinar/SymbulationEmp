#ifndef SGP_CONFIG_H
#define SGP_CONFIG_H

#include "../ConfigSetup.h"

#include "../../Empirical/include/emp/config/config.hpp"

#include <unordered_map>
#include <string>

namespace sgpmode {

// TODO - rename TASK_TYPE to be more specific

EMP_EXTEND_CONFIG(SymConfigSGP, SymConfigBase,
  GROUP(SGP, "Complex Genomes Settings"),
  VALUE(CYCLES_PER_UPDATE, size_t, 4, "Number of CPU cycles that organisms run every update"),
  VALUE(THREAD_COUNT, size_t, 12, "Number of threads used to process organisms in parallel"),
  VALUE(RANDOM_ANCESTOR, bool, false, "Randomize ancestor genomes instead of using the blank genome with just NOT and reproduction"),
  VALUE(TASK_TYPE, bool, 1, "If random ancestor off, 1 for NOT + repro starting genome, 0 for repro starting program"),
  VALUE(DONATION_STEAL_INST, bool, 1, "1 if you want donate and steal instructions in the instruction set, 0 if not"),
  VALUE(RANDOM_IO_INPUT, bool, true, "1 to give organisms random input when they IO, 0 to give them only ones"),
  VALUE(FIND_NEIGHBOR_HOST_ATTEMPTS, size_t, 4, "How many times to attempt finding a neighboring host for symbiont to horizontally transmit into"),
  VALUE(SYM_DONATE_PROP, double, 0.2, "Proportion of points for sym to donate to host on donate"),

  VALUE(LIMITED_TASK_RESET_INTERVAL, size_t, 8, "Number of updates before an org is allowed to complete a non-unlimited task again"),
  VALUE(STEAL_PENALTY, double, 0.10, "Proportion of resources that are lost when stealing from a host"),
  VALUE(DONATE_PENALTY, double, 0.10, "Proportion of resources that are lost when donating to a host"),

  GROUP(SGP_MUTATION, "SGP mutation group"),
  VALUE(SGP_MUT_PER_BIT_RATE, double, 0.01, "Per-bit mutation rate for sgp programs"),

  VALUE(ORGANISM_TYPE, std::string, "default", "What sgp organisms should population the world? (0 for default SGP, 1 for Health organisms, 2 for stress organisms)"),
  VALUE(VT_TASK_MATCH, bool, 0, "Should task matching be required for vertical transmission? (0 for no, 1 for yes)"),
  VALUE(TRACK_PARENT_TASKS, bool, 0, "Should parental task completion data be used for reproductive task matching (instead of the individual's task completion data?) (0 for no, 1 for yes"),

  GROUP(STRESS, "Stress Settings"),
  VALUE(STRESS_TYPE, std::string, "mutualist", "What kind of stress symbionts should be incorporated in stressful environments? (0 for mutualists, 1 for parasites, 2 for neutrals"),
  VALUE(EXTINCTION_FREQUENCY, size_t, 2000, "How often should extinction events occur (in updates)?"),
  VALUE(PARASITE_DEATH_CHANCE, double, 0.5, "What death chance does a parasite confer?"),
  VALUE(MUTUALIST_DEATH_CHANCE, double, 0.125, "What death chance does a mutualist confer?"),
  VALUE(BASE_DEATH_CHANCE, double, 0.25, "What death chance does a host have in the absence of symbionts?"),

  GROUP(TASK_ENVIRONMENT, "Task environment settings"),
  VALUE(TASK_ENV_CFG_PATH, std::string, "environment.json", "Json file that provides environment configuration"),
  VALUE(TASK_IO_BANK_SIZE, size_t, 100000, "How many possible task input/output combinations to pre-generate?"),
  VALUE(TASK_IO_UNIQUE_OUTPUT, bool, true, "Should each output in the pregenerated io combinations be unique?"),

  GROUP(DATA, "Data settings"),
  VALUE(PRINT_INTERVAL, size_t, 1, "How often to print run status")
)

}

#endif