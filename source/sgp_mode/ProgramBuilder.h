#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "hardware/SGPHardwareSpec.h"
#include "hardware/GenomeLibrary.h"

#include "cereal/archives/binary.hpp"
#include "cereal/archives/json.hpp"

#include "emp/base/assert_warning.hpp"
#include "emp/base/error.hpp"
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

  /**
   * Input: A program object to be rectified inplace.
   *
   * Output: None
   *
   * Purpose: Removes disabled instructions from a program, warning if any
   * were actually removed.
   */
  void RectifyWithWarning(program_t& program) const {
    const program_t before{program};
    program.Rectify(rectifier);
    emp_assert_warning(
      before == program,
      "ProgramBuilder program contained disabled instructions."
    );
  }

public:
  /**
   * Input: A sgp opcode rectifier object, which must outlive this builder.
   *
   * Output: None
   *
   * Purpose: Constructs a ProgramBuilder, storing the rectifier by reference.
   */
  ProgramBuilder(
    rectifier_t& opcode_rectifier
  ) : rectifier(opcode_rectifier)
  { }

  // TODO - finish drafting

  // TODO - add functions for switching "instruction modes"
  /**
   * Input: A tag.
   *
   * Output: None
   *
   * Purpose: Configures the tag to be assigned to the global anchor that marks where execution
   * begins.
   */
  void SetStartTag(const tag_t& tag) {
    start_tag = tag;
  }

  /**
   * Input: None
   *
   * Output: The tag used for the global anchor.
   *
   * Purpose: Accessor function.
   */
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

  /**
   * Input: Program to extend, instruction name, up to three register arguments, and an optional
   * tag.
   *
   * Output: None
   *
   * Purpose: Appends an instruction, looking its opcode up by name.
   */
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

  /**
   * Input: Program to extend, opcode, up to three register arguments, and an optional tag.
   *
   * Output: None
   *
   * Purpose: Appends an instruction using directly-specified opcode.
   */
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

  /**
   * Input: Program being built, instruction name, and a tag.
   *
   * Output: None
   *
   * Purpose: Appends a tagged instruction, looking its opcode up by name.
   */
  void AddInst(
    program_t& program,
    const std::string& op_name,
    tag_t tag
  ) {
    AddInst(program, op_name, 0, 0, 0, tag);
  }

  /**
   * Input: Program being built, opcode, and a tag.
   *
   * Output: None
   *
   * Purpose: Appends a tagged instruction, using directly-specified opcode.
   */
  void AddInst(
    program_t& program,
    uint8_t opcode,
    tag_t tag
  ) {
    AddInst(program, opcode, 0, 0, 0, tag);
  }

  /**
   * Input: Program being built.
   *
   * Output: None
   *
   * Purpose: Appends the global anchor instruction with start tag, that marks where execution
   * begins.
   */
  void AddStartAnchor(program_t& program) {
    AddInst(
      program,
      "Global Anchor",
      start_tag
    );
  }

  /**
   * Input: Program being built.
   *
   * Output: None
   *
   * Purpose: Extends a program to calculate NOT, without I/O.
   */
  void AddTask_Not(program_t& program) {
    // nand r0, r0, r0
    AddInst(program, nand_op);
  }

  /**
   * Input: Program being built.
   *
   * Output: None
   *
   * Purpose: Extends a program to complete the NOT task, including I/O.
   */
  void AddTask_NotIO(program_t& program) {
    AddInst(program, io_op);
    AddTask_Not(program);
    AddInst(program, io_op);
  }

  /**
   * Input: Program being built.
   *
   * Output: None
   *
   * Purpose: Extends a program to calculate NAND, without I/O.
   */
  void AddTask_Nand(program_t& program) {
    // nand r0, r1, r0
    AddInst(program, nand_op, 0, 1, 0);
  }

  /**
   * Input: Program being built.
   *
   * Output: None
   *
   * Purpose: Extends a program to complete the NAND task, including I/O.
   */
  void AddTask_NandIO(program_t& program) {
    AddInst(program, io_op);
    AddInst(program, io_op, 1);
    AddTask_Nand(program);
    AddInst(program, io_op);
  }

  /**
   * Input: Program being built.
   *
   * Output: None
   *
   * Purpose: Extends a program to calculate AND, without I/O.
   */
  void AddTask_And(
    program_t& program
  ) {
    // ~(a nand b)
    // nand r0, r1, r0
    // nand r0, r0, r0
    AddInst(program, nand_op, 0, 1, 0);
    AddInst(program, nand_op);
  }

  /**
   * Input: Program being built.
   *
   * Output: None
   *
   * Purpose: Extends a program to complete the AND task, including I/O.
   */
  void AddTask_AndIO(
    program_t& program
  ) {
    AddInst(program, io_op);
    AddInst(program, io_op, 1);
    AddTask_And(program);
    AddInst(program, io_op);
  }

  /**
   * Input: Program being built.
   *
   * Output: None
   *
   * Purpose: Extends a program to calculate OR-NOT, without I/O.
   */
  void AddTask_OrNot(program_t& program) {
    // (~a) nand b
    // nand r0, r0, r0
    // nand r0, r1, r0

    AddInst(program, nand_op);
    AddInst(program, nand_op, 0, 1, 0);

  }

  /**
   * Input: Program being built.
   *
   * Output: None
   *
   * Purpose: Extends a program to complete the OR-NOT task, including I/O.
   */
  void AddTask_OrNotIO(program_t& program) {
    AddInst(program, io_op);
    AddInst(program, io_op, 1);
    AddTask_OrNot(program);
    AddInst(program, io_op);
  }

  /**
   * Input: Program being built.
   *
   * Output: None
   *
   * Purpose: Extends a program to calculate OR, without I/O.
   */
  void AddTask_Or(program_t& program) {
    // (~a) nand (~b)
    // nand r0, r0, r0
    // nand r1, r1, r1
    // nand r0, r1, r0
    AddInst(program, nand_op, 0, 0, 0);
    AddInst(program, nand_op, 1, 1, 1);
    AddInst(program, nand_op, 0, 1, 0);
  }

  /**
   * Input: Program being built.
   *
   * Output: None
   *
   * Purpose: Extends a program to complete the OR task, including I/O.
   */
  void AddTask_OrIO(program_t& program) {
    AddInst(program, io_op);
    AddInst(program, io_op, 1);
    AddTask_Or(program);
    AddInst(program, io_op);
  }

  /**
   * Input: Program being built.
   *
   * Output: None
   *
   * Purpose: Extends a program to calculate AND-NOT, without I/O.
   */
  void AddTask_AndNot(program_t& program) {
    // ~(a nand (~b))
    // nand r1, r1, r1
    // nand r0, r1, r0
    // nand r0, r0, r0
    AddInst(program, nand_op, 1, 1, 1);
    AddInst(program, nand_op, 0, 1, 0);
    AddInst(program, nand_op, 0, 0, 0);
  }

  /**
   * Input: Program being built.
   *
   * Output: None
   *
   * Purpose: Extends a program to complete the AND-NOT task, including I/O.
   */
  void AddTask_AndNotIO(program_t& program) {
    AddInst(program, io_op);
    AddInst(program, io_op, 1);
    AddTask_AndNot(program);
    AddInst(program, io_op);
  }

  /**
   * Input: Program being built.
   *
   * Output: None
   *
   * Purpose: Extends a program to calculate NOR, without I/O.
   */
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

  /**
   * Input: Program being built.
   *
   * Output: None
   *
   * Purpose: Extends a program to complete the NOR task, including I/O.
   */
  void AddTask_NorIO(program_t& program) {
    AddInst(program, io_op);
    AddInst(program, io_op, 1);
    AddTask_Nor(program);
    AddInst(program, io_op);
  }

  /**
   * Input: Program being built.
   *
   * Output: None
   *
   * Purpose: Extends a program to calculate XOR, without I/O.
   */
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

  /**
   * Input: Program being built.
   *
   * Output: None
   *
   * Purpose: Extends a program to complete the XOR task, including I/O.
   */
  void AddTask_XorIO(program_t& program) {
    AddInst(program, io_op);
    AddInst(program, io_op, 1);
    AddTask_Xor(program);
    AddInst(program, io_op);
  }

  /**
   * Input: Program being built.
   *
   * Output: None
   *
   * Purpose: Extends a program to calculate equals, without I/O.
   */
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

  /**
   * Input: Program being built.
   *
   * Output: None
   *
   * Purpose: Extends a program to complete the EQU task, including I/O.
   */
  void AddTask_EquIO(program_t& program) {
    AddInst(program, io_op);
    AddInst(program, io_op, 1);
    AddTask_Equ(program);
    AddInst(program, io_op);
  }

  /**
   * Input: Desired program length.
   *
   * Output: The constructed program, padded to length with no-ops.
   *
   * Purpose: Builds a program that can reproduce and perform the NOT task.
   */
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
    RectifyWithWarning(program);
    return program;
  }

  /**
   * Input: Desired program length.
   *
   * Output: The constructed program, padded to length with no-ops.
   *
   * Purpose: Builds a program that can reproduce but performs no tasks.
   */
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

  /**
   * Input: Desired program length.
   *
   * Output: The constructed program, padded to length with no-ops.
   *
   * Purpose: Builds a program that can perform the NOT and NAND tasks and reproduce.
   */
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
    RectifyWithWarning(program);
    return program;
  }

  /**
   * Input: Desired program length.
   *
   * Output: The constructed program, padded to length with no-ops.
   *
   * Purpose: Builds a program that can performs the NAND task and reproduce.
   */
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
    RectifyWithWarning(program);
    return program;
  }

  /**
   * Input: A JSON-serialized program string.
   *
   * Output: The deserialized program object.
   *
   * Purpose: Loads a human-readable program representation from a string.
   *
   * Note: Warns if the program contains disabled instructions.
   */
  program_t ParseJsonString(const std::string& json_str) {
    program_t program(json_str.c_str());
    RectifyWithWarning(program);
    return program;
  }

  /**
   * Input: Path to a program file with a ".json" or ".bin" extension.
   *
   * Output: The deserialized program object.
   *
   * Purpose: Loads a program from a JSON or binary file, depending on the path's file extension.
   *
   * Note: Warns if the program contains disabled instructions.
   */
  program_t LoadProgramFile(const std::filesystem::path& path) {
    program_t program(path);
    RectifyWithWarning(program);
    return program;
  }

  /**
   * Input: A program object to serialize.
   *
   * Output: A JSON string representing the program.
   *
   * Purpose: Converts a program object to human-readable format.
   */
  std::string MakeJsonString(const program_t& program) {
    std::ostringstream oss;
    {
      cereal::JSONOutputArchive archive(oss);
      archive(program);
    }
    return oss.str();
  }

  /**
   * Input: A program to serialize, and a destination path with a ".json" or ".bin" extension.
   *
   * Output: None
   *
   * Purpose: Saves a program to a file, choosing JSON or binary serialization based on the file
   * extension.
   */
  void SaveProgramFile(
    const program_t& program, const std::filesystem::path& path
  ) {
    if ( path.extension() == ".json" ) {
      std::ofstream os(path);
      cereal::JSONOutputArchive archive(os);
      archive( program );
    } else if ( path.extension() == ".bin" ) {
      std::ofstream os(path);
      cereal::BinaryOutputArchive archive(os);
      archive( program );
    } else emp_error(
      "unknown sgpl::Program file format ", path.extension(), " ", path
    );
  }

};

}