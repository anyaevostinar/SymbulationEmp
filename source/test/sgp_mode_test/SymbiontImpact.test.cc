#include "../../source/sgp_mode/GenomeLibrary.h"
#include "../../source/sgp_mode/SymbiontImpact.h"

TEST_CASE("Parasitic sym is parasitic", "[sgp]") {
  emp::Random random(61);
  SymConfigBase config;
  config.RANDOM_ANCESTOR(false);
  config.SYM_HORIZ_TRANS_RES(100);

  SGPWorld world(random, &config, LogicTasks);
  // This is needed so that the world update number is not divisible by 30 /
  // CYCLES_PER_UPDATE, otherwise the used resources would be reset every time
  // Host::Process is called which would break this parasitism
  // TODO come up with a better system for resetting used resources
  world.Update();

  ProgramBuilder hbuilder;
  hbuilder.addNot();
  SGPHost host(&random, &world, &config, hbuilder.build(100));

  ProgramBuilder sbuilder;
  sbuilder.addNot();
  // add four instructions so the sym is guaranteed to complete NOT first
  for (size_t i = 0; i < config.CYCLES_PER_UPDATE(); i++) {
    sbuilder.add("Nop-0");
  }
  SGPSymbiont symbiont(&random, &world, &config, sbuilder.build(100));

  double score = CheckSymbiont(host, symbiont, world);
  REQUIRE(score < 0.0);
}

TEST_CASE("Mutualistic sym is mutualistic", "[sgp]") {
  emp::Random random(61);
  SymConfigBase config;
  config.RANDOM_ANCESTOR(false);
  config.SYM_HORIZ_TRANS_RES(100);

  SGPWorld world(random, &config, LogicTasks);
  world.Update();

  ProgramBuilder hbuilder;
  // Needs the symbiont to do NOT so the dependency of AND is satisfied
  hbuilder.addAnd();
  SGPHost host(&random, &world, &config, hbuilder.build(100));

  ProgramBuilder sbuilder;
  sbuilder.addNot();
  SGPSymbiont symbiont(&random, &world, &config, sbuilder.build(100));

  double score = CheckSymbiont(host, symbiont, world);
  REQUIRE(score > 0.0);
}