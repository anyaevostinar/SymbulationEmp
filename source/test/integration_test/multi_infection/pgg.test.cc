#include "../../test_utils.h"
#include "../../../default_mode/SymWorld.h"
#include "../../../default_mode/WorldSetup.cc"
#include "../../../default_mode/DataNodes.h"
#include "../../../pgg_mode/PGGHost.h"
#include "../../../pgg_mode/PGGSymbiont.h"
#include "../../../pgg_mode/PGGWorld.h"
#include "../../../pgg_mode/PGGWorldSetup.cc"

TEST_CASE("Multi-infection results", "[integration]") {
	emp::Random random(29);
	SymConfigPGG config;
	PGGWorld world(random, &config);
	config.HOST_INT(0);
	config.SYM_INT(0);
	config.PGG(1);
	config.MUTATION_SIZE(0.05);
	config.UPDATES(2000);
	// @AML: These world size parameters weren't taking effect in the original test
	// (Setup would call SetupHosts / SetupSymbionts using default world size values)
	// This is also why this integration test ran so slowly. We had a 10k host pop size
	// with each host having up to 10 symbionts in the final run experiment call.
	// Moving original width=5, height=5 up caused the test to fail (I think
	// because the world was so small, the pop was subjected to lots of randomness).
	// Made the world slightly bigger (but much smaller than default 100x100), and
	// everything behaves as expected.
	config.WORLD_WIDTH(10);
	config.WORLD_HEIGHT(10);
	world.Setup();

	emp::DataMonitor<double, emp::data::Histogram>& sym_val_node = world.GetHostedSymIntValDataNode();
	emp::DataMonitor<double, emp::data::Histogram>& host_val_node = world.GetHostIntValDataNode();

	WHEN("There are few symbionts per host") {
		config.SYM_LIMIT(1);
		WHEN("Vertical transmission rate is intermediate") {
			config.VERTICAL_TRANSMISSION(0.7);
			world.RunExperiment(false);
			THEN("Mutualism evolves") {
				REQUIRE(sym_val_node.GetMean() > 0.1);
				REQUIRE(host_val_node.GetMean() > 0.1);
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