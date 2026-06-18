#ifndef WORLD_SETUP_C
#define WORLD_SETUP_C

#include "SymWorld.h"
#include "Host.h"
#include "Symbiont.h"
#include "SpatialStructure.h"
#include "utils.h"

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
  // Configure population's spatial connectivity
  SetupSpatialStructure();
  // Calculate initial population size
  if (my_config->INIT_POP_SIZE() == -1) {
    // SetupSpatialStructure will have resized the world's pop size.
    POP_SIZE = GetSize();
  } else {
    POP_SIZE = my_config->INIT_POP_SIZE();
  }
  SetupHosts(&POP_SIZE);
  long unsigned int total_syms = POP_SIZE * start_moi;
  SetupSymbionts(&total_syms);
}

void SymWorld::SetupSpatialStructure() {
  const std::string& cfg_spatial_struct_mode = my_config->SPATIAL_STRUCT_MODE();
  // Check if spatial structure mode is valid.
  // If not, provide some user feedback.
  // NOTE: Best way to setup this validate function?
  //    Currently set to just exit on invalid input (i.e., don't want to continue
  //    running with misconfiguration)
  utils::ValidateConfigMode(
    spatial_struct_mode_cfg_mapping,
    "SPATIAL_STRUCT_MODE",
    cfg_spatial_struct_mode
  );
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
  Resize(spatial_structure.GetNumPositions());
  SetPopStruct_Custom(false);
}

void SymWorld::SetupPhylogenyTracking() {
  emp_assert(my_config->PHYLOGENY());
  const std::string& cfg_taxon_type = my_config->PHYLOGENY_TAXON_TYPE();
  // Check if cfg taxon type is valid.
  // If not, provide some user feedback.
  utils::ValidateConfigMode(
    phylo_taxon_type_cfg_mapping,
    "PHYLOGENY_TAXON_TYPE",
    cfg_taxon_type
  );
  phylo_taxon_type = phylo_taxon_type_cfg_mapping.at(cfg_taxon_type);
  // NOTE: For now, hosts / symbionts share taxon type mode.
  switch (phylo_taxon_type) {
    case PHYLO_TAXON_TYPE::INTERACTION_VALUE_BINNED:
      // NOTE: Moved this definition here to centralize where all of these modes are defined.
      calc_host_info_fun = [&](Organism& org) {
        const size_t num_phylo_bins = my_config->NUM_PHYLO_BINS();
        // classify orgs into bins base on interaction values,
        // inclusive of lower bound, exclusive of upper
        const float size_of_bin = 2.0 / num_phylo_bins;
        const double int_val = org.GetIntVal();
        float prog = (int_val + 1);
        prog = (prog/size_of_bin) + (0.0000000000001);
        size_t bin = (size_t) prog;
        // if (bin >= num_phylo_bins) bin = num_phylo_bins - 1;
        bin = num_phylo_bins - (size_t)(bin >= num_phylo_bins);
        return bin;
      };
      calc_sym_info_fun = calc_host_info_fun;
      break;
    case PHYLO_TAXON_TYPE::INTERACTION_VALUE_EXACT:
      calc_host_info_fun = [&](Organism& org) {
        return org.GetIntVal();
      };
      calc_sym_info_fun = [&](Organism& org) {
        return org.GetIntVal();
      };
      break;
    case PHYLO_TAXON_TYPE::TAG:
      calc_host_info_fun = [&](Organism& org) {
        return org.GetTag().GetValue();
      };

      calc_sym_info_fun = [&](Organism& org) {
        return org.GetTag().GetValue();
      };
      break;
    case PHYLO_TAXON_TYPE::INDIVIDUAL:
      calc_host_info_fun = [&](Organism& org) {
        return (long unsigned) host_sys->GetNextID();
      };

      calc_sym_info_fun = [&](Organism& org) {
        return (long unsigned) sym_sys->GetNextID();
      };
      break;
    default:
      emp_error("Unimplemented phylogeny taxon type.");
      break;
  }

  host_sys = emp::NewPtr<host_systematics_t>(GetCalcHostInfoFun());
  sym_sys = emp::NewPtr<sym_systematics_t>(GetCalcSymInfoFun());

  AddSystematics(host_sys);
  sym_sys->SetStorePosition(false);

  sym_sys->AddSnapshotFun([](const taxon_t::sym_taxon_t& t) { return std::to_string(t.GetInfo()); }, "info");
  host_sys->AddSnapshotFun([](const taxon_t::host_taxon_t& t) { return std::to_string(t.GetInfo()); }, "info");

  // NOTE: Why not output this for all modes? Would make analysis scripts easier to port from one exp to another.
  if (phylo_taxon_type == PHYLO_TAXON_TYPE::TAG || phylo_taxon_type == PHYLO_TAXON_TYPE::INDIVIDUAL) {
    sym_sys->AddSnapshotFun(
      [](const taxon_t::sym_taxon_t& t) { return std::to_string((t.GetData()).GetIntVal()); },
      "mean_int_val"
    );
    host_sys->AddSnapshotFun(
      [](const taxon_t::host_taxon_t& t) { return std::to_string(t.GetData().GetIntVal()); },
      "mean_int_val"
    );
  }
  if (phylo_taxon_type == PHYLO_TAXON_TYPE::INDIVIDUAL) {
    sym_sys->AddSnapshotFun(
      [](const taxon_t::sym_taxon_t& t) { return std::to_string(t.GetData().GetHostSwitch()); },
      "lineage_host_switch_count"
    );
  }

  // NOTE: Could move the if statement out of experiment runtime by adjusting the functor based on config
  /* Original code:
   on_placement_sig.AddAction(
    [this](emp::WorldPosition pos) {
      GetOrgPtr(pos.GetIndex())->SetTaxon(host_sys->GetTaxonAt(pos).Cast<taxon_t::base_taxon_t>());
      if (GetPhylogenyTaxonType() == PHYLO_TAXON_TYPE::INDIVIDUAL) {
        GetOrgPtr(pos.GetIndex())->GetTaxon()->GetData().RecordIntVal(GetOrgPtr(pos.GetIndex())->GetIntVal());
      }
    }
  );
  */
  if (GetPhylogenyTaxonType() == PHYLO_TAXON_TYPE::INDIVIDUAL) {
    on_placement_sig.AddAction(
      [this](emp::WorldPosition pos) {
        auto org_ptr = GetOrgPtr(pos.GetIndex());
        org_ptr->SetTaxon(host_sys->GetTaxonAt(pos).Cast<taxon_t::base_taxon_t>());
        org_ptr->GetTaxon()->GetData().RecordIntVal(GetOrgPtr(pos.GetIndex())->GetIntVal());
      }
    );
  } else {
    on_placement_sig.AddAction(
      [this](emp::WorldPosition pos) {
        GetOrgPtr(pos.GetIndex())->SetTaxon(host_sys->GetTaxonAt(pos).Cast<taxon_t::base_taxon_t>());
      }
    );
  }

  if (phylo_taxon_type == PHYLO_TAXON_TYPE::INDIVIDUAL) {
    std::function<void(emp::Ptr<taxon_t::sym_taxon_t >, Organism&)> inherit_parental_data =
      [&](emp::Ptr<taxon_t::sym_taxon_t > taxon, Organism& org) {
        if (taxon->GetParent()) {
          taxon->GetData().SetHostSwitch(taxon->GetParent()->GetData().GetHostSwitch());
        } else {
          taxon->GetData().SetHostSwitch(0);
        }
        taxon->GetData().RecordIntVal(org.GetIntVal());
      };
    sym_sys->OnNew(inherit_parental_data);
  }

  if (my_config->STORE_EXTINCT()) {
    sym_sys->SetStoreOutside(true);
    host_sys->SetStoreOutside(true);
  }
}

