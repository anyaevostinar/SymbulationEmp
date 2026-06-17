#ifndef WORLD_SETUP_C
#define WORLD_SETUP_C

#include "SymWorld.h"
#include "Host.h"
#include "Symbiont.h"
#include "SpatialStructure.h"

#include "../../Empirical/include/emp/datastructs/map_utils.hpp"

/**
 * Input: The number of hosts.
 *
 * Output: None.
 *
 * Purpose: To populate the world with hosts with appropriate phenotypes.
 */
void SymWorld::SetupHosts(long unsigned int* POP_SIZE){
  for (size_t i = 0; i < *POP_SIZE; i++) {
    emp::Ptr<Host> new_org;
    new_org.New(&GetRandom(), this, my_config, my_config->HOST_INT());
    if (my_config->TAG_MATCHING()) {
      emp::BitSet<TAG_LENGTH> new_tag = emp::BitSet<TAG_LENGTH>(GetRandom(), my_config->STARTING_TAGS_ONE_PROB());
      new_org->SetTag(new_tag);
    }
    InjectHost(new_org);
  }
}


/**
 * Input: The number of symbionts.
 *
 * Output: None.
 *
 * Purpose: To populate the world with symbionts with appropriate phenotypes.
 */
void SymWorld::SetupSymbionts(long unsigned int *total_syms) {
  for (size_t j = 0; j < *total_syms; j++) {
    emp::Ptr<Symbiont> new_sym = emp::NewPtr<Symbiont>(&GetRandom(), this, my_config, my_config->SYM_INT(), 0);
    if (my_config->TAG_MATCHING()) {
      emp::BitSet<TAG_LENGTH> new_tag = emp::BitSet<TAG_LENGTH>(GetRandom(), my_config->STARTING_TAGS_ONE_PROB());
      new_sym->SetTag(new_tag); // if this sym is hosted, this tag will be overwritten upon injection
    }
    InjectSymbiont(new_sym);
  }
}

/**
 * Input: None.
 *
 * Output: None.
 *
 * Purpose: Prepare the world for an experiment by applying the configuration settings
 * and populating the world with hosts and symbionts.
 */
void SymWorld::Setup() {
  double start_moi = my_config->START_MOI();
  long unsigned int POP_SIZE;
  SetupSpatialStructure();

  SetupHosts(&POP_SIZE);

  long unsigned int total_syms = POP_SIZE * start_moi;
  SetupSymbionts(&total_syms);
}

void SymWorld::SetupSpatialStructure() {
  const std::string& cfg_spatial_struct_mode = my_config->SPATIAL_STRUCT_MODE();
  // Check if spatial structure mode is valid.
  // If not, provide some user feedback.
  const bool valid_cfg = emp::Has(spatial_struct_mode_cfg_mapping, cfg_spatial_struct_mode);
  if (!valid_cfg) {
    std::cout << "Invalid input for SPATIAL_STRUCT_MODE (" << cfg_spatial_struct_mode << ")." << std::endl;
    std::cout << "  Valid options include: " << std::endl;
    for (const auto& [cfg_name, mode] : spatial_struct_mode_cfg_mapping) {
      std::cout << "    " << cfg_name << std::endl;
    }
    exit(-1);
  }
  // Set world's spatial structure mode.
  spatial_struct_mode = spatial_struct_mode_cfg_mapping.at(cfg_spatial_struct_mode);

  switch (spatial_struct_mode) {
    case SPATIAL_STRUCT_MODE::WELL_MIXED:
      SetupSpatialStructure_WellMixed();
      break;
    case SPATIAL_STRUCT_MODE::GRID:
      SetupSpatialStructure_Grid();
      break;
    case SPATIAL_STRUCT_MODE::LOAD:
      SetupSpatialStructure_Load();
      break;
    default:
      emp_error("Given spatial structure mode undefined.");
      break;
  }

  // // set world structure (either mixed or a grid with some dimensions) and set synchronous generations to false
  // if (my_config->SpatialStructure() == "WellMixed") {

  //   SetPopStruct_Mixed(false);

  // }
  // else{
  //   fun_find_birth_pos =
  //   [this](emp::Ptr<Organism>, emp::WorldPosition parent_pos) {
  //     int pos = GetNeighborCellID(parent_pos.GetIndex());

  //     // Ensure the position is valid
  //     if (pos < 0) {
  //       return emp::WorldPosition();
  //     }

  //     return emp::WorldPosition(pos);
  //   };

  //   if (my_config->SpatialStructure() == "Grid") {
  //     SetPopStruct_Grid(my_config->GRID_X(), my_config->GRID_Y(), false);
  //     ConfigureToroidalGrid(spatial_structure, my_config->GRID_X(), my_config->GRID_Y());
  //   }
  //   else if (my_config->SpatialStructure() == "LoadFile") {
  //     SetPopStruct_Grid(my_config->GRID_X(), my_config->GRID_Y(), false);
  //     spatial_structure.LoadStructureFromMatrix(my_config->LoadFile());
  //   }
  //   else {
  //     std::cerr << "Invalid Spatial Structure config option: " << my_config->SpatialStructure() << std::endl;
  //     exit(1);
  //   }
  // }
  // // Change in here for spatial structure as pop size is NOT equal to grid size
  // if (my_config->INIT_POP_SIZE() == -1) {
  //   // This could be changed to look for load file, and load files pulls for the Spatial Struc, while grid and well mixed are handled differently
  //   if (my_config->SpatialStructure() != "WellMixed") {
  //     POP_SIZE = spatial_structure.GetNumPositions();
  //   }
  //   else {
  //     POP_SIZE = my_config->GRID_X() * my_config->GRID_Y();
  //   }
  // }
  // else {
  //   // Should probaly check that the pop size is not as large as the size of the spatial struct
  //   // Also we might want to indicate where each pop starts in spatial structs
  //   POP_SIZE = my_config->INIT_POP_SIZE();
  // }
  // spatial_structure.Print();
  // Resize(my_config->GRID_X(), my_config->GRID_Y());
}


void SymWorld::SetupSpatialStructure_WellMixed() {
  // Resize world to maximum population size
  const size_t max_world_size = my_config->WORLD_WIDTH() * my_config->WORLD_HEIGHT();
  Resize(max_world_size);
  // Set world structure to well-mixed with asynchronous generations.
  SetPopStruct_Mixed(false);
}


void SymWorld::SetupSpatialStructure_Grid() {
  // Resize world to maximum population size
  const size_t world_width = my_config->WORLD_WIDTH();
  const size_t world_height = my_config->WORLD_HEIGHT();
  Resize(world_width, world_height);
  // Set world structure to grid with asynchronous generations.
  // NOTE: set pop struct grid calls resize
  SetPopStruct_Grid(world_width, world_height, false);
}


void SymWorld::SetupSpatialStructure_Load() {
  const std::string& load_mode = my_config->SPATIAL_STRUCT_LOAD_MODE();
  if (load_mode == "matrix") {
    spatial_structure.LoadStructureFromMatrix(my_config->SPATIAL_STRUCT_CFG_PATH());
  } else if (load_mode == "edges") {
    spatial_structure.LoadStructureFromEdgeCSV(my_config->SPATIAL_STRUCT_CFG_PATH());
  } else {
    std::cout << "Unknown spatial structure load mode (" << load_mode << ")" << std::endl;
    exit(-1);
  }
  // Initial population size should not exceed world size
  emp_assert(my_config->INIT_POP_SIZE() <= spatial_structure.GetNumPositions());
  // Resize world according to spatial structure max size
  // Resize(max_world_size);
  SetPopStruct_Custom(false);
}


#endif
