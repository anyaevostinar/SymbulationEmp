#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/SGPWorldSetup.cc"
#include "../../sgp_mode/SGPDataNodes.h"

TEST_CASE("SGPSymbiont Reproduce", "[sgp]") {
  emp::Random random(31);
  SymConfigSGP config;
  SGPWorld world(random, &config, LogicTasks);
  emp::Ptr<SGPSymbiont> sym_parent = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(100));

  THEN("Symbiont child increases its lineage reproduction count") {
    emp::Ptr<SGPSymbiont> sym_baby = (sym_parent->Reproduce()).DynamicCast<SGPSymbiont>();
    REQUIRE(sym_parent->GetReproCount() == sym_baby->GetReproCount() - 1);
    sym_baby.Delete();
    sym_parent.Delete();
  }

  WHEN("Parental task tracking is on") {
    emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));
    config.TRACK_PARENT_TASKS(1);
    world.AddOrgAt(host, 0);
    host->AddSymbiont(sym_parent);

    for (int i = 0; i < 25; i++) {
      world.Update();
    }

    emp::Ptr<SGPSymbiont> sym_baby = (sym_parent->Reproduce()).DynamicCast<SGPSymbiont>();

    THEN("Symbiont child inherits its parent's empty task bitset") {
      REQUIRE(sym_parent->GetCPU().state.parent_tasks_performed->None());

      REQUIRE(sym_parent->GetCPU().state.tasks_performed->Get(0));
      REQUIRE(sym_parent->GetCPU().state.tasks_performed->CountOnes() == 1);

      REQUIRE(sym_baby->GetCPU().state.parent_tasks_performed->Get(0));
      REQUIRE(sym_baby->GetCPU().state.parent_tasks_performed->CountOnes() == 1);
    }


    THEN("The symbiont child tracks any gains or loses in task completions") {
      // in this second generation, we expect the symbiont to gain the NOT task
      // since first-gen organisms are marked as having parents who have completed no tasks
      REQUIRE(sym_baby->GetCPU().state.task_change_lose[0] == 0);
      REQUIRE(sym_baby->GetCPU().state.task_change_gain[0] == 1);
      for (unsigned int i = 1; i < CPU_BITSET_LENGTH; i++) {
        REQUIRE(sym_baby->GetCPU().state.task_change_lose[i] == 0);
        REQUIRE(sym_baby->GetCPU().state.task_change_gain[i] == 0);
      }
    }

    THEN("The symbiont child tracks how its tasks compare to its parent's partner's tasks") {
      REQUIRE(sym_baby->GetCPU().state.task_toward_partner[0] == 0);
      REQUIRE(sym_baby->GetCPU().state.task_from_partner[0] == 1);

      for (unsigned int i = 1; i < CPU_BITSET_LENGTH; i++) {
        REQUIRE(sym_baby->GetCPU().state.task_toward_partner[i] == 0);
        REQUIRE(sym_baby->GetCPU().state.task_from_partner[i] == 0);
      }
    }
    sym_baby.Delete();
  }

  TEST_CASE("Symbiont comparison operators", "[sgp]") {
    emp::Ptr<SGPSymbiont> clone1 = emp::NewPtr<SGPSymbiont>(*sym_parent);
    emp::Ptr<SGPSymbiont> clone2 = emp::NewPtr<SGPSymbiont>(*sym_parent);
    emp::Ptr<SGPSymbiont> different = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(99)); // For comparing

    THEN("Symbionts with the same genomes are equal") {
      REQUIRE(*sym_parent == *clone1);
      REQUIRE(*clone1 == *clone2);
    }

    THEN("Symbionts with different genomes are not equal") {
      REQUIRE_FALSE(*sym_parent == *different);
    }

    THEN("operator< reflects program ordering") {
      // Can't assert true/false without knowing bitcode ordering,
      // assert that bitcode ordering is well-defined
      bool lt = *sym_parent < *different || *different < *sym_parent;
      REQUIRE(lt);
    }

    clone1.Delete();
    clone2.Delete();
    different.Delete();
  }

  TEST_CASE("SGPSymbiont destructor cleans up shared pointers and in-progress reproduction", "[sgp]") {
    emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(100));
    sym->GetCPU().state.in_progress_repro = 3;
    world.to_reproduce.resize(5);
    world.to_reproduce[3].second = emp::WorldPosition(1, 2);

    REQUIRE(world.to_reproduce[3].second.IsValid());
    sym.Delete();

    THEN("Reproduction queue is invalidated after symbiont is destroyed") {
      REQUIRE_FALSE(world.to_reproduce[3].second.IsValid());
    }
  }
}