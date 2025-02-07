#ifndef SGP_HOST_C
#define SGP_HOST_C

#include "SGPHost.h"
#include "SGPSymbiont.h"


/**
* Input: None.
*
* Output: A new host baby of the current host, mutated.
*
* Purpose: To create a new baby host and reset this host's points to 0.
*/
emp::Ptr<Organism> SGPHost::Reproduce() {
  emp::Ptr<SGPHost> host_baby = Host::Reproduce().DynamicCast<SGPHost>();
  host_baby->SetReproCount(reproductions + 1);
  // This organism is reproducing, so it must have gotten off the queue
  cpu.state.in_progress_repro = -1;
  if (sgp_config->TRACK_PARENT_TASKS()) {
    host_baby->GetCPU().state.parent_tasks_performed->Import(*GetCPU().state.tasks_performed);

    for (int i = 0; i < CPU_BITSET_LENGTH; i++) {
      host_baby->GetCPU().state.task_change_lose[i] = cpu.state.task_change_lose[i];
      host_baby->GetCPU().state.task_change_gain[i] = cpu.state.task_change_gain[i];
      // lineage task gain / loss
      if (cpu.state.tasks_performed->Get(i) && !cpu.state.parent_tasks_performed->Get(i)) {
        // child gains the ability to be infected by syms whose parents have done this task
        host_baby->GetCPU().state.task_change_gain[i] = cpu.state.task_change_gain[i] + 1;
      }
      else if (!cpu.state.tasks_performed->Get(i) && cpu.state.parent_tasks_performed->Get(i)) {
        // child loses the ability to be infected by syms with whom this parent had only this task in common 
        host_baby->GetCPU().state.task_change_lose[i] = cpu.state.task_change_lose[i] + 1;
      }

      // divergence from/convergence towards parent's partner
      host_baby->GetCPU().state.task_from_partner[i] = cpu.state.task_from_partner[i];
      host_baby->GetCPU().state.task_toward_partner[i] = cpu.state.task_toward_partner[i];
      if (HasSym()) {
        emp::Ptr<emp::BitSet<CPU_BITSET_LENGTH>> symbiont_tasks = syms[0].DynamicCast<SGPSymbiont>()->GetCPU().state.parent_tasks_performed;
        if (cpu.state.parent_tasks_performed->Get(i) != symbiont_tasks->Get(i) &&
          cpu.state.tasks_performed->Get(i) == symbiont_tasks->Get(i)) {
          // parent != partner and child == partner
          host_baby->GetCPU().state.task_toward_partner[i] = cpu.state.task_toward_partner[i] + 1;
        }
        else if (cpu.state.parent_tasks_performed->Get(i) == symbiont_tasks->Get(i) &&
          cpu.state.tasks_performed->Get(i) != symbiont_tasks->Get(i)) {
          // parent == partner and child != partner
          host_baby->GetCPU().state.task_from_partner[i] = cpu.state.task_from_partner[i] + 1;
        }
      }
    }
  }
  return host_baby;
}


#endif