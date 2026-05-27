#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorld.cc"
#include "../../../sgp_mode/SGPWorldSetup.cc"
#include "../../../sgp_mode/SGPWorldData.cc"
#include "../../../sgp_mode/ProgramBuilder.h"

#include "../../../catch/catch.hpp"

/**
 * This file is dedicated to unit tests for GenomeLibrary
 */

using world_t = sgpmode::SGPWorld;
using cpu_state_t = sgpmode::CPUState<world_t>;
using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
using program_t = typename world_t::sgp_prog_t;
using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;

program_t Build(program_t& old_program, size_t length, sgpmode::ProgramBuilder<hw_spec_t>& builder) {
  builder.AddInst(old_program, "Reproduce");

  program_t program;
  // Set everything to 0 - this makes them no-ops since that's the first
  // inst in the library
  program.resize(length - old_program.size());
  program[0].op_code = sgpmode::Library::GetOpCode("Global Anchor");
  program[0].tag = builder.GetStartTag();

  program.insert(program.end(), old_program.begin(), old_program.end());

  return program;
}

TEST_CASE("All task specific programs are built correctly","[sgp][sgp-unit]"){
  GIVEN("A program builder"){
    size_t program_len = 100;
    sgpl::OpCodeRectifier<sgpmode::Library> rectifier;
    sgpmode::ProgramBuilder<hw_spec_t> builder(rectifier);
    WHEN("A NOT program is built"){
      program_t program;
      builder.AddStartAnchor(program);
      builder.AddTask_NotIO(program);

      program.resize(program_len - 1);
      builder.AddInst(program, "Reproduce");
      
      program.Rectify(rectifier);

      THEN("The program is of length 100"){
        REQUIRE(program.size() == program_len);
      }

      THEN("The program starts with a global anchor"){
        REQUIRE(program[0].op_code == sgpmode::Library::GetOpCode("Global Anchor"));
      }
      THEN("The program contains IO, Nand, IO"){
        REQUIRE(program[1].op_code == sgpmode::Library::GetOpCode("IO"));
        REQUIRE(program[2].op_code == sgpmode::Library::GetOpCode("Nand"));
        REQUIRE(program[3].op_code == sgpmode::Library::GetOpCode("IO"));
      }
      THEN("The program contains 93 nop instructions in a row"){
        for (size_t i = 4; i < (program.size() - 1); ++i) {
          REQUIRE(program[i].op_code == 0);
        }
      }
      THEN("The program ends with Reproduce"){
        REQUIRE(program[99].op_code == sgpmode::Library::GetOpCode("Reproduce"));
      }
    }

    WHEN("A NAND program is built"){
      program_t program;
      builder.AddStartAnchor(program);
      builder.AddTask_NandIO(program);

      program.resize(program_len - 1);
      builder.AddInst(program, "Reproduce");
      
      program.Rectify(rectifier);

      THEN("The program is of length 100"){
        REQUIRE(program.size() == program_len);
      }

      THEN("The program starts with a global anchor"){
        REQUIRE(program[0].op_code == sgpmode::Library::GetOpCode("Global Anchor"));
      }

      THEN("The program contains IO, IO, Nand, IO"){
        REQUIRE(program[1].op_code == sgpmode::Library::GetOpCode("IO"));
        REQUIRE(program[2].op_code == sgpmode::Library::GetOpCode("IO"));
        REQUIRE(program[3].op_code == sgpmode::Library::GetOpCode("Nand"));
        REQUIRE(program[4].op_code == sgpmode::Library::GetOpCode("IO"));
      }

      THEN("The program contains 92 nop instructions in a row"){
        for (size_t i = 5; i < (program.size() - 1); ++i) {
          REQUIRE(program[i].op_code == 0);
        }
      }
      THEN("The program ends with Reproduce"){
        REQUIRE(program[99].op_code == sgpmode::Library::GetOpCode("Reproduce"));
      }
    }

    WHEN("A AND program is built"){
      program_t program;
      builder.AddStartAnchor(program);
      builder.AddTask_AndIO(program);

      program.resize(program_len - 1);
      builder.AddInst(program, "Reproduce");
      
      program.Rectify(rectifier);

      THEN("The program is of length 100"){
       REQUIRE(program.size() == program_len);
      }

      THEN("The program starts with a global anchor"){
        REQUIRE(program[0].op_code == sgpmode::Library::GetOpCode("Global Anchor"));
      }

      THEN("The program contains IO, IO, Nand, Nand, IO"){
        REQUIRE(program[1].op_code == sgpmode::Library::GetOpCode("IO"));
        REQUIRE(program[2].op_code == sgpmode::Library::GetOpCode("IO"));
        REQUIRE(program[3].op_code == sgpmode::Library::GetOpCode("Nand"));
        REQUIRE(program[4].op_code == sgpmode::Library::GetOpCode("Nand"));
        REQUIRE(program[5].op_code == sgpmode::Library::GetOpCode("IO"));
      }

      THEN("The program contains 91 nop instructions in a row"){
        for (size_t i = 6; i < (program.size() - 1); ++i) {
          REQUIRE(program[i].op_code == 0);
        }
      }
      THEN("The program ends with Reproduce"){
        REQUIRE(program[99].op_code == sgpmode::Library::GetOpCode("Reproduce"));
      }
    }

    WHEN("A ORN program is built"){
      program_t program;
      builder.AddStartAnchor(program);
      builder.AddTask_OrNotIO(program);

      program.resize(program_len - 1);
      builder.AddInst(program, "Reproduce");
      
      program.Rectify(rectifier);

      THEN("The program is of length 100"){
        REQUIRE(program.size() == program_len);
      }

      THEN("The program starts with a global anchor"){
        REQUIRE(program[0].op_code == sgpmode::Library::GetOpCode("Global Anchor"));
      }

      THEN("The program contains IO, IO, Nand, Nand, IO"){
        REQUIRE(program[1].op_code == sgpmode::Library::GetOpCode("IO"));
        REQUIRE(program[2].op_code == sgpmode::Library::GetOpCode("IO"));
        REQUIRE(program[3].op_code == sgpmode::Library::GetOpCode("Nand"));
        REQUIRE(program[4].op_code == sgpmode::Library::GetOpCode("Nand"));
        REQUIRE(program[5].op_code == sgpmode::Library::GetOpCode("IO"));
      }

      THEN("The program contains 91 nop instructions in a row"){
        for (size_t i = 6; i < (program.size() - 1); ++i) {
          REQUIRE(program[i].op_code == 0);
        }
      }
      THEN("The program ends with Reproduce"){
        REQUIRE(program[99].op_code == sgpmode::Library::GetOpCode("Reproduce"));
      }
    }
    
    WHEN("A OR program is built"){
      program_t program;
      builder.AddStartAnchor(program);
      builder.AddTask_OrIO(program);

      program.resize(program_len - 1);
      builder.AddInst(program, "Reproduce");
      
      program.Rectify(rectifier);

      THEN("The program is of length 100"){
        REQUIRE(program.size() == program_len);
      }

      THEN("The program starts with a global anchor"){
        REQUIRE(program[0].op_code == sgpmode::Library::GetOpCode("Global Anchor"));
      }

      THEN("The program contains IO, IO, Nand, Nand, Nand, IO"){
        REQUIRE(program[1].op_code == sgpmode::Library::GetOpCode("IO"));
        REQUIRE(program[2].op_code == sgpmode::Library::GetOpCode("IO"));
        REQUIRE(program[3].op_code == sgpmode::Library::GetOpCode("Nand"));
        REQUIRE(program[4].op_code == sgpmode::Library::GetOpCode("Nand"));
        REQUIRE(program[5].op_code == sgpmode::Library::GetOpCode("Nand"));
        REQUIRE(program[6].op_code == sgpmode::Library::GetOpCode("IO"));
      }

      THEN("The program contains 90 nop instructions in a row"){
        for (size_t i = 7; i < (program.size() - 1); ++i) {
          REQUIRE(program[i].op_code == 0);
        }
      }
      THEN("The program ends with Reproduce"){
        REQUIRE(program[99].op_code == sgpmode::Library::GetOpCode("Reproduce"));
      }
    }

    WHEN("A ANDN program is built"){
      program_t program;
      builder.AddStartAnchor(program);
      builder.AddTask_AndNotIO(program);

      program.resize(program_len - 1);
      builder.AddInst(program, "Reproduce");
      
      program.Rectify(rectifier);

      THEN("The program is of length 100"){
        REQUIRE(program.size() == program_len);
      }

      THEN("The program starts with a global anchor"){
        REQUIRE(program[0].op_code == sgpmode::Library::GetOpCode("Global Anchor"));
      }

      THEN("The program contains IO, IO, Nand, Nand, Nand, IO"){
        REQUIRE(program[1].op_code == sgpmode::Library::GetOpCode("IO"));
        REQUIRE(program[2].op_code == sgpmode::Library::GetOpCode("IO"));
        REQUIRE(program[3].op_code == sgpmode::Library::GetOpCode("Nand"));
        REQUIRE(program[4].op_code == sgpmode::Library::GetOpCode("Nand"));
        REQUIRE(program[5].op_code == sgpmode::Library::GetOpCode("Nand"));
        REQUIRE(program[6].op_code == sgpmode::Library::GetOpCode("IO"));
      }

      THEN("The program contains 90 nop instructions in a row"){
        for (size_t i = 7; i < (program.size() - 1); ++i) {
          REQUIRE(program[i].op_code == 0);
        }
      }
      THEN("The program ends with Reproduce"){
        REQUIRE(program[99].op_code == sgpmode::Library::GetOpCode("Reproduce"));
      }
    }

    WHEN("A NOR program is built"){
      program_t program;
      builder.AddStartAnchor(program);
      builder.AddTask_NorIO(program);

      program.resize(program_len - 1);
      builder.AddInst(program, "Reproduce");
      
      program.Rectify(rectifier);

      THEN("The program is of length 100"){
        REQUIRE(program.size() == program_len);
      }

      THEN("The program starts with a global anchor"){
        REQUIRE(program[0].op_code == sgpmode::Library::GetOpCode("Global Anchor"));
      }

      THEN("The program contains IO, IO, Nand, Nand, Nand, Nand, IO"){
        REQUIRE(program[1].op_code == sgpmode::Library::GetOpCode("IO"));
        REQUIRE(program[2].op_code == sgpmode::Library::GetOpCode("IO"));
        REQUIRE(program[3].op_code == sgpmode::Library::GetOpCode("Nand"));
        REQUIRE(program[4].op_code == sgpmode::Library::GetOpCode("Nand"));
        REQUIRE(program[5].op_code == sgpmode::Library::GetOpCode("Nand"));
        REQUIRE(program[6].op_code == sgpmode::Library::GetOpCode("Nand"));
        REQUIRE(program[7].op_code == sgpmode::Library::GetOpCode("IO"));
      }

      THEN("The program contains 89 nop instructions in a row"){
        for (size_t i = 8; i < (program.size() - 1); ++i) {
          REQUIRE(program[i].op_code == 0);
        }
      }
      THEN("The program ends with Reproduce"){
        REQUIRE(program[99].op_code == sgpmode::Library::GetOpCode("Reproduce"));
      }
    }

    WHEN("A XOR program is built"){
      program_t program;
      builder.AddStartAnchor(program);
      builder.AddTask_XorIO(program);

      program.resize(program_len - 1);
      builder.AddInst(program, "Reproduce");
      
      program.Rectify(rectifier);

      THEN("The program is of length 100"){
       REQUIRE(program.size() == program_len);
      }

      THEN("The program starts with a global anchor"){
        REQUIRE(program[0].op_code == sgpmode::Library::GetOpCode("Global Anchor"));
      }

      THEN("The program contains IO, IO, Nand, Nand, Nand, Nand, Nand, IO"){
        REQUIRE(program[1].op_code == sgpmode::Library::GetOpCode("IO"));
        REQUIRE(program[2].op_code == sgpmode::Library::GetOpCode("IO"));
        REQUIRE(program[3].op_code == sgpmode::Library::GetOpCode("Nand"));
        REQUIRE(program[4].op_code == sgpmode::Library::GetOpCode("Nand"));
        REQUIRE(program[5].op_code == sgpmode::Library::GetOpCode("Nand"));
        REQUIRE(program[6].op_code == sgpmode::Library::GetOpCode("Nand"));
        REQUIRE(program[7].op_code == sgpmode::Library::GetOpCode("Nand"));
        REQUIRE(program[8].op_code == sgpmode::Library::GetOpCode("IO"));
      }

      THEN("The program contains 88 nop instructions in a row"){
        for (size_t i = 9; i < (program.size() - 9); ++i) {
          REQUIRE(program[i].op_code == 0);
        }
      }
      THEN("The program ends with Reproduce"){
        REQUIRE(program[99].op_code == sgpmode::Library::GetOpCode("Reproduce"));
      }
    }

    WHEN("A EQU program is built"){
      program_t program;
      builder.AddStartAnchor(program);
      builder.AddTask_EquIO(program);

      program.resize(program_len - 1);
      builder.AddInst(program, "Reproduce");
      
      program.Rectify(rectifier);

      THEN("The program is of length 100"){
        REQUIRE(program.size() == program_len);
      }

      THEN("The program starts with a global anchor"){
        REQUIRE(program[0].op_code == sgpmode::Library::GetOpCode("Global Anchor"));
      }

      THEN("The program contains IO, IO, Nand, Nand, Nand, Nand, Nand, Nand, IO"){
        REQUIRE(program[1].op_code == sgpmode::Library::GetOpCode("IO"));
        REQUIRE(program[2].op_code == sgpmode::Library::GetOpCode("IO"));
        REQUIRE(program[3].op_code == sgpmode::Library::GetOpCode("Nand"));
        REQUIRE(program[4].op_code == sgpmode::Library::GetOpCode("Nand"));
        REQUIRE(program[5].op_code == sgpmode::Library::GetOpCode("Nand"));
        REQUIRE(program[6].op_code == sgpmode::Library::GetOpCode("Nand"));
        REQUIRE(program[7].op_code == sgpmode::Library::GetOpCode("Nand"));
        REQUIRE(program[8].op_code == sgpmode::Library::GetOpCode("Nand"));
        REQUIRE(program[9].op_code == sgpmode::Library::GetOpCode("IO"));
      }

      THEN("The program contains 87 nop instructions in a row"){
        for (size_t i = 10; i < (program.size() - 1); ++i) {
          REQUIRE(program[i].op_code == 0);
        }
      }
      THEN("The program ends with Reproduce"){
        REQUIRE(program[99].op_code == sgpmode::Library::GetOpCode("Reproduce"));
      }
    }
  }
}

