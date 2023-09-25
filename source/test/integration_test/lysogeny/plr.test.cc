#include "../../../lysis_mode/LysisWorldSetup.cc"

TEST_CASE("Prophage Loss Rate Results", "[integration]"){
    emp::Ptr<emp::Random> random = new emp::Random(5);
    SymConfigLysis config;

    //smaller world than actual experiment, starting with full population
    config.GRID_X(100);
    config.GRID_Y(100);

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
    config.UPDATES(3000);

    WHEN("Prophage Loss Rate is 0.05"){
        LysisWorld w(*random, &config);
        LysisWorld * world = &w;

        config.PROPHAGE_LOSS_RATE(0.05);

        auto & node = world->GetLysisChanceDataNode();

        world->Setup();

        world->RunExperiment(false);

        THEN("Phage evolve to be lytic"){
            double avg = node.GetMean();
            REQUIRE(avg >= 0.75);
        }
    }

    WHEN("Prophage Loss Rate is 0.025"){
        LysisWorld w(*random, &config);
        LysisWorld * world = &w;

        config.PROPHAGE_LOSS_RATE(0.025);

        auto & node = world->GetLysisChanceDataNode();

        world->Setup();

        world->RunExperiment(false);

        THEN("Phage evolve to be temperate"){
            double avg = node.GetMean();
            REQUIRE(avg >= 0.30);
            REQUIRE(avg <= 0.50);
        }
    }

    WHEN("Prophage Loss Rate is 0"){
        LysisWorld w(*random, &config);
        LysisWorld * world = &w;

        config.PROPHAGE_LOSS_RATE(0);

        auto & node = world->GetLysisChanceDataNode();

        world->Setup();

        world->RunExperiment(false);

        THEN("Phage evolve to be lysogenic"){
            double avg = node.GetMean();
            REQUIRE(avg <= 0.10);
        }
    }
}
