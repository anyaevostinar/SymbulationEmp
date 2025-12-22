#include "../../../sgp_mode/GenomeLibrary.h"
#include "../../../sgp_mode/CPU.h"

#include "../../../catch/catch.hpp"

/**
 * This file is dedicated to unit tests for GenomeLibrary
 */

TEST_CASE("BuildNoRepro creates obligate mutualist program", "[sgp][sgp-unit]") {
  GIVEN("A program created by BuildNoRepro"){
    ProgramBuilder builder;
    size_t program_len = 100;

    sgpl::Program<Spec> program = builder.BuildNoRepro(program_len);

    REQUIRE(program.size() == program_len);

    THEN("The last 5 instructions of the program is Donate"){
      for (size_t i = program.size() - 5; i < program.size(); ++i) {
        REQUIRE(program[i].op_code == Library::GetOpCode("Donate"));
      }
    }

    THEN("The program does not have the reproduce instruction"){
      for (auto &inst : program) {
        REQUIRE(inst.op_code != Library::GetOpCode("Reproduce"));
      }
    }

    THEN("The program starts with a Global Anchor"){
      REQUIRE(program[0].op_code == Library::GetOpCode("Global Anchor"));
      REQUIRE(program[0].tag == START_TAG);
    }
  }

}

TEST_CASE("All programs are built correctly","[sgp][sgp-unit]"){
  size_t program_len = 100;
  ProgramBuilder builder;
  WHEN("A NOT program is built"){
    builder.AddNot();
    sgpl::Program<Spec> program = builder.Build(program_len);

    THEN("The program is of length 100"){
    REQUIRE(program.size() == program_len);
    }

    THEN("The program starts with a global anchor"){
      REQUIRE(program[0].op_code == Library::GetOpCode("Global Anchor"));
    }

    THEN("The program contains 93 nop instructions in a row"){
    for (size_t i = 1; i < (program.size() - 4); ++i) {
      REQUIRE(program[i].op_code == 0);
      }
    }
    THEN("The program ends with SharedIO, Nand, SharedIO, Reproduce"){
      REQUIRE(program[96].op_code == Library::GetOpCode("SharedIO"));
      REQUIRE(program[97].op_code == Library::GetOpCode("Nand"));
      REQUIRE(program[98].op_code == Library::GetOpCode("SharedIO"));
      REQUIRE(program[99].op_code == Library::GetOpCode("Reproduce"));
    }
  }

  WHEN("A NAND program is built"){
    builder.AddNand();
    sgpl::Program<Spec> program = builder.Build(program_len);

    THEN("The program is of length 100"){
    REQUIRE(program.size() == program_len);
    }

    THEN("The program starts with a global anchor"){
      REQUIRE(program[0].op_code == Library::GetOpCode("Global Anchor"));
    }

    THEN("The program contains 92 nop instructions in a row"){
    for (size_t i = 1; i < (program.size() - 5); ++i) {
      REQUIRE(program[i].op_code == 0);
      }
    }
    THEN("The program ends with SharedIO, SharedIO, Nand, SharedIO, Reproduce"){
      REQUIRE(program[95].op_code == Library::GetOpCode("SharedIO"));
      REQUIRE(program[96].op_code == Library::GetOpCode("SharedIO"));
      REQUIRE(program[97].op_code == Library::GetOpCode("Nand"));
      REQUIRE(program[98].op_code == Library::GetOpCode("SharedIO"));
      REQUIRE(program[99].op_code == Library::GetOpCode("Reproduce"));
    }
  }

  WHEN("A AND program is built"){
    builder.AddAnd();
    sgpl::Program<Spec> program = builder.Build(program_len);

    THEN("The program is of length 100"){
    REQUIRE(program.size() == program_len);
    }

    THEN("The program starts with a global anchor"){
      REQUIRE(program[0].op_code == Library::GetOpCode("Global Anchor"));
    }

    THEN("The program contains 91 nop instructions in a row"){
    for (size_t i = 1; i < (program.size() - 6); ++i) {
      REQUIRE(program[i].op_code == 0);
      }
    }
    THEN("The program ends with SharedIO, SharedIO, Nand, Nand, SharedIO, Reproduce"){
      REQUIRE(program[94].op_code == Library::GetOpCode("SharedIO"));
      REQUIRE(program[95].op_code == Library::GetOpCode("SharedIO"));
      REQUIRE(program[96].op_code == Library::GetOpCode("Nand"));
      REQUIRE(program[97].op_code == Library::GetOpCode("Nand"));
      REQUIRE(program[98].op_code == Library::GetOpCode("SharedIO"));
      REQUIRE(program[99].op_code == Library::GetOpCode("Reproduce"));
    }
  }

  WHEN("A ORN program is built"){
    builder.AddOrn();
    sgpl::Program<Spec> program = builder.Build(program_len);

    THEN("The program is of length 100"){
    REQUIRE(program.size() == program_len);
    }

    THEN("The program starts with a global anchor"){
      REQUIRE(program[0].op_code == Library::GetOpCode("Global Anchor"));
    }

    THEN("The program contains 91 nop instructions in a row"){
    for (size_t i = 1; i < (program.size() - 6); ++i) {
      REQUIRE(program[i].op_code == 0);
      }
    }
    THEN("The program ends with SharedIO, SharedIO, Nand, Nand, SharedIO, Reproduce"){
      REQUIRE(program[94].op_code == Library::GetOpCode("SharedIO"));
      REQUIRE(program[95].op_code == Library::GetOpCode("SharedIO"));
      REQUIRE(program[96].op_code == Library::GetOpCode("Nand"));
      REQUIRE(program[97].op_code == Library::GetOpCode("Nand"));
      REQUIRE(program[98].op_code == Library::GetOpCode("SharedIO"));
      REQUIRE(program[99].op_code == Library::GetOpCode("Reproduce"));
    }
  }
  WHEN("A OR program is built"){
    builder.AddOr();
    sgpl::Program<Spec> program = builder.Build(program_len);

    THEN("The program is of length 100"){
    REQUIRE(program.size() == program_len);
    }

    THEN("The program starts with a global anchor"){
      REQUIRE(program[0].op_code == Library::GetOpCode("Global Anchor"));
    }

    THEN("The program contains 90 nop instructions in a row"){
    for (size_t i = 1; i < (program.size() - 7); ++i) {
      REQUIRE(program[i].op_code == 0);
      }
    }
    THEN("The program ends with SharedIO, SharedIO, Nand, Nand, Nand, SharedIO, Reproduce"){
      REQUIRE(program[93].op_code == Library::GetOpCode("SharedIO"));
      REQUIRE(program[94].op_code == Library::GetOpCode("SharedIO"));
      REQUIRE(program[95].op_code == Library::GetOpCode("Nand"));
      REQUIRE(program[96].op_code == Library::GetOpCode("Nand"));
      REQUIRE(program[97].op_code == Library::GetOpCode("Nand"));
      REQUIRE(program[98].op_code == Library::GetOpCode("SharedIO"));
      REQUIRE(program[99].op_code == Library::GetOpCode("Reproduce"));
    }
  }

  WHEN("A ANDN program is built"){
    builder.AddAndn();
    sgpl::Program<Spec> program = builder.Build(program_len);

    THEN("The program is of length 100"){
    REQUIRE(program.size() == program_len);
    }

    THEN("The program starts with a global anchor"){
      REQUIRE(program[0].op_code == Library::GetOpCode("Global Anchor"));
    }

    THEN("The program contains 90 nop instructions in a row"){
    for (size_t i = 1; i < (program.size() - 7); ++i) {
      REQUIRE(program[i].op_code == 0);
      }
    }
    THEN("The program ends with SharedIO, SharedIO, Nand, Nand, Nand, SharedIO, Reproduce"){
      REQUIRE(program[93].op_code == Library::GetOpCode("SharedIO"));
      REQUIRE(program[94].op_code == Library::GetOpCode("SharedIO"));
      REQUIRE(program[95].op_code == Library::GetOpCode("Nand"));
      REQUIRE(program[96].op_code == Library::GetOpCode("Nand"));
      REQUIRE(program[97].op_code == Library::GetOpCode("Nand"));
      REQUIRE(program[98].op_code == Library::GetOpCode("SharedIO"));
      REQUIRE(program[99].op_code == Library::GetOpCode("Reproduce"));
    }
  }

  WHEN("A NOR program is built"){
    builder.AddNor();
    sgpl::Program<Spec> program = builder.Build(program_len);

    THEN("The program is of length 100"){
    REQUIRE(program.size() == program_len);
    }

    THEN("The program starts with a global anchor"){
      REQUIRE(program[0].op_code == Library::GetOpCode("Global Anchor"));
    }

    THEN("The program contains 89 nop instructions in a row"){
    for (size_t i = 1; i < (program.size() - 8); ++i) {
      REQUIRE(program[i].op_code == 0);
      }
    }
    THEN("The program ends with SharedIO, SharedIO, Nand, Nand, Nand, Nand, SharedIO, Reproduce"){
      REQUIRE(program[92].op_code == Library::GetOpCode("SharedIO"));
      REQUIRE(program[93].op_code == Library::GetOpCode("SharedIO"));
      REQUIRE(program[94].op_code == Library::GetOpCode("Nand"));
      REQUIRE(program[95].op_code == Library::GetOpCode("Nand"));
      REQUIRE(program[96].op_code == Library::GetOpCode("Nand"));
      REQUIRE(program[97].op_code == Library::GetOpCode("Nand"));
      REQUIRE(program[98].op_code == Library::GetOpCode("SharedIO"));
      REQUIRE(program[99].op_code == Library::GetOpCode("Reproduce"));
    }
  }

  WHEN("A XOR program is built"){
    builder.AddXor();
    sgpl::Program<Spec> program = builder.Build(program_len);

    THEN("The program is of length 100"){
    REQUIRE(program.size() == program_len);
    }

    THEN("The program starts with a global anchor"){
      REQUIRE(program[0].op_code == Library::GetOpCode("Global Anchor"));
    }

    THEN("The program contains 88 nop instructions in a row"){
    for (size_t i = 1; i < (program.size() - 9); ++i) {
      REQUIRE(program[i].op_code == 0);
      }
    }
    THEN("The program ends with SharedIO, SharedIO, Nand, Nand, Nand, Nand, Nand, SharedIO, Reproduce"){
      REQUIRE(program[91].op_code == Library::GetOpCode("SharedIO"));
      REQUIRE(program[92].op_code == Library::GetOpCode("SharedIO"));
      REQUIRE(program[93].op_code == Library::GetOpCode("Nand"));
      REQUIRE(program[94].op_code == Library::GetOpCode("Nand"));
      REQUIRE(program[95].op_code == Library::GetOpCode("Nand"));
      REQUIRE(program[96].op_code == Library::GetOpCode("Nand"));
      REQUIRE(program[97].op_code == Library::GetOpCode("Nand"));
      REQUIRE(program[98].op_code == Library::GetOpCode("SharedIO"));
      REQUIRE(program[99].op_code == Library::GetOpCode("Reproduce"));
    }
  }

  WHEN("A EQU program is built"){
    builder.AddEqu();
    sgpl::Program<Spec> program = builder.Build(program_len);

    THEN("The program is of length 100"){
    REQUIRE(program.size() == program_len);
    }

    THEN("The program starts with a global anchor"){
      REQUIRE(program[0].op_code == Library::GetOpCode("Global Anchor"));
    }

    THEN("The program contains 87 nop instructions in a row"){
    for (size_t i = 1; i < (program.size() - 10); ++i) {
      REQUIRE(program[i].op_code == 0);
      }
    }
    THEN("The program ends with SharedIO, SharedIO, Nand, Nand, Nand, Nand, Nand, Nand, SharedIO, Reproduce"){
      REQUIRE(program[90].op_code == Library::GetOpCode("SharedIO"));
      REQUIRE(program[91].op_code == Library::GetOpCode("SharedIO"));
      REQUIRE(program[92].op_code == Library::GetOpCode("Nand"));
      REQUIRE(program[93].op_code == Library::GetOpCode("Nand"));
      REQUIRE(program[94].op_code == Library::GetOpCode("Nand"));
      REQUIRE(program[95].op_code == Library::GetOpCode("Nand"));
      REQUIRE(program[96].op_code == Library::GetOpCode("Nand"));
      REQUIRE(program[97].op_code == Library::GetOpCode("Nand"));
      REQUIRE(program[98].op_code == Library::GetOpCode("SharedIO"));
      REQUIRE(program[99].op_code == Library::GetOpCode("Reproduce"));
    }
  }
}

