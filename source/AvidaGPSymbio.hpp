/* Made: June 23, 2021
   Author: Originally created by Edwin Flores-Cardoso, built on by Tiffany-Ellen Vo
   Purpose: 

*/
#ifndef AVIDA_GP_SYMBIO_H
#define AVIDA_GP_SYMBIO_H

#include "../../Empirical/include/emp/hardware/AvidaGP.hpp"
#include "../../Empirical/include/emp/config/ArgManager.hpp"
#include "../../Empirical/include/emp/Evolve/World.hpp"
#include <iostream>
using std::endl; using std::cout;

/*
* Class represents a single symbiont that inherits from AvidaCPU_base class 
*/
class Symbiont : public emp::AvidaCPU_Base<Symbiont> {
 public:
  using base_t = AvidaCPU_Base<Symbiont>;
  using typename base_t::genome_t;
  using typename base_t::inst_lib_t;

  Symbiont(const genome_t & in_genome) : AvidaCPU_Base(in_genome) { ; }
  Symbiont(emp::Ptr<const inst_lib_t> inst_lib) : AvidaCPU_Base(genome_t(inst_lib)) { ; }
  Symbiont(const inst_lib_t & inst_lib) : AvidaCPU_Base(genome_t(&inst_lib)) { ; }

  Symbiont() = default;
  Symbiont(const Symbiont &) = default;
  Symbiont(Symbiont &&) = default;


   // what does this line mean?
  virtual ~Symbiont() { ; }


};

#endif
