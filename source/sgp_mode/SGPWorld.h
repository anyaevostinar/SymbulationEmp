#ifndef SGPWORLD_H
#define SGPWORLD_H

#include "../default_mode/SymWorld.h"
#include "Scheduler.h"
#include "SGPConfigSetup.h"
#include "SyncDataMonitor.h"
#include "spec.h"
#include "SGPHost.h"
#include "hardware/SGPHardwareSpec.h"
#include "hardware/GenomeLibrary.h"
#include "hardware/SGPHardware.h"
#include "SGPSymbiont.h"

#include "emp/Evolve/World_structure.hpp"
#include "emp/data/DataNode.hpp"
#include "emp/math/Random.hpp"


namespace sgpmode {

const size_t PROGRAM_LENGTH = 100;

// TODO - init necessary hardware state on organism birth (e.g., stack limit)
class SGPWorld : public SymWorld {
public:
  using hw_spec_t = SGPHardwareSpec<Library, CPUState<SGPWorld>, SGPWorld>;

protected:
  // TODO - scheduler could be SGP scheduler? It will only work with SGPWorld anyway?
  Scheduler scheduler;

  /**
    *
    * Purpose: Holds all configuration settings and points to same configuration
    * object as my_config from superclass, but with the correct subtype.
    *
    */
  //  emp::Ptr<SymConfigSGP> sgp_config = nullptr;
  SymConfigSGP& sgp_config;

  // ---- Internal helper functions ----
  void DoReproduction();
  // Internal helper function to handle host births.
  //   Handles both host do birth and triggering vertical transmission on any
  //   symbionts within the host.
  //   Need to pass in parent pointer because parent may no longer exist at the
  //   given world position when this function is called.
  emp::WorldPosition HostDoBirth(
    emp::Ptr<Organism> host_offspring_ptr,
    emp::Ptr<Organism> host_parent_ptr,
    emp::WorldPosition parent_pos
  );

  // Internal helper function to delete dead organisms in graveyard.
  void ProcessGraveyard();

public:
  SGPWorld(
    emp::Random& rnd,
    emp::Ptr<SymConfigSGP> _config
  ) :
    SymWorld(rnd, _config),
    scheduler(*this, _config->THREAD_COUNT()),
    sgp_config(*_config)
  {
    // TODO - taskset

  }

  ~SGPWorld() {
    /* TODO */
  }

  /* TODO - tasks */

  /**
   * Input: None
   *
   * Output: The sgp configuration used for this world.
   *
   * Purpose: Allows accessing the world's sgp config.
   */
  const SymConfigSGP& GetConfig() const { return sgp_config; }


  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To simulate a timestep in the world, which includes calling the
   * process functions for hosts and symbionts and updating the data nodes.
   */
  void Update() override {
    /* TODO */
  }

  // Prototypes for setup methods
  // TODO - distinguish between world configuration and population initialization
  //  Why? Tests, different mains configure initial population differently
  /**
   * Input: None.
   *
   * Output: None.
   *
   * Purpose: Prepare the SGPWorld for an experiment by applying the configuration settings
   * and populating the world with hosts and symbionts.
   */
  void Setup() override;
  void SetupHosts(long unsigned int* POP_SIZE) override;
  void SetupSymbionts(long unsigned int* total_syms) override;

  // Internal helper function to configure scheduler.
  // Called internally on world setup.
  void SetupScheduler();        // TODO - shift to private function (will need to refactor tests)
  void SetupSymReproduction();  // TODO - shift to private function (will need to refactor tests)
  void SetupHostReproduction(); // TODO - shift to private function (will need to refactor tests)
  void SetupHostSymInteractions(); // TODO - shift to private function (will need to refactor tests)

  // Prototypes for reproduction handling methods
  // SymDoBirth is for horizontal transmission and birthing free-living symbionts.
  emp::WorldPosition SymDoBirth(
    emp::Ptr<Organism> sym_baby,
    emp::WorldPosition parent_pos
  ) override;

  /**
   * Input: An organism pointer to add to the graveyard
   *
   * Output: None
   *
   * Purpose: To add organisms to the graveyard
   */
  void SendToGraveyard(emp::Ptr<Organism> org) override;

  void CreateDataFiles() override;
};

}

#endif