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

emp::WorldPosition DoBirth(emp::Ptr<GPHost> mem, size_t parent_pos, size_t copy_count) {
    before_repro_sig.Trigger(parent_pos);
    emp::WorldPosition pos;                                        // Position of each offspring placed.
    for (size_t i = 0; i < copy_count; i++) {                 // Loop through offspring, adding each
      emp::Ptr<GPHost> new_org = emp::NewPtr<GPHost>(*mem);
      emp::vector<emp::Ptr<GPSymbiont>> new_symbiont = mem->GetSymbio();
      
      /*//for loop here, should be iterating through mem
      for (size_t j = 0; j < new_symbiont.size(); j++){ // loop through mem symbionts, copy each one
        //get vector of symbionts
        // change this to pointer 
        emp::vector<GPSymbiont> symbiont_copy = new_symbiont; //copy symbiont from mem
        //then add to new org

        
      } */

      offspring_ready_sig.Trigger(*new_org, parent_pos);
      pos = fun_find_birth_pos(new_org, parent_pos);

      if (pos.IsValid()) AddOrgAt(new_org, pos, parent_pos);  // If placement pos is valid, do so!
      else new_org.Delete();                                  // Otherwise delete the organism.
    }

    return pos;
  }

};
 /// ==ELITE== Selection picks a set of the most fit individuals from the population to move to
  /// the next generation.  Find top e_count individuals and make copy_count copies of each.
  /// @param world The emp::World object with the organisms to be selected.
  /// @param e_count How many distinct organisms should be chosen, starting from the most fit.
  /// @param copy_count How many copies should be made of each elite organism?
  void AvidaGPEliteSelect(AvidaGPWorld & world, size_t e_count=1, size_t copy_count=1) {
    emp_assert(e_count > 0 && e_count <= world.GetNumOrgs(), e_count);
    emp_assert(copy_count > 0);

    // Load the population into a multimap, sorted by fitness.
    std::multimap<double, size_t> fit_map;
    for (size_t id = 0; id < world.GetSize(); id++) {
      if (world.IsOccupied(id)) {
        const double cur_fit = world.CalcFitnessID(id);
        fit_map.insert( std::make_pair(cur_fit, id) );
      }
    }

    // Grab the top fitnesses and move them into the next generation.
    auto m = fit_map.rbegin();
    for (size_t i = 0; i < e_count; i++) {
      const size_t repro_id = m->second;
      world.DoBirth(world.GetOrgPtr(repro_id), repro_id, copy_count);
      ++m;
    }
  }

 void AvidaGPTournamentSelect(AvidaGPWorld & world, size_t t_size, size_t tourny_count=1) {
    emp_assert(t_size > 0, "Cannot have a tournament with zero organisms.", t_size, world.GetNumOrgs());
    emp_assert(t_size <= world.GetNumOrgs(), "Tournament too big for world.", t_size, world.GetNumOrgs());
    emp_assert(tourny_count > 0);

    emp::vector<size_t> entries;
    for (size_t T = 0; T < tourny_count; T++) {
      entries.resize(0);
      // Choose organisms for this tournament (with replacement!)
      for (size_t i=0; i < t_size; i++) entries.push_back(
        world.GetRandomOrgID()
      );

      double best_fit = world.CalcFitnessID(entries[0]);
      size_t best_id = entries[0];

      // Search for a higher fit org in the tournament.
      for (size_t i = 1; i < t_size; i++) {
        const double cur_fit = world.CalcFitnessID(entries[i]);
        if (cur_fit > best_fit) {
          best_fit = cur_fit;
          best_id = entries[i];
        }
      }

      // Place the highest fitness into the next generation!
      world.DoBirth(world.GetOrgPtr(best_id), best_id, 1 );
    }
  }


#endif
