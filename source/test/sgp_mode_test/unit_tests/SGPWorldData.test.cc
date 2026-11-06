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

    std::filesystem::path expected_task_discovery_fpath = config.FILE_PATH() + "/" + "TaskDiscovery" + config.FILE_NAME() + ".csv";
    INFO("TaskDiscovery file is created");
    REQUIRE(std::filesystem::exists(expected_task_discovery_fpath));
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

TEST_CASE("SGP mode task discovery data", "[sgp][sgp-functional]") {
  namespace filesys = std::filesystem;
  const size_t num_updates = 2;
  emp::Random random(2);
  sgpmode::SymConfigSGP config;
  test_utils::SetWellMixed(config, 10, 0);
  config.SPATIAL_DATA_INTERVAL(10);
  // Set reproduction requirements unobtainably high.
  config.HOST_REPRO_RES(10000);
  config.SYM_HORIZ_TRANS_RES(10000);
  config.TASK_IO_BANK_SIZE(10);
  config.UPDATES(num_updates);
  // task discovery dta file output on world end
  config.CYCLES_PER_UPDATE(100);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  const size_t num_tasks = 9;
  config.FILE_PATH("SGPData_test_output");
  filesys::path output_dir = config.FILE_PATH();
  filesys::path expected_fpath(
    output_dir / ("TaskDiscovery" + config.FILE_NAME() + ".csv")
  );
  // Remove any old spatial data file to make sure a new one isn't created
  // (would exist from previous test if this isn't the first time we've run
  //  tests)
  if (filesys::exists(expected_fpath)) {
    filesys::remove(expected_fpath);
  }

  world_t world(random, &config);
  WHEN("Setup is called") {
    world.Setup();
    const auto& tracker = world.GetTaskDiscoveryTracker();
    THEN("Task discovery file should be created.") {
      REQUIRE(filesys::exists(expected_fpath));
    }
    THEN("Task discovery tracker should be initialized with 9 tasks, all undiscovered") {
      REQUIRE(tracker.GetHostTaskDiscoveries().size() == num_tasks);
      REQUIRE(tracker.GetHostParentOffspringTaskDiscoveries().size() == num_tasks);
      REQUIRE(tracker.GetSymTaskDiscoveries().size() == num_tasks);
      REQUIRE(tracker.GetSymParentOffspringTaskDiscoveries().size() == num_tasks);
      for (size_t task_i = 0; task_i < num_tasks; ++task_i) {
        REQUIRE(!tracker.IsHostTaskDiscovered(task_i));
        REQUIRE(!tracker.IsHostParentOffspringTaskDiscovered(task_i));
        REQUIRE(!tracker.IsSymTaskDiscovered(task_i));
        REQUIRE(!tracker.IsSymParentOffspringTaskDiscovered(task_i));
      }
    }
    WHEN("A nand host-sym pair is injected and the world is run") {
      auto& prog_builder = world.GetProgramBuilder();
      const size_t nand_task_id = world.GetTaskEnv().GetTaskSet().GetID("NAND");
      emp::Ptr<sgpmode::SGPHost<hw_spec_t>> host = emp::NewPtr<sgpmode::SGPHost<hw_spec_t>>(
        &random,
        &world,
        &config,
        prog_builder.CreateNandProgram(100)
      );
      host->GetHardware().GetCPUState().SetParentTaskPerformed(nand_task_id);
      emp::Ptr<sgpmode::SGPSymbiont<hw_spec_t>> hosted_symbiont = emp::NewPtr<sgpmode::SGPSymbiont<hw_spec_t>> (
        &random,
        &world,
        &config,
        prog_builder.CreateNandProgram(100)
      );
      hosted_symbiont->GetHardware().GetCPUState().SetParentTaskPerformed(nand_task_id);
      host->AddSymbiont(hosted_symbiont);
      world.AddOrgAt(host, 0);
      world.Run();
      THEN("Nand should be discovered and all other tasks should be undiscovered") {
        // Check that nand was marked as discovered, but no others
        for (size_t task_i = 0; task_i < num_tasks; ++task_i) {
          if (task_i == nand_task_id) {
            REQUIRE(tracker.IsHostTaskDiscovered(task_i));
            REQUIRE(tracker.IsHostParentOffspringTaskDiscovered(task_i));
            REQUIRE(tracker.IsSymTaskDiscovered(task_i));
            REQUIRE(tracker.IsSymParentOffspringTaskDiscovered(task_i));
          } else {
            REQUIRE(!tracker.IsHostTaskDiscovered(task_i));
            REQUIRE(!tracker.IsHostParentOffspringTaskDiscovered(task_i));
            REQUIRE(!tracker.IsSymTaskDiscovered(task_i));
            REQUIRE(!tracker.IsSymParentOffspringTaskDiscovered(task_i));
          }
        }
      }
    }
  }
}