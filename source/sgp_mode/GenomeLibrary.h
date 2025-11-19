#ifndef GENOME_LIBRARY
#define GENOME_LIBRARY

#include "Instructions.h"
#include "sgpl/operations/flow_global/Anchor.hpp"
#include "sgpl/program/Instruction.hpp"
#include "sgpl/program/Program.hpp"
#include "sgpl/spec/Spec.hpp"
#include <cstddef>
#include <limits>

using Library = sgpl::OpLibrary<
    sgpl::Nop<>,
    // single argument math
    inst::ShiftLeft, inst::ShiftRight, inst::Increment, inst::Decrement,
    // biological operations
    inst::Reproduce, 
    inst::SharedIO,
    inst::Steal,
    inst::Donate, 
    // double argument math
    inst::Add, inst::Subtract, inst::Nand,
    // Stack manipulation
    inst::Push, inst::Pop, inst::SwapStack, inst::Swap,
    // no h-search
    inst::JumpIfNEq, inst::JumpIfLess, 
    sgpl::global::Anchor 
    >;

using Spec = sgpl::Spec<Library, CPUState>;

// Instead of picking an anchor to start at randomly, start at the anchor that
// has the most bits set by matching with the maximum valued tag. This way
// organisms can evolve to designate a certain anchor as the entry.
const Spec::tag_t START_TAG(std::numeric_limits<uint64_t>::max());
const size_t PROGRAM_LENGTH = 100;

/**
 * Allows building up a program without knowing the final size.
 * When it's done and `build()` is called, the instructions added to the builder
 * will be located at the end of the generated program, right before
 * `reproduce`.
 */
class ProgramBuilder : emp::vector<sgpl::Instruction<Spec>> {
public:
  void Add(const std::string op_name, uint8_t arg0 = 0, uint8_t arg1 = 0,
           uint8_t arg2 = 0) {
    sgpl::Instruction<Spec> inst;
    inst.op_code = Library::GetOpCode(op_name);
    inst.args = {arg0, arg1, arg2};
    push_back(inst);
  }

   void AddNop(uint8_t arg0 = 0, uint8_t arg1 = 0,
           uint8_t arg2 = 0) {
    sgpl::Instruction<Spec> inst;
    inst.op_code = 0;
    inst.args = {arg0, arg1, arg2};
    push_back(inst);
  }

  sgpl::Program<Spec> Build(size_t length) {
    Add("Reproduce");

    sgpl::Program<Spec> program;
    // Set everything to 0 - this makes them no-ops since that's the first
    // inst in the library
    program.resize(length - size());
    program[0].op_code = Library::GetOpCode("Global Anchor");
    program[0].tag = START_TAG;

    program.insert(program.end(), begin(), end());

    return program;
  }

  sgpl::Program<Spec> BuildNoRepro(size_t length) {
    //For making an obligate mutualist
    Add("Donate");
    Add("Donate");
    Add("Donate");
    Add("Donate");
    Add("Donate");
    sgpl::Program<Spec> program;
    // Set everything to 0 - this makes them no-ops since that's the first
    // inst in the library
    program.resize(length - size());
    program[0].op_code = Library::GetOpCode("Global Anchor");
    program[0].tag = START_TAG;

    program.insert(program.end(), begin(), end());

    return program;
  }

  void AddNot() {
    // sharedio   r0
    // nand       r0, r0, r0
    // sharedio   r0
    Add("SharedIO");
    Add("Nand");
    Add("SharedIO");
  }

  void AddPrivateNot() {
    // privateio   r0
    // nand       r0, r0, r0
    // privateio   r0
    Add("PrivateIO");
    Add("Nand");
    Add("PrivateIO");
  }

  void AddNand() {
    // sharedio   r0
    // sharedio   r1
    // nand       r0, r1, r0
    // sharedio   r0
    Add("SharedIO");
    Add("SharedIO", 1);
    Add("Nand", 0, 1, 0);
    Add("SharedIO");
  }

  void AddPrivateNand() {
    // sharedio   r0
    // sharedio   r1
    // nand       r0, r1, r0
    // sharedio   r0
    Add("PrivateIO");
    Add("PrivateIO", 1);
    Add("Nand", 0, 1, 0);
    Add("PrivateIO");
  }

  void AddAnd() {
    // ~(a nand b)
    // sharedio   r0
    // sharedio   r1
    // nand       r0, r1, r0
    // nand       r0, r0, r0
    // sharedio   r0
    Add("SharedIO");
    Add("SharedIO", 1);
    Add("Nand", 0, 1, 0);
    Add("Nand");
    Add("SharedIO");
  }

  void AddOrn() {
    // (~a) nand b
    // sharedio   r0
    // sharedio   r1
    // nand       r0, r0, r0
    // nand       r0, r1, r0
    // sharedio   r0
    Add("SharedIO");
    Add("SharedIO", 1);
    Add("Nand");
    Add("Nand", 0, 1, 0);
    Add("SharedIO");
  }

  void AddOr() {
    // (~a) nand (~b)
    // sharedio   r0
    // sharedio   r1
    // nand       r0, r0, r0
    // nand       r1, r1, r1
    // nand       r0, r1, r0
    // sharedio   r0
    Add("SharedIO");
    Add("SharedIO", 1);
    Add("Nand", 0, 0, 0);
    Add("Nand", 1, 1, 1);
    Add("Nand", 0, 1, 0);
    Add("SharedIO");
  }

  void AddAndn() {
    // ~(a nand (~b))
    // sharedio   r0
    // sharedio   r1
    // nand       r1, r1, r1
    // nand       r0, r1, r0
    // nand       r0, r0, r0
    // sharedio   r0
    Add("SharedIO");
    Add("SharedIO", 1);
    Add("Nand", 1, 1, 1);
    Add("Nand", 0, 1, 0);
    Add("Nand", 0, 0, 0);
    Add("SharedIO");
  }

