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
 * This file is dedicated to ensuring that SGPDataNodes create the correct files and track data correctly 
 */

using world_t = sgpmode::SGPWorld;
using cpu_state_t = sgpmode::CPUState<world_t>;
using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;


TEST_CASE("Correct data files are created and written to", "[sgp][sgp-functional]") {
  GIVEN("An SGPWorld with no mutation that is configured to create data files"){
    sgpmode::SymConfigSGP config;
    config.SEED(2);
    config.INTERACTION_MECHANISM("default");
    config.FILE_NAME("DataTest");
    config.POP_SIZE(0);
    config.GRID_X(2);
    config.GRID_Y(2);
    config.HOST_MIN_CYCLES_BEFORE_REPRO(10000);
    config.SYM_MIN_CYCLES_BEFORE_REPRO(10000);
    config.DATA_INT(2);
    size_t prog_length = 20; 
    config.CYCLES_PER_UPDATE(prog_length);
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.FILE_PATH("SGPData_test_output");
    
    emp::Random random(config.SEED());

    std::filesystem::path org_count_fpath = config.FILE_PATH() + "/" + "OrganismCounts"+config.FILE_NAME() + ".csv";
    std::filesystem::path transmission_fpath = config.FILE_PATH() + "/" + "TransmissionRates"+config.FILE_NAME() + ".csv";
    std::filesystem::path tasks_fpath = config.FILE_PATH() + "/" + "Tasks"+config.FILE_NAME() + ".csv";
    std::filesystem::path cur_update_info_fpath = config.FILE_PATH() + "/" + "CurrentUpdateInfo"+config.FILE_NAME() + ".csv";
    std::filesystem::path sym_int_vals_fpath = config.FILE_PATH() + "/" + "SymbiontInteractionValues"+config.FILE_NAME() + ".csv";

    WHEN("The World is ran for 3 Updates with 1 host"){
      world_t world(random, &config);
      world.Setup(); 

      //Creates a host that only does NOT operations
      emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, world.GetProgramBuilder().CreateNotProgram(prog_length));

      //Adds host to world
      world.AddOrgAt(host, 0);
      for (int i = 0; i < 3; i++){
        world.Update();
      }
      
      // OrganismCounts
      std::ifstream file(org_count_fpath);
      std::string str; 
      THEN("The OrganismCounts file should contain 3 lines"){
        std::getline(file, str);
        THEN("The first should be a header"){
          REQUIRE(str == "update,host_count,hosted_sym_count,free_sym_count");
        }
        std::getline(file, str);
        THEN("The second should be 1 host alive at update 0"){
          REQUIRE(str == "0,1,0,0");
        }
        std::getline(file, str);
        THEN("The third should be 1 host alive at update 2"){
          REQUIRE(str == "2,1,0,0");
        }
      }

      // SymbiontInteractionValues
      std::ifstream file2(sym_int_vals_fpath);
      std::string str2; 
      THEN("The SymbiontInteractionValues File should contain 3 lines"){
        std::getline(file2, str2);
        THEN("The first should be a header"){
          REQUIRE(str2 == "update,mean_intval,count,Hist_-1,Hist_-0.9,Hist_-0.8,Hist_-0.7,Hist_-0.6,Hist_-0.5,Hist_-0.4,Hist_-0.3,Hist_-0.2,Hist_-0.1,Hist_0.0,Hist_0.1,Hist_0.2,Hist_0.3,Hist_0.4,Hist_0.5,Hist_0.6,Hist_0.7,Hist_0.8,Hist_0.9");
        }
        std::getline(file2, str2);
        THEN("The second should be all zeroes and a nan in the mean column because there are no symbionts"){
          REQUIRE(str2 == "0,-nan,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
        }
        std::getline(file2, str2);
        THEN("The third should be all zeroes and a nan in the mean column because there are no symbionts"){
          REQUIRE(str2 == "2,-nan,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
        }
      }
      
      // Tasks
      std::ifstream file3(tasks_fpath);
      std::string str3; 
      THEN("The Task File should contain 3 lines"){
        std::getline(file3, str3);
        THEN("The first should be a header"){
          REQUIRE(str3 == "update,host_task_NAND,host_task_NOT,host_task_OR_NOT,host_task_AND,host_task_OR,host_task_AND_NOT,host_task_NOR,host_task_XOR,host_task_EQU,sym_task_NAND,sym_task_NOT,sym_task_OR_NOT,sym_task_AND,sym_task_OR,sym_task_AND_NOT,sym_task_NOR,sym_task_XOR,sym_task_EQU");
        }
        std::getline(file3, str3);
        THEN("The second should be all zeroes because at 0 updates in no tasks should have been completed"){
          REQUIRE(str3 == "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
        }
        std::getline(file3, str3);
        THEN("The third should contain 2 NOT host tasks because in 2 updates 1 host can perform their genome 2 times"){
          REQUIRE(str3 == "2,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
        }
      }
      
      // TransmissionRates 
      std::ifstream file4(transmission_fpath);
      std::string str4; 
      THEN("The TransmissionRates File should contain 3 lines"){
        std::getline(file4, str4);
        THEN("The first should be a header"){
          REQUIRE(str4 == "update,horiz_attempt_-1_-0.8,horiz_attempt_-0.8_-0.6,horiz_attempt_-0.6_0.4,horiz_attempt_-0.4_0.2,horiz_attempt_-0.2_0,horiz_attempt_0_0.2,horiz_attempt_0.2_0.4,horiz_attempt_0.4_0.6,horiz_attempt_0.6_0.8,horiz_attempt_0.8_1,horiz_success_-1_-0.8,horiz_success_-0.8_-0.6,horiz_success_-0.6_0.4,horiz_success_-0.4_0.2,horiz_success_-0.2_0,horiz_success_0_0.2,horiz_success_0.2_0.4,horiz_success_0.4_0.6,horiz_success_0.6_0.8,horiz_success_0.8_1,vert_attempt_-1_-0.8,vert_attempt_-0.8_-0.6,vert_attempt_-0.6_0.4,vert_attempt_-0.4_0.2,vert_attempt_-0.2_0,vert_attempt_0_0.2,vert_attempt_0.2_0.4,vert_attempt_0.4_0.6,vert_attempt_0.6_0.8,vert_attempt_0.8_1,vert_success_-1_-0.8,vert_success_-0.8_-0.6,vert_success_-0.6_0.4,vert_success_-0.4_0.2,vert_success_-0.2_0,vert_success_0_0.2,vert_success_0.2_0.4,vert_success_0.4_0.6,vert_success_0.6_0.8,vert_success_0.8_1");
        }
        std::getline(file4, str4);
        THEN("The second should be all zeroes because there are no symbionts"){
          REQUIRE(str4 == "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
        }
        std::getline(file4, str4);
        THEN("The third should be at 2 updates and the rest be zero because there are no symbionts"){
          REQUIRE(str4 == "2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
        }
      }

      // CurrentUpdateInfo
      std::ifstream file5(cur_update_info_fpath);
      std::string str5; 
      THEN("The CurrentUpdateInfo File should contain 3 lines"){
        std::getline(file5, str5);
        THEN("The first should be a header"){
          REQUIRE(str5 == "update,host_mean_generations,host_variance_generations,sym_mean_generations,sym_variance_generations,NAND_in_host_profile_counts,NOT_in_host_profile_counts,OR_NOT_in_host_profile_counts,AND_in_host_profile_counts,OR_in_host_profile_counts,AND_NOT_in_host_profile_counts,NOR_in_host_profile_counts,XOR_in_host_profile_counts,EQU_in_host_profile_counts,NAND_in_sym_profile_counts,NOT_in_sym_profile_counts,OR_NOT_in_sym_profile_counts,AND_in_sym_profile_counts,OR_in_sym_profile_counts,AND_NOT_in_sym_profile_counts,NOR_in_sym_profile_counts,XOR_in_sym_profile_counts,EQU_in_sym_profile_counts,NAND_in_host_parent_org_counts,NOT_in_host_parent_org_counts,OR_NOT_in_host_parent_org_counts,AND_in_host_parent_org_counts,OR_in_host_parent_org_counts,AND_NOT_in_host_parent_org_counts,NOR_in_host_parent_org_counts,XOR_in_host_parent_org_counts,EQU_in_host_parent_org_counts,NAND_in_sym_parent_org_counts,NOT_in_sym_parent_org_counts,OR_NOT_in_sym_parent_org_counts,AND_in_sym_parent_org_counts,OR_in_sym_parent_org_counts,AND_NOT_in_sym_parent_org_counts,NOR_in_sym_parent_org_counts,XOR_in_sym_parent_org_counts,EQU_in_sym_parent_org_counts,NAND_in_host_current_org_counts,NOT_in_host_current_org_counts,OR_NOT_in_host_current_org_counts,AND_in_host_current_org_counts,OR_in_host_current_org_counts,AND_NOT_in_host_current_org_counts,NOR_in_host_current_org_counts,XOR_in_host_current_org_counts,EQU_in_host_current_org_counts,NAND_in_sym_current_org_counts,NOT_in_sym_current_org_counts,OR_NOT_in_sym_current_org_counts,AND_in_sym_current_org_counts,OR_in_sym_current_org_counts,AND_NOT_in_sym_current_org_counts,NOR_in_sym_current_org_counts,XOR_in_sym_current_org_counts,EQU_in_sym_current_org_counts,NAND_host_sym_profile_matches,NOT_host_sym_profile_matches,OR_NOT_host_sym_profile_matches,AND_host_sym_profile_matches,OR_host_sym_profile_matches,AND_NOT_host_sym_profile_matches,NOR_host_sym_profile_matches,XOR_host_sym_profile_matches,EQU_host_sym_profile_matches,NAND_host_sym_profile_mismatches,NOT_host_sym_profile_mismatches,OR_NOT_host_sym_profile_mismatches,AND_host_sym_profile_mismatches,OR_host_sym_profile_mismatches,AND_NOT_host_sym_profile_mismatches,NOR_host_sym_profile_mismatches,XOR_host_sym_profile_mismatches,EQU_host_sym_profile_mismatches,host_sym_perfect_matches_total,host_sym_any_matches_total,host_parent_num_task_sets,host_parent_entropy_task_sets,host_current_num_task_sets,host_current_entropy_task_sets,sym_parent_num_task_sets,sym_parent_entropy_task_sets,sym_current_num_task_sets,sym_current_entropy_task_sets");
        }
        // setuphosts should toggle parent task completions; we're adding hosts manually, so their parents won't be toggled 
        std::getline(file5, str5);
        THEN("The second should be 1 count of unique host / host parent task sets, and 0s/nans elsewhere") {
          REQUIRE(str5 == "0,0,-nan,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0");
        }
        std::getline(file5, str5);
        THEN("The second should be 1 count of unique host / host parent task sets, 1 count of a host who completed NOT, and 0s/nans elsewhere"){
          REQUIRE(str5 == "2,0,-nan,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0");
        }
      }

      std::filesystem::remove(org_count_fpath);
      std::filesystem::remove(sym_int_vals_fpath);
      std::filesystem::remove(tasks_fpath);
      std::filesystem::remove(transmission_fpath);
      std::filesystem::remove(cur_update_info_fpath);

      THEN("All previously created data files have been removed"){
        REQUIRE(!std::filesystem::exists(org_count_fpath));
        REQUIRE(!std::filesystem::exists(sym_int_vals_fpath));
        REQUIRE(!std::filesystem::exists(tasks_fpath));
        REQUIRE(!std::filesystem::exists(transmission_fpath));
        REQUIRE(!std::filesystem::exists(cur_update_info_fpath));
      }
    }
    
    WHEN("The World is ran for 3 Updates with 2 hosts and 1 symbiont"){
      config.SYM_INT(1);
      config.TASK_PROFILE_MODE("self-all");

      world_t world(random, &config);
      world.Setup(); 

      //Creates a host that only does NOT operations
      emp::Ptr<sgp_host_t> not_host = emp::NewPtr<sgp_host_t>(&random, &world, &config, world.GetProgramBuilder().CreateNotProgram(prog_length));
      emp::Ptr<sgp_host_t> not_nand_host = emp::NewPtr<sgp_host_t>(&random, &world, &config, world.GetProgramBuilder().CreateNotNandProgram(prog_length));
      emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config, world.GetProgramBuilder().CreateNotNandProgram(prog_length));
      symbiont->SetIntVal(config.SYM_INT());

      //Adds hosts to world
      world.AddOrgAt(not_host, 0);
      world.AddOrgAt(not_nand_host, 1);
      not_host->AddSymbiont(symbiont);

      for (int i = 0; i < 3; i++){
        world.Update();
      }
      
      // OrganismCounts
      std::ifstream file(org_count_fpath);
      std::string str; 
      THEN("The OrganismCounts file should contain 3 lines"){
        std::getline(file, str);
        THEN("The first should be a header"){
          REQUIRE(str == "update,host_count,hosted_sym_count,free_sym_count");
        }
        std::getline(file, str);
        THEN("The second should be 2 hosts, 1 symbiont alive at update 0"){
          REQUIRE(str == "0,2,1,0");
        }
        std::getline(file, str);
        THEN("The third hould be 2 hosts, 1 symbiont alive at update 2"){
          REQUIRE(str == "2,2,1,0");
        }
      }

      // SymbiontInteractionValues
      std::ifstream file2(sym_int_vals_fpath);
      std::string str2; 
      THEN("The SymbiontInteractionValues File should contain 3 lines"){
        std::getline(file2, str2);
        THEN("The first should be a header"){
          REQUIRE(str2 == "update,mean_intval,count,Hist_-1,Hist_-0.9,Hist_-0.8,Hist_-0.7,Hist_-0.6,Hist_-0.5,Hist_-0.4,Hist_-0.3,Hist_-0.2,Hist_-0.1,Hist_0.0,Hist_0.1,Hist_0.2,Hist_0.3,Hist_0.4,Hist_0.5,Hist_0.6,Hist_0.7,Hist_0.8,Hist_0.9");
        }
        std::getline(file2, str2);
        THEN("The second should be all a mean of 1, a count of one, and one in the Hist_0.9 column"){
          REQUIRE(str2 == "0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1");
        }
        std::getline(file2, str2);
        THEN("The third should be all a mean of 1, a count of one, and one in the Hist_0.9 column"){
          REQUIRE(str2 == "2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1");
        }
      }
      
      // Tasks
      std::ifstream file3(tasks_fpath);
      std::string str3; 
      THEN("The Task File should contain 3 lines"){
        std::getline(file3, str3);
        THEN("The first should be a header"){
          REQUIRE(str3 == "update,host_task_NAND,host_task_NOT,host_task_OR_NOT,host_task_AND,host_task_OR,host_task_AND_NOT,host_task_NOR,host_task_XOR,host_task_EQU,sym_task_NAND,sym_task_NOT,sym_task_OR_NOT,sym_task_AND,sym_task_OR,sym_task_AND_NOT,sym_task_NOR,sym_task_XOR,sym_task_EQU");
        }
        std::getline(file3, str3);
        THEN("The second should be 1s in host and symbiont NOT tasks because the NOTNAND organisms will have been marked completed for their first task (NOT) "){
          REQUIRE(str3 == "0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0");
        }
        std::getline(file3, str3);
        THEN("The third should contain 4 host NOT tasks, 2 host NAND tasks, 2 sym NAND task, and 2 sym NOT tasks"){
          REQUIRE(str3 == "2,2,4,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0");
        }
      }
      
      // TransmissionRates 
      std::ifstream file4(transmission_fpath);
      std::string str4; 
      THEN("The TransmissionRates File should contain 3 lines"){
        std::getline(file4, str4);
        THEN("The first should be a header"){
          REQUIRE(str4 == "update,horiz_attempt_-1_-0.8,horiz_attempt_-0.8_-0.6,horiz_attempt_-0.6_0.4,horiz_attempt_-0.4_0.2,horiz_attempt_-0.2_0,horiz_attempt_0_0.2,horiz_attempt_0.2_0.4,horiz_attempt_0.4_0.6,horiz_attempt_0.6_0.8,horiz_attempt_0.8_1,horiz_success_-1_-0.8,horiz_success_-0.8_-0.6,horiz_success_-0.6_0.4,horiz_success_-0.4_0.2,horiz_success_-0.2_0,horiz_success_0_0.2,horiz_success_0.2_0.4,horiz_success_0.4_0.6,horiz_success_0.6_0.8,horiz_success_0.8_1,vert_attempt_-1_-0.8,vert_attempt_-0.8_-0.6,vert_attempt_-0.6_0.4,vert_attempt_-0.4_0.2,vert_attempt_-0.2_0,vert_attempt_0_0.2,vert_attempt_0.2_0.4,vert_attempt_0.4_0.6,vert_attempt_0.6_0.8,vert_attempt_0.8_1,vert_success_-1_-0.8,vert_success_-0.8_-0.6,vert_success_-0.6_0.4,vert_success_-0.4_0.2,vert_success_-0.2_0,vert_success_0_0.2,vert_success_0.2_0.4,vert_success_0.4_0.6,vert_success_0.6_0.8,vert_success_0.8_1");
        }
        std::getline(file4, str4);
        THEN("The second should be all zeroes because the symbiont hasn't transmitted"){
          REQUIRE(str4 == "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
        }
        std::getline(file4, str4);
        THEN("The third should be at 2 updates and the rest should be all zeroes because the symbiont hasn't transmitted"){
          REQUIRE(str4 == "2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
        }
      }

      // CurrentUpdateInfo
      std::ifstream file5(cur_update_info_fpath);
      std::string str5; 
      THEN("The CurrentUpdateInfo File should contain 3 lines"){
        std::getline(file5, str5);
        THEN("The first should be a header"){
          REQUIRE(str5 == "update,host_mean_generations,host_variance_generations,sym_mean_generations,sym_variance_generations,NAND_in_host_profile_counts,NOT_in_host_profile_counts,OR_NOT_in_host_profile_counts,AND_in_host_profile_counts,OR_in_host_profile_counts,AND_NOT_in_host_profile_counts,NOR_in_host_profile_counts,XOR_in_host_profile_counts,EQU_in_host_profile_counts,NAND_in_sym_profile_counts,NOT_in_sym_profile_counts,OR_NOT_in_sym_profile_counts,AND_in_sym_profile_counts,OR_in_sym_profile_counts,AND_NOT_in_sym_profile_counts,NOR_in_sym_profile_counts,XOR_in_sym_profile_counts,EQU_in_sym_profile_counts,NAND_in_host_parent_org_counts,NOT_in_host_parent_org_counts,OR_NOT_in_host_parent_org_counts,AND_in_host_parent_org_counts,OR_in_host_parent_org_counts,AND_NOT_in_host_parent_org_counts,NOR_in_host_parent_org_counts,XOR_in_host_parent_org_counts,EQU_in_host_parent_org_counts,NAND_in_sym_parent_org_counts,NOT_in_sym_parent_org_counts,OR_NOT_in_sym_parent_org_counts,AND_in_sym_parent_org_counts,OR_in_sym_parent_org_counts,AND_NOT_in_sym_parent_org_counts,NOR_in_sym_parent_org_counts,XOR_in_sym_parent_org_counts,EQU_in_sym_parent_org_counts,NAND_in_host_current_org_counts,NOT_in_host_current_org_counts,OR_NOT_in_host_current_org_counts,AND_in_host_current_org_counts,OR_in_host_current_org_counts,AND_NOT_in_host_current_org_counts,NOR_in_host_current_org_counts,XOR_in_host_current_org_counts,EQU_in_host_current_org_counts,NAND_in_sym_current_org_counts,NOT_in_sym_current_org_counts,OR_NOT_in_sym_current_org_counts,AND_in_sym_current_org_counts,OR_in_sym_current_org_counts,AND_NOT_in_sym_current_org_counts,NOR_in_sym_current_org_counts,XOR_in_sym_current_org_counts,EQU_in_sym_current_org_counts,NAND_host_sym_profile_matches,NOT_host_sym_profile_matches,OR_NOT_host_sym_profile_matches,AND_host_sym_profile_matches,OR_host_sym_profile_matches,AND_NOT_host_sym_profile_matches,NOR_host_sym_profile_matches,XOR_host_sym_profile_matches,EQU_host_sym_profile_matches,NAND_host_sym_profile_mismatches,NOT_host_sym_profile_mismatches,OR_NOT_host_sym_profile_mismatches,AND_host_sym_profile_mismatches,OR_host_sym_profile_mismatches,AND_NOT_host_sym_profile_mismatches,NOR_host_sym_profile_mismatches,XOR_host_sym_profile_mismatches,EQU_host_sym_profile_mismatches,host_sym_perfect_matches_total,host_sym_any_matches_total,host_parent_num_task_sets,host_parent_entropy_task_sets,host_current_num_task_sets,host_current_entropy_task_sets,sym_parent_num_task_sets,sym_parent_entropy_task_sets,sym_current_num_task_sets,sym_current_entropy_task_sets");
        }
        std::getline(file5, str5);
        THEN("The second should accurately track generations and task count/match/mismatch at update 0") {
          REQUIRE(str5 == "0,0,0,0,-nan,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,2,1,1,0,1,0");
        }
        std::getline(file5, str5);
        THEN("The third should accurately track generations and task count/match/mismatch at update 2"){
          REQUIRE(str5 == "2,0,0,0,-nan,1,2,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,1,0,2,1,1,0,1,0");
        }
      }

      std::filesystem::remove(org_count_fpath);
      std::filesystem::remove(sym_int_vals_fpath);
      std::filesystem::remove(tasks_fpath);
      std::filesystem::remove(transmission_fpath);
      std::filesystem::remove(cur_update_info_fpath);

      THEN("All previously created data files have been removed"){
        REQUIRE(!std::filesystem::exists(org_count_fpath));
        REQUIRE(!std::filesystem::exists(sym_int_vals_fpath));
        REQUIRE(!std::filesystem::exists(tasks_fpath));
        REQUIRE(!std::filesystem::exists(transmission_fpath));
        REQUIRE(!std::filesystem::exists(cur_update_info_fpath));
      }
    }
  }
}