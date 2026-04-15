#ifndef HEALTHHOST_H
#define HEALTHHOST_H

#include "SGPHost.h"

class HealthHost : public SGPHost {
    public:

    //Tracks whether an update needs to be given to a symbiont or received from a symbiont
    int cycles_given = 1;

    //Variables that are currently used to give symbionts some starting cycles and then give them scraps throughout
    //the rest of their updates.
    int honorary_cycles = 0;
    int starting_updates = 1;
    emp::Ptr<Organism> last_sym = NULL; 

    /**
     * Constructs a new SGPHost as an ancestor organism, with a blank genome that knows how to do a simple task.
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

    /**
     * Input: An integer amount for the number of updates that are being transferred
     *
     * Output: None.
     *
     * Purpose: To change the number of cycles a host is set to give/be given.
     */
    void CycleTransfer(int amount) override {
      cycles_given += amount;
    }

    /**
     * Input: None.
     *
     * Output: A integer for the amount of cycles the host is set to give/be given.
     *
     * Purpose: For symbionts to check if their host already owes cycles.
     */
    int GetCyclesGiven() override {
      return cycles_given;
    }

     /**
     * Input: None.
     *
     * Output: The string 'HealthHost', which is the name of this organism
     *
     * Purpose: Allows tests to check that an organism is of the proper type. 
     */
    std::string const GetName() override{
      return "HealthHost";
    }
    
    /** 
     * Input: The location of the host.
     * 
     * Output: None
     * 
     * Purpose: To process the host, meaning: running its program code, which can
     * include reproduction and acquisition of resources; removing dead syms;
     * processing alive syms; Giving CPU cycles to syms; Receiving CPU cycles from syms;. 
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
            if(cycles_given >= 1){
              if(random->P(sgp_config->CPU_TRANSFER_CHANCE())){
                host_cycle += sgp_config->SYNERGY();
                sym_cycle -= 1;
                cycles_given = 0;
              }
            }
            if(cycles_given <= -1){
              if(random->P(sgp_config->CPU_TRANSFER_CHANCE())){
                host_cycle = 0;
                sym_cycle += sgp_config->SYNERGY();
                cycles_given = 0;
              }
            }

            //This sequence checks if the symbiont will receive a cycle, if it has not then it checks if
            //one of its bonus updates is left, if so it uses one, if not it ticks up the counter
            //for when it receives said bonus update. 
            //Allows for symbiont to reach steals but to still need them
            emp::vector<emp::Ptr<Organism>> &syms = GetSymbionts();
            emp::Ptr<Organism> cur_sym = syms[0];
            if(last_sym != cur_sym){
              last_sym = cur_sym;
              cycles_given = 0;
              starting_updates = 1;
            }

            if(sym_cycle == 0 && sgp_config->DONATION_STEAL_INST()){
              if(starting_updates > 0){
                sym_cycle += 1;
                starting_updates -= 1;
              }
            }
            if(starting_updates < 1){
              honorary_cycles += 1;
              if(honorary_cycles == 5){
                starting_updates += 1;
                honorary_cycles = 0;
              }
            }
          }
          else{
            if (sgp_config->SYMBIONT_TYPE() == MUTUALIST) {
              //Host with mutualist gains 50% of CPU from mutualist
              if (random->P(sgp_config->CPU_TRANSFER_CHANCE())) {
                host_cycle = 1 + sgp_config->SYNERGY();
                sym_cycle = 0;
              } else {
                host_cycle = 1;
                sym_cycle = 1;
              }
            }
            else if (sgp_config->SYMBIONT_TYPE() == PARASITE) {
              //Host with parasite loses 50% of CPU to parasite
              if (random->P(sgp_config->CPU_TRANSFER_CHANCE())) {
                host_cycle = 0;
                sym_cycle = sgp_config->SYNERGY();
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
              emp::Ptr<Organism> cur_sym = syms[j];
              if (GetDead()) {
                return; // If previous symbiont killed host, we're done
              }
              // sym position should have host index as id and
              // position in syms list + 1 as index (0 as fls index)
              emp::WorldPosition sym_pos = emp::WorldPosition(j + 1, pos.GetIndex());
              if (!cur_sym->GetDead()) {
                cur_sym->Process(sym_pos);
              }
              if (cur_sym->GetDead()) {
                syms.erase(syms.begin() + j); // if the symbiont dies during their
                                              // process, remove from syms list
                cur_sym.Delete();
              }    
            } 
          } 
        }
        GrowOlder();
    }

};

  
#endif // HEALTHHOST_H
