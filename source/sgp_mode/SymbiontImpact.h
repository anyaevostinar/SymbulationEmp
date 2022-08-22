#ifndef PHENOTYPE_ANALYSIS_H
#define PHENOTYPE_ANALYSIS_H

#include "SGPHost.h"
#include "SGPSymbiont.h"
#include <cstddef>
#include <emp/Evolve/World_structure.hpp>

const size_t SYM_CHECK_UPDATES = 100;

/**
 * Input: A copy of the host and symbiont to check.
 *
 * Output: A measure of how mutualistic the symbiont is, going from very
 * mutualistic (1.0) to very parasitic (-1.0).
 *
 * Purpose: Runs the host and symbiont in a test environment to determine the
 * effect the symbiont has on the host.
 */
double CheckSymbiont(SGPHost host, SGPSymbiont symbiont,
                     const SGPWorld &world) {
  host.ClearSyms();
  host.ClearReproSyms();
  host.SetPoints(0.0);
  for (size_t i = 0; i < SYM_CHECK_UPDATES; i++) {
    // world.GetUpdate() isn't changing since the whole world isn't being
    // updated, so we need to do this resetting manually
    if (i % (30 / world.GetConfig()->CYCLES_PER_UPDATE()) == 0)
      host.GetCPU().state.used_resources->reset();
    host.GetCPU().RunCPUStep(emp::WorldPosition::invalid_id,
                             world.GetConfig()->CYCLES_PER_UPDATE());
  }
  double no_sym = host.GetPoints();
  host.GetCPU().Reset();
  host.SetPoints(0.0);
  host.AddSymbiont(&symbiont);
  symbiont.SetPoints(0.0);
  for (size_t i = 0; i < SYM_CHECK_UPDATES; i++) {
    if (i % (30 / world.GetConfig()->CYCLES_PER_UPDATE()) == 0)
      host.GetCPU().state.used_resources->reset();
    // Instead of randomly ordering the host and symbiont (like in
    // Host::Process()), alternate every update. That way it's deterministic and
    // we don't need to have access to the emp::Random.
    if (i % 2 == 0) {
      host.GetCPU().RunCPUStep(emp::WorldPosition::invalid_id,
                               world.GetConfig()->CYCLES_PER_UPDATE());
    }
    symbiont.GetCPU().RunCPUStep(emp::WorldPosition::invalid_id,
                                 world.GetConfig()->CYCLES_PER_UPDATE());
    if (i % 2 != 0) {
      host.GetCPU().RunCPUStep(emp::WorldPosition::invalid_id,
                               world.GetConfig()->CYCLES_PER_UPDATE());
    }
  }
  double with_sym = host.GetPoints();
  // The host can't free the symbiont pointer because it's not heap-allocated
  host.ClearSyms();
  double change = (with_sym - no_sym) / fmax(fmax(no_sym, with_sym), 1);
  return change;
}

#endif