/* Made: June 23, 2021
   Author: Originally created by Edwin Flores-Cardoso, built on by Tiffany-Ellen Vo
   Purpose: 
*/

#ifndef AVIDA_GP_HOST_H
#define AVIDA_GP_HOST_H

#include "AvidaGPSymbio.hpp"

//FIX THESE TO CORRENT EMPIRICAL FOLDERS
#include "../../Empirical/include/emp/hardware/AvidaGP.hpp"
#include "../../Empirical/include/emp/config/ArgManager.hpp"
#include "../../Empirical/include/emp/Evolve/World.hpp"
#include <iostream>
#include <vector>

using std::endl; using std::cout;

/*
* Host class made to use AvidaGP's structs in its file. 
*/
class GPHost : public emp::AvidaCPU_Base<GPHost>{
 private:
  emp::vector<GPSymbiont> syms;

  //make instance of points here, would this be vector symbiont?
  emp::vector<GPHost> points;

  /*
  bool HasSym() {
    return syms.size() != 0;
  }
  */

public:
  using base_t = AvidaCPU_Base<GPHost>;
  using typename base_t::genome_t;
  using typename base_t::inst_lib_t;
  
  //Constructors
  GPHost(const genome_t & in_genome) : AvidaCPU_Base(in_genome) { ; }
  GPHost(emp::Ptr<const inst_lib_t> inst_lib) : AvidaCPU_Base(genome_t(inst_lib)) { ; }
  GPHost(const inst_lib_t & inst_lib) : AvidaCPU_Base(genome_t(&inst_lib)) { ; }
  GPHost() = default;
  GPHost(const GPHost &) = default;
  GPHost(GPHost &&) = default;

  //Sets hosts' Symbionts
  void SetSymbio(emp::vector<GPSymbiont> sym_in ){
    for (GPSymbiont s : sym_in){
      syms.push_back(s);
    }
  }

  //returns hosts' symbionts
  emp::vector<GPSymbiont> GetSymbio(){
    return syms;
  }

  //Destructor 
  //virtual ~Host() { ; }

  
};

#endif