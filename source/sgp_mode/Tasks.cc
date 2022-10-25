#include "Tasks.h"
#include "SGPWorld.h"

void SquareTask::MarkPerformed(CPUState &state, uint32_t output, size_t task_id,
                               bool shared) {
  OutputTask::MarkPerformed(state, output, task_id, shared);
  state.internalEnvironment->insert(state.internalEnvironment->begin(),
                                    sqrt(output));
  // oh no, include cycle issues 😔
  auto &data_node = state.organism->IsHost()
                        ? state.world->data_node_host_squares
                        : state.world->data_node_sym_squares;
  std::lock_guard<std::mutex> lock(state.world->squares_mutex);
  data_node[output]++;
}
