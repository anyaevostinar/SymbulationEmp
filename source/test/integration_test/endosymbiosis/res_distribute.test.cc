#include "../../../default_mode/Host.h"
#include "../../../default_mode/Symbiont.h"
#include "../../../default_mode/SymWorld.h"
#include "../../../default_mode/WorldSetup.cc"

TEST_CASE("Resource Distribution Results", "[integration]"){
  emp::Random random(29);
  SymConfigBase config;
  SymWorld world(random, &config);


  config.HOST_INT(0); //host-sym interaction does not exist at the beginning
  config.SYM_INT(0);
  config.SYM_INFECTION_CHANCE(0);
  config.MUTATION_SIZE(0.05);
  config.FREE_LIVING_SYMS(1);
  config.VERTICAL_TRANSMISSION(0.5);
  config.HOST_AGE_MAX(60);
  config.SYM_AGE_MAX(30);
  config.ECTOSYMBIOTIC_IMMUNITY(1);
  config.SYM_VERT_TRANS_RES(200);
  config.SYM_HORIZ_TRANS_RES(300);
  config.HOST_REPRO_RES(600);
  config.GRID_X(100);
  config.GRID_Y(100);
  config.UPDATES(1000); //shorter than standard experiments
  config.RES_DISTRIBUTE(500); //intermediary host res amount

  worldSetup(&world, &config);
  REQUIRE(1 == 1);

  emp::DataMonitor<double, emp::data::Histogram>& host_val_node = world.GetHostIntValDataNode();
  emp::DataMonitor<double, emp::data::Histogram>& free_sym_val_node = world.GetFreeSymIntValDataNode();
  emp::DataMonitor<double, emp::data::Histogram>& hosted_sym_val_node = world.GetHostedSymIntValDataNode();
  emp::DataMonitor<int>& free_sym_count_node = world.GetCountFreeSymsDataNode();
  emp::DataMonitor<int>& hosted_sym_count_node = world.GetCountHostedSymsDataNode();

  WHEN("Ectosymbiosis and endosymbiosis are not permited"){
    config.ECTOSYMBIOSIS(0);
    config.SYM_LIMIT(0);
    config.FREE_SYM_RES_DISTRIBUTE(500);

    world.RunExperiment(false);

    THEN("Free living syms and hosts don't interact"){
      //no hosted syms, no average
      REQUIRE(std::isnan(hosted_sym_val_node.GetMean()));

      //hosts and free syms have interaction values that are very close to 0.
      double free_sym_mean = free_sym_val_node.GetMean();
      double host_mean = host_val_node.GetMean();

      //mutation always happens & mutation size is 0.05, so check it's within a 2x range
      REQUIRE(free_sym_mean < 0.1);
      REQUIRE(free_sym_mean > -0.1);
      REQUIRE(host_mean < 0.1);
      REQUIRE(host_mean > -0.1);
    }
  }

  WHEN("Only ectosymbiosis is permitted"){
    config.ECTOSYMBIOSIS(1);
    config.SYM_LIMIT(0);

    WHEN("Symbionts get few resources from the world"){
      config.FREE_SYM_RES_DISTRIBUTE(10);
      world.RunExperiment(false);
      THEN("Ectosymbionts become very parasitic"){
        double fls_val_mean = free_sym_val_node.GetMean();
        REQUIRE(fls_val_mean < -0.5);
      }
    }

    WHEN("Symbionts get intermediary resources from the world"){
      config.FREE_SYM_RES_DISTRIBUTE(100);
      world.RunExperiment(false);
      THEN("Ectosymbionts become very parasitic"){
        double fls_val_mean = free_sym_val_node.GetMean();
        REQUIRE(fls_val_mean < -0.5);
      }
    }

    WHEN("Symbionts get many resources from the world"){
      config.FREE_SYM_RES_DISTRIBUTE(1000);
      world.RunExperiment(false);
      THEN("Ectosymbionts remain neutral"){
        double fls_val_mean = free_sym_val_node.GetMean();
        REQUIRE(fls_val_mean > -0.2);
        REQUIRE(fls_val_mean < 0.2);
      }
    }
  }

  WHEN("Only endosymbiosis is permitted"){
    config.ECTOSYMBIOSIS(0);
    config.SYM_LIMIT(1);

    WHEN("Symbionts get few resources from the world"){
      config.FREE_SYM_RES_DISTRIBUTE(10);
      world.RunExperiment(false);
      THEN("Endosymbionts go extinct"){
        REQUIRE(hosted_sym_count_node.GetTotal() == 0);
      }
    }

    WHEN("Symbionts get intermediary resources from the world"){
      config.FREE_SYM_RES_DISTRIBUTE(100);
      world.RunExperiment(false);
      THEN("There is a high ratio of endosymbionts to ecotsymbionts"){
        double ratio = hosted_sym_count_node.GetTotal() / free_sym_count_node.GetTotal();
        REQUIRE(ratio > 3);
      }
      THEN("Endosymbionts may go mutualistic"){
        double hosted_sym_mean = hosted_sym_val_node.GetMean();
        REQUIRE(hosted_sym_mean > 0);

      }
    }

    WHEN("Symbionts get many resources from the world"){
      config.FREE_SYM_RES_DISTRIBUTE(1000);
      world.RunExperiment(false);

      THEN("Endosymbionts may become parasitic"){
        double hosted_sym_mean = hosted_sym_val_node.GetMean();
        REQUIRE(hosted_sym_mean < 0.0);
      }
      THEN("There may be a low ratio of endosymbionts to ectosymbionts"){
        double ratio = hosted_sym_count_node.GetTotal() / free_sym_count_node.GetTotal();
        REQUIRE(ratio < 1);
      }
    }
  }

  WHEN("Both endosymbiosis and ectosymbiosis is permitted"){
    config.ECTOSYMBIOSIS(1);
    config.SYM_LIMIT(1);

    WHEN("Symbionts get few resources from the world"){
      config.FREE_SYM_RES_DISTRIBUTE(10);
      world.RunExperiment(false);
      THEN("There is an intermediary ratio of endosymbionts to ectosymbionts"){
        double ratio = hosted_sym_count_node.GetTotal() / free_sym_count_node.GetTotal();
        REQUIRE(ratio < 2);
        REQUIRE(ratio > 1);

      }
      THEN("Ectosymbionts sometimes become parasitic"){
        REQUIRE(free_sym_val_node.GetMean() < 0.0);
      }
      THEN("Endosymbionts sometimes become mutualistic"){
        REQUIRE(hosted_sym_val_node.GetMean() > 0.0);
      }
    }

    WHEN("Symbionts get intermediary resources from the world"){
      config.UPDATES(4000); // behavior is much less consistent, let it run for longer
      config.FREE_SYM_RES_DISTRIBUTE(100);
      world.RunExperiment(false);
      THEN("There is a high ratio of endosymbionts to ecotsymbionts"){
        double ratio = hosted_sym_count_node.GetTotal() / free_sym_count_node.GetTotal();
        REQUIRE(ratio > 3);
      }
      THEN("Ectosymbionts sometimes become parasitic"){
        REQUIRE(free_sym_val_node.GetMean() < 0.0);
      }
      THEN("Endosymbionts sometimes become parasitic"){
        REQUIRE(hosted_sym_val_node.GetMean() < -0.5);
      }
    }

    WHEN("Symbionts get many resources from the world"){
      config.UPDATES(4000); // behavior is much less consistent, let it run for longer
      config.FREE_SYM_RES_DISTRIBUTE(1000);
      world.RunExperiment(false);

      THEN("Ectosymbionts sometimes become mutualistic"){
        REQUIRE(free_sym_val_node.GetMean() > 0.0);
      }
      THEN("Endosymbionts sometimes become mutualistic"){
        REQUIRE(hosted_sym_val_node.GetMean() > 0.0);
      }


      THEN("There is an intermediary ratio of endosymbionts to ectosymbionts"){
        double ratio = hosted_sym_count_node.GetTotal() / free_sym_count_node.GetTotal();
        REQUIRE(ratio < 2);
        REQUIRE(ratio > 1);
      }
    }

  }
}
