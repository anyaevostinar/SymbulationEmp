#pragma once

namespace sgpmode {

// NOTE - We can set this up to be configurable (e.g., support different modes,
//        ability to "layer on" different mutaiton types).
//        For now, it just replicates functionality of previous mutator
template<typename PROGRAM_T>
class SGPMutator {
public:
  using program_t = PROGRAM_T;
protected:
  double per_bit_mut_rate = 0.0;
public:

  void SetPerBitMutationRate(double rate) {
    per_bit_mut_rate = rate;
  }

  void MutateProgram(program_t& program) {
    /*
      ApplyMutations for sgplite:
        - Calculate number of mutations:
          - Poisson(program size in bytes * 8, bit_mut_rate)
        - Flips random bits in underlying program data, fix any broken opcodes that happen as a result
    */
    program.ApplyPointMutations(per_bit_mut_rate);
  }


};

}