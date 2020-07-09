#ifndef SYM_CONFIG_H
#define SYM_CONFIG_H

/*
Configuration for parameters. Refactored to make code more readable and editable
*/
#include "../../Empirical/source/config/ArgManager.h"
using namespace std;

EMP_BUILD_CONFIG(SymConfigBase,
    VALUE(SEED, int, 17, "What value should the random seed be? If seed <= 0, then it is randomly re-chosen."),
    VALUE(DATA_INT, int, 100, "How frequently, in updates, should data print?"),
    VALUE(MUTATION_RATE, double, 0.002, "Standard deviation of the distribution to mutate by"),
    VALUE(SYNERGY, double, 5, "Amount symbiont's returned resources should be multiplied by"),
    VALUE(VERTICAL_TRANSMISSION, double, 0, "Value 0 to 1 of probability of symbiont vertically transmitting when host reproduces"),
    VALUE(HOST_INT, double, -2, "Interaction value from -1 to 1 that hosts should have initially, -2 for random"),
    VALUE(SYM_INT, double, -2, "Interaction value from -1 to 1 that symbionts should have initially, -2 for random"),
    VALUE(GRID_X, int, 50, "Width of the world, just multiplied by the height to get total size"),
    VALUE(GRID_Y, int, 50, "Height of world, just multiplied by width to get total size"),
    VALUE(UPDATES, int, 1001, "Number of updates to run before quitting"),
    VALUE(SYM_LIMIT, int, 1, "Number of symbiont allowed to infect a single host"),
    VALUE(LYSIS, bool, 0, "Should lysis occur? 0 for no, 1 for yes"),
    VALUE(HORIZ_TRANS, bool, 1, "Should non-lytic horizontal transmission occur? 0 for no, 1 for yes"),
    VALUE(BURST_SIZE, int, 10, "If there is lysis, this is how many symbionts should be produced during lysis. This will be divided by burst_time and that many symbionts will be produced every update"),
    VALUE(BURST_TIME, int, 10, "If lysis enabled, this is how many updates will pass before lysis occurs"),
    VALUE(HOST_REPRO_RES, double, 1000, "How many resources required for host reproduction"),
    VALUE(SYM_LYSIS_RES, double, 1, "How many resources required for symbiont to create offspring for lysis each update"),
    VALUE(SYM_HORIZ_TRANS_RES, double, 100, "How many resources required for symbiont non-lytic horizontal transmission"),
    VALUE(START_MOI, int, 1, "Ratio of symbionts to hosts that experiment should start with"),
    VALUE(GRID, bool, 0, "Do offspring get placed immediately next to parents on grid, same for symbiont spreading"),
    VALUE(FILE_PATH, string, "", "Output file path"),
    VALUE(FILE_NAME, string, "_data_", "Root output file name")
)

#endif