  void AddNor() {
    // ~((~a) nand (~b))
    // sharedio   r0
    // sharedio   r1
    // nand       r0, r0, r0
    // nand       r1, r1, r1
    // nand       r0, r1, r0
    // nand       r0, r0, r0
    // sharedio   r0
    Add("SharedIO");
    Add("SharedIO", 1);
    Add("Nand", 0, 0, 0);
    Add("Nand", 1, 1, 1);
    Add("Nand", 0, 1, 0);
    Add("Nand", 0, 0, 0);
    Add("SharedIO");
  }

  void AddXor() {
    // (a & ~b) | (~a & b) --> (a nand ~b) nand (~a nand b)
    // sharedio   r0
    // sharedio   r1
    //
    // nand       r3, r1, r1
    // nand       r3, r3, r0
    //
    // nand       r2, r0, r0
    // nand       r2, r2, r1
    //
    // nand       r0, r2, r3
    // sharedio   r0
    Add("SharedIO");
    Add("SharedIO", 1);

    Add("Nand", 3, 1, 1);
    Add("Nand", 3, 3, 0);

    Add("Nand", 2, 0, 0);
    Add("Nand", 2, 2, 1);

    Add("Nand", 0, 2, 3);
    Add("SharedIO");
  }

  void AddEqu() {
    // ~(a ^ b)
    // sharedio   r0
    // sharedio   r1
    //
    // nand       r3, r1, r1
    // nand       r3, r3, r0
    //
    // nand       r2, r0, r0
    // nand       r2, r2, r1
    //
    // nand       r0, r2, r3
    // nand       r0, r0, r0
    // sharedio   r0
    Add("SharedIO");
    Add("SharedIO", 1);

    Add("Nand", 3, 1, 1);
    Add("Nand", 3, 3, 0);

    Add("Nand", 2, 0, 0);
    Add("Nand", 2, 2, 1);

    Add("Nand", 0, 2, 3);
    Add("Nand", 0, 0, 0);
    Add("SharedIO");
  }


   /**
     * Input: The number of steal instructions to add.
     *
     * Output: None.
     *
     * Purpose: Spread out steal instructions throughout the organism's genome in order to allow stealing 
     * during a symbiont's entire genome and not just in one place. Without the spread of instructions symbionts were unable to steal regularly and died.
     */
  void AddStartSteal(int steal_count){
    
    if(steal_count > 0){
      int diff = 96/steal_count;
    
      for (int i = 0; i < steal_count; i++){
        Add("Steal");
        for (int x = 1; x < (diff-1); x++){
          AddNop();
        
        }
        
        
      }
    }
    
  }

  /**
     * Input: The number of donate instructions to add.
     *
     * Output: None.
     *
     * Purpose: Spread out donate instructions throughout the organism's genome in order to allow donations 
     * during a symbiont's entire genome and not just in one place. Without the spread of instructions symbionts were unable to donate regularly.
     */
  void AddStartDonate(int donate_count){
    
    if(donate_count > 0){
      int diff = 96/donate_count;
      
      for (int i = 0; i < donate_count; i++){
        Add("Donate");
        for (int x = 1; x < (diff-1); x++){
          AddNop();
        
        }
        
        
      }
    }
    
  }
};

sgpl::Program<Spec> CreateReproProgram(size_t length) {
  ProgramBuilder program;
  return program.Build(length);
}

sgpl::Program<Spec> CreateNotProgram(size_t length) {
  ProgramBuilder program;
  program.AddNot();
  return program.Build(length);
}

sgpl::Program<Spec> CreateParasiteNotProgram(size_t length, int steal_count) {
  ProgramBuilder program;
  if(steal_count < 0){
    steal_count = 0;
    std::cout << "CPU_TRANSFER_AMOUNT was too low, has been clamped to 0" << std::endl;
  }

  if(steal_count > 95){
    steal_count = 95;
    std::cout << "CPU_TRANSFER_AMOUNT was too high, has been clamped to 95" << std::endl;
  }
  program.AddStartSteal(steal_count);
  program.AddNot();
  
  return program.Build(length);
}

sgpl::Program<Spec> CreateMutualistNotProgram(size_t length, int donate_count) {
  ProgramBuilder program;
  if(donate_count < 0){
    donate_count = 0;
    std::cout << "CPU_TRANSFER_AMOUNT was too low, has been clamped to 0" << std::endl;
  }

  if(donate_count > 95){
    donate_count = 95;
    std::cout << "CPU_TRANSFER_AMOUNT was too high, has been clamped to 95" << std::endl;
  }
  program.AddStartDonate(donate_count);
  program.AddNot();
  return program.Build(length);
}

sgpl::Program<Spec> CreateEquProgram(size_t length) {
  ProgramBuilder program;
  program.AddEqu();
  return program.Build(length);
}


/**
 * Picks what type of starting program should be created based on the config and
 * creates it.
 */
sgpl::Program<Spec> CreateStartProgram(emp::Ptr<SymConfigSGP> config) {
  if(config->DONATION_STEAL_INST() == 1){
    if(config->SYMBIONT_TYPE() == 1){
      return CreateParasiteNotProgram(PROGRAM_LENGTH, config->CPU_TRANSFER_AMOUNT());
    }
    else if(config->SYMBIONT_TYPE() == 0){
      return CreateMutualistNotProgram(PROGRAM_LENGTH, config->CPU_TRANSFER_AMOUNT());
    }
    else{
      return CreateNotProgram(PROGRAM_LENGTH);
    }
  }
  else{
      return CreateNotProgram(PROGRAM_LENGTH);
    }
}

#endif