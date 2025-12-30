#ifndef SGP_CONFIG_H
#define SGP_CONFIG_H
#include "../../Empirical/include/emp/config/config.hpp"
#include "../ConfigSetup.h"

enum SGPInteractionMechanism {DEFAULT = 0, HEALTH, STRESS, NUTRIENT};
enum SymbiontType { MUTUALIST = 0, PARASITE, NEUTRAL };

EMP_EXTEND_CONFIG(SymConfigSGP, SymConfigBase,
  GROUP(SGP, "Complex Genomes Settings"),
  VALUE(CYCLES_PER_UPDATE, size_t, 4, "Number of CPU cycles that organisms run every update"),
  VALUE(DIFFERENT_TASK_VALUES, bool, 0, "Should tasks have different values (1, 2, 4, 8, 16)? If false, all will be 5"),
  VALUE(INTERACTION_MECHANISM, size_t, DEFAULT, "What sgp organisms should population the world? (0 for default SGP, 1 for Health organisms, 2 for stress organisms, 3 for nutrient organisms)"),
  VALUE(SYMBIONT_TYPE, size_t, MUTUALIST, "What kind of symbionts should be incorporated in stressful environments? (0 for mutualists, 1 for parasites, 2 for neutrals"),
  VALUE(VT_TASK_MATCH, bool, 0, "Should task matching be required for vertical transmission? (0 for no, 1 for yes)"),
  VALUE(HT_TASK_MATCH, bool, 1, "Should task matching be required for horizontal transmission? (0 for no, 1 for yes)"),
  VALUE(TRACK_PARENT_TASKS, int, 0, "Should parental task completion data be used for reproductive task matching (instead of the individual's task completion data?) (0 to use only current tasks, 1 to use only the parental tasks, 2 to use an OR of the parent and current tasks)"),
  VALUE(HOST_ONLY_FIRST_TASK_CREDIT, bool, 0, "Should hosts only get credit for their first task? (0 for no, 1 for yes)"),
  VALUE(SYM_ONLY_FIRST_TASK_CREDIT, bool, 0, "Should symbionts only get credit for their first task? (0 for no, 1 for yes)"),
  VALUE(HOST_MIN_CYCLES_BEFORE_REPRO, size_t, 0, "How many CPU cycles must a host execute before it can reproduce?"),
  VALUE(SYM_MIN_CYCLES_BEFORE_REPRO, size_t, 0, "How many CPU cycles must a symbiont execute before it can horizontally reproduce?"),
  VALUE(PREFERENTIAL_OUSTING, size_t, 0, "Should preferential ousting be on, and if so how? (0 = no preferential ousting, 1 = the incoming symbiont must have an equal or better match than the current symbiont in order to oust, 2 = the incoming symbiont must have a strictly better match than the current symbiont in order to oust)"),
  VALUE(ALLOW_TRANSITION_EVOLUTION, int, 0, "Should symbionts be allowed to evolve from mutualists to parasites and vice versa"),
  
  GROUP(STRESS, "Stress Settings"),
  VALUE(EXTINCTION_FREQUENCY, size_t, 2000, "How often should extinction events occur (in updates)?"),
  VALUE(PARASITE_DEATH_CHANCE, double, 0.5, "What death chance does a parasite confer?"),
  VALUE(MUTUALIST_DEATH_CHANCE, double, 0.125, "What death chance does a mutualist confer?"),
  VALUE(BASE_DEATH_CHANCE, double, 0.25, "What death chance does a host have in the absence of symbionts?"),
  VALUE(SAFE_TIME, size_t, 0, "How many updates at the beginning of the experiment should we wait before starting extinction events?"),
  VALUE(PARASITE_NUM_OFFSPRING_ON_STRESS_INTERACTION, size_t, 0, "How many offspring can stress parasite produce for free (no points or cycles required) when host dies during stress event?"),

  GROUP(NUTRIENT, "Nutrient Settings"),
  VALUE(NUTRIENT_DONATE_STEAL_PROP, double, 0.5, "What proportion of points should a symbiont donate to its host and what proportion of points should a symbiont steal from its host during nutrient type interactions"),

  GROUP(HEALTH, "Health Settings"),
  VALUE(CPU_TRANSFER_CHANCE, double, 0.5, "What is the chance for cycles to be stolen/donated?"),

  GROUP(HEALTH_INSTRUCTIONS, "Health Instruction Settings"),
  VALUE(DONATION_STEAL_INST, bool, 0, "1 if you want donate and steal instructions in the instruction set, 0 if not")

)

#endif
