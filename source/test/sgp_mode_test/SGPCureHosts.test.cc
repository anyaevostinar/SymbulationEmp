#include "emp/math/Random.hpp"

#include "../../sgp_mode/hardware/SGPHardware.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPWorld.cc"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPWorldSetup.cc"
#include "../../sgp_mode/SGPWorldData.cc"
#include "../../sgp_mode/ProgramBuilder.h"

#include "../../catch/catch.hpp"


TEST_CASE("SGP Cure Hosts tests", "[sgp]"){
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
  using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;
  
  // set up configs
  sgpmode::SymConfigSGP config;
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.FILE_PATH("SGPSymbiont_test_output");
  config.SEED(61);
  config.POP_SIZE(2);
  config.START_MOI(1); // sym to host ratio
  config.CURE(1);
  int num_updates = 2;
  int total_updates = 4;
  config.CURE_UPDATES(num_updates);
  config.UPDATES(total_updates);
  // config.SYM_LIMIT(2) // allow multiple symbionts to infect one host

  // initalize world
  emp::Random random(config.SEED());
  world_t world(random, &config);
  world.Setup();

  // get hosts
  auto& get_host1 = world.GetOrg(0);
  sgp_host_t& host1 = static_cast<sgp_host_t&>(get_host1);
  auto& get_host2 = world.GetOrg(1);
  sgp_host_t& host2 = static_cast<sgp_host_t&>(get_host2);
  // get symbionts
  auto& get_sym1 = *(host1.GetSymbionts()[0]);
  sgp_sym_t& sym1 = static_cast<sgp_sym_t&>(get_sym1);
  auto& get_sym2 = *(host2.GetSymbionts()[0]);
  sgp_sym_t& sym2 = static_cast<sgp_sym_t&>(get_sym2);

  // host has symbiont
  REQUIRE(host1.HasSym());
  REQUIRE(host2.HasSym());

  // symbiont has host
  REQUIRE(sym1.GetHardware().GetCPUState().HasHost());
  REQUIRE(sym2.GetHardware().GetCPUState().HasHost());

  // Cure Hosts Segmentation fault core dumped
  world.CureHosts();
  REQUIRE(host1.HasSym() == false);
  REQUIRE(host2.HasSym() == false);
  // Run causes segmentation fault core dumped, on update num_updates
  // world.Run();


  // WHEN("Updates are run") {
  //   // segmentation violation signal on num_updates
  //   for (int i = 0; i < total_updates; i++){
  //     printf("Update: %d", i);
  //     world.Update();
  //     // if (i > num_updates){
  //     //   REQUIRE(host1.HasSym() == false);
  //     //   REQUIRE(host2.HasSym() == false);
  //     // }
  //     // else {
  //     //   REQUIRE(host1.HasSym());
  //     //   REQUIRE(host2.HasSym());
  //     // }
  //   }
  // }


} //TEST_CASE