TEST_CASE("CreateNotProgram()", "[sgp][sgp-unit]"){
  GIVEN("A program builder"){
    size_t program_len = 100;
    sgpl::OpCodeRectifier<sgpmode::Library> rectifier;
    sgpmode::ProgramBuilder<hw_spec_t> builder(rectifier);

    WHEN("CreateNotProgram is called"){
      program_t program = builder.CreateNotProgram(program_len);

      THEN("The program is of length 100"){
        REQUIRE(program.size() == program_len);
      }

      THEN("The program starts with a global anchor"){
        REQUIRE(program[0].op_code == sgpmode::Library::GetOpCode("Global Anchor"));
      }

      THEN("The program contains IO, Nand"){
        REQUIRE(program[1].op_code == sgpmode::Library::GetOpCode("IO"));
        REQUIRE(program[2].op_code == sgpmode::Library::GetOpCode("Nand"));
      }

      THEN("The program contains 95 nop instructions in a row"){
        for (size_t i = 3; i < (program.size() - 1); ++i) {
          REQUIRE(program[i].op_code == 0);
        }
      }

      THEN("The program ends with Reproduce"){
        REQUIRE(program[99].op_code == sgpmode::Library::GetOpCode("Reproduce"));
      }
    }
  }
}

TEST_CASE("CreateReproProgram()", "[sgp][sgp-unit]"){
  GIVEN("A program builder"){
    size_t program_len = 100;
    sgpl::OpCodeRectifier<sgpmode::Library> rectifier;
    sgpmode::ProgramBuilder<hw_spec_t> builder(rectifier);

    WHEN("CreateReproProgram is called"){
      program_t program = builder.CreateReproProgram(program_len);

      THEN("The program is of length 100"){
        REQUIRE(program.size() == program_len);
      }

      THEN("The program starts with a global anchor"){
        REQUIRE(program[0].op_code == sgpmode::Library::GetOpCode("Global Anchor"));
      }

      THEN("The program contains 97 nop instructions in a row"){
        for (size_t i = 1; i < (program.size() - 1); ++i) {
          REQUIRE(program[i].op_code == 0);
        }
      }

      THEN("The program ends with Reproduce"){
        REQUIRE(program[99].op_code == sgpmode::Library::GetOpCode("Reproduce"));
      }
    }
  }
}

