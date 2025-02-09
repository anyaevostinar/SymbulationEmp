#pragma once

#include "hardware/SGPHardwareSpec.h"
#include "hardware/GenomeLibrary.h"

#include "emp/base/vector.hpp"

#include "sgpl/program/Instruction.hpp"
#include "sgpl/program/Program.hpp"
#include "sgpl/library/OpLibrary.h"


namespace sgpmode {

template<typename HW_SPEC_T>
class ProgramBuilder {
public:
  using hw_spec_t = HW_SPEC_T;
  using program_t = sgpl::Program<hw_spec_t>;
  using inst_t = sgpl::Instruction<hw_spec_t>;

protected:
  std::string nand_op_name = inst::Nand::name();
  // std::string

public:
  // TODO - finish drafting


};

template <typename HW_SPEC_T>
void AddInst(
  sgpl::Program<HW_SPEC_T>& program,
  const std::string& op_name,
  uint8_t arg0 = 0,
  uint8_t arg1 = 0,
  uint8_t arg2 = 0
) {
  sgpl::Instruction<HW_SPEC_T> inst;
  inst.op_code = Library::GetOpCode(op_name);
  inst.args = {arg0, arg1, arg2};
  program.emplace_back(inst);
}

template <typename HW_SPEC_T>
void AddTask_Not(
  sgpl::Program<HW_SPEC_T>& program,
  const std::string& op_name,
  uint8_t arg0 = 0,
  uint8_t arg1 = 0,
  uint8_t arg2 = 0
) {
  sgpl::Instruction<HW_SPEC_T> inst;
  inst.op_code = Library::GetOpCode(op_name);
  inst.args = {arg0, arg1, arg2};
  program.emplace_back(inst);
}

// TODO - re-implement program builder

}