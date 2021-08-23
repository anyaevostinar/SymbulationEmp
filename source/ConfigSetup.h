#ifndef CONFIG_H
#define CONFIG_H
#include "../../Empirical/include/emp/config/config.hpp"

//TODO: use GROUPs
EMP_BUILD_CONFIG(SymConfigBase,
    GROUP(MAIN, "Global Settings"),
    VALUE(SEED, int, 10, "What value should the random seed be? If seed <= 0, then it is randomly re-chosen."),
    VALUE(COMPETITION_MODE, bool, 0, "Should a competition between two types be conducted? (Which is specified in the code)"),
    VALUE(DATA_INT, int, 100, "How frequently, in updates, should data print?"),
    VALUE(MUTATION_SIZE, double, 0.002, "Standard deviation of the distribution to mutate by"),
    VALUE(HORIZ_MUTATION_SIZE, double, -1, "Standard deviation of the distribution to mutate by for horizontal transmission, if -1 MUTATION_SIZE used"),
    VALUE(MUTATION_RATE, double, 1, "Value 0 to 1 of probability of mutation"),
    VALUE(HORIZ_MUTATION_RATE, double, -1, "Value 0 to 1 of probability of mutation for horizontal transmission, if -1 MUTATION_RATE used"),
    VALUE(SYNERGY, double, 5, "Amount symbiont's returned resources should be multiplied by"),
    VALUE(VERTICAL_TRANSMISSION, double, 0.7, "Value 0 to 1 of probability of symbiont vertically transmitting when host reproduces"),
    VALUE(HOST_INT, double, -2, "Interaction value from -1 to 1 that hosts should have initially, -2 for random"),
    VALUE(SYM_INT, double, -2, "Interaction value from -1 to 1 that symbionts should have initially, -2 for random"),
    VALUE(GRID_X, int, 100, "Width of the world, just multiplied by the height to get total size"),
    VALUE(GRID_Y, int, 100, "Height of world, just multiplied by width to get total size"),
    VALUE(POP_SIZE, int, -1, "Starting size of the host population, -1 for full starting population"),
    VALUE(UPDATES, int, 1001, "Number of updates to run before quitting"),
    VALUE(RES_DISTRIBUTE, int, 100, "Number of resources to give to each host each update if they are available"),
    VALUE(LIMITED_RES_TOTAL, int, -1, "Number of total resources available over the entire run, -1 for unlimited"),
    VALUE(HORIZ_TRANS, bool, 1, "Should non-lytic horizontal transmission occur? 0 for no, 1 for yes"),
    VALUE(HOST_REPRO_RES, double, 1000, "How many resources required for host reproduction"),
    VALUE(SYM_HORIZ_TRANS_RES, double, 100, "How many resources required for symbiont non-lytic horizontal transmission"),
    VALUE(GRID, bool, 0, "Do offspring get placed immediately next to parents on grid, same for symbiont spreading"),
    VALUE(EFFICIENT_SYM, bool, 0, "Do you want symbionts that also have an efficiency value that evolves"),
    VALUE(EFFICIENCY_MUT_RATE, double, -1, "The horizontal transmission mutation rate of the efficiency trait in symbionts, -1 if same as HORIZ_MUT_RATE"),
    VALUE(SYM_INFECTION_CHANCE, double, 1, "The chance (between 0 and 1) that a sym will infect a parallel host on process"),

    GROUP(LYSIS, "Lysis Settings, coming soon to the GUI!"),
    VALUE(LYSIS_CHANCE, double, -1, "Chance of lysis vs. lysogeny for starting population of phage, -1 for random distribution"),
    VALUE(CHANCE_OF_INDUCTION, double, 0, "Chance of induction for starting lysogenic phage, -1 for random distribution"),
    VALUE(MUTATE_LYSIS_CHANCE, int, 0, "Should the chance of lysis mutate? 0 for no, 1 for yes"),
    VALUE(SYM_LIMIT, int, 1, "Number of symbiont allowed to infect a single host"),
    VALUE(LYSIS, bool, 0, "Should lysis occur? 0 for no, 1 for yes"),
    VALUE(MUTATE_INDUCTION_CHANCE, bool, 0, "Should the chance of Induction mutate? 0 for no, 1 for yes"),
    VALUE(BURST_SIZE, int, 10, "If there is lysis, this is how many symbionts should be produced during lysis. This will be divided by burst_time and that many symbionts will be produced every update"),
    VALUE(BURST_TIME, int, 10, "If lysis enabled, this is how many updates will pass before lysis occurs"),
    VALUE(PROPHAGE_LOSS_RATE, double, 0, "Rate at which infected lysogens become re-susceptible to new phage"),
    VALUE(BENEFIT_TO_HOST, bool, 0, "Should lysogenic phage give a benefit to their hosts? 0 for no, 1 for yes"),
    VALUE(PHAGE_INC_VAL, double, 0, "The compatibility of the prophage to its placement within the bacterium's genome, from 0 to 1, -1 for random distribution"),
    VALUE(HOST_INC_VAL, double, 0, "The compatibility of the bacterium for the phage's placement in its genome, from 0 to 1, -1 for random distribution"),
    VALUE(MUTATE_HOST_INC_VAL, bool, 0, "Whether the bacterium genome mutates or not, 0 or 1"),
    VALUE(MUTATE_PHAGE_INC_VAL, bool, 0, "whether the incorporation_val mutates. 0 or 1"),
    VALUE(SYM_LYSIS_RES, double, 1, "How many resources required for symbiont to create offspring for lysis each update"),
    VALUE(START_MOI, double, 1, "Ratio of symbionts to hosts that experiment should start with"),
    VALUE(PHAGE_EXCLUDE, bool, 0, "Do phage have a decreased chance of getting into the host if there is already a lytic phage?"),
    VALUE(FREE_LIVING_SYMS, bool, 0, "Are symbionts able to live outside of the host?"),
    VALUE(MOVE_FREE_SYMS, bool, 0, "Should the free living syms move around in the world? 0 for no, 1 for yes"),

    VALUE(FILE_PATH, std::string, "", "Output file path"),
    VALUE(FILE_NAME, std::string, "_data_", "Root output file name"),

    GROUP(PGG,"Public Goods Game Settings"),
    VALUE(PGG_DONATE, double, 0, "Ratio of symbionts‘ energy to Pgg pool that experiment should start with"),
    VALUE(PGG, int, 0, "whether have social goods game among syms" ),
    VALUE(PGG_SYNERGY, double, 1.1, "Amount symbiont's returned resources should be multiplied by when doing PGG")

)
#endif
