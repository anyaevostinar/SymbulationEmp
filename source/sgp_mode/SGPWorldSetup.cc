#ifndef SGP_WORLD_SETUP_C
#define SGP_WORLD_SETUP_C

#include "../ConfigSetup.h"
#include "SGPHost.h"
#include "SGPSymbiont.h"
#include "SGPWorld.h"

void SGPWorld::SetupHosts(unsigned long *POP_SIZE) {
  for (size_t i = 0; i < *POP_SIZE; i++) {
    emp::Ptr<SGPHost> new_org = emp::NewPtr<SGPHost>(
        &GetRandom(), this, my_config, CreateNotProgram(100), my_config->HOST_INT());
    if(my_config->START_MOI()==1){
      emp::Ptr<SGPSymbiont> new_sym = emp::NewPtr<SGPSymbiont>(
          &GetRandom(), this, my_config, my_config->SYM_INT());
      new_org->AddSymbiont(new_sym);
    }
    InjectHost(new_org);
  }
}

void SGPWorld::SetupSymbionts(unsigned long *total_syms) {

}

int SGPWorld::GetNeighborHost (size_t id, emp::Ptr<emp::BitSet<64>> parent_tasks){
  // Attempt to find host that matches some tasks
  for (int i = 0; i < 10; i++) {
    emp::WorldPosition neighbor = GetRandomNeighborPos(id);
    if (neighbor.IsValid() && IsOccupied(neighbor)){
      //check if neighbor host does any task that parent sym did
      emp::Ptr<emp::BitSet<64>> host_tasks = GetOrgPtr(neighbor.GetIndex()).DynamicCast<SGPHost>()->GetCPU().state.tasks_performed;
      for(int i =host_tasks->size()-1; i>-1; i--){
        if(parent_tasks->Get(i) && host_tasks->Get(i)) {
          //both parent sym and host can do this task, parasite can infect
          return neighbor.GetIndex();
        }
      }
    }
  }
  //Otherwise parasite can't infect host
  return -1;
}

emp::WorldPosition SGPWorld::SymDoBirth(emp::Ptr<Organism> sym_baby, emp::WorldPosition parent_pos) {
   size_t i = parent_pos.GetPopID();
    emp::Ptr<Organism> parent = GetOrgPtr(i)->GetSymbionts()[parent_pos.GetIndex()-1];
    emp::Ptr<emp::BitSet<64>> parent_tasks = parent.DynamicCast<SGPSymbiont>()->GetCPU().state.tasks_performed;
    if(my_config->FREE_LIVING_SYMS() == 0){
      int new_host_pos = GetNeighborHost(i, parent_tasks);
      if (new_host_pos > -1) { //-1 means no living neighbors
        int new_index = pop[new_host_pos]->AddSymbiont(sym_baby);
        if(new_index > 0){ //sym successfully infected
          return emp::WorldPosition(new_index, new_host_pos);
        } else { //sym got killed trying to infect
          return emp::WorldPosition();
        }
      } else {
        sym_baby.Delete();
        return emp::WorldPosition();
      }
    } else {
      return MoveIntoNewFreeWorldPos(sym_baby, parent_pos);
    }
  }

#endif
