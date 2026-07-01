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
  using tag_t = typename hw_spec_t::tag_t;
  using rectifier_t = sgpl::OpCodeRectifier<Library>;

protected:
  // Can add extra flexibility in future to configure
  // what instructions to use (e.g., different IO instructions, etc)
  uint8_t nand_op = Library::GetOpCode(inst::Nand::name());
  uint8_t io_op = Library::GetOpCode(inst::IO::name());
  uint8_t repro_op = Library::GetOpCode(inst::Reproduce::name());

  tag_t start_tag;

  rectifier_t& rectifier;

public:
  ProgramBuilder(
    rectifier_t& opcode_rectifier
  ) : rectifier(opcode_rectifier)
  { }

  // TODO - finish drafting

  // TODO - add functions for switching "instruction modes"
  void SetStartTag(const tag_t& tag) {
    start_tag = tag;
  }

  const tag_t& GetStartTag() const {
    return start_tag;
  }

  // Allow io opcode to be configured in case we need to change to a special/different
  //  io instruction.
  void SetIOInst(uint8_t opcode) {
    emp_assert(opcode < Library::GetSize());
    io_op = opcode;
  }

  // Allow reproduce opcode to be configured in case we need to change to a special/different
  //  reproduce instruction.
  void SetReproduceInst(uint8_t opcode) {
    emp_assert(opcode < Library::GetSize());
    repro_op = opcode;
  }

  // Allow nand opcode to be configured in case we need to change to a special/different
  //  nand instruction.
  void SetNandInst(uint8_t opcode) {
    emp_assert(opcode < Library::GetSize());
    nand_op = opcode;
  }

  void AddInst(
    program_t& program,
    const std::string& op_name,
    uint8_t arg0 = 0,
    uint8_t arg1 = 0,
    uint8_t arg2 = 0,
    tag_t tag={}
  ) {
    AddInst(
      program,
      Library::GetOpCode(op_name),
      arg0,
      arg1,
      arg2,
      tag
    );
  }

  void AddInst(
    program_t& program,
    uint8_t opcode,
    uint8_t arg0 = 0,
    uint8_t arg1 = 0,
    uint8_t arg2 = 0,
    tag_t tag={}
  ) {
    emp_assert(opcode < rectifier.mapper.size());
    inst_t inst;
    // NOTE - Should we add instruction if rectifier is going to remap? Could either
    //        bail out on add or add re-mapped instruction.
    // Rectify opcode to disallow disabled instructions
    inst.op_code = rectifier.mapper[opcode];
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

  void AddInst(
    program_t& program,
    uint8_t opcode,
    tag_t tag
  ) {
    AddInst(program, opcode, 0, 0, 0, tag);
  }

  void AddStartAnchor(program_t& program) {
    AddInst(
      program,
      "Global Anchor",
      start_tag
    );
  }

  void AddTask_Not(program_t& program) {
    // nand r0, r0, r0
    AddInst(program, nand_op);
  }

  void AddTask_NotIO(program_t& program) {
    AddInst(program, io_op);
    AddTask_Not(program);
    AddInst(program, io_op);
  }

  void AddTask_Nand(program_t& program) {
    // nand r0, r1, r0
    AddInst(program, nand_op, 0, 1, 0);
  }

  void AddTask_NandIO(program_t& program) {
    AddInst(program, io_op);
    AddInst(program, io_op, 1);
    AddTask_Nand(program);
    AddInst(program, io_op);
  }

  void AddTask_And(
    program_t& program
  ) {
    // ~(a nand b)
    // nand r0, r1, r0
    // nand r0, r0, r0
    AddInst(program, nand_op, 0, 1, 0);
    AddInst(program, nand_op);
  }

  void AddTask_AndIO(
    program_t& program
  ) {
    AddInst(program, io_op);
    AddInst(program, io_op, 1);
    AddTask_And(program);
    AddInst(program, io_op);
  }

  void AddTask_OrNot(program_t& program) {
    // (~a) nand b
    // nand r0, r0, r0
    // nand r0, r1, r0

    AddInst(program, nand_op);
    AddInst(program, nand_op, 0, 1, 0);

  }

  void AddTask_OrNotIO(program_t& program) {
    AddInst(program, io_op);
    AddInst(program, io_op, 1);
    AddTask_OrNot(program);
    AddInst(program, io_op);
  }

  void AddTask_Or(program_t& program) {
    // (~a) nand (~b)
    // nand r0, r0, r0
    // nand r1, r1, r1
    // nand r0, r1, r0
    AddInst(program, nand_op, 0, 0, 0);
    AddInst(program, nand_op, 1, 1, 1);
    AddInst(program, nand_op, 0, 1, 0);
  }

  void AddTask_OrIO(program_t& program) {
    AddInst(program, io_op);
    AddInst(program, io_op, 1);
    AddTask_Or(program);
    AddInst(program, io_op);
  }

  void AddTask_AndNot(program_t& program) {
    // ~(a nand (~b))
    // nand r1, r1, r1
    // nand r0, r1, r0
    // nand r0, r0, r0
    AddInst(program, nand_op, 1, 1, 1);
    AddInst(program, nand_op, 0, 1, 0);
    AddInst(program, nand_op, 0, 0, 0);
  }

  void AddTask_AndNotIO(program_t& program) {
    AddInst(program, io_op);
    AddInst(program, io_op, 1);
    AddTask_AndNot(program);
    AddInst(program, io_op);
  }

  void AddTask_Nor(program_t& program) {
    // ~((~a) nand (~b))
    // nand r0, r0, r0
    // nand r1, r1, r1
    // nand r0, r1, r0
    // nand r0, r0, r0
    AddInst(program, nand_op, 0, 0, 0);
    AddInst(program, nand_op, 1, 1, 1);
    AddInst(program, nand_op, 0, 1, 0);
    AddInst(program, nand_op, 0, 0, 0);
  }

  void AddTask_NorIO(program_t& program) {
    AddInst(program, io_op);
    AddInst(program, io_op, 1);
    AddTask_Nor(program);
    AddInst(program, io_op);
  }

  void AddTask_Xor(program_t& program) {
    // (a & ~b) | (~a & b) --> (a nand ~b) nand (~a nand b)
    // nand r3, r1, r1
    // nand r3, r3, r0
    //
    // nand r2, r0, r0
    // nand r2, r2, r1
    //
    // nand r0, r2, r3

    AddInst(program, nand_op, 3, 1, 1);
    AddInst(program, nand_op, 3, 3, 0);
    AddInst(program, nand_op, 2, 0, 0);
    AddInst(program, nand_op, 2, 2, 1);
    AddInst(program, nand_op, 0, 2, 3);

  }

  void AddTask_XorIO(program_t& program) {
    AddInst(program, io_op);
    AddInst(program, io_op, 1);
    AddTask_Xor(program);
    AddInst(program, io_op);
  }

  void AddTask_Equ(program_t& program) {
    // ~(a ^ b)
    //
    // nand r3, r1, r1
    // nand r3, r3, r0
    //
    // nand r2, r0, r0
    // nand r2, r2, r1
    //
    // nand r0, r2, r3
    // nand r0, r0, r0
    AddInst(program, nand_op, 3, 1, 1);
    AddInst(program, nand_op, 3, 3, 0);

    AddInst(program, nand_op, 2, 0, 0);
    AddInst(program, nand_op, 2, 2, 1);

    AddInst(program, nand_op, 0, 2, 3);
    AddInst(program, nand_op, 0, 0, 0);
  }

  void AddTask_EquIO(program_t& program) {
    AddInst(program, io_op);
    AddInst(program, io_op, 1);
    AddTask_Equ(program);
    AddInst(program, io_op);
  }

  program_t CreateNotProgram(size_t length) {
    program_t program; // Create empty program
    // Add start anchor
    AddStartAnchor(program);
    // Add task and IO manually so that repeated nots play nice with task crediting
    //   Add a not implementation that properly cycles the input buffer
    //   so that when it executes this multiple times, it performs a not with
    //   each value in the input buffer.
    AddInst(program, io_op);
    AddTask_Not(program);
    // Nop filler is length minus current size + repro instructions
    // const size_t nop_filler = length - (program.size() + 1);
    program.resize(length - 1);
    AddInst(program, repro_op);
    // Remove any deleted instructions
    program.Rectify(rectifier);
    return program;
  }

  program_t CreateReproProgram(size_t length) {
    program_t program;
    // Add start anchor
    AddInst(
      program,
      "Global Anchor",
      start_tag
    );
    // Add Nop filler
    program.resize(length - 1);
    AddInst(program, repro_op);
    // Remove any deleted instructions
    program.Rectify(rectifier);
    return program;
  }

  program_t CreateNotNandProgram(size_t length) {
    program_t program; // Create empty program
    // Add start anchor
    AddInst(
      program,
      "Global Anchor",
      start_tag
    );
    // Add not instruction
    AddInst(program, io_op); 
    AddTask_Not(program);  // Add not task
    AddInst(program, io_op); 
    AddInst(program, io_op, 1);
    AddTask_Nand(program); // Add nand task, IO will happen at start of next time through genome
    // Nop filler is length minus current size + repro instructions
    // const size_t nop_filler = length - (program.size() + 1);
    program.resize(length - 1);
    AddInst(program, repro_op);
    // Remove any deleted instructions
    program.Rectify(rectifier);
    return program;
  }

  program_t CreateNandProgram(size_t length) {
    program_t program; // Create empty program
    // Add start anchor
    AddInst(
      program,
      "Global Anchor",
      start_tag
    );
    // Add nand instruction
    AddInst(program, io_op);
    AddInst(program, io_op, 1);
    AddInst(program, io_op);
    AddTask_Nand(program);
    // Nop filler is length minus current size + repro instructions
    // const size_t nop_filler = length - (program.size() + 1);
    program.resize(length - 1);
    AddInst(program, repro_op);
    // Remove any deleted instructions
    program.Rectify(rectifier);
    return program;
  }

};

}