void SymWorld::SetupTagMatching() {
  emp_assert(my_config->TAG_MATCHING());
  const std::string& cfg_tag_metric = my_config->TAG_METRIC();
  // Check if cfg taxon type is valid.
  // If not, provide some user feedback.
  utils::ValidateConfigMode(
    tag_metric_type_cfg_mapping,
    "TAG_METRIC",
    cfg_tag_metric
  );
  tag_metric_type = tag_metric_type_cfg_mapping.at(cfg_tag_metric);

  if (my_config->NORMALIZE_TAG_DISTANCES()) {
    switch (tag_metric_type) {
      case TAG_METRIC_TYPE::HAMMING:
        tag_metric = emp::NewPtr<emp::UnifMod<emp::HammingMetric<TAG_LENGTH>>>();
        break;
      case TAG_METRIC_TYPE::STREAK:
        tag_metric = emp::NewPtr<emp::UnifMod<emp::StreakMetric<TAG_LENGTH>>>();
        break;
      case TAG_METRIC_TYPE::HASH:
        tag_metric = emp::NewPtr<emp::UnifMod<emp::HashMetric<TAG_LENGTH>>>();
        break;
      default:
        emp_error("Unimplemented tag metric type.");
        break;
    }
  } else {
    switch (tag_metric_type) {
      case TAG_METRIC_TYPE::HAMMING:
        tag_metric = emp::NewPtr<emp::HammingMetric<TAG_LENGTH>>();
        break;
      case TAG_METRIC_TYPE::STREAK:
        tag_metric = emp::NewPtr<emp::StreakMetric<TAG_LENGTH>>();
        break;
      case TAG_METRIC_TYPE::HASH:
        tag_metric = emp::NewPtr<emp::HashMetric<TAG_LENGTH>>();
        break;
      default:
        emp_error("Unimplemented tag metric type.");
        break;
    }
  }
}


#endif
