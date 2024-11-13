#ifndef PGG_CONFIG_H
#define PGG_CONFIG_H
#include "../../Empirical/include/emp/config/config.hpp"
#include "../ConfigSetup.h"

EMP_EXTEND_CONFIG(SymConfigPGG, SymConfigBase,
  GROUP(PGG, "Public Goods Game Settings"),
  VALUE(PGG_DONATE, double, 0, "Ratio of symbionts‘ energy to PGG pool that experiment should start with"),
  VALUE(PGG, int, 0, "whether have social goods game among syms"),
  VALUE(PGG_SYNERGY, double, 1.1, "Amount symbiont's returned resources should be multiplied by when doing PGG"),
  VALUE(PGG_DONATE_RANDOM, bool, 0, "Make the ratio of symbionts' energy to PGG pool be random for each organism at the start of the experiment"),
  VALUE(PGG_DONATE_MIN, double, 0.0, "The minimum value of the symbionts' donation rate, if randomly selected, at initialization"),
  VALUE(PGG_DONATE_MAX, double, 1.0, "The maximum value of the symbionts' donation rate, if randomly selected, at initialization"),
  VALUE(PGG_DONATE_NORMAL, bool, 0, "Make the random initialization of the symbionts' donation rate be initialized using a normal distribution"),
  VALUE(PGG_DONATE_NORMAL_MEAN, double, 0.5, "The mean of the normal distribution of the initialization of the symbionts' donation rate"),
  VALUE(PGG_DONATE_NORMAL_STD, double, 0.167, "The standard deviation of the normal distribution of the initialization of the symbionts' donation rate")
)

#endif
