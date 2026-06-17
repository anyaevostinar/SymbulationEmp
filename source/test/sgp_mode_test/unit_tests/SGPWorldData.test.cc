#include <cstddef>
#include <cstdint>
#include <limits>
#include <filesystem>
#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"
#include "../../../sgp_mode/SGPWorldData.cc"
#include "emp/datastructs/map_utils.hpp"
#include "emp/math/info_theory.hpp"
#include "emp/math/stats.hpp"

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
  config.INIT_POP_SIZE(0);
  config.GRID_X(2);
  config.GRID_Y(2);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.FILE_PATH("SGPData_test_output");
  config.FILE_PATH("_test");
  emp::Random random(config.SEED());

  world_t world(random, &config);

  WHEN("The world calls CreateDataFiles"){
    world.CreateDataFiles();

    std::filesystem::path expected_org_count_fpath = config.FILE_PATH() + "/" + "OrganismCounts"+config.FILE_NAME() + ".csv";
    INFO("OrganismCounts file is created");
    REQUIRE(std::filesystem::exists(expected_org_count_fpath));

    std::filesystem::path expected_transmission_fpath = config.FILE_PATH() + "/" + "TransmissionRates"+config.FILE_NAME() + ".csv";
    INFO("TransmissionRates file is created");
    REQUIRE(std::filesystem::exists(expected_transmission_fpath));

    std::filesystem::path expected_tasks_fpath = config.FILE_PATH() + "/" + "Tasks"+config.FILE_NAME() + ".csv";
    INFO("Tasks file is created");
    REQUIRE(std::filesystem::exists(expected_tasks_fpath));

    std::filesystem::path expected_cur_update_info_fpath = config.FILE_PATH() + "/" + "CurrentUpdateInfo"+config.FILE_NAME() + ".csv";
    INFO("CurrentUpdateInfo file is created");
    REQUIRE(std::filesystem::exists(expected_cur_update_info_fpath));

    std::filesystem::path expected_sym_int_vals_fpath = config.FILE_PATH() + "/" + "SymbiontInteractionValues"+config.FILE_NAME() + ".csv";
    INFO("SymbiontInteractionValues file is created");
    REQUIRE(std::filesystem::exists(expected_sym_int_vals_fpath));
  }
}