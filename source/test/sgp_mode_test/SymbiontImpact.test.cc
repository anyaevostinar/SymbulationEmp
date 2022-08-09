#include "../../source/sgp_mode/SymbiontImpact.h"
#include "../../source/sgp_mode/GenomeLibrary.h"

TEST_CASE("Parasitic sym is parasitic", "[sgp]") {
  emp::Random random(61);
  SymConfigBase config;
  config.RANDOM_ANCESTOR(false);
  config.SYM_HORIZ_TRANS_RES(100);

  SGPWorld world(random, &config, LogicTasks);

  ProgramBuilder hbuilder;
  hbuilder.AddNot();
  SGPHost host(&random, &world, &config, hbuilder.Build(100));

  ProgramBuilder sbuilder;
  sbuilder.AddNot();
  // add four instructions so the sym is guaranteed to complete NOT first
  for (size_t i = 0; i < config.CYCLES_PER_UPDATE(); i++) {
    sbuilder.Add("Nop-0");
  }
  SGPSymbiont symbiont(&random, &world, &config, sbuilder.Build(100));

  double score = CheckSymbiont(host, symbiont, world);
  REQUIRE(score < 0.0);
}

TEST_CASE("Mutualistic sym is mutualistic", "[sgp]") {
  emp::Random random(61);
  SymConfigBase config;
  config.RANDOM_ANCESTOR(false);
  config.SYM_HORIZ_TRANS_RES(100);

  SGPWorld world(random, &config, LogicTasks);

  ProgramBuilder hbuilder;
  // Needs the symbiont to do NOT so the dependency of AND is satisfied
  hbuilder.AddAnd();
  SGPHost host(&random, &world, &config, hbuilder.Build(100));

  ProgramBuilder sbuilder;
  sbuilder.AddNot();
  SGPSymbiont symbiont(&random, &world, &config, sbuilder.Build(100));

  double score = CheckSymbiont(host, symbiont, world);
  REQUIRE(score > 0.0);
}