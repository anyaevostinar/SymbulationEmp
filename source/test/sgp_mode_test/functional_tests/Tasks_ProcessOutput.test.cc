#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"

#include "../../../catch/catch.hpp"

TEST_CASE("ProcessOutput Functionality with LogicTasks", "[sgp][sgp-functional]"){
  emp::Random random(1);
  SymConfigSGP config;
  config.SEED(1);
  config.MUTATION_RATE(0.0);
  config.MUTATION_SIZE(0.002);
  config.TRACK_PARENT_TASKS(1);
  config.VT_TASK_MATCH(1);
  config.HOST_ONLY_FIRST_TASK_CREDIT(0);
  config.SYM_ONLY_FIRST_TASK_CREDIT(0);

  SGPWorld world(random, &config, LogicTasks);

  ProgramBuilder not_program;
  not_program.AddNot();

  emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, not_program.Build(100));
  world.AddOrgAt(host, 0);
  GIVEN("Two Inputs"){
    //These two inputs were chosen because their is no output for any operation that is the same as another output
    host->GetCPU().state.input_buf.push(734856699);
    host->GetCPU().state.input_buf.push(1177728054);

    WHEN("ProcessOutput is run on an organism with those inputs, The NOT of the first input, and ONLY_FIRST_TASK_CREDIT is 0"){
      //The result of applying the NOT bitwise operations to the binary form of 734856699
      long not_output = 3560110596;
      
      host->GetCPU().state.world->GetTaskSet().ProcessOutput(host->GetCPU().state,not_output,0);
      THEN("Organism should have 5 points"){
        REQUIRE(host->GetPoints() == 5);
      }
      THEN("Organism should be marked as having performed NOT"){
        for (int i = 0; i < 9; i++){
          if(i == 0){
            REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 1);
          }
          else{
            REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 0);
          }
        }
      }
      THEN("World data should have marked 1 completion for NOT"){
        for (auto data : world.GetTaskSet()) {
          if(data.task.name == "NOT"){
            REQUIRE(data.n_succeeds_host == 1);
          }
          else{
            REQUIRE(data.n_succeeds_host == 0);
          }
        }
      }
    }

    WHEN("ProcessOutput is run on an organism with those inputs, The NAND of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
      //The result of applying the NAND bitwise operations to the binary form of 734856699 and 1177728054
      long nand_output = 4261411789;
      host->GetCPU().state.world->GetTaskSet().ProcessOutput(host->GetCPU().state,nand_output,0);
      THEN("Organism should have 5 points"){
        REQUIRE(host->GetPoints() == 5);
      }
      THEN("Organism should be marked as having performed NAND"){
        for (int i = 0; i < 9; i++){
          if(i == 1){
          REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 1);
          }
          else{
          REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 0);
          }
        }
      }
      THEN("World data should have marked 1 completion for NAND"){
        for (auto data : world.GetTaskSet()) {
          if(data.task.name == "NAND"){
            REQUIRE(data.n_succeeds_host == 1);
          }
          else{
            REQUIRE(data.n_succeeds_host == 0);
          }
        }
      }
    }

    WHEN("ProcessOutput is run on an organism with those inputs, The AND of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
      //The result of applying the AND bitwise operations to the binary form of 734856699 and 1177728054
      long and_output = 33555506;
      host->GetCPU().state.world->GetTaskSet().ProcessOutput(host->GetCPU().state,and_output,0);
      THEN("Organism should have 5 points"){
        REQUIRE(host->GetPoints() == 5);
      }
      THEN("Organism should be marked as having performed AND"){
        for (int i = 0; i < 9; i++){
          if(i == 2){
            REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 1);
          }
          else{
            REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 0);
          }
        }
      }
      THEN("World data should have marked 1 completion for AND"){
        for (auto data : world.GetTaskSet()) {
          if(data.task.name == "AND"){
            REQUIRE(data.n_succeeds_host == 1);
          }
          else{
            REQUIRE(data.n_succeeds_host == 0);
          }
        }
      }
    }

    WHEN("ProcessOutput is run on an organism with those inputs, The ORN of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
      //The result of applying the ORN bitwise operations to the binary form of 734856699 and 1177728054
      long orn_output = 3150794747;
      host->GetCPU().state.world->GetTaskSet().ProcessOutput(host->GetCPU().state,orn_output,0);
      THEN("Organism should have 5 points"){
        REQUIRE(host->GetPoints() == 5);
      }
      THEN("Organism should be marked as having performed ORN"){
        for (int i = 0; i < 9; i++){
          if(i == 3){
            REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 1);
          }
          else{
            REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 0);
          }
        }
      }
      THEN("World data should have marked 1 completion for ORN"){
        for (auto data : world.GetTaskSet()) {
          if(data.task.name == "ORN"){
          
          REQUIRE(data.n_succeeds_host == 1);
          }
          else{
            REQUIRE(data.n_succeeds_host == 0);
          }
        }
      }
    }

    WHEN("ProcessOutput is run on an organism with those inputs, The OR of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
      //The result of applying the OR bitwise operations to the binary form of 734856699 and 1177728054
      long or_output = 1879029247;
      host->GetCPU().state.world->GetTaskSet().ProcessOutput(host->GetCPU().state,or_output,0);
      THEN("Organism should have 5 points"){
        REQUIRE(host->GetPoints() == 5);
      }
      THEN("Organism should be marked as having performed OR"){
        for (int i = 0; i < 9; i++){
          if(i == 4){
            REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 1);
          }
          else{
            REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 0);
          }
        }
      }
      THEN("World data should have marked 1 completion for OR"){
        for (auto data : world.GetTaskSet()) {
          if(data.task.name == "OR"){
            REQUIRE(data.n_succeeds_host == 1);
          }
          else{
            REQUIRE(data.n_succeeds_host == 0);
          }
        }
      }
    }

    WHEN("ProcessOutput is run on an organism with those inputs, The ANDN of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
      //The result of applying the ANDN bitwise operations to the binary form of 734856699 and 1177728054
      long andn_output = 701301193;
      host->GetCPU().state.world->GetTaskSet().ProcessOutput(host->GetCPU().state,andn_output,0);
      THEN("Organism should have 5 points"){
        REQUIRE(host->GetPoints() == 5);
      }
      THEN("Organism should be marked as having performed ANDN"){
        for (int i = 0; i < 9; i++){
          if(i == 5){
            REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 1);
          }
          else{
            REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 0);
          }
        }
      }
      THEN("World data should have marked 1 completion for ANDN"){
        for (auto data : world.GetTaskSet()) {
          if(data.task.name == "ANDN"){
            REQUIRE(data.n_succeeds_host == 1);
          }
          else{
            REQUIRE(data.n_succeeds_host == 0);
          }
        }
      }
    }

    WHEN("ProcessOutput is run on an organism with those inputs, The NOR of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
      //The result of applying the NOR bitwise operations to the binary form of 734856699 and 1177728054
      long nor_output = 2415938048;
      host->GetCPU().state.world->GetTaskSet().ProcessOutput(host->GetCPU().state,nor_output,0);
      THEN("Organism should have 5 points"){
        REQUIRE(host->GetPoints() == 5);
      }
      THEN("Organism should be marked as having performed NOR"){
        for (int i = 0; i < 9; i++){
          if(i == 6){
            REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 1);
          }
          else{
            REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 0);
          }
        }
      }
      THEN("World data should have marked 1 completion for NOR"){
        for (auto data : world.GetTaskSet()) {
          if(data.task.name == "NOR"){
            REQUIRE(data.n_succeeds_host == 1);
          }
          else{
            REQUIRE(data.n_succeeds_host == 0);
          }
        }
      }
    }

    WHEN("ProcessOutput is run on an organism with those inputs, The XOR of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
      //The result of applying the XOR bitwise operations to the binary form of 734856699 and 1177728054
      long xor_output = 1845473741;
      host->GetCPU().state.world->GetTaskSet().ProcessOutput(host->GetCPU().state,xor_output,0);
      THEN("Organism should have 5 points"){
        REQUIRE(host->GetPoints() == 5);
      }
      THEN("Organism should be marked as having performed XOR"){
        for (int i = 0; i < 9; i++){
          if(i == 7){
            REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 1);
          }
          else{
            REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 0);
          }
        }
      }
      THEN("World data should have marked 1 completion for XOR"){
        for (auto data : world.GetTaskSet()) {
          if(data.task.name == "XOR"){  
            REQUIRE(data.n_succeeds_host == 1);
          }
          else{
            REQUIRE(data.n_succeeds_host == 0);
          }
        }
      }
    }

    WHEN("ProcessOutput is run on an organism with those inputs, The EQU of the two inputs, and ONLY_FIRST_TASK_CREDIT is 0"){
      //The result of applying the EQU bitwise operations to the binary form of 734856699 and 1177728054
      long equ_output = 2449493554;
      host->GetCPU().state.world->GetTaskSet().ProcessOutput(host->GetCPU().state,equ_output,0);
      THEN("Organism should have 5 points"){
        REQUIRE(host->GetPoints() == 5);
      }
      THEN("Organism should be marked as having performed EQU"){
        for (int i = 0; i < 9; i++){
          if(i == 8){
            REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 1);
          }
          else{
            REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 0);
          }
        }
      }
      THEN("World data should have marked 1 completion for EQU"){
        for (auto data : world.GetTaskSet()) {
          if(data.task.name == "EQU"){
            REQUIRE(data.n_succeeds_host == 1);
          }
          else{
            REQUIRE(data.n_succeeds_host == 0);
          }
        }
      }
    }
  }
}

