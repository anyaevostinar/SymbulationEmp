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
#include "Organism.h"
#include <set>
#include <math.h>

//class definition taken from SymWorld.h
class AvidaGPWorld : public emp::World<Organism>{
private:
  double vertTrans = 0; 
  int total_res = -1;
  bool limited_res = false;
  

  double resources_per_host_per_update = 0;
  
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_hostintval; // New() reallocates this pointer
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_symintval;
  emp::Ptr<emp::DataMonitor<int>> data_node_hostcount;
  emp::Ptr<emp::DataMonitor<int>> data_node_symcount;
  emp::Ptr<emp::DataMonitor<double>> data_node_burst_size;
  emp::Ptr<emp::DataMonitor<double>> data_node_efficiency;
  emp::Ptr<emp::DataMonitor<int>> data_node_cfu;



public:
  //set fun_print_org to equal function that prints hosts/syms correctly
  SymWorld(emp::Random & _random) : emp::World<Organism>(_random) {
    fun_print_org = [](Organism & org, std::ostream & os) {
      //os << PrintHost(&org);
      os << "This doesn't work currently";
    };
  }

//DoBirth function taken from SymWorld.h
//Overriding World's DoBirth to take a pointer instead of a reference
//Because it takes a pointer, it doesn't support birthing multiple copies
  emp::WorldPosition DoBirth(emp::Ptr<Organism> new_org, size_t parent_pos) {
    before_repro_sig.Trigger(parent_pos);
    emp::WorldPosition pos;                                        // Position of each offspring placed.
    
    offspring_ready_sig.Trigger(*new_org, parent_pos);
    pos = fun_find_birth_pos(new_org, parent_pos);

    if (pos.IsValid() && pos.GetIndex() != parent_pos) AddOrgAt(new_org, pos, parent_pos);  // If placement pos is valid, do so!
    else new_org.Delete();                                  // Otherwise delete the organism.
    return pos;
  }
};

#endif