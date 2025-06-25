#ifndef TASKS_C
#define TASKS_C

#include "Tasks.h"
#include "SGPWorld.h"

void Task::MarkPerformed(CPUState &state, uint32_t output, size_t task_id,
                         bool shared) {

  
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
        state.used_resources->Set(task_id);


          if (dependencies.size()) {
            size_t total = num_dep_completes;
            for (size_t i : dependencies) {
              // Subtract just as much as needed from each dependency until we've
              // accumulated `num_dep_completes` completions
              size_t subtract = std::min(state.available_dependencies[i], total);
              total -= subtract;
              state.available_dependencies[i] -= subtract;

              subtract = std::min((*state.shared_available_dependencies)[i], total);
              total -= subtract;
              (*state.shared_available_dependencies)[i] -= subtract;

              if (total == 0) {
                break;
              }
            }
        }

        if (shared) {
          (*state.shared_available_dependencies)[task_id]++;
        } else {
          
          state.available_dependencies[task_id]++;
          
        }
    }
  }
  else{
        
        //Hosts and Symbionts get credit for all tasks they complete

        state.tasks_performed->Set(task_id);
        state.used_resources->Set(task_id);


        if (dependencies.size()) {
          size_t total = num_dep_completes;
          for (size_t i : dependencies) {
            // Subtract just as much as needed from each dependency until we've
            // accumulated `num_dep_completes` completions
            size_t subtract = std::min(state.available_dependencies[i], total);
            total -= subtract;
            state.available_dependencies[i] -= subtract;

            subtract = std::min((*state.shared_available_dependencies)[i], total);
            total -= subtract;
            (*state.shared_available_dependencies)[i] -= subtract;

            if (total == 0) {
              break;
            }
          }
        }

        if (shared) {
          (*state.shared_available_dependencies)[task_id]++;
        } else {
          state.available_dependencies[task_id]++;
        }
        }

  
}

#endif