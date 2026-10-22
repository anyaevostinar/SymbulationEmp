#pragma once

#include "sgpl/program/Program.hpp"
#include "sgpl/library/OpLibrary.hpp"

namespace sgpmode {

// NOTE - We can set this up to be configurable (e.g., support different modes,
//        ability to "layer on" different mutaiton types).
//        For now, it just replicates functionality of previous mutator
/**
* Used to mutate the program of organisms
* @tparam PROGRAM_T: The type representing the program that is going to be mutated
* @tparam INST_LIBRARY_T: Contains all instructions that can be mutated.
*/
template<typename PROGRAM_T, typename INST_LIBRARY_T>
class SGPMutator {
public:
  using program_t = PROGRAM_T;
  using lib_t = INST_LIBRARY_T;
  using rectifier_t = sgpl::OpCodeRectifier<lib_t>;

protected:
  double per_bit_mut_rate = 0.0;
  rectifier_t& prog_rectifier;
public:
  SGPMutator(
    rectifier_t& opcode_rectifier
  ) : prog_rectifier(opcode_rectifier) { }

  /**
   * Input: New rate of amount of bits to be mutated.
   *
   * Output: None.
   *
   * Purpose: Sets number of bits to be mutated on mutation. 
   */
  void SetPerBitMutationRate(double rate) {
    per_bit_mut_rate = rate;
  }

  /**
   * Input: A program.
   *
   * Output: None.
   *
   * Purpose: Randomly flips bits in program data to allow for program mutation. 
   */
  void MutateProgram(program_t& program) {
    /*
      ApplyMutations for sgplite:
        - Calculate number of mutations:
          - Poisson(program size in bytes * 8, bit_mut_rate)
        - Flips random bits in underlying program data, fix any broken opcodes that happen as a result
    */
    program.ApplyPointMutations(
      per_bit_mut_rate,
      prog_rectifier
    );
  }


};

}