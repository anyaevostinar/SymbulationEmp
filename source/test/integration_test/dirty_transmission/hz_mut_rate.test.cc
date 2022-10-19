#include "../../../efficient_mode/EfficientWorldSetup.cc"

TEST_CASE("Horizontal Mutation Rate Results", "[integration]"){
  emp::Random random(14);
  SymConfigBase config;
  EfficientWorld world(random, &config);

  config.HOST_INT(-2);
  config.SYM_INT(-2);
  config.EFFICIENT_SYM(1);
  config.SYNERGY(2);
  config.MUTATION_RATE(0.1);
  config.HOST_AGE_MAX(100);
  config.SYM_AGE_MAX(50);
  config.UPDATES(2001);

  config.GRID_X(10);
  config.GRID_Y(10);

  emp::DataMonitor<double, emp::data::Histogram>& sym_val_node = world.GetHostedSymIntValDataNode();
  emp::DataMonitor<double>& sym_eff_node = world.GetEfficiencyDataNode();

  WHEN("Vertical transmission rate is low"){
	config.VERTICAL_TRANSMISSION(0.1);
	world.Setup();
	WHEN("Horizontal transmission mutation rate is low") {
	  config.HORIZ_MUTATION_RATE(0.1);
	  world.RunExperiment(false);
	  THEN("Symbionts evolve to be parasitic and efficient") {
		REQUIRE(sym_val_node.GetMean() < -0.7);
		REQUIRE(sym_eff_node.GetMean() > 0.94);
	  }
	}
	WHEN("Horizontal transmission mutation rate is intermediate") {
	  config.HORIZ_MUTATION_RATE(0.5);
	  world.RunExperiment(false);
	  THEN("Symbionts evolve to be parasitic and efficient") {
		REQUIRE(sym_val_node.GetMean() < -0.6);
		REQUIRE(sym_eff_node.GetMean() > 0.94);
	  }
	}
	WHEN("Horizontal transmission mutation rate is high") {
	  config.HORIZ_MUTATION_RATE(1);
	  world.RunExperiment(false);
	  THEN("Symbionts evolve to be parasitic and efficient") {
		REQUIRE(sym_val_node.GetMean() < -0.7);
		REQUIRE(sym_eff_node.GetMean() > 0.95);
	  }
	}
  }
  WHEN("Vertical transmission rate is intermediate") {
	//other tests work fine with both constrained and paper-replicating world conditions 
	// (large to reduce drift vs. small for speed 
	//but in intermediate case need larger world to replicate paper results
	config.GRID_X(100);
	config.GRID_Y(100);
	config.VERTICAL_TRANSMISSION(0.3);
	world.Setup();
	WHEN("Horizontal transmission mutation rate is low") {
	  config.HORIZ_MUTATION_RATE(0.1);
	  world.RunExperiment(false);
	  THEN("Symbionts evolve to be slightly parasitic and very efficient") {
		double sym_val_mean = sym_val_node.GetMean();
		REQUIRE(sym_val_mean < 0);
		REQUIRE(sym_val_mean > -0.7);
		REQUIRE(sym_eff_node.GetMean() > 0.94);
	  }
	}
	WHEN("Horizontal transmission mutation rate is intermediate") {
	  config.HORIZ_MUTATION_RATE(0.5);
	  world.RunExperiment(false);
	  THEN("Symbionts are evolve between slight parasitism and slight mutualism and evolve to be less efficient") {
		double sym_val_mean = sym_val_node.GetMean();
		REQUIRE(sym_val_mean < 0.5);
		REQUIRE(sym_val_mean > -0.5);
		REQUIRE(sym_eff_node.GetMean() > 0.94);
	  }
	}
	WHEN("Horizontal transmission mutation rate is high") {
	  config.HORIZ_MUTATION_RATE(1);
	  world.RunExperiment(false);
	  THEN("Symbionts evolve to be slightly mutualistic and less efficient") {
		double sym_val_mean = sym_val_node.GetMean();
		REQUIRE(sym_val_mean > 0);
		REQUIRE(sym_val_mean < 0.7);
		REQUIRE(sym_eff_node.GetMean() > 0.94);
	  }
	}
  }
  WHEN("Vertical transmission rate is high") {
	config.VERTICAL_TRANSMISSION(0.9);
	world.Setup();
	WHEN("Horizontal transmission mutation rate is low") {
	  config.HORIZ_MUTATION_RATE(0.1);
	  world.RunExperiment(false);
	  THEN("Symbionts evolve to be mutualistic and efficient") {
		REQUIRE(sym_val_node.GetMean() > 0.7);
		REQUIRE(sym_eff_node.GetMean() > 0.94);
	  }
	}
	WHEN("Horizontal transmission mutation rate is intermediate") {
	  config.HORIZ_MUTATION_RATE(0.5);
	  world.RunExperiment(false);
	  THEN("Symbionts evolve to be mutualistic and efficient") {
		REQUIRE(sym_val_node.GetMean() > 0.7);
		REQUIRE(sym_eff_node.GetMean() > 0.94);
	  }
	}
	WHEN("Horizontal transmission mutation rate is high") {
	  config.HORIZ_MUTATION_RATE(1);
	  world.RunExperiment(false);
	  THEN("Symbionts evolve to be mutualistic and efficient") {
		REQUIRE(sym_val_node.GetMean() > 0.7);
		REQUIRE(sym_eff_node.GetMean() > 0.94);
	  }
	}
  }
}