TEST_CASE("CreateNotNandProgram()", "[sgp][sgp-unit]"){
  GIVEN("A program builder"){
    size_t program_len = 100;
    sgpl::OpCodeRectifier<sgpmode::Library> rectifier;
    sgpmode::ProgramBuilder<hw_spec_t> builder(rectifier);

    WHEN("CreateNotNandProgram is called"){
      program_t program = builder.CreateNotNandProgram(program_len);

      THEN("The program is of length 100"){
        REQUIRE(program.size() == program_len);
      }

      THEN("The program starts with a global anchor"){
        REQUIRE(program[0].op_code == sgpmode::Library::GetOpCode("Global Anchor"));
      }

      THEN("The program contains IO, Nand, IO, IO, Nand"){
        REQUIRE(program[1].op_code == sgpmode::Library::GetOpCode("IO"));
        REQUIRE(program[2].op_code == sgpmode::Library::GetOpCode("Nand"));
        REQUIRE(program[3].op_code == sgpmode::Library::GetOpCode("IO"));
        REQUIRE(program[4].op_code == sgpmode::Library::GetOpCode("IO"));
        REQUIRE(program[5].op_code == sgpmode::Library::GetOpCode("Nand"));
      }

      THEN("The program contains 92 nop instructions in a row"){
        for (size_t i = 6; i < (program.size() - 1); ++i) {
          REQUIRE(program[i].op_code == 0);
        }
      }

      THEN("The program ends with Reproduce"){
        REQUIRE(program[99].op_code == sgpmode::Library::GetOpCode("Reproduce"));
      }
    }
  }
}

