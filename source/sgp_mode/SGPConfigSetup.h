#ifndef SGP_CONFIG_H
#define SGP_CONFIG_H
#include "../../Empirical/include/emp/config/config.hpp"
#include "../ConfigSetup.h"

enum SGPOrganismType {DEFAULT = 0, HEALTH, STRESS};
enum StressSymbiontType { MUTUALIST = 0, PARASITE, NEUTRAL };

EMP_EXTEND_CONFIG(SymConfigSGP, SymConfigBase,
  GROUP(SGP, "Complex Genomes Settings"),
  VALUE(CYCLES_PER_UPDATE, size_t, 4, "Number of CPU cycles that organisms run every update"),
  VALUE(DONATION_STEAL_INST, bool, 0, "1 if you want donate and steal instructions in the instruction set, 0 if not"),
  VALUE(RANDOM_IO_INPUT, bool, true, "1 to give organisms random input when they IO, 0 to give them only ones"),
  VALUE(ORGANISM_TYPE, size_t, DEFAULT, "What sgp organisms should population the world? (0 for default SGP, 1 for Health organisms, 2 for stress organisms)"),
  VALUE(VT_TASK_MATCH, bool, 0, "Should task matching be required for vertical transmission? (0 for no, 1 for yes)"),
  VALUE(TRACK_PARENT_TASKS, bool, 0, "Should parental task completion data be used for reproductive task matching (instead of the individual's task completion data?) (0 for no, 1 for yes"),
  VALUE(NUTRIENT_DONATE_STEAL_PROP, double, 0.5, "What proportion of points should a symbiont donate to its host and what proportion of points should a symbiont steal from its host during nutrient type interactions"),


  GROUP(STRESS, "Stress Settings"),
  VALUE(STRESS_TYPE, size_t, MUTUALIST, "What kind of stress symbionts should be incorporated in stressful environments? (0 for mutualists, 1 for parasites, 2 for neutrals"),
  VALUE(EXTINCTION_FREQUENCY, size_t, 2000, "How often should extinction events occur (in updates)?"),
  VALUE(PARASITE_DEATH_CHANCE, double, 0.5, "What death chance does a parasite confer?"),
  VALUE(MUTUALIST_DEATH_CHANCE, double, 0.125, "What death chance does a mutualist confer?"),
  VALUE(BASE_DEATH_CHANCE, double, 0.25, "What death chance does a host have in the absence of symbionts?"),
  VALUE(CPU_TRANSFER_CHANCE, double, 0.5, "What is the chance for cycles to be stolen/donated?"),
  VALUE(CPU_TRANSFER_AMOUNT, double, 1, "How many updates should be stolen/donated by instruction based symbionts"),
  VALUE(ONLY_FIRST_TASK_CREDIT, int, 0, "Should organisms only get credit for their first task"),
  VALUE(ALLOW_TRANSITION_EVOLUTION, int, 0, "Should symbionts be allowed to evolve from mutualists to parasties and vice versa"),
  VALUE(BONUS_UPDATE_WAIT, int, 5, "If DONATION_STEAL_INST is 1 how many updates till a symbiont receives a free extra update"),
  VALUE(STARTING_BONUS, int, 1, "How many bonus updates do symbionts start with")
)

#endif