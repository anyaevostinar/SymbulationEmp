// #ifndef NUTRIENTHOST_H
// #define NUTRIENTHOST_H
// #include "../../source/sgp_mode/SGPHost.h"

// using namespace sgpmode;

// class NutrientHost : public SGPHost {
// public:
//   /**
//    * Constructs a new SGPHost as an ancestor organism, with either a random
//    * genome or a blank genome that knows how to do a simple task depending on
//    * the config setting RANDOM_ANCESTOR.
//    */
//   NutrientHost(emp::Ptr<emp::Random> _random, emp::Ptr<SGPWorld> _world,
//                emp::Ptr<SymConfigSGP> _config, double _intval = 0.0,
//                emp::vector<emp::Ptr<Organism>> _syms = {},
//                emp::vector<emp::Ptr<Organism>> _repro_syms = {},
//                double _points = 0.0)
//       : SGPHost(_random, _world, _config, _intval, _syms, _repro_syms, _points) {}

//   /**
//    * Constructs an SGPHost with a copy of the provided genome.
//    */
//   NutrientHost(emp::Ptr<emp::Random> _random, emp::Ptr<SGPWorld> _world,
//                emp::Ptr<SymConfigSGP> _config, const sgpl::Program<Spec> &genome,
//                double _intval = 0.0, emp::vector<emp::Ptr<Organism>> _syms = {},
//                emp::vector<emp::Ptr<Organism>> _repro_syms = {},
//                double _points = 0.0)
//       : SGPHost(_random, _world, _config, genome, _intval, _syms, _repro_syms, _points) {}

//   NutrientHost(const SGPHost &host) : SGPHost(host) {}

//   /**
//    * Input: None.
//    *
//    * Output: A new host with the same properties as this host.
//    *
//    * Purpose: To avoid creating an organism via constructor in other methods.
//    */
//   emp::Ptr<Organism> MakeNew() override {
//     emp::Ptr<SGPHost> host_baby = emp::NewPtr<NutrientHost>(
//         random, GetWorld(), sgp_config, GetCPU().GetProgram(), GetIntVal());
//     return host_baby;
//   }

//   /**
//    * Add a symbiont to the host, with ousting if necessary.
//    * Input: Symbiont organism (_in).
//    * Output: The number of symbionts (after addition), or 0 if it couldn't be added.
//    */
//   int AddSymbiont(emp::Ptr<Organism> _in) override {
//     // Ousting logic: if the symbiont limit is reached, remove the last one.
//     if ((int)syms.size() >= my_config->SYM_LIMIT()) {
//       emp::Ptr<Organism> last_sym = syms.back();
//       syms.pop_back();
//       last_sym->SetDead(); // Remove dead symbiont from host.
//     }

//     // Add the new symbiont if there is space and it's allowed.
//     if ((int)syms.size() < my_config->SYM_LIMIT() && SymAllowedIn()) {
//       syms.push_back(_in);
//       _in->SetHost(this);
//       _in->UponInjection();
//       return syms.size();
//     } else {
//       _in.Delete(); // Symbiont not added, delete it.
//       return 0;
//     }
//   }

//   /**
//    * Input: The location of the host.
//    *
//    * Output: None
//    *
//    * Purpose: Perform logic for the host and interact with symbionts.
//    */
//   void Process(emp::WorldPosition pos) override 
//           // check for if host lost too many resources 
//     if (GetDead()) {
//         /// otherwise stop because host is dead
//         return; 
//     }

//     bool host_cycle = true;
//     // If the host is performing a cycle, continue the host's operations
//     if (host_cycle) {

//       GetCPU().RunCPUStep(pos, sgp_config->CYCLES_PER_UPDATE());
//     }

//     // Process symbionts only if the host didn't lose resources to them.
//     if (HasSym() && !host_cycle) {
//       emp::vector<emp::Ptr<Organism>> &syms = GetSymbionts();
//       for (size_t j = 0; j < syms.size(); j++) {
//         emp::Ptr<Organism> curSym = syms[j];

//         ////----------------------------------


        
//         // Check if the symbiont performed a matching operation with the host.
//         bool symbiont_resources_taken = false;
//         /// main check to see if 
//         // TODO: need to ask about this 
//         if (curSym->HasPerformedMatchingOperation()) {
//           //// -------------------------------------------------------------
//           // Symbiont steals resources if it performed a matching operation.
//           symbiont_resources_taken = random->StealResources(0.5); // Symbiont takes 50%
//         }

//         //// ---- split resrouces 
//         ///  new 
//         if (!symbiont_resources_taken) {
//             ///
//             DistribResources();
//         }


//         ////

//         ///------------------------------

//         // Process the symbiont, updating its state.
//         emp::WorldPosition sym_pos = emp::WorldPosition(j + 1, pos.GetIndex());
//         if (!curSym->GetDead()) {
//           curSym->Process(sym_pos);
//         }

//         // Remove any symbiont that dies during processing.
//         if (curSym->GetDead()) {
//           syms.erase(syms.begin() + j); // Remove dead symbionts from the list.
//           curSym.Delete();
//         }
//       }
//     } // End of symbionts processing.
//   }
// };

// #endif // NUTRIENTHOST_H