TEST_CASE("CreateStartProgram()", "[sgp][sgp-unit]"){

  SymConfigSGP config;
  WHEN("Donation steal inst is off"){
    config.DONATION_STEAL_INST(0);
    sgpl::Program<Spec> program = CreateStartProgram(&config);

    THEN("The program ends with SharedIO, SharedIO, Nand, SharedIO, Reproduce"){
        REQUIRE(program[95].op_code == Library::GetOpCode("SharedIO"));
        REQUIRE(program[96].op_code == Library::GetOpCode("SharedIO"));
        REQUIRE(program[97].op_code == Library::GetOpCode("Nand"));
        REQUIRE(program[98].op_code == Library::GetOpCode("SharedIO"));
        REQUIRE(program[99].op_code == Library::GetOpCode("Reproduce"));
      }
  }
  WHEN("Donation steal inst is on"){
    config.DONATION_STEAL_INST(1);
    WHEN("Symbiont type is Mutualist"){
      config.SYMBIONT_TYPE(MUTUALIST);
      sgpl::Program<Spec> program = CreateStartProgram(&config);
      size_t donate_count = 0;
      for (size_t i = 1; i < program.size(); ++i) {
        if(program[i].op_code == Library::GetOpCode("Donate")){
          donate_count += 1;
        }
      }
      REQUIRE(donate_count == 94);

      THEN("The program ends with SharedIO, SharedIO, Nand, SharedIO, Reproduce"){
        REQUIRE(program[95].op_code == Library::GetOpCode("SharedIO"));
        REQUIRE(program[96].op_code == Library::GetOpCode("SharedIO"));
        REQUIRE(program[97].op_code == Library::GetOpCode("Nand"));
        REQUIRE(program[98].op_code == Library::GetOpCode("SharedIO"));
        REQUIRE(program[99].op_code == Library::GetOpCode("Reproduce"));
      }
      
    }
    WHEN("Symbiont type is Parasite"){
      config.SYMBIONT_TYPE(PARASITE);
      sgpl::Program<Spec> program = CreateStartProgram(&config);
      size_t steal_count = 0;
      for (size_t i = 1; i < program.size(); ++i) {
        if(program[i].op_code == Library::GetOpCode("Steal")){
          steal_count += 1;
        }
      }
      REQUIRE(steal_count == 94);
      
      THEN("The program ends with SharedIO, SharedIO, Nand, SharedIO, Reproduce"){
        REQUIRE(program[95].op_code == Library::GetOpCode("SharedIO"));
        REQUIRE(program[96].op_code == Library::GetOpCode("SharedIO"));
        REQUIRE(program[97].op_code == Library::GetOpCode("Nand"));
        REQUIRE(program[98].op_code == Library::GetOpCode("SharedIO"));
        REQUIRE(program[99].op_code == Library::GetOpCode("Reproduce"));
      }
    }
    WHEN("Symbiont type is Neutral"){
      config.SYMBIONT_TYPE(2);
      sgpl::Program<Spec> program = CreateStartProgram(&config);

      THEN("The program ends with SharedIO, SharedIO, Nand, SharedIO, Reproduce"){
        REQUIRE(program[95].op_code == Library::GetOpCode("SharedIO"));
        REQUIRE(program[96].op_code == Library::GetOpCode("SharedIO"));
        REQUIRE(program[97].op_code == Library::GetOpCode("Nand"));
        REQUIRE(program[98].op_code == Library::GetOpCode("SharedIO"));
        REQUIRE(program[99].op_code == Library::GetOpCode("Reproduce"));
      }
    }
  }
}