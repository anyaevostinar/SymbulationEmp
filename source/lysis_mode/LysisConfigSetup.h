#ifndef LYSIS_CONFIG_H
#define LYSIS_CONFIG_H
#include "../../Empirical/include/emp/config/config.hpp"
#include "../ConfigSetup.h"

EMP_EXTEND_CONFIG(SymConfigLysis, SymConfigBase,
  GROUP(LYSIS, "Lysis Settings, coming soon to the GUI!"),
  VALUE(LYSIS_CHANCE, double, -1, "Chance of lysis vs. lysogeny for starting population of phage, -1 for random distribution"),
  VALUE(CHANCE_OF_INDUCTION, double, 0, "Chance of induction for starting lysogenic phage, -1 for random distribution"),
  VALUE(LYSIS, bool, 0, "Should lysis occur? 0 for no, 1 for yes"),
  VALUE(BURST_SIZE, int, 10, "If there is lysis, this is how many symbionts should be produced during lysis. This will be divided by burst_time and that many symbionts will be produced every update"),
  VALUE(BURST_TIME, int, 10, "If lysis enabled, this is how many updates will pass before lysis occurs"),
  VALUE(PROPHAGE_LOSS_RATE, double, 0, "Rate at which infected lysogens become re-susceptible to new phage"),
  VALUE(BENEFIT_TO_HOST, bool, 0, "Should lysogenic phage give a benefit to their hosts? 0 for no, 1 for yes"),
  VALUE(PHAGE_INC_VAL, double, 0, "The compatibility of the prophage to its placement within the bacterium's genome, from 0 to 1, -1 for random distribution"),
  VALUE(HOST_INC_VAL, double, 0, "The compatibility of the bacterium for the phage's placement in its genome, from 0 to 1, -1 for random distribution"),
  VALUE(SYM_LYSIS_RES, double, 1, "How many resources required for symbiont to create offspring for lysis each update")
)

#endif
