#include "../../../pgg_mode/PGGWorldSetup.cc"


TEST_CASE("Multi-infection results", "[integration]") {
  emp::Random random(29);
  SymConfigBase config;
  PGGWorld world(random, &config);
  config.HOST_INT(0);
  config.SYM_INT(0);
  config.PGG(1);
  config.MUTATION_SIZE(0.05);
  config.UPDATES(2000);
  world.Setup();

  config.GRID_X(5);
  config.GRID_Y(5);
  
  emp::DataMonitor<double, emp::data::Histogram>& sym_val_node = world.GetHostedSymIntValDataNode();
  emp::DataMonitor<double, emp::data::Histogram>& host_val_node = world.GetHostIntValDataNode();

  
  WHEN("There are few symbionts per host") {
	config.SYM_LIMIT(1);
	WHEN("Vertical transmission rate is intermediate") {
	  config.VERTICAL_TRANSMISSION(0.7);
	  world.RunExperiment(false);
	  THEN("Mutualism evolves"){
		REQUIRE(sym_val_node.GetMean() > 0.1);
		REQUIRE(host_val_node.GetMean() > 0.1);
	  }
	}
	WHEN("Vertical transmission rate is high") {
	  config.VERTICAL_TRANSMISSION(1);
	  world.RunExperiment(false);
	  THEN("Mutualism evolves"){
		REQUIRE(sym_val_node.GetMean() > 0.1);
		REQUIRE(host_val_node.GetMean() > 0.1);
	  }
	}
  }
  WHEN("There can be many symbionts per host") {
	config.SYM_LIMIT(10);
	WHEN("Vertical transmission rate is intermediate") {
	  config.VERTICAL_TRANSMISSION(0.7);
	  world.RunExperiment(false);
	  THEN("Mutualism does not evolve") {
		REQUIRE(sym_val_node.GetMean() < -0.1);
		REQUIRE(host_val_node.GetMean() < -0.1);
	  }
	}
	WHEN("Vertical transmission rate is high") {
	  config.VERTICAL_TRANSMISSION(1);
	  world.RunExperiment(false);
	  THEN("Mutualism evolves") {
		REQUIRE(sym_val_node.GetMean() > 0.1);
		REQUIRE(host_val_node.GetMean() > 0.1);
	  }
	}
  }
}