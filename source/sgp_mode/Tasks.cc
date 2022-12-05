#include "Tasks.h"
#include "SGPWorld.h"

void Task::MarkPerformed(CPUState &state, uint32_t output, size_t task_id,
                         bool shared) {
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

void SquareTask::MarkPerformed(CPUState &state, uint32_t output, size_t task_id,
                               bool shared) {
  OutputTask::MarkPerformed(state, output, task_id, shared);
  state.internalEnvironment->insert(state.internalEnvironment->begin(),
                                    sqrt(output));
  auto &data_node = state.organism->IsHost()
                        ? state.world->data_node_host_squares
                        : state.world->data_node_sym_squares;
  std::lock_guard<std::mutex> lock(state.world->squares_mutex);
  data_node[output]++;
}