TEST_CASE("CreateNandProgram()", "[sgp][sgp-unit]"){
  GIVEN("A program builder"){
    size_t program_len = 100;
    sgpl::OpCodeRectifier<sgpmode::Library> rectifier;
    sgpmode::ProgramBuilder<hw_spec_t> builder(rectifier);

    WHEN("CreateNandProgram is called"){
      program_t program = builder.CreateNandProgram(program_len);

      THEN("The program is of length 100"){
        REQUIRE(program.size() == program_len);
      }

      THEN("The program starts with a global anchor"){
        REQUIRE(program[0].op_code == sgpmode::Library::GetOpCode("Global Anchor"));
      }

      THEN("The program contains IO, IO, Nand"){
        REQUIRE(program[1].op_code == sgpmode::Library::GetOpCode("IO"));
        REQUIRE(program[2].op_code == sgpmode::Library::GetOpCode("IO"));
        REQUIRE(program[3].op_code == sgpmode::Library::GetOpCode("Nand"));
      }

      THEN("The program contains 94 nop instructions in a row"){
        for (size_t i = 4; i < (program.size() - 1); ++i) {
          REQUIRE(program[i].op_code == 0);
        }
      }

      THEN("The program ends with Reproduce"){
        REQUIRE(program[99].op_code == sgpmode::Library::GetOpCode("Reproduce"));
      }
    }
  }
}