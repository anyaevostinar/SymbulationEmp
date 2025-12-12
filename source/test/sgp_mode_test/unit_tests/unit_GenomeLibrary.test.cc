#include "../../../sgp_mode/GenomeLibrary.h"
#include "../../../sgp_mode/CPU.h"

#include "../../../catch/catch.hpp"

TEST_CASE("BuildNoRepro creates obligate mutualist program", "[sgp]") {
  ProgramBuilder builder;
  size_t program_len = 100;

  sgpl::Program<Spec> program = builder.BuildNoRepro(program_len);

  REQUIRE(program.size() == program_len);

  for (size_t i = program.size() - 5; i < program.size(); ++i) {
    REQUIRE(program[i].op_code == Library::GetOpCode("Donate"));
  }

  for (auto &inst : program) {
    REQUIRE(inst.op_code != Library::GetOpCode("Reproduce"));
  }

  REQUIRE(program[0].op_code == Library::GetOpCode("Global Anchor"));
  REQUIRE(program[0].tag == START_TAG);

}