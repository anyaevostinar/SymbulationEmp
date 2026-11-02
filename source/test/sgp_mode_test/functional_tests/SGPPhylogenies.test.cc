#include "../../test_utils.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"

/**
 * This file is dedicated to testing SGPWorld phylogenies
 */


TEST_CASE("Host task-cycle phylogeny is tracked correctly", "[sgp][sgp-functional]") {
    using world_t = sgpmode::SGPWorld;
    using cpu_state_t = sgpmode::CPUState<world_t>;
    using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
    using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;

    emp::Random random(2509);
    sgpmode::SymConfigSGP config;
    test_utils::SetWellMixed(config, 4, 0);
    config.INIT_POP_SIZE(0);
    config.TASK_IO_BANK_SIZE(10);
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.EVENTS_CFG_PATH("source/test/sgp_mode_test/no-events.json");

    // SGP phylogeny settings
    config.PHYLOGENY(1);
    config.PHYLOGENY_TAXON_TYPE("task-cycle-binned");
    
    sgpmode::SGPWorld world(random, &config);
    world.Setup();
    
    auto& prog_builder = world.GetProgramBuilder();
    emp::Ptr<emp::Systematics<Organism, taxon_t::info_t, datastruct::HostTaxonData>> host_sys =  world.GetHostSys();

    const size_t not_task_id = world.GetTaskEnv().GetTaskSet().GetID("NOT");
    const size_t equ_task_id = world.GetTaskEnv().GetTaskSet().GetID("EQU");
    const size_t parent_cycles_since_repro = 10; 
    
    emp::Ptr<sgp_host_t> host_parent = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateNotProgram(100));
    host_parent->GetHardware().GetCPUState().SetParentTaskPerformed(not_task_id, true);
    host_parent->GetHardware().GetCPUState().SetParentTaskPerformed(equ_task_id, true);
    host_parent->GetHardware().GetCPUState().SetParentCPUCyclesSinceRepro(parent_cycles_since_repro);

    world.AddOrgAt(host_parent, 0);
    
    WHEN("A host is added to the world"){
        THEN("The host is tracked by the host systematic"){
            REQUIRE(host_parent->GetTaxon()->GetInfo() == 10000001.10);
            REQUIRE(host_sys->GetNumActive() == 1);
        }
    }
    
    WHEN("A host has a child with the same parent task set but a different cycle count"){
        host_parent->GetHardware().GetCPUState().SetCPUCyclesSinceRepro(parent_cycles_since_repro + 1);
        emp::Ptr<sgp_host_t> host_child = static_cast<sgp_host_t*>(host_parent->Reproduce().Raw());

        host_child->GetHardware().GetCPUState().SetParentTaskPerformed(not_task_id, true);
        host_child->GetHardware().GetCPUState().SetParentTaskPerformed(equ_task_id, true);

        world.AddOrgAt(host_child, 1, 0);
        THEN("The parent and children are in different taxa"){
            REQUIRE(host_child->GetTaxon()->GetInfo() == 10000001.11);
            REQUIRE(host_parent->GetTaxon() != host_child->GetTaxon());
        }
    }
    
    WHEN("A host has a child with the a different parent task set but the same cycle count"){
        host_parent->GetHardware().GetCPUState().SetCPUCyclesSinceRepro(parent_cycles_since_repro);
        emp::Ptr<sgp_host_t> host_child = static_cast<sgp_host_t*>(host_parent->Reproduce().Raw());

        host_child->GetHardware().GetCPUState().SetParentTaskPerformed(equ_task_id, true);

        world.AddOrgAt(host_child, 1, 0);
        THEN("The parent and children are in different taxa"){
            REQUIRE(host_child->GetTaxon()->GetInfo() == 1.10);
            REQUIRE(host_parent->GetTaxon() != host_child->GetTaxon());
        }
    }

    WHEN("A host has a child with the same parent task set and the same cycle count"){
        host_parent->GetHardware().GetCPUState().SetCPUCyclesSinceRepro(parent_cycles_since_repro);
        emp::Ptr<sgp_host_t> host_child = static_cast<sgp_host_t*>(host_parent->Reproduce().Raw());

        host_child->GetHardware().GetCPUState().SetParentTaskPerformed(not_task_id, true);
        host_child->GetHardware().GetCPUState().SetParentTaskPerformed(equ_task_id, true);

        world.AddOrgAt(host_child, 1, 0);
        THEN("The parent and children are in the same taxon"){
            REQUIRE(host_child->GetTaxon()->GetInfo() == host_parent->GetTaxon()->GetInfo());
            REQUIRE(host_parent->GetTaxon() == host_child->GetTaxon());
        }
    }
}

TEST_CASE("Symbiont task-cycle phylogeny is tracked correctly", "[sgp][sgp-functional]") {
    using world_t = sgpmode::SGPWorld;
    using cpu_state_t = sgpmode::CPUState<world_t>;
    using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
    using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
    using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;

    emp::Random random(3578);
    sgpmode::SymConfigSGP config;
    test_utils::SetWellMixed(config, 4, 0);
    config.INIT_POP_SIZE(0);
    config.TASK_IO_BANK_SIZE(10);
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.EVENTS_CFG_PATH("source/test/sgp_mode_test/no-events.json");

    // SGP phylogeny settings
    config.PHYLOGENY(1);
    config.PHYLOGENY_TAXON_TYPE("task-cycle-binned");
    
    sgpmode::SGPWorld world(random, &config);
    world.Setup();
    
    auto& prog_builder = world.GetProgramBuilder();
    emp::Ptr<emp::Systematics<Organism, taxon_t::info_t, datastruct::SymbiontTaxonData>> sym_sys =  world.GetSymSys();

    const size_t not_task_id = world.GetTaskEnv().GetTaskSet().GetID("NOT");
    const size_t nor_task_id = world.GetTaskEnv().GetTaskSet().GetID("NOR");
    const size_t parent_cycles_since_repro = 4; 
    
    emp::Ptr<sgp_host_t> host_parent = emp::NewPtr<sgp_host_t>(&random, &world, &config, prog_builder.CreateNotProgram(100));
    emp::Ptr<sgp_sym_t> symbiont_parent = emp::NewPtr<sgp_sym_t>(&random, &world, &config, prog_builder.CreateNotProgram(100));
    auto& parent_cpustate = symbiont_parent->GetHardware().GetCPUState();
    parent_cpustate.SetParentTaskPerformed(not_task_id, true);
    parent_cpustate.SetParentTaskPerformed(nor_task_id, true);
    parent_cpustate.SetParentCPUCyclesSinceRepro(parent_cycles_since_repro);

    world.AddOrgAt(host_parent, 0);
    host_parent->AddSymbiont(symbiont_parent);
    world.AddSymToSystematic(symbiont_parent);
    
    WHEN("A symbiont is added to the world"){
        THEN("The symbiont is tracked by the symbiont systematic"){
            REQUIRE(symbiont_parent->GetTaxon()->GetInfo() == 10000100.4);
            REQUIRE(sym_sys->GetNumActive() == 1);
        }
    }    
    
     WHEN("A symbiont has a child with the same parent task set but a different cycle count"){
        parent_cpustate.SetCPUCyclesSinceRepro(parent_cycles_since_repro + 1);
        parent_cpustate.SetTaskPerformed(not_task_id, true);
        parent_cpustate.SetTaskPerformed(nor_task_id, true);

        emp::Ptr<sgp_sym_t> symbiont_child = static_cast<sgp_sym_t*>(symbiont_parent->Reproduce().Raw());

        THEN("The parent and children are in different taxa"){
            REQUIRE(symbiont_child->GetTaxon()->GetInfo() == 10000100.5);
            REQUIRE(symbiont_parent->GetTaxon() != symbiont_child->GetTaxon());
        }
        symbiont_child.Delete();
    }
    
    WHEN("A symbiont has a child with the a different parent task set but the same cycle count"){
        parent_cpustate.SetCPUCyclesSinceRepro(parent_cycles_since_repro);
        parent_cpustate.SetTaskPerformed(nor_task_id, true);

        emp::Ptr<sgp_sym_t> symbiont_child = static_cast<sgp_sym_t*>(symbiont_parent->Reproduce().Raw());
        
        THEN("The parent and children are in different taxa"){
            REQUIRE(symbiont_child->GetTaxon()->GetInfo() == 100.4);
            REQUIRE(symbiont_parent->GetTaxon() != symbiont_child->GetTaxon());
        }
        symbiont_child.Delete();
    }

    WHEN("A symbiont has a child with the same parent task set and the same cycle count"){
        parent_cpustate.SetCPUCyclesSinceRepro(parent_cycles_since_repro);
        parent_cpustate.SetTaskPerformed(not_task_id, true);
        parent_cpustate.SetTaskPerformed(nor_task_id, true);

        emp::Ptr<sgp_sym_t> symbiont_child = static_cast<sgp_sym_t*>(symbiont_parent->Reproduce().Raw());
    
        THEN("The parent and children are in the same taxon"){
            REQUIRE(symbiont_child->GetTaxon()->GetInfo() == symbiont_parent->GetTaxon()->GetInfo());
            REQUIRE(symbiont_parent->GetTaxon() == symbiont_child->GetTaxon());
        }
        symbiont_child.Delete();
    }
}

