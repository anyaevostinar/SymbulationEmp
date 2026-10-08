#include "../../test_utils.h"
#include "../../../default_mode/SymWorld.h"
#include "../../../default_mode/WorldSetup.cc"
#include "../../../default_mode/DataNodes.h"
#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorld.cc"
#include "../../../sgp_mode/SGPWorldSetup.cc"
#include "../../../sgp_mode/SGPWorldData.cc"
#include "../../../sgp_mode/SGPW_InteractionMechanismSetup.cc"
#include "../../../sgp_mode/SGPW_TaskProfileSetup.cc"
#include "../../../sgp_mode/ProgramBuilder.h"

#include "emp/datastructs/map_utils.hpp"
#include "emp/math/info_theory.hpp"
#include "emp/math/stats.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <filesystem>

/**
 * This file is dedicated to ensuring that SGPWorldData methods work as expected
 */

using world_t = sgpmode::SGPWorld;
using cpu_state_t = sgpmode::CPUState<world_t>;
using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;


TEST_CASE("CreateDataFiles creates data files", "[sgp][sgp-functional]") {
  sgpmode::SymConfigSGP config;
  test_utils::SetWellMixed(config, 4, 0);
  config.TASK_IO_BANK_SIZE(10);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.FILE_PATH("SGPData_test_output");
  emp::Random random(config.SEED());

  world_t world(random, &config);

  WHEN("The world calls CreateDataFiles") {
    world.CreateDataFiles();

    std::filesystem::path expected_org_count_fpath = config.FILE_PATH() + "/" + "OrganismCounts" + config.FILE_NAME() + ".csv";
    INFO("OrganismCounts file is created");
    REQUIRE(std::filesystem::exists(expected_org_count_fpath));

    std::filesystem::path expected_transmission_fpath = config.FILE_PATH() + "/" + "TransmissionRates" + config.FILE_NAME() + ".csv";
    INFO("TransmissionRates file is created");
    REQUIRE(std::filesystem::exists(expected_transmission_fpath));

    std::filesystem::path expected_tasks_fpath = config.FILE_PATH() + "/" + "Tasks" + config.FILE_NAME() + ".csv";
    INFO("Tasks file is created");
    REQUIRE(std::filesystem::exists(expected_tasks_fpath));

    std::filesystem::path expected_cur_update_info_fpath = config.FILE_PATH() + "/" + "CurrentUpdateInfo" + config.FILE_NAME() + ".csv";
    INFO("CurrentUpdateInfo file is created");
    REQUIRE(std::filesystem::exists(expected_cur_update_info_fpath));

    std::filesystem::path expected_sym_int_vals_fpath = config.FILE_PATH() + "/" + "SymbiontInteractionValues" + config.FILE_NAME() + ".csv";
    INFO("SymbiontInteractionValues file is created");
    REQUIRE(std::filesystem::exists(expected_sym_int_vals_fpath));
  }
}

TEST_CASE("SGP mode spatial data", "[sgp][sgp-functional]") {
  namespace filesys = std::filesystem;
  emp::Random random(2);
  sgpmode::SymConfigSGP config;
  test_utils::SetWellMixed(config, 10, 10);
  config.SPATIAL_DATA_INTERVAL(10);
  config.TASK_IO_BANK_SIZE(10);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.FILE_PATH("SGPData_test_output");
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
  GIVEN("an sgp world with SPATIAL_DATA_OUTPUT disabled") {
    config.SPATIAL_DATA_OUTPUT(false);
    world_t world(random, &config);
    WHEN("Setup is called") {
      world.Setup();
      THEN("No spatial data output file should be created.") {
        // No spatial structure data file should have been created
        REQUIRE(!filesys::exists(expected_fpath));
      }
    }
  }
  GIVEN("an sgp world with SPATIAL_DATA_OUTPUT enabled") {
    config.SPATIAL_DATA_OUTPUT(true);
    world_t world(random, &config);
    WHEN("Setup is called") {
      world.Setup();
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
    }
  }
}