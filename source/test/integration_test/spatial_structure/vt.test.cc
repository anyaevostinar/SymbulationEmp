#include "../../../default_mode/WorldSetup.cc"


TEST_CASE("Vertical Transmission Results", "[integration]"){
  //based on paper: Vostinar and Ofria, 2018, "Spatial Structure Can Decrease Symbiotic Cooperation."

  emp::Random random(29);
  SymConfigBase config;
  SymWorld world(random, &config);

  config.MUTATION_SIZE(0.05);

  emp::DataMonitor<double, emp::data::Histogram>& sym_val_node = world.GetHostedSymIntValDataNode();
  emp::DataMonitor<int>& sym_count_node = world.GetCountHostedSymsDataNode();
  
  WHEN("Mutualists are already present in the population (mixed population)") {
    config.HOST_INT(-2);
    config.SYM_INT(-2);
    world.Setup();

    WHEN("Vertical transmission rate is 0") {
      config.VERTICAL_TRANSMISSION(0);
      world.RunExperiment(false);
      THEN("Clear parasitism evolves") {
        REQUIRE(sym_val_node.GetMean() < -0.5);
      }
    }
    WHEN("Vertical transmission rate is 1") {
      config.VERTICAL_TRANSMISSION(1);
      world.RunExperiment(false);
      THEN("Clear mutualism evolves") {
        REQUIRE(sym_val_node.GetMean() > 0.5);
      }
    }
    WHEN("Vertical transmission rate  is low (0.09)") {
      config.VERTICAL_TRANSMISSION(0.09);
      int parasite_count = 0;
      int mutualist_count = 0;
      world.RunExperiment(false);
      THEN("A cooexistence between parasitic and mutualistic symbionts may develop") {
        for (int i = 0; i < 6; i++) {
          parasite_count += sym_val_node.GetHistCounts()[i];
          mutualist_count += sym_val_node.GetHistCounts()[19-i];
        }
        REQUIRE(mutualist_count > 0);
        REQUIRE(parasite_count > 0);
        REQUIRE(parasite_count / mutualist_count > 0.1);
      }
    }
  }
  WHEN("Organisms begin the experiment with an interaction value of 0 (all-neutral population)") {
    config.HOST_INT(0);
    config.SYM_INT(0);
   

    WHEN("Vertical transmission rate is 1") {
      config.VERTICAL_TRANSMISSION(1);
      world.Setup();
      world.RunExperiment(false);
      THEN("Mutualists evolve"){
        REQUIRE(sym_val_node.GetMean() > 0.5);
      }
    }
    WHEN("Vertical transmission rate is less than 1") {
      config.VERTICAL_TRANSMISSION(0.2);
      config.MUTATION_SIZE(0.01);
      config.START_MOI(0.5);
      world.Setup();
      world.RunExperiment(false);
      THEN("Symbionts go extinct"){
        REQUIRE(sym_count_node.GetTotal() == 0);
      }
    }
  }
  WHEN("Symbionts begin the experiment with parasitic phenotypes"){
    config.HOST_INT(0);
    // not using random parasite values, which the paper used
    config.SYM_INT(-0.5); 
    world.Setup();

    WHEN("Vertical transmission rate is 0") {
      config.VERTICAL_TRANSMISSION(0);
      world.RunExperiment(false);
      THEN("Symbionts become very parasitic"){
        REQUIRE(sym_val_node.GetMean() < -0.5);
      }
    }
    WHEN("Vertical transmission rate is intermediate (0.5)"){
      config.VERTICAL_TRANSMISSION(0.5);
      world.RunExperiment(false);
      THEN("Symbionts become slightly parasitic"){
        REQUIRE(sym_val_node.GetMean() < -0.2);
      
      }
    }
    WHEN("Vertical transmission rate is high"){
      config.VERTICAL_TRANSMISSION(0.7);
      config.MUTATION_RATE(0.95);
      config.MUTATION_SIZE(0.04);
      world.RunExperiment(false);
      THEN("Symbionts become neutral"){
        double mean_sym_val = sym_val_node.GetMean();
        REQUIRE(mean_sym_val > -0.2);
        REQUIRE(mean_sym_val < 0.2);
      }
    }
  }
  WHEN("Symbionts begin the experiment with mutualistic phenotypes"){
    config.HOST_INT(0);
    // not using random parasite values, which the paper used
    config.SYM_INT(0.5);
    world.Setup();

    WHEN("Vertical transmission rate is 0") {
      config.VERTICAL_TRANSMISSION(0);
      world.RunExperiment(false);
      THEN("Symbionts become extinct") {
        REQUIRE(sym_count_node.GetTotal() == 0);
      }
    }
    WHEN("Vertical transmission rate is intermediate (0.5)") {
      config.VERTICAL_TRANSMISSION(0.5);
      world.RunExperiment(false);
      THEN("Symbionts become mutualistic") {
        REQUIRE(sym_val_node.GetMean() > 0.3);
      }
    }
    WHEN("Vertical transmission rate is high") {
      config.VERTICAL_TRANSMISSION(1);
      world.RunExperiment(false);
      THEN("Symbionts become very mutualistic") {
        REQUIRE(sym_val_node.GetMean() > 0.7);
      }
    }
  }
  
  WHEN("Spatial structure is on") {
    SymWorld off_world(random, &config);
    emp::DataMonitor<double, emp::data::Histogram>& off_sym_val_node = off_world.GetHostedSymIntValDataNode();
    emp::DataMonitor<int>& off_sym_count_node = off_world.GetCountHostedSymsDataNode();

    config.HOST_INT(0);
    config.SYM_INT(0);
    config.GRID(1);
    world.Setup();

    WHEN("Vertical transmission rate is high-intermediate (0.7)"){
      config.VERTICAL_TRANSMISSION(0.7);
      world.RunExperiment(false);
      
      config.GRID(0);
      off_world.Setup(); 
      off_world.RunExperiment(false);

      double grid_on_sym_val = sym_val_node.GetMean();
      double grid_off_sym_val = off_sym_val_node.GetMean();

      THEN("Symbionts evolve to be more parasitic compared to when spatial structure is off"){
        REQUIRE(off_sym_count_node.GetTotal() > 0);
        REQUIRE(sym_count_node.GetTotal() > 0);
        REQUIRE(grid_on_sym_val < grid_off_sym_val);
      }
    }

    WHEN("Vertical transmission rate is low-intermediate (0.3)") {
      config.VERTICAL_TRANSMISSION(0.3);
      world.RunExperiment(false);

      config.GRID(0);
      off_world.Setup();
      off_world.RunExperiment(false);

      double grid_on_sym_val = sym_val_node.GetMean();
      double grid_off_sym_val = off_sym_val_node.GetMean();

      THEN("Symbionts evolve to be less parasitic compared to when spatial structure is off") {
        REQUIRE(off_sym_count_node.GetTotal() > 0);
        REQUIRE(sym_count_node.GetTotal() > 0);
        REQUIRE(grid_on_sym_val > grid_off_sym_val);
      }
    }
  }
}