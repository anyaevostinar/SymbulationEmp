
TEST_CASE("Reproduce instruction", "[sgp]") {
    // reproduce fails if:
    // pos is invalid
    // repro happened once this update already
    // not enough points (sym)
    // not enough points (host)
    // not enough cycles (sym)
    // no enough cycles (host)

    // otherwise, reproduce succeeds. in which case:
    // point total is decremented (host)
    // point total is decremented (sym)
    // state marked as reproduction in progress (host)
    // state marked as reproduction in progress (sym)
    // org position added to repro queue (host)
    // org position added to repro queue (sym)  

    emp::Random random(26);
    SymConfigSGP config;
    SGPWorld world(random, &config, LogicTasks);
    
    emp::WorldPosition fake_pos = emp::WorldPosition(5,0);
    size_t host_repro_res = 20; 
    size_t sym_horiz_trans_res = 10;
    size_t host_min_cycles = 3; 
    size_t sym_min_cycles = 2;

    config.HOST_MIN_CYCLES_BEFORE_REPRO(host_min_cycles);
    config.SYM_MIN_CYCLES_BEFORE_REPRO(sym_min_cycles);
    config.HOST_REPRO_RES(host_repro_res);
    config.SYM_HORIZ_TRANS_RES(sym_horiz_trans_res);

    // create repro program
    sgpl::Program<Spec> program;
    // Set everything to 0 - this makes them no-ops since that's the first
    // inst in the library
    program.resize(100);
    program[0].op_code = Library::GetOpCode("Global Anchor");
    program[0].tag = START_TAG;
    for (int i = 1 ; i < 100; i++){
        program[i].op_code = Library::GetOpCode("Reproduce");
    }

    WHEN("A host reproduce instruction is called"){
        size_t start_point_amount = host_repro_res + 7;
        emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, program);
        
        // remove all other limiting factors
        host->SetPoints(start_point_amount);
        REQUIRE(fake_pos.IsValid());
        REQUIRE(host->GetCPU().state.in_progress_repro == -1);

        WHEN("The host has not completed enough CPU cycles"){
            host->GetCPU().RunCPUStep(fake_pos, host_min_cycles); // completing the required number
            // of CPU cycles enables the org to reproduce the next time runcpustep is called,
            // but NOT during that sets them over the edge

            THEN("The host is not added to the reproduction queue"){
                REQUIRE(world.to_reproduce.size() == 0);
            }
            THEN("The host is not charged points for reproduction"){
                REQUIRE(host->GetPoints() == start_point_amount);
            }
            THEN("The host state is not marked as having reproduction in progress"){
                REQUIRE(host->GetCPU().state.in_progress_repro == -1);
            }
            THEN("The host CPU cycle counter is still counting"){
                REQUIRE(host->GetCPU().state.cpu_cycles_since_repro == host_min_cycles);
            }
        }

        WHEN("The host has completed enough CPU cycles"){
            host->GetCPU().RunCPUStep(fake_pos, host_min_cycles); // accumulate enough cycles
            host->GetCPU().RunCPUStep(fake_pos, 1); // call repro

            THEN("The host is added to the reproduction queue"){
                REQUIRE(world.to_reproduce.size() == 1);
                REQUIRE(world.to_reproduce.at(0) == host);
            }
            THEN("The host is charged points for reproduction"){
                REQUIRE(host->GetPoints() == start_point_amount - host_repro_res);
            }
            THEN("The host state is marked as having reproduction in progress"){
                REQUIRE(host->GetCPU().state.in_progress_repro == 0);
            }
            THEN("The host CPU cycle counter is reset to 0"){
                REQUIRE(host->GetCPU().state.cpu_cycles_since_repro == 1);
                // in repro, cycles will be set to 0. the completion of the
                // cycle which calls repro will then increment the counter to 1.
            }
        }
        host.Delete();
    }

    WHEN("A symbiont reproduce instruction is called"){
        size_t start_point_amount = sym_horiz_trans_res + 7;
        emp::Ptr<SGPSymbiont> symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config, program);
        
        // remove all other limiting factors
        symbiont->SetPoints(start_point_amount);
        REQUIRE(fake_pos.IsValid());
        REQUIRE(symbiont->GetCPU().state.in_progress_repro == -1);

        WHEN("The symbiont has not completed enough CPU cycles"){
            symbiont->GetCPU().RunCPUStep(fake_pos, sym_min_cycles); // completing the required number
            // of CPU cycles enables the org to reproduce the next cycle

            THEN("The symbiont is not added to the reproduction queue"){
                REQUIRE(world.to_reproduce.size() == 0);
            }
            THEN("The symbiont is not charged points for reproduction"){
                REQUIRE(symbiont->GetPoints() == start_point_amount);
            }
            THEN("The symbiont state is not marked as having reproduction in progress"){
                REQUIRE(symbiont->GetCPU().state.in_progress_repro == -1);
            }
            THEN("The symbiont CPU cycle counter is still counting"){
                REQUIRE(symbiont->GetCPU().state.cpu_cycles_since_repro == sym_min_cycles);
            }
        }

        WHEN("The symbiont has completed enough CPU cycles"){
            symbiont->GetCPU().RunCPUStep(fake_pos, sym_min_cycles);
            symbiont->GetCPU().RunCPUStep(fake_pos, 1);
            THEN("The symbiont is added to the reproduction queue"){
                REQUIRE(world.to_reproduce.size() == 1);
                REQUIRE(world.to_reproduce.at(0) == symbiont);
            }
            THEN("The symbiont is charged points for reproduction"){
                REQUIRE(symbiont->GetPoints() == start_point_amount - sym_horiz_trans_res);
            }
            THEN("The symbiont state is marked as having reproduction in progress"){
                REQUIRE(symbiont->GetCPU().state.in_progress_repro == 0);
            }
            THEN("The symbiont CPU cycle counter is reset to 0"){
                REQUIRE(symbiont->GetCPU().state.cpu_cycles_since_repro == 1);
            }
        }
        symbiont.Delete();
    }

}