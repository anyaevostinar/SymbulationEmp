#include "../../../lysis_mode/Bacterium.h"
#include "../../../lysis_mode/Phage.h"
#include "../../../lysis_mode/LysisWorld.h"
#include "../../../lysis_mode/LysisWorldSetup.cc"

TEST_CASE("Prophage Loss Rate Results", "[integration]"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;

    //smaller world than actual experiment, starting with full population
    config.GRID_X(40);
    config.GRID_Y(40);

    //PLR settings that are different than defaults
    config.MUTATION_SIZE(0.02);
    config.SYNERGY(2);
    config.VERTICAL_TRANSMISSION(0);
    config.HOST_INT(-0.5);
    config.SYM_INT(-1);
    config.RES_DISTRIBUTE(50);
    config.HOST_REPRO_RES(600);
    config.LYSIS(1);
    config.MUTATE_LYSIS_CHANCE(1);
    config.BURST_SIZE(999999999);
    config.BURST_TIME(100);
    config.SYM_LYSIS_RES(10);
    config.START_MOI(0.5);
    config.FREE_LIVING_SYMS(1);

    WHEN("Prophage Loss Rate is 0.05"){
        LysisWorld w(*random);
        LysisWorld * world = &w;

        config.PROPHAGE_LOSS_RATE(0.05);

        auto & node = world->GetLysisChanceDataNode();

        worldSetup(world, &config);
        int numupdates = 10;

        //Loop through updates
        for (int i = 0; i < numupdates; i++) {
            world->Update();
        }

        THEN("Phage evolve to be lytic"){
            
        }
    }
}