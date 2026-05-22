#include "../../../sgp_mode/hardware/SGPHardware.h"
#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorld.cc"
#include "../../../sgp_mode/SGPWorldSetup.cc"
#include "../../../sgp_mode/SGPWorldData.cc"
#include "../../../sgp_mode/ProgramBuilder.h"

#include "../../../catch/catch.hpp"

TEST_CASE("Test SenseTask instruction", "[sgp]"){
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
  using program_t = typename world_t::sgp_prog_t;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
  using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;
  using tag_t = typename hw_spec_t::tag_t;

  sgpmode::SymConfigSGP config;
  config.CYCLES_PER_UPDATE(0);
  config.HOST_REPRO_RES(1);
  config.SEED(61);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.FILE_PATH("Instructions_test_output");
  config.POP_SIZE(1);
  config.START_MOI(1);
  config.TASK_IO_UNIQUE_OUTPUT(true);
  config.SYM_DONATE_PROP(0.5);
  config.FREE_LIVING_SYMS(false);
  config.SYM_LIMIT(1);
  config.PHAGE_EXCLUDE(false);
  config.ENABLE_TEMP_CHANGING_ENVIRONMENT(true);
  config.TEMP_CHANGING_ENVIRONMENT_ORG_TYPE("plastic-both");

  emp::Random random(config.SEED());
  world_t world(random, &config);
  world.Setup();
  auto& prog_builder = world.GetProgramBuilder();

  // World should have one organism inside.
  REQUIRE(world.IsOccupied(0));
  auto& org = world.GetOrg(0);
  auto& sgp_host = static_cast<sgp_host_t&>(org);
  hardware_t& host_hw = sgp_host.GetHardware();
  REQUIRE(host_hw.GetCPUState().GetNumTasks() == 9);

  // Host should have one symbiont inside.
  REQUIRE(sgp_host.HasSym());
  auto& sgp_sym = static_cast<sgp_sym_t&>(*sgp_host.GetSymbionts().at(0));
  hardware_t& sym_hw = sgp_sym.GetHardware();
  REQUIRE(sym_hw.GetCPUState().GetNumTasks() == 9);
  
  // grab task ids
  size_t not_task_id = world.GetTaskEnv().GetTaskSet().GetID("NOT");
  size_t nand_task_id = world.GetTaskEnv().GetTaskSet().GetID("NAND");
  
  // setup correct reward/punishment values for update 0
  world.Update();

  SECTION("Test SenseTask instruction") {
    WHEN("A host runs a task which is rewarded and then the SenseTask instruction") {
      program_t host_program;
      prog_builder.AddStartAnchor(host_program);
      prog_builder.AddInst(host_program, "IO", 0);
      prog_builder.AddTask_Not(host_program);
      prog_builder.AddInst(host_program, "SenseTask", 0, 1);
      host_hw.Reset();
      host_hw.SetProgram(host_program);
      world.AssignNewEnvIO(host_hw.GetCPUState());
      
      // NOT is currently rewarded. 
      REQUIRE(world.GetTaskEnv().GetHostTaskReq(not_task_id).task_value > 0);
      REQUIRE(sgp_host.IsHost());
      
      host_hw.SetRegisters({3, 2, 5}); // Initial register values

      THEN("SenseTask puts a 1 into register 1"){
        // Run host program
        host_hw.RunCPUStep(1); // Anchor
        REQUIRE(host_hw.GetRegister(0) == 3);
        REQUIRE(host_hw.GetRegister(1) == 2);
        REQUIRE(host_hw.GetRegister(2) == 5);

        host_hw.RunCPUStep(1); // io   r0 // puts input1 into a
        REQUIRE(host_hw.GetRegister(0) == 3165823790);
        REQUIRE(host_hw.GetRegister(1) == 2);
        REQUIRE(host_hw.GetRegister(2) == 5);
        
        host_hw.RunCPUStep(1); // nand r0, r0, r0 
        REQUIRE(host_hw.GetRegister(0) == 1129143505); 
        REQUIRE(host_hw.GetRegister(1) == 2);
        REQUIRE(host_hw.GetRegister(2) == 5);

        host_hw.RunCPUStep(1); // SenseTask r0 r1 // puts 1 into b
        REQUIRE(host_hw.GetRegister(0) == 1129143505);
        REQUIRE(host_hw.GetRegister(1) == 1);
        REQUIRE(host_hw.GetRegister(2) == 5);
      }
    }  

    WHEN("A host runs a task which is punished and then the SenseTask instruction") {
      program_t host_program;
      prog_builder.AddStartAnchor(host_program);
      prog_builder.AddInst(host_program, "IO", 0);
      prog_builder.AddInst(host_program, "IO", 1);
      prog_builder.AddTask_Nand(host_program);
      prog_builder.AddInst(host_program, "SenseTask", 0, 1);
      host_hw.Reset();
      host_hw.SetProgram(host_program);
      world.AssignNewEnvIO(host_hw.GetCPUState());

      // NAND is currently punished. 
      REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value < 0);
      REQUIRE(sgp_host.IsHost());
      
      host_hw.SetRegisters({7, 12, 9}); // Initial register values

      THEN("SenseTask puts a 0 into register 1"){
        // Run host program
        host_hw.RunCPUStep(1); // Anchor
        REQUIRE(host_hw.GetRegister(0) == 7);
        REQUIRE(host_hw.GetRegister(1) == 12);
        REQUIRE(host_hw.GetRegister(2) == 9);

        host_hw.RunCPUStep(1); //  io   r0 // puts input1 into a
        REQUIRE(host_hw.GetRegister(0) == 3165823790); 
        REQUIRE(host_hw.GetRegister(1) == 12);
        REQUIRE(host_hw.GetRegister(2) == 9);

        host_hw.RunCPUStep(1); //  io   r1 // puts input2 into b
        REQUIRE(host_hw.GetRegister(0) == 3165823790);  
        REQUIRE(host_hw.GetRegister(1) == 2437641460);   
        REQUIRE(host_hw.GetRegister(2) == 9);            
  
        host_hw.RunCPUStep(1); //  nand r0, r1, r0 // put nand into a
        REQUIRE(host_hw.GetRegister(0) == 1878908891);  
        REQUIRE(host_hw.GetRegister(1) == 2437641460);
        REQUIRE(host_hw.GetRegister(2) == 9);

        host_hw.RunCPUStep(1); // SenseTask r0 r1 // puts 0 into b
        REQUIRE(host_hw.GetRegister(0) == 1878908891); 
        REQUIRE(host_hw.GetRegister(1) == 0);
        REQUIRE(host_hw.GetRegister(2) == 9);
      }
    }

    WHEN("A symbiont runs a task which is rewarded and then the SenseTask instruction"){
      program_t sym_program;
      prog_builder.AddStartAnchor(sym_program);
      prog_builder.AddInst(sym_program, "IO", 0);
      prog_builder.AddTask_Not(sym_program);
      prog_builder.AddInst(sym_program, "SenseTask", 0, 1);
      sym_hw.Reset();
      sym_hw.SetProgram(sym_program);
      world.AssignNewEnvIO(sym_hw.GetCPUState());

      // NOT is currently rewarded. 
      REQUIRE(world.GetTaskEnv().GetHostTaskReq(not_task_id).task_value > 0);
      REQUIRE(sgp_host.IsHost());
      
      sym_hw.SetRegisters({3, 2, 5}); // Initial register values

      THEN("SenseTask puts a 1 into register 1"){
        // Run symbiont program
        sym_hw.RunCPUStep(1); // Anchor
        REQUIRE(sym_hw.GetRegister(0) == 3);
        REQUIRE(sym_hw.GetRegister(1) == 2);
        REQUIRE(sym_hw.GetRegister(2) == 5);
        
        sym_hw.RunCPUStep(1); // io   r0 // puts input1 into a
        REQUIRE(sym_hw.GetRegister(0) == 3165823790);
        REQUIRE(sym_hw.GetRegister(1) == 2);
        REQUIRE(sym_hw.GetRegister(2) == 5);

        sym_hw.RunCPUStep(1); // nand r0, r0, r0 
        REQUIRE(sym_hw.GetRegister(0) == 1129143505); 
        REQUIRE(sym_hw.GetRegister(1) == 2);
        REQUIRE(sym_hw.GetRegister(2) == 5);

        sym_hw.RunCPUStep(1); // SenseTask r0 r1 // puts  1 into b
        REQUIRE(sym_hw.GetRegister(0) == 1129143505);
        REQUIRE(sym_hw.GetRegister(1) == 1);
        REQUIRE(sym_hw.GetRegister(2) == 5);
      }
    }

    WHEN("A symbiont runs a task which is punished and then the SenseTask instruction") {
      program_t sym_program;
      prog_builder.AddStartAnchor(sym_program);
      prog_builder.AddInst(sym_program, "IO", 0);
      prog_builder.AddInst(sym_program, "IO", 1);
      prog_builder.AddTask_Nand(sym_program);
      prog_builder.AddInst(sym_program, "SenseTask", 0, 1);
      sym_hw.Reset();
      sym_hw.SetProgram(sym_program);
      world.AssignNewEnvIO(sym_hw.GetCPUState());

      // NAND is currently punished. 
      REQUIRE(world.GetTaskEnv().GetHostTaskReq(nand_task_id).task_value < 0);
      REQUIRE(sgp_host.IsHost());
      
      sym_hw.SetRegisters({7, 12, 9}); // Initial register values

      THEN("SenseTask puts a 0 into register 1"){
        // Run symbiont program
        sym_hw.RunCPUStep(1); // Anchor
        REQUIRE(sym_hw.GetRegister(0) == 7);
        REQUIRE(sym_hw.GetRegister(1) == 12);
        REQUIRE(sym_hw.GetRegister(2) == 9);

        sym_hw.RunCPUStep(1); //  io   r0 // puts input1 into a
        REQUIRE(sym_hw.GetRegister(0) == 3165823790); 
        REQUIRE(sym_hw.GetRegister(1) == 12);
        REQUIRE(sym_hw.GetRegister(2) == 9);

        sym_hw.RunCPUStep(1); //  io   r1 // puts input2 into b
        REQUIRE(sym_hw.GetRegister(0) == 3165823790);   
        REQUIRE(sym_hw.GetRegister(1) == 2437641460);  
        REQUIRE(sym_hw.GetRegister(2) == 9);            

        sym_hw.RunCPUStep(1); //  nand r0, r1, r0 // put nand into a
        REQUIRE(sym_hw.GetRegister(0) == 1878908891); 
        REQUIRE(sym_hw.GetRegister(1) == 2437641460);
        REQUIRE(sym_hw.GetRegister(2) == 9);

        sym_hw.RunCPUStep(1); // SenseTask r0 r1 // puts 0 into b
        REQUIRE(sym_hw.GetRegister(0) == 1878908891); 
        REQUIRE(sym_hw.GetRegister(1) == 0);
        REQUIRE(sym_hw.GetRegister(2) == 9);
      }
    }
  }
}