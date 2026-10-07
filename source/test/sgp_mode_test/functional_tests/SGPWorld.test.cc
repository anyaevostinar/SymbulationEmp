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

/**
 * This file is dedicated to testing SGPWorld functionality
 */

// TODO - refactor task match checks into compatibiliity mode checks
//        (test all compatibility modes)
TEST_CASE("A world containing a single infected host and its symbiont is updated correctly", "[sgp][sgp-functional]") {
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;

  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.FREE_LIVING_SYMS(1);
  test_utils::SetWellMixed(config, 4, 0);
  config.TASK_IO_BANK_SIZE(10);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.EVENTS_CFG_PATH("source/test/sgp_mode_test/no-events.json");

  sgpmode::SGPWorld world(random, &config);
  world.Setup();

  auto& prog_builder = world.GetProgramBuilder();

  emp::Ptr<sgpmode::SGPHost<hw_spec_t>> infected_host = emp::NewPtr<sgpmode::SGPHost<hw_spec_t>>(&random, &world, &config, prog_builder.CreateNotProgram(100));
  emp::Ptr<sgpmode::SGPSymbiont<hw_spec_t>> hosted_symbiont = emp::NewPtr<sgpmode::SGPSymbiont<hw_spec_t>> (&random, &world, &config, prog_builder.CreateNotProgram(100));

  infected_host->AddSymbiont(hosted_symbiont);
  world.AddOrgAt(infected_host, 0);

  THEN("An infected host can be added to the world") {
    REQUIRE(world.GetNumOrgs() == 1);
  }

  for (int i = 0; i < 10; i++) {
    world.Update();
  }

  THEN("An infected host persists and is managed by the world") {
    REQUIRE(world.GetNumOrgs() == 1);
  }
}

TEST_CASE("A world containing a single uninfected host is updated correctly", "[sgp][sgp-functional]") {
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;

  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.FREE_LIVING_SYMS(1);
  test_utils::SetWellMixed(config, 4, 0);
  config.TASK_IO_BANK_SIZE(10);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.EVENTS_CFG_PATH("source/test/sgp_mode_test/no-events.json");

  sgpmode::SGPWorld world(random, &config);
  world.Setup();

  auto& prog_builder = world.GetProgramBuilder();

  emp::Ptr<sgpmode::SGPHost<hw_spec_t>> uninfected_host = emp::NewPtr<sgpmode::SGPHost<hw_spec_t>>(&random, &world, &config, prog_builder.CreateNotProgram(100));

  world.AddOrgAt(uninfected_host, 1);

  THEN("An uninfected host can be added to the world") {
    REQUIRE(world.GetNumOrgs() == 1);
  }

  for (int i = 0; i < 10; i++) {
    world.Update();
  }

  THEN("An uninfected host persists and is managed by the world") {
    REQUIRE(world.GetNumOrgs() == 1);
  }
}

TEST_CASE("SGP GetDominantInfo", "[sgp][sgp-functional]"){
  GIVEN("An SGPWorld with 6 hosts and 3 different genomes"){
    using world_t = sgpmode::SGPWorld;
    using cpu_state_t = sgpmode::CPUState<world_t>;
    using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;

    emp::Random random(61);
    sgpmode::SymConfigSGP config;
    config.FREE_LIVING_SYMS(0);
    config.INIT_POP_SIZE(0);
    test_utils::SetWellMixed(config, 6, 0);
    config.TASK_IO_BANK_SIZE(10);
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.EVENTS_CFG_PATH("source/test/sgp_mode_test/no-events.json");

    sgpmode::SGPWorld world(random, &config);
    world.Setup();

    auto& prog_builder = world.GetProgramBuilder();

    size_t nand_host_count = 3;
    size_t not_host_count = 2;
    size_t not_nand_host_count = 1;
    for(size_t i = 0; i < not_host_count; i ++){
      world.AddOrgAt(emp::NewPtr<sgpmode::SGPHost<hw_spec_t>>(&random, &world, &config, prog_builder.CreateNotProgram(100)), i);
    }
    for(size_t i = not_host_count; i < not_host_count + nand_host_count; i ++){
      world.AddOrgAt(emp::NewPtr<sgpmode::SGPHost<hw_spec_t>>(&random, &world, &config, prog_builder.CreateNandProgram(100)), i);
    }
    for(size_t i = not_host_count + nand_host_count; i < not_host_count + nand_host_count + not_nand_host_count; i ++){
      world.AddOrgAt(emp::NewPtr<sgpmode::SGPHost<hw_spec_t>>(&random, &world, &config, prog_builder.CreateNotNandProgram(100)), i);
    }

    WHEN("GetDominantInfo() is called and DOMINANT_COUNT is 2"){
      config.DOMINANT_COUNT(2);

      emp::vector<std::pair<emp::Ptr<Organism>, size_t>> dominant_organisms = world.GetDominantInfo();  

      program_t& not_program = world.GetOrgPtr(0).DynamicCast<sgp_host_t>()->GetHardware().GetProgram();
      program_t& nand_program = world.GetOrgPtr(not_host_count).DynamicCast<sgp_host_t>()->GetHardware().GetProgram();
      //program_t nand_program = builder.LoadProgramFile(path);

      THEN("Hosts with the two most common genomes are written"){
        // only the config-specified number of dominant organisms should be selected
        REQUIRE(dominant_organisms.size() == 2);

        // the most dominant program should be NAND
        REQUIRE(dominant_organisms[0].second == nand_host_count);
        REQUIRE(dominant_organisms[0].first.DynamicCast<sgp_host_t>()->GetHardware().GetProgram() == nand_program);

        // the second most dominant program should be NOT
        REQUIRE(dominant_organisms[1].second == not_host_count);
        REQUIRE(dominant_organisms[1].first.DynamicCast<sgp_host_t>()->GetHardware().GetProgram() == not_program);
      }
    }
  }
}

/* TODO need update CollectCurrentUpdateData to support free living symbionts before uncommenting this test

TEST_CASE("A world containing a single free living symbiont is updated correctly", "[sgp][sgp-functional]") {
  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.FREE_LIVING_SYMS(1);
  config.WORLD_WIDTH(2);
  config.WORLD_HEIGHT(2);
  config.INIT_POP_SIZE(0);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");

  sgpmode::SGPWorld world(random, &config);
  world.Setup();
  world.Resize(2,2);

  auto& prog_builder = world.GetProgramBuilder();
  emp::Ptr<sgpmode::SGPSymbiont<hw_spec_t>> free_symbiont = emp::NewPtr<sgpmode::SGPSymbiont<hw_spec_t>>(&random, &world, &config, prog_builder.CreateNotProgram(100));

  world.AddOrgAt(free_symbiont, emp::WorldPosition(0, 0));

  THEN("A free living symbiont can be added to the world") {
    REQUIRE(world.GetNumOrgs() == 1);
  }

  THEN("A free living symbiont can run one CPU step"){
    free_symbiont->GetHardware().RunCPUStep(1);
  }

  THEN("A free living symbiont can run two CPU steps"){
    free_symbiont->GetHardware().RunCPUStep(1);
    free_symbiont->GetHardware().RunCPUStep(1);
  }

  for (int i = 0; i < 10; i++) {
    world.Update();
  }

  THEN("A free living symbiont persists and is managed by the world") {
    REQUIRE(world.GetNumOrgs() == 1);
  }
}
*/