#include "../test_utils.h"
#include "../../default_mode/SpatialDataManager.h"
#include "../../default_mode/SymWorld.h"
#include "../../default_mode/WorldSetup.cc"
#include "../../default_mode/DataNodes.h"

#include "emp/io/File.hpp"

#include <filesystem>

TEST_CASE("SpatialDataManager", "[default]") {
  using sym_world_t = SymWorld;
  namespace filesys = std::filesystem;
  emp::Random random(2);
  SymConfigBase config;
  config.SPATIAL_DATA_INTERVAL(10);
  test_utils::SetWellMixed(config, 10, 10);
  config.FILE_PATH("SpatialDataManager_test_output");
  filesys::path output_dir = config.FILE_PATH();
  filesys::path expected_fpath(
    output_dir / ("Spatial" + config.FILE_NAME() + ".csv")
  );
  // Remove any old spatial data file to make sure a new one isn't created
  // (would exist from previous test if this isn't the first time we've run
  //  tests)
  if (filesys::exists(expected_fpath)) {
    filesys::remove(expected_fpath);
  }
  GIVEN("a world with SPATIAL_DATA_OUTPUT disabled") {
    config.SPATIAL_DATA_OUTPUT(false);
    sym_world_t world(random, &config);
    WHEN("CreateDataFiles is called") {
      world.Setup();
      world.CreateDataFiles();
      THEN("No spatial data output file should be created.") {
        // No spatial structure data file should have been created
        REQUIRE(!filesys::exists(expected_fpath));
      }
    }
  }
  GIVEN("a world with SPATIAL_DATA_OUTPUT enabled") {
    config.SPATIAL_DATA_OUTPUT(true);
    config.FREE_LIVING_SYMS(true); // Make sure additional free-living syms columns are added
    sym_world_t world(random, &config);
    WHEN("CreateDataFiles is called") {
      world.Setup();
      world.CreateDataFiles();
      THEN("Spatial data output file should be created.") {
        REQUIRE(filesys::exists(expected_fpath));
      }
      WHEN("World is updated") {
        world.Update();
        THEN("Spatial data output file should have world size + 1 lines.") {
          emp::File spatial_file(expected_fpath.string());
          spatial_file.RemoveEmpty(); // Removing any empty trailing lines.
          REQUIRE(spatial_file.GetNumLines() == world.GetSize() + 1);
        }
      }
      world.CleanupGraveyard();
    }
  }
}
