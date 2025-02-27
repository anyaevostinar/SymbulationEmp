#pragma once

#include "hardware/SGPHardwareSpec.h"
#include "hardware/GenomeLibrary.h"

#include "emp/base/vector.hpp"

#include "sgpl/program/Instruction.hpp"
#include "sgpl/program/Program.hpp"
#include "sgpl/library/OpLibrary.hpp"


namespace sgpmode {

template<typename HW_SPEC_T>
class ProgramBuilder {
public:
  using hw_spec_t = HW_SPEC_T;
  using program_t = sgpl::Program<hw_spec_t>;
  using inst_t = sgpl::Instruction<hw_spec_t>;
  using tag_t = typename hw_spect_t::tag_t;

protected:
  // Can add extra flexibility in future to configure
  // what instructions to use (e.g., different IO instructions, etc)
  std::string nand_op_name = inst::Nand::name();
  std::string io_op_name = "SharedIO";//inst::SharedIO::name();
  // std::string

public:
  // TODO - finish drafting

  // TODO - add functions for switching "instruction modes"

  void AddInst(
    program_t& program,
    const std::string& op_name,
    uint8_t arg0 = 0,
    uint8_t arg1 = 0,
    uint8_t arg2 = 0,
    tag_t tag={}
  ) {
    inst_t inst;
    inst.op_code = Library::GetOpCode(op_name);
    inst.args = {arg0, arg1, arg2};
    inst.tag = tag;
    program.emplace_back(inst);
  }

  void AddInst(
    program_t& program,
    const std::string& op_name,
    tag_t tag
  ) {
    AddInst(program, op_name, 0, 0, 0, tag);
  }

  void AddTask_Not(
    program_t& program
  ) {
    // sharedio   r0
    // nand       r0, r0, r0
    // sharedio   r0
    AddInst(program, io_op_name);
    AddInst(program, nand_op_name);
    AddInst(program, io_op_name);
  }

  program_t CreateNotProgram(size_t length) {
    program_t program; // Create empty program
    // Add start anchor
    AddInst(
      program,
      "Global Anchor",
      hw_spec_t::START_TAG
    );
    // Add not instruction
    AddTask_Not(program); // Add not task
    // Nop filler is length minus current size + repro instructions
    // const size_t nop_filler = length - (program.size() + 1);
    program.resize(length - 1);
    AddInst("Reproduce")
  }

};


// TODO - re-implement program builder

}