TEST_CASE("ProcessOutput Functionality with LogicTasksDiff", "[sgp][sgp-functional]"){
  emp::Random random(1);
  SymConfigSGP config;
  config.SEED(1);
  config.MUTATION_RATE(0.0);
  config.MUTATION_SIZE(0.002);
  config.TRACK_PARENT_TASKS(1);
  config.VT_TASK_MATCH(1);
  config.HOST_ONLY_FIRST_TASK_CREDIT(0);
  config.SYM_ONLY_FIRST_TASK_CREDIT(0);
  config.DIFFERENT_TASK_VALUES(1);

  SGPWorld world(random, &config, LogicTasksDiff);

  ProgramBuilder not_program;
  not_program.AddNot();

  emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, not_program.Build(100));
  world.AddOrgAt(host, 0);
  GIVEN("Two Inputs"){
    //These two inputs were chosen because there is no output for any operation that is the same as another output
    host->GetCPU().state.input_buf.push(734856699);
    host->GetCPU().state.input_buf.push(1177728054);

    WHEN("ProcessOutput is run on an organism with those inputs, The NOT of the first input, and ONLY_FIRST_TASK_CREDIT is 0"){
      //The result of applying the NOT bitwise operations to the binary form of 734856699
      long not_output = 3560110596;
      
      host->GetCPU().state.world->GetTaskSet().ProcessOutput(host->GetCPU().state,not_output,0);
      THEN("Organism should have 1 point"){
        REQUIRE(host->GetPoints() == 1);
      }
      THEN("Organism should be marked as having performed NOT"){
        for (int i = 0; i < 9; i++){
          if(i == 0){
            REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 1);
          }
          else{
            REQUIRE(host->GetCPU().state.tasks_performed->Get(i) == 0);
          }
        }
      }
      THEN("World data should have marked 1 completion for NOT"){
        for (auto data : world.GetTaskSet()) {
          if(data.task.name == "NOT"){
            REQUIRE(data.n_succeeds_host == 1);
          }
          else{
            REQUIRE(data.n_succeeds_host == 0);
          }
        }
      }
    }
  }
}