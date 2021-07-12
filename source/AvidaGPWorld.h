/* Made: July 1, 2021
   Author: Tiffany-Ellen Vo
   Purpose: World file to override world's DoBirth method to produce/reproduce hosts; simplified version of SymWorld.h
*/

#ifndef AVIDA_GP_WORLD_H
#define AVIDA_GP_WORLD_H

//Header files--------------------------------------------------
#include "../../Empirical/include/emp/Evolve/World.hpp"
#include "../../Empirical/include/emp/data/DataFile.hpp"
#include "../../Empirical/include/emp/math/random_utils.hpp"
#include "../../Empirical/include/emp/math/Random.hpp"
#include "../../Empirical/include/emp/Evolve/World_structure.hpp"
#include "AvidaGPSymbio.hpp"
#include "AvidaGPHost.hpp"
#include <set>
#include <math.h>
#include <iostream>
#include <string>


//class definition taken from SymWorld.h
class AvidaGPWorld : public emp::World<GPHost>{
private:

public:
//call empirical world constructor
  AvidaGPWorld(emp::Random & _random) : emp::World<GPHost>(_random) {}

emp::WorldPosition DoBirth(const genome_t & mem, size_t parent_pos, size_t copy_count) {
    before_repro_sig.Trigger(parent_pos);
    emp::WorldPosition pos;                                        // Position of each offspring placed.
    for (size_t i = 0; i < copy_count; i++) {                 // Loop through offspring, adding each
      emp::Ptr<GPHost> new_org = emp::NewPtr<GPHost>(mem);
      emp::vector<GPSymbiont> new_symbiont = mem->GetSymbio();
      //for loop here, should be iterating through mem
      for (size_t j = 0; j < mem.size(); j++){ // loop through mem symbionts, copy each one
        //get vector of symbionts
        //emp::vector<GPSymbiont> symbiont_copy = new_symbiont; //copy symbiont from mem
  
        
      } 

      offspring_ready_sig.Trigger(*new_org, parent_pos);
      pos = fun_find_birth_pos(new_org, parent_pos);

      if (pos.IsValid()) AddOrgAt(new_org, pos, parent_pos);  // If placement pos is valid, do so!
      else new_org.Delete();                                  // Otherwise delete the organism.
    }

    return pos;
  }

};

#endif
