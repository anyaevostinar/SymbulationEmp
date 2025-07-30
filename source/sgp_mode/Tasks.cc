#ifndef TASKS_C
#define TASKS_C

#include "Tasks.h"
#include "SGPWorld.h"

//TODO: why can't this be in the header file?
void Task::MarkPerformed(CPUState &state, uint32_t output, size_t task_id) {
  if (state.world->GetConfig()->ONLY_FIRST_TASK_CREDIT()){
    //Iterates through all tasks and checks if any of them have been done
    bool donePreviousTask = false;
    for(int i = 0; i < CPU_BITSET_LENGTH; i++){
      if(state.tasks_performed->Get(i) != 0){
        donePreviousTask = true;
        break;
      }
    }
    //If no other tasks have been done, set the current task to have been completed 
    if(!donePreviousTask){
      state.tasks_performed->Set(task_id);
    }
  } else{
    //Hosts and Symbionts get credit for all tasks they complete
    state.tasks_performed->Set(task_id);
  }
}

#endif