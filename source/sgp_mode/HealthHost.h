#ifndef HEALTHHOST_H
#define HEALTHHOST_H

#include "SGPHost.h"

class HealthHost : public SGPHost {

    public:

      /**
   * Constructs a new SGPHost as an ancestor organism, with either a random
   * genome or a blank genome that knows how to do a simple task depending on
   * the config setting RANDOM_ANCESTOR.
   */
  HealthHost(emp::Ptr<emp::Random> _random, emp::Ptr<SGPWorld> _world,
          emp::Ptr<SymConfigSGP> _config, double _intval = 0.0,
          emp::vector<emp::Ptr<Organism>> _syms = {},
          emp::vector<emp::Ptr<Organism>> _repro_syms = {},
          double _points = 0.0)
      : SGPHost(_random, _world, _config, _intval, _syms, _repro_syms, _points){}

        /**
   * Constructs an SGPHost with a copy of the provided genome.
   */
  HealthHost(emp::Ptr<emp::Random> _random, emp::Ptr<SGPWorld> _world,
          emp::Ptr<SymConfigSGP> _config, const sgpl::Program<Spec> &genome,
          double _intval = 0.0, emp::vector<emp::Ptr<Organism>> _syms = {},
          emp::vector<emp::Ptr<Organism>> _repro_syms = {},
          double _points = 0.0)
      : SGPHost(_random, _world, _config, _intval, _syms, _repro_syms, _points) {}

  HealthHost(const SGPHost &host)
      : SGPHost(host) {}
    /**
     * Input: None.
     *
     * Output: A new host with same properties as this host.
     *
     * Purpose: To avoid creating an organism via constructor in other methods.
     */
    emp::Ptr<Organism> MakeNew() {
    emp::Ptr<SGPHost> host_baby = emp::NewPtr<HealthHost>(
        random, GetWorld(), sgp_config, GetCPU().GetProgram(), GetIntVal());
    // This organism is reproducing, so it must have gotten off the queue
    GetCPU().state.in_progress_repro = -1;
    return host_baby;
  }

    /** 
     * Input: The location of the host.
     * 
     * Output: None
     * 
     * Purpose: TBD
     */
    void Process(emp::WorldPosition pos) override {
        if (GetDead()) {
            // Handle the case where the host is dead
            //TODO: can this be handled somewhere else so it doesn't get duplicated?
            //Or setup function?
            return;
        }

        //Host with parasite loses 50% of CPU to parasite
        bool host_cycle = true;
        if (HasSym()) {
        host_cycle = random->P(0.5);
        }

        
        if (host_cycle) {
        GetCPU().RunCPUStep(pos, sgp_config->CYCLES_PER_UPDATE());
        }

        if (HasSym() && !host_cycle) { // let each sym do whatever they need to do
        emp::vector<emp::Ptr<Organism>> &syms = GetSymbionts();
        for (size_t j = 0; j < syms.size(); j++) {
            emp::Ptr<Organism> curSym = syms[j];
            if (GetDead()) {
            return; // If previous symbiont killed host, we're done
            }
            // sym position should have host index as id and
            // position in syms list + 1 as index (0 as fls index)
            emp::WorldPosition sym_pos = emp::WorldPosition(j + 1, pos.GetIndex());
            if (!curSym->GetDead()) {
            curSym->Process(sym_pos);
            }
            if (curSym->GetDead()) {
            syms.erase(syms.begin() + j); // if the symbiont dies during their
                                            // process, remove from syms list
            curSym.Delete();
            }
        } // for each sym in syms
        }   // if org has syms


    }
};
#endif // HEALTHHOST_H