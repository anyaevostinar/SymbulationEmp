#ifndef HEALTHHOST_H
#define HEALTHHOST_H

#include "SGPHost.h"

class HealthHost : public SGPHost {

    public:

    int cycles_given = 0;

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
      : SGPHost(_random, _world, _config, genome, _intval, _syms, _repro_syms, _points) {}

  HealthHost(const SGPHost &host)
      : SGPHost(host) {}
    /**
     * Input: None.
     *
     * Output: A new host with same properties as this host.
     *
     * Purpose: To avoid creating an organism via constructor in other methods.
     */
    emp::Ptr<Organism> MakeNew() override {
    emp::Ptr<HealthHost> host_baby = emp::NewPtr<HealthHost>(
        random, GetWorld(), sgp_config, GetCPU().GetProgram(), GetIntVal());
    return host_baby;
  }
    void CycleTransfer(int amount) override {
      cycles_given += amount;
    }

    int GetCyclesGiven(){
      return cycles_given;
    }

    std::string const GetName() override{
      return "HealthHost";
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

        int host_cycle = 1;
        int sym_cycle = 0;
        if (HasSym()) {
          
          if(sgp_config->DONATION_STEAL_INST()){
            sym_cycle = 1;

            if(cycles_given >= 1){
              if(random->P(sgp_config->CPU_TRANSFER_CHANCE())){
                host_cycle += 1;
                sym_cycle -= 1;
              }
              cycles_given = 0;
              
            }else if(cycles_given <= -1){
              if(random->P(sgp_config->CPU_TRANSFER_CHANCE())){
                host_cycle -= 1;
                sym_cycle += 1;
              }
              cycles_given = 0;

            }
          }
          else{
            if (sgp_config->STRESS_TYPE() == MUTUALIST) {
            //Host with mutualist gains 50% of CPU from mutualist
            if (random->P(sgp_config->CPU_TRANSFER_CHANCE())) {
              host_cycle = 2;
              sym_cycle = 0;
            } else {
              host_cycle = 1;
              sym_cycle = 1;
            }
          }
          else if (sgp_config->STRESS_TYPE() == PARASITE) {
            //Host with parasite loses 50% of CPU to parasite
            if (random->P(sgp_config->CPU_TRANSFER_CHANCE())) {
              host_cycle = 0;
              sym_cycle = 1;
            } else {
              host_cycle = 1;
              sym_cycle = 0;
            }
          }
          }
        }
        
        //Loops running CPU steps
        for(int i = 0; i < host_cycle; i++){
          GetCPU().RunCPUStep(pos, sgp_config->CYCLES_PER_UPDATE());
        }

        if (HasSym() && sym_cycle > 0) { // let each sym do whatever they need to do
          for (int i = 0; i < sym_cycle; i++){
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
      GrowOlder();
    }
};

  
#endif // HEALTHHOST_H