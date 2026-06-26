#ifndef SYM_WORLD_H
#define SYM_WORLD_H

#include "SpatialStructure.h"

#include "../../Empirical/include/emp/Evolve/World.hpp"
#include "../../Empirical/include/emp/data/DataFile.hpp"
#include "../../Empirical/include/emp/Evolve/Systematics.hpp"
#include "../../Empirical/include/emp/math/random_utils.hpp"
#include "../../Empirical/include/emp/math/Random.hpp"
#include "../../Empirical/include/emp/matching/MatchBin.hpp"

#include "../Organism.h"

#include <cstdlib>
#include <set>
#include <math.h>
#include <unordered_map>

namespace taxon_t {
  using info_t = double;

  using base_taxon_t = emp::Taxon<info_t, datastruct::TaxonDataBase>;
  using host_taxon_t = emp::Taxon<info_t, datastruct::HostTaxonData>;
  using sym_taxon_t = emp::Taxon<info_t, datastruct::SymbiontTaxonData>;
}

class SymWorld : public emp::World<Organism> {
public:
  // takes an organism (to classify), and returns an int (the org's taxon)
  using base_world_t = emp::World<Organism>;
  using fun_calc_info_t = std::function<taxon_t::info_t(Organism &)>;
  using tag_t = emp::BitSet<TAG_LENGTH>;
  using tag_metric_t = emp::BaseMetric<tag_t, tag_t>;
  using pop_t = typename emp::World<Organism>::pop_t;
  using host_systematics_t = emp::Systematics<Organism, taxon_t::info_t, datastruct::HostTaxonData>;
  using sym_systematics_t = emp::Systematics<Organism, taxon_t::info_t, datastruct::SymbiontTaxonData>;

  // AML TODO: UPDATE pop mode tests
  // AML TODO: +phylogeny mode tests
  // AML TODO: +tag metric mode tests
  enum class SPATIAL_STRUCT_MODE { WELL_MIXED, GRID, LOAD };
  static const std::unordered_map<std::string, SPATIAL_STRUCT_MODE> spatial_struct_mode_cfg_mapping;

  enum class PHYLO_TAXON_TYPE { INTERACTION_VALUE_BINNED, INTERACTION_VALUE_EXACT, TAG, INDIVIDUAL };
  static const std::unordered_map<std::string, PHYLO_TAXON_TYPE> phylo_taxon_type_cfg_mapping;

  enum class TAG_METRIC_TYPE { HAMMING, STREAK, HASH };
  static const std::unordered_map<std::string, TAG_METRIC_TYPE> tag_metric_type_cfg_mapping;

protected:


  /**
    *
    * Purpose: Represents the total resources in the world. This can be set with SetTotalRes()
    *
  */
  int total_res = -1;

  /**
    *
    * Purpose: Represents the free living sym environment, parallel to "pop" for hosts
    *
  */
  pop_t sym_pop;

  /**
    *
    * Purpose: Represents the set of organisms which have been unlinked from
    * their standard managing structures and need to be deleted at the end
    * of every update.
    *
  */
  emp::vector<emp::Ptr<Organism>> graveyard = {};

  /**
    *
    * Purpose: Represents a standard function object which determines which taxon an organism belongs to.
    *
  */
  fun_calc_info_t calc_host_info_fun;

  /**
    *
    * Purpose: Represents a standard function object which determines which taxon a symbiont belongs to.
    *
  */
  fun_calc_info_t calc_sym_info_fun;


  /**
    *
    * Purpose: Represents the configuration settings for a particular run.
    *
  */
  emp::Ptr<SymConfigBase> my_config = NULL;

  /**
    *
    * Purpose: Represents the systematics object tracking hosts.
    *
  */
  emp::Ptr<host_systematics_t> host_sys;

  /**
    *
    * Purpose: Represents the systematics object tracking symbionts.
    *
  */
  emp::Ptr<sym_systematics_t> sym_sys;

  /**
   * Purpose: Tracks world configuration for phylogeny taxon type.
   */
  PHYLO_TAXON_TYPE phylo_taxon_type;

  /**
    *
    * Purpose: Represents the tag distance calculator.
    *
  */
  emp::Ptr<tag_metric_t> tag_metric;

  /**
   * Purpose: Tracks world configuration for tag metric type.
   */
  TAG_METRIC_TYPE tag_metric_type;

  /**
   *
   * Purpose: Maintains population's spatial structure represented as a graph,
   *          except when using a well-mixed population (this is not used to
   *          store a fully connected graph for efficiency reasons).
   */
  SpatialStructure spatial_structure;

  /**
   * Purspose: Stores which spatial structure mode the world is configured as.
   */
  SPATIAL_STRUCT_MODE spatial_struct_mode;

  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_hostintval; // New() reallocates this pointer
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_symintval;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_freesymintval;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_hostedsymintval;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_syminfectchance;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_freesyminfectchance;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_hostedsyminfectchance;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_tag_dist;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_within_host_variance; // for alpha diversity
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_within_host_mean; // for beta diversity
  emp::Ptr<emp::DataMonitor<size_t>> data_node_host_repro_count;
  emp::Ptr<emp::DataMonitor<size_t>> data_node_sym_repro_count;
  emp::Ptr<emp::DataMonitor<double>> data_node_host_towards_partner_rate;
  emp::Ptr<emp::DataMonitor<double>> data_node_host_from_partner_rate;
  emp::Ptr<emp::DataMonitor<double>> data_node_sym_towards_partner_rate;
  emp::Ptr<emp::DataMonitor<double>> data_node_sym_from_partner_rate;
  emp::Ptr<emp::DataMonitor<double>> data_node_host_permissiveness;
  emp::Ptr<emp::DataMonitor<int>> data_node_host_tag_richness;
  emp::Ptr<emp::DataMonitor<double>> data_node_host_tag_shannon;
  emp::Ptr<emp::DataMonitor<int>> data_node_symbiont_tag_richness;
  emp::Ptr<emp::DataMonitor<double>> data_node_symbiont_tag_shannon;
  emp::Ptr<emp::DataMonitor<int>> data_node_hostcount;
  emp::Ptr<emp::DataMonitor<int>> data_node_symcount;
  emp::Ptr<emp::DataMonitor<int>> data_node_freesymcount;
  emp::Ptr<emp::DataMonitor<int>> data_node_hostedsymcount;
  emp::Ptr<emp::DataMonitor<int>> data_node_uninf_hosts;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_attempts_horiztrans;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_tagfail_horiztrans;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_sizefail_horiztrans;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_successes_horiztrans;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_attempts_verttrans;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_successes_verttrans;

  // the taxon IDs of the first mutualistic pair (where BOTH sym and host are mutualistic)
  uint64_t first_mut_sym = 0;
  uint64_t first_mut_host = 0;

  emp::Signal<void()> on_analyze_population_sig;

  // SetupHosts and SetupSymbionts moved to protected because they assume that
  // SetupSpatialStructure has been called prior to hosts/symbionts getting setup.
  virtual void SetupHosts(long unsigned int* POP_SIZE);
  virtual void SetupSymbionts(long unsigned int* total_syms);

  /**
   * Purpose: Internal setup helper function to hold setup-time configuration for
   *          different spatial structure modes.
   */
  void SetupSpatialStructure();

  /**
   * Purpose: Internal setup helper function used by SetupSpatialStructure().
   */
  void SetupSpatialStructure_WellMixed();

  /**
   * Purpose: Internal setup helper function used by SetupSpatialStructure().
   */
  void SetupSpatialStructure_Grid();

  /**
   * Purpose: Internal setup helper function used by SetupSpatialStructure().
   */
  void SetupSpatialStructure_Load();

  void SetupPhylogenyTracking();
  void SetupTagMatching();

public:
  /**
   * Input: The world's random seed and a pointer to this world's config object
   *
   * Output: None
   *
   * Purpose: To construct an instance of SymWorld
   */
  SymWorld(emp::Random& _random, emp::Ptr<SymConfigBase> _config) : emp::World<Organism>(_random) {
    fun_print_org = [](Organism& org, std::ostream& os) {
      //os << PrintHost(&org);
      os << "This doesn't work currently";
    };
    my_config = _config;
    total_res = my_config->LIMITED_RES_TOTAL();

    emp_assert(!(my_config->TAG_MATCHING() && my_config->FREE_LIVING_SYMS()));

    if (my_config->PHYLOGENY()) {
      SetupPhylogenyTracking();
    }
    if (my_config->TAG_MATCHING()) {
      SetupTagMatching();
    }
  }


  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To destruct the objects belonging to SymWorld to conserve memory.
   */
  virtual ~SymWorld() {
    if (data_node_hostintval) data_node_hostintval.Delete();
    if (data_node_symintval) data_node_symintval.Delete();
    if (data_node_freesymintval) data_node_freesymintval.Delete();
    if (data_node_hostedsymintval) data_node_hostedsymintval.Delete();
    if (data_node_syminfectchance) data_node_syminfectchance.Delete();
    if (data_node_freesyminfectchance) data_node_freesyminfectchance.Delete();
    if (data_node_hostedsyminfectchance) data_node_hostedsyminfectchance.Delete();
    if (data_node_within_host_mean) data_node_within_host_mean.Delete();
    if (data_node_within_host_variance) data_node_within_host_variance.Delete();
    if (data_node_host_repro_count) data_node_host_repro_count.Delete();
    if (data_node_sym_repro_count) data_node_sym_repro_count.Delete();
    if (data_node_host_towards_partner_rate) data_node_host_towards_partner_rate.Delete();
    if (data_node_host_from_partner_rate) data_node_host_from_partner_rate.Delete();
    if (data_node_sym_towards_partner_rate) data_node_sym_towards_partner_rate.Delete();
    if (data_node_sym_from_partner_rate) data_node_sym_from_partner_rate.Delete();
    if (data_node_host_permissiveness) data_node_host_permissiveness.Delete();
    if (data_node_host_tag_richness) data_node_host_tag_richness.Delete();
    if (data_node_host_tag_shannon) data_node_host_tag_shannon.Delete();
    if (data_node_symbiont_tag_richness) data_node_symbiont_tag_richness.Delete();
    if (data_node_symbiont_tag_shannon) data_node_symbiont_tag_shannon.Delete();
    if (data_node_hostcount) data_node_hostcount.Delete();
    if (data_node_symcount) data_node_symcount.Delete();
    if (data_node_tag_dist) data_node_tag_dist.Delete();
    if (data_node_freesymcount) data_node_freesymcount.Delete();
    if (data_node_hostedsymcount) data_node_hostedsymcount.Delete();
    if (data_node_uninf_hosts) data_node_uninf_hosts.Delete();
    if (data_node_attempts_horiztrans) data_node_attempts_horiztrans.Delete();
    if (data_node_tagfail_horiztrans) data_node_tagfail_horiztrans.Delete();
    if (data_node_sizefail_horiztrans) data_node_sizefail_horiztrans.Delete();
    if (data_node_successes_horiztrans) data_node_successes_horiztrans.Delete();
    if (data_node_attempts_verttrans) data_node_attempts_verttrans.Delete();
    if (data_node_successes_verttrans) data_node_successes_verttrans.Delete();

    for (size_t i = 0; i < sym_pop.size(); i++) { //host population deletion is handled by empirical world destructor
      if (sym_pop[i]) {
        DoSymDeath(i);
      }
    }

    if (my_config->PHYLOGENY()) { //host systematic deletion is handled by empirical world destructor
      Clear(); // delete hosts here so that hosted symbionts get
      // deleted and unlinked from the sym_sys
      sym_sys.Delete();
    }

    if (my_config->TAG_MATCHING()) {
      tag_metric.Delete();
    }
  }

  /**
   * TODO: explain why overwritting empirical's mixed
   */
  void SetPopStruct_Mixed(bool synchronous_gen=false) {
    emp::World<Organism>::SetPopStruct_Mixed(synchronous_gen);
    // TODO: redefine inject to not grow pop

    // For well-mixed, we need to alter the Empirical World neighbor finding to not allow the current location to be returned.
    // H/t to Kai Johnson for suggestion to exclude upper cell and swap it in if needed
    // Neighbors are anywhere in the same population except the pos.
    fun_get_neighbor = [this](emp::WorldPosition pos) {
      if (pop.size() <= 1 ) return emp::WorldPosition(); // if there are no neighbors, return an invalid position
      // leave out the last cell and swap it in if potential_neighbor is the same as pos
      size_t potential_neighbor = GetRandomCellID(0, pop.size()-1);
      if (potential_neighbor == pos.GetIndex()) {
        potential_neighbor = pop.size() - 1;
      }
      return pos.SetIndex(potential_neighbor);
    };

    // Neighbors are anywhere in same population, so all organisms are neighbors except for the focal organism.
    // This might not actually need to be changed for SymWorld, but consistency seemed important
    fun_is_neighbor = [](emp::WorldPosition pos1, emp::WorldPosition pos2) {
      if ((pos1.GetPopID() == pos2.GetPopID()) && (pos1.GetIndex() == pos2.GetIndex())) return false;
      else return true;
    };
  }

  /**
   *
   */
  void SetPopStruct_Custom(bool synchronous_gen = false) {
    const size_t max_world_size = spatial_structure.GetNumPositions();
    Resize(max_world_size);
    // Mirrors Empirical world's SetPopStruct functions.
    pop_sizes.resize(0);
    is_synchronous = synchronous_gen;
    is_space_structured = true;
    is_pheno_structured = false;
    // -- Setup Functions --
    // Inject into a random position in the world
    fun_find_inject_pos = [this](emp::Ptr<Organism> new_org) {
      (void) new_org;
      return emp::WorldPosition(
        GetRandom().GetUInt(spatial_structure.GetNumPositions())
      );
    };
    // Setup neighbors function
    fun_get_neighbor = [this](emp::WorldPosition pos) {
      auto neighbor = spatial_structure.GetRandomNeighbor(GetRandom(), pos.GetIndex());
      // If no valid neighbors, return invalid position.
      if (!neighbor) {
        return emp::WorldPosition();
      }
      // Must be a valid neighbor.
      emp_assert(neighbor.value() < GetSize());
      return pos.SetIndex(neighbor.value());
    };

    fun_is_neighbor = [this](emp::WorldPosition pos1, emp::WorldPosition pos2) {
      // NOTE: Order matters for spatial_structure.
      const bool one_to_two = spatial_structure.IsConnected(
        pos1.GetIndex(),
        pos2.GetIndex()
      );
      const bool two_to_one = spatial_structure.IsConnected(
        pos2.GetIndex(),
        pos1.GetIndex()
      );
      return one_to_two || two_to_one;
    };

    // NOTE: This is what's happening in other structure modes (copied from World's set structure functions),
    //        but do we actually want to override to use graveyard?
    fun_kill_org = [this]() {
      // const size_t kill_id = GetRandomCellID();
      const size_t kill_id = GetRandom().GetUInt(spatial_structure.GetNumPositions());
      emp_assert(kill_id < GetSize());
      RemoveOrgAt(kill_id);
      return kill_id;
    };

    // Adapted from World.h SetPopStruct_Grid
    if (synchronous_gen) {
      // Place births in a neighboring position in the new grid.
      fun_find_birth_pos = [this](
        emp::Ptr<Organism> new_org,
        emp::WorldPosition parent_pos
      ) {
        emp_assert(new_org);                                         // New organism must exist.
        emp::WorldPosition next_pos = fun_get_neighbor(parent_pos);  // Place near parent.
        return next_pos.SetPopID(1);                                 // Adjust position to next pop and place..
      };
      SetAttribute("SynchronousGen", "True");
    } else {
      // Asynchronous: always go to a neighbor in current population.
      fun_find_birth_pos = [this](
        emp::Ptr<Organism> new_org,
        emp::WorldPosition parent_pos
      ) {
        return emp::WorldPosition(fun_get_neighbor(parent_pos)); // Place org in existing population.
      };
      SetAttribute("SynchronousGen", "False");
    }

    SetAttribute("PopStruct", "Custom");
    SetSynchronousSystematics(synchronous_gen);
  }

  /**
   * Input: None
   *
   * Output: The pop_t value that represents the world's population.
   *
   * Purpose: To get the world's population of organisms.
   */
  // @AML NOTE: Should this return a copy of a vector or a reference?
  pop_t& GetPop() { return pop; }

  const pop_t& GetPop() const { return pop; }


  /**
   * Input: None
   *
   * Output: The pop_t value that represent the world's symbiont
   * population.
   *
   * Purpose: To get the world's symbiont population.
   */
  // @AML NOTE: Should this return a copy of a vector or a reference?
  pop_t& GetSymPop() { return sym_pop; }

  const pop_t& GetSymPop() const { return sym_pop; }

  /**
   * Input: None
   * Output: The world's spatial structure mode.
   * Purpose: Get the world's currently configured spatial structure mode.
   *          Spatial structure mode is configured on setup. It is intentionally
   *          not able to be modified by a setter function.
   */
  SPATIAL_STRUCT_MODE GetSpatialStructureMode() const { return spatial_struct_mode; }
  bool IsWellMixedPopStruct() { return spatial_struct_mode == SPATIAL_STRUCT_MODE::WELL_MIXED; }
  bool IsGridPopStruct() { return spatial_struct_mode == SPATIAL_STRUCT_MODE::GRID; }
  bool IsCustomPopStruct() { return spatial_struct_mode == SPATIAL_STRUCT_MODE::LOAD; }

  TAG_METRIC_TYPE GetTagMetricType() const { return tag_metric_type; }
  PHYLO_TAXON_TYPE GetPhylogenyTaxonType() const { return phylo_taxon_type; }

  /**
   * Input: A pointer to the tag distance metric object
   *
   * Output: None
   *
   * Purpose: To set the world's tag distance calculator
   */
  void SetTagMetric(emp::Ptr<tag_metric_t> _in) {
    tag_metric = _in;
  }

  /**
   * Input: None
   *
   * Output: A pointer to the tag distance metric object
   *
   * Purpose: To get the world's tag distance calculator
   */
  emp::Ptr<tag_metric_t> GetTagMetric() {
    return tag_metric;
  }

  double CalcTagMetric(const tag_t& tag_a, const tag_t& tag_b) const {
    return (*tag_metric)(tag_a, tag_b);
  }

  /**
   * Input: None
   *
   * Output: A reference to the world graveyard.
   *
   * Purpose: To get the world's graveyard.
   */
  emp::vector<emp::Ptr<Organism>>& GetGraveyard() { return graveyard; }

  /**
   * Input: None
   *
   * Output: The configuration used for this world.
   *
   * Purpose: Allows accessing the world's config.
   */
  const emp::Ptr<SymConfigBase> GetConfig() const { return my_config; }

  // AML: A little nicer to work with:
  // const SymConfigBase& GetConfig() const { return *my_config; }


  /**
   * Input: None
   *
   * Output: The boolean representing if vertical transmission will occur
   *
   * Purpose: To determine if vertical transmission will occur
   */
  bool WillTransmit() {
    bool result = GetRandom().GetDouble(0.0, 1.0) < my_config->VERTICAL_TRANSMISSION();
    return result;
  }


  /**
   * Input: None
   *
   * Output: The systematic object tracking hosts
   *
   * Purpose: To retrieve the host systematic
   */
  emp::Ptr<host_systematics_t> GetHostSys() {
    return host_sys;
  }


  /**
   * Input: None
   *
   * Output: The systematic object tracking hosts
   *
   * Purpose: To retrieve the symbiont systematic
   */
  emp::Ptr<sym_systematics_t> GetSymSys() {
    return sym_sys;
  }


  /**
   * Input: None
   *
   * Output: The standard function object that determines which bin hosts
   * should belong to depending on their interaction value
   *
   * Purpose: To classify hosts based on their interaction value.
   */
  fun_calc_info_t GetCalcHostInfoFun() {
    // NOTE: Probably don't want one of the taxon type modes defined separately from
    // others?
    emp_assert(calc_host_info_fun);
    return calc_host_info_fun;
  }

  /**
   * Input: None
   *
   * Output: The standard function object that determines which bin symbionts
   * should belong to depending on their interaction value
   *
   * Purpose: To classify symbionts based on their interaction value.
   */
  fun_calc_info_t GetCalcSymInfoFun() {
    emp_assert(calc_sym_info_fun);
    // By default the sym info function is the same as the host one,
    // but separating them allows us to change the sym info function
    // to something else if we need to.
    if (!calc_sym_info_fun) {
      calc_sym_info_fun = GetCalcHostInfoFun();
    }
    return calc_sym_info_fun;
  }

  /**
   * Input: The symbiont to be added to the systematic
   *
   * Output: the taxon the symbiont is added to.
   *
   * Purpose: To add a symbiont to the systematic and to set it to track its taxon
   */
  emp::Ptr<taxon_t::base_taxon_t> AddSymToSystematic(
    emp::Ptr<Organism> sym,
    emp::Ptr<taxon_t::base_taxon_t> parent_taxon=nullptr
  ) {
    emp::Ptr<taxon_t::base_taxon_t> taxon = sym_sys->AddOrg(
      *sym,
      emp::WorldPosition(0, 0),
      parent_taxon.Cast<taxon_t::sym_taxon_t>()
    ).Cast<taxon_t::base_taxon_t>();
    sym->SetTaxon(taxon);
    return taxon;
  }


  /**
   * Input: The amount of resources an organism wants from the world.
   *
   * Output: If there are unlimited resources or the total resources are greater than those requested,
   * returns the amount of desired resources.
   * If total_res is less than the desired resources, but greater than 0,
   * then total_res will be returned. If none of these are true, then 0 will be returned.
   *
   * Purpose: To determine how many resources to distribute to each organism.
   */
  float PullResources(float desired_resources) {
    // if LIMITED_RES_TOTAL == -1, unlimited, even if limited resources was on before
    if (total_res == -1 || my_config->LIMITED_RES_TOTAL() == -1) {
      return desired_resources;
    } else {
      if (total_res>=desired_resources) {
        total_res = total_res - desired_resources;
        return desired_resources;
      } else if (total_res>0) {
        float resources_to_return = total_res;
        total_res = 0.0;
        return resources_to_return;
      } else {
        return 0.0;
      }
    }
  }


  /**
   * Input: The size_t representing the world's new width;
   * the size_t representing the world's new height.
   *
   * Output: None
   *
   * Purpose: To overwrite the Empirical resize so that sym_pop is also resized
   */
  void Resize(size_t new_width, size_t new_height) {
    const size_t new_size = new_width * new_height;
    Resize(new_size);
    pop_sizes.resize(2);
    pop_sizes[0] = new_width; pop_sizes[1] = new_height;
  }


  /**
   * Input: The size_t representing the new size of the world
   *
   * Output: None
   *
   * Purpose: To override the Empirical Resize function with
   * a single-arg method that can be used for AddOrgAt vector
   * expansions
   */
  void Resize(size_t new_size) {
    // TODO: Update to include organism removal?
    pop.resize(new_size);
    sym_pop.resize(new_size);
  }


  /**
   * Input: An organism pointer to add to the graveyard
   *
   * Output: None
   *
   * Purpose: To add organisms to the graveyard (also sets it to dead)
   */
  virtual void SendToGraveyard(emp::Ptr<Organism> org) {
    emp_assert(
      org != nullptr,
      "Tried to send a null organism to the graveyard."
    );
    org->SetDead();
    graveyard.push_back(org);
  }


  /**
   * Input: The pointer to the new organism;
   * the world position of the location to add
   * the new organism.
   *
   * Output: None
   *
   * Purpose: To overwrite the empirical AddOrgAt function to permit syms to
   * be added into sym_pop
   */
  void AddOrgAt(emp::Ptr<Organism> new_org, emp::WorldPosition pos, emp::WorldPosition p_pos=emp::WorldPosition()) {
    emp_assert(new_org);         // The new organism must exist.
    emp_assert(pos.IsValid());   // Position must be legal.
    new_org->SetLocation(pos);

    // SYMBIONTS have position in the overall world as their ID
    // HOSTS have position in the overall world as their index

    // If the pos is out of bounds, expand the worlds so that they can fit it.
    // AML NOTE: We don't want automatic world resizing for grid / custom spatial structure(?)
    //           The added organism will be isolated.
    if (pos.GetPopID() >= sym_pop.size() || pos.GetIndex() >= pop.size()) {
      emp_assert(IsWellMixedPopStruct(), "Should not dynamically grow population capacity outside of well-mixed population structure mode.");
      // Resize world to accomodate given pop ID (which ever is bigger between sym and host)
      Resize(emp::Max(pos.GetIndex(), pos.GetPopID()) + 1);
    }

    if (new_org->IsHost()) { // if the org is a host, use the empirical addorgat function
      emp::World<Organism>::AddOrgAt(new_org, pos, p_pos);
      if (new_org->HasSym()) {
        // Sometimes we add the symbionts before putting the organism into the world, which messes up the syms' location
        for (size_t j = 0; j < new_org->GetSymbionts().size(); j++) {
          emp::Ptr<Organism> cur_sym = new_org->GetSymbionts()[j];
          cur_sym->SetLocation(emp::WorldPosition(j+1, pos.GetIndex()));
        }
      }
    } else { // if it is not a host, then add it to the sym population
      // for symbionts, their place in their host's world is indicated by their ID
      size_t pos_id = pos.GetPopID();

      // run before-placement actions
      before_placement_sig.Trigger(*new_org, pos_id);

      // place symbiont
      if (!sym_pop[pos_id]) {
        ++num_orgs;
      } else {
        SendToGraveyard(sym_pop[pos_id]); // don't delete it yet, that can cause a seg fault
      }
      //set the cell to point to the new sym
      sym_pop[pos_id] = new_org;
    }
  }


  // Overriding World's DoBirth to take a pointer instead of a reference
  // Because it takes a pointer, it doesn't support birthing multiple copies
  /**
   * Input: (1) The pointer to the organism that is being birthed;
   * (2) The size_t location of the parent organism.
   *
   * Output: The WorldPosition of the position of the new organism.
   *
   * Purpose: To introduce new organisms to the world.
   */
  emp::WorldPosition DoBirth(emp::Ptr<Organism> new_org, emp::WorldPosition p_pos) {
    size_t parent_pos = p_pos.GetIndex();
    before_repro_sig.Trigger(parent_pos);
    emp::WorldPosition pos; // Position of each offspring placed.

    offspring_ready_sig.Trigger(*new_org, parent_pos);
    pos = fun_find_birth_pos(new_org, parent_pos);
    if (pos.IsValid() && (pos.GetIndex() != parent_pos)) {
      //Add to the specified position, overwriting what may exist there
      AddOrgAt(new_org, pos, parent_pos);
      if (my_config->PHYLOGENY() && my_config->TRACK_PHYLOGENY_INTERACTIONS()) {
        datastruct::TaxonDataBase& my_data = new_org->GetTaxon()->GetData();
        datastruct::HostTaxonData* d = static_cast<datastruct::HostTaxonData*>(&my_data);

        for (auto sym : new_org->GetSymbionts()) {
          d->AddInteraction(sym->GetTaxon());
          if (first_mut_host == 0 && new_org->GetIntVal() > 0 && sym->GetIntVal() > 0) {
            first_mut_host = new_org->GetTaxon()->GetID();
            first_mut_sym = sym->GetTaxon()->GetID();
          }
        }
      }
    } else {
      new_org.Delete();
    } // Otherwise delete the organism.
    return pos;
  }


  /**
   * Input: The world position of the host to perform death upon
   *
   * Output: None
   *
   * Purpose: To overwrite the empirical DoDeath function to permit cleanup
   * of false-start (<1 update duration) host taxa when unpruned trees are
   * being recorded.
   */
  void DoDeath(const emp::WorldPosition pos) {
    if (my_config->PHYLOGENY()) {
      emp::Ptr<taxon_t::host_taxon_t> taxon = host_sys->GetTaxonAt(pos);
      if (my_config->STORE_EXTINCT() && taxon->GetOriginationTime() == GetUpdate() && taxon->GetTotalOffspring() == 0) {
        host_sys->RemoveOrg(pos);
        host_sys->outside_taxa.erase(taxon);
        taxon.Delete();
      }
    }
    emp::World<Organism>::DoDeath(pos);
  }


  /**
   * Input: The size_t value representing the location whose neighbors
   * are being searched.
   *
   * Output: If there are no occupied neighboring positions, -1 will be returned.
   * If there are occupied neighboring positions, then the location of one
   * occupied position will be returned.
   *
   * Purpose: To determine the location of a valid occupied neighboring position.
   */
  // TODO: write updated test for different spatial structure modes
  int GetNeighborHost(size_t id) {
    // Attempt to use GetRandomNeighborPos first, since it's much faster
    for (size_t i = 0; i < 3; i++) {
      emp::WorldPosition neighbor = GetRandomNeighborPos(id);
      if (neighbor.IsValid() && IsOccupied(neighbor)) {
        return neighbor.GetIndex();
      }
    }

    // Then enumerate all occupied neighbors, in case many neighbors are unoccupied
    const emp::vector<size_t> valid_neighbors{GetValidNeighborOrgIDs(id)};
    if (valid_neighbors.empty()) {
      return -1;
    } else {
      const int rand_index = GetRandom().GetUInt(0, valid_neighbors.size());
      return valid_neighbors[rand_index];
    }
  }

  // Overwrite emp::World get valid neighbor org ids to account for different
  // spatial structure modes.
  /**
   * Purpose: returns vector of valid, occupied neighboring positions from position ID
   */
  emp::vector<size_t> GetValidNeighborOrgIDs(size_t id) {
    emp::vector<size_t> neighbor_ids;
    switch(spatial_struct_mode) {
      case SPATIAL_STRUCT_MODE::WELL_MIXED:
        // In well-mixed mode, use base neighbor organism ids
        return base_world_t::GetValidNeighborOrgIDs(id);
      case SPATIAL_STRUCT_MODE::GRID: {
        const size_t grid_width = my_config->WORLD_WIDTH();
        const size_t grid_height = my_config->WORLD_HEIGHT();
        using dir_t = grid2D_utils::GRID_DIR;
        emp_assert(GetSize() == grid_width * grid_height);
        // emp world uses a 8-neighborhood grid
        for (dir_t dir : grid2D_utils::grid_directions) {
          const size_t neighbor_id = grid2D_utils::GetGridNeighbor(
            id,
            dir,
            grid_width,
            grid_height
          );
          // This check is copied over from emp::World's version of this function.
          if ((bool) (pop[neighbor_id].Raw())) {
            neighbor_ids.emplace_back(neighbor_id);
          }
        }
        return neighbor_ids;
      }
      case SPATIAL_STRUCT_MODE::LOAD: {
        const auto& neighboring_positions = spatial_structure.GetNeighbors(id);
        for (size_t neighbor_id : neighboring_positions) {
          // This check is copied over from emp::World's version of this function.
          if ((bool) (pop[neighbor_id].Raw())) {
            neighbor_ids.emplace_back(neighbor_id);
          }
        }
        return neighbor_ids;
      }
      default:
        emp_error("Unknown spatial structure mode");
        return neighbor_ids;
    }
  }


  /**
   * Input: The pointer to a host that will be added to the world.
   *        This function assumes that the pop vector has been sized.
   *
   * Output: None
   *
   * Purpose: To add a host to the world at a random location.
   */
  void InjectHost(emp::Ptr<Organism> new_host) {
    AddOrgAt(new_host, emp::WorldPosition(GetRandomCellID()));
  }


  /**
   * Input: The pointer to an organism that will be injected into the world.
   *
   * Output: None
   *
   * Purpose: To add a symbiont to the world, either into a host or into a sym world cell.
   */
  void InjectSymbiont(emp::Ptr<Organism> new_sym) {
    size_t new_loc;
    if (my_config->PHYLOGENY()) {
      // NOTE: Is it intended to add to phylogeny even when inject fails?
      AddSymToSystematic(new_sym);
    }
    if (!my_config->FREE_LIVING_SYMS()) {
      new_loc = GetRandomOrgID();
      // If the position is acceptable, add the sym to the host in that position
      if (IsOccupied(new_loc)) {
        const bool success = pop[new_loc]->AddSymbiont(new_sym) != 0;
        if (success) {
          if (my_config->TAG_MATCHING()) {
            new_sym->SetTag(pop[new_loc]->GetTag());
          }
          if (my_config->PHYLOGENY() && my_config->TRACK_PHYLOGENY_INTERACTIONS()) {
            datastruct::HostTaxonData* d = static_cast<datastruct::HostTaxonData*>(&pop[new_loc]->GetTaxon()->GetData());
            d->AddInteraction(new_sym->GetTaxon());
          }
        }
      } else {
        new_sym.Delete();
      }
    } else {
      new_loc = GetRandomCellID();
      // if the position is within bounds, add the sym to it
      if (new_loc < sym_pop.size()) {
        AddOrgAt(new_sym, emp::WorldPosition(0, new_loc));
      } else {
        new_sym.Delete();
      }
    }
  }


  //Definitions of data node functions, expanded in DataNodes.h
  virtual void CreateDataFiles();
  void MapPhylogenyInteractions();
  void WritePhylogenyFile(const std::string& filename);
  void WriteOrgDumpFile(const std::string& filename);
  void WriteTagMatrixFile(const std::string& filename);
  void WriteDominantPhylogenyFiles(const std::string& filename);
  emp::Ptr<emp::Taxon<taxon_t::info_t>> GetDominantSymTaxon();
  emp::Ptr<emp::Taxon<taxon_t::info_t>> GetDominantHostTaxon();
  emp::vector<emp::Ptr<emp::Taxon<taxon_t::info_t>>> GetDominantFreeHostedSymTaxon();
  emp::DataFile& SetupSymIntValFile(const std::string & filename);
  emp::DataFile& SetupHostIntValFile(const std::string & filename);
  emp::DataFile& SetupFreeLivingSymFile(const std::string & filename);
  emp::DataFile& SetupReproHistFile(const std::string& filename);
  emp::DataFile& SetupTransmissionFile(const std::string& filename);
  emp::DataFile& SetupTagDistFile(const std::string& filename);
  emp::DataFile& SetupSymDiversityFile(const std::string& filename);
  virtual void SetupTransmissionFileColumns(emp::DataFile& file);
  virtual void SetupHostFileColumns(emp::DataFile& file);
  emp::DataMonitor<int>& GetHostCountDataNode();
  emp::DataMonitor<int>& GetSymCountDataNode();
  emp::DataMonitor<int>& GetCountHostedSymsDataNode();
  emp::DataMonitor<int>& GetCountFreeSymsDataNode();
  emp::DataMonitor<int>& GetUninfectedHostsDataNode();
  emp::DataMonitor<double, emp::data::Histogram>& GetHorizontalTransmissionAttemptCount();
  emp::DataMonitor<double, emp::data::Histogram>& GetHorizontalTransmissionTagFailCount();
  emp::DataMonitor<double, emp::data::Histogram>& GetHorizontalTransmissionSizeFailCount();
  emp::DataMonitor<double, emp::data::Histogram>& GetHorizontalTransmissionSuccessCount();
  emp::DataMonitor<double, emp::data::Histogram>& GetVerticalTransmissionAttemptCount();
  emp::DataMonitor<double, emp::data::Histogram>& GetVerticalTransmissionSuccessCount();
  emp::DataMonitor<size_t>& GetHostReproCountDataNode();
  emp::DataMonitor<size_t>& GetSymReproCountDataNode();
  emp::DataMonitor<double>& GetSymTowardsPartnerRateDataNode();
  emp::DataMonitor<double>& GetSymFromPartnerRateDataNode();
  emp::DataMonitor<double>& GetHostTowardsPartnerRateDataNode();
  emp::DataMonitor<double>& GetHostFromPartnerRateDataNode();
  emp::DataMonitor<double>& GetHostTagPermissiveness();
  emp::DataMonitor<int>& GetHostTagRichness();
  emp::DataMonitor<double>& GetHostTagShannonDiversity();
  emp::DataMonitor<int>& GetSymbiontTagRichness();
  emp::DataMonitor<double>& GetSymbiontTagShannonDiversity();
  emp::DataMonitor<double, emp::data::Histogram>& GetTagDistanceDataNode();
  emp::DataMonitor<double, emp::data::Histogram>& GetHostIntValDataNode();
  emp::DataMonitor<double, emp::data::Histogram>& GetSymIntValDataNode();
  emp::DataMonitor<double, emp::data::Histogram>& GetFreeSymIntValDataNode();
  emp::DataMonitor<double, emp::data::Histogram>& GetHostedSymIntValDataNode();
  emp::DataMonitor<double, emp::data::Histogram>& GetSymInfectChanceDataNode();
  emp::DataMonitor<double, emp::data::Histogram>& GetFreeSymInfectChanceDataNode();
  emp::DataMonitor<double, emp::data::Histogram>& GetHostedSymInfectChanceDataNode();
  emp::DataMonitor<double, emp::data::Histogram>& GetWithinHostMeanDataNode();
  emp::DataMonitor<double, emp::data::Histogram>& GetWithinHostVarianceDataNode();


  // Definitions of setup functions, expanded in WorldSetup.cc
  virtual void Setup();

  /**
   * Input: The pointer to the symbiont that is moving, the WorldPosition of its
   * current location.
   *
   * Output: The WorldPosition object describing the symbiont's new location (it describes an
   * invalid position if the symbiont is deleted during movement)
   *
   * Purpose: To move a symbiont into a new world position in the sym pop.
   */
  emp::WorldPosition MoveIntoNewFreeWorldPos(emp::Ptr<Organism> sym, emp::WorldPosition parent_pos) {
    size_t i = parent_pos.GetPopID();
    emp::WorldPosition indexed_id = GetRandomNeighborPos(i);
    emp::WorldPosition new_pos = emp::WorldPosition(0, indexed_id.GetIndex()); //GetRandomNeighborPos returns a WorldPosition with the chosen location in the index spot, but we use the pop id to track the location of the symbiont in the world, so we need to switch those around. The 0 means that this position is not in a host.

    if (IsInboundsPos(new_pos)) {
      sym->SetHost(nullptr);
      AddOrgAt(sym, new_pos, parent_pos);
      return new_pos;
    } else {
      sym.Delete();
      return emp::WorldPosition(); //lack of parameters results in invalid position
    }
  }

  /**
   * Input: The WorldPosition object to be checked.
   *
   * Output: Wether the input object is within world bounds.
   *
   * Purpose: To determine whether the location of free-living organisms
   * is within the bounds of the free-living worlds (the size of the pop and
   * sym_pop vectors).
   */
  bool IsInboundsPos(emp::WorldPosition pos) {
    if (!pos.IsValid()) {
      return false;
    } else if (pos.GetIndex() >= pop.size()) {
      return false;
    } else if (pos.GetPopID() >= sym_pop.size()) {
      return false;
    }
    return true;
  }


  /**
   * Input: The pointer to the organism that is being birthed, and the WorldPosition location
   * of the parent symbiont.
   *
   * Output: The WorldPosition object describing the position the symbiont was born into (index = position in a host, 0 for free living and offset by one for position in host
   * sym vector. id = position of self or host in sym_pop or pop vector). An invalid WorldPosition object is returned if the sym was killed.
   *
   * Purpose: To birth a new symbiont. If free living symbionts is on, the new symbiont
   * can be put into an unoccupied place in the world. If not, then it will be placed
   * in a host near its parent's location, or deleted if the parent's location has
   * no eligible near-by hosts.
   */
   virtual emp::WorldPosition SymDoBirth(emp::Ptr<Organism> sym_baby, emp::WorldPosition parent_pos) {
    const size_t i = parent_pos.GetPopID();
    if (my_config->FREE_LIVING_SYMS() == 0) {
      const int new_host_pos = GetNeighborHost(i);
      if (new_host_pos > -1) { //-1 means no living neighbors
        emp::Ptr<Organism> sym_parent;
        if (parent_pos.GetIndex() == 0) { // free living parent
          sym_parent = GetSymAt(i);
        } else { // hosted parent
          emp_assert(pop[i]->HasSym() && pop[i]->GetSymbionts().size() >= (parent_pos.GetIndex() - 1));
          sym_parent = pop[i]->GetSymbionts().at(parent_pos.GetIndex() - 1);
        }

        // infections can fail from size limits or tag mismatch
        // (or, theoretically, no neighbouring hosts)
        const bool size_failed = pop[new_host_pos]->GetSymbionts().size() >= (long unsigned)my_config->SYM_LIMIT();
        bool tag_failed = false;
        if (my_config->TAG_MATCHING()) {
          const double tag_distance = (*tag_metric)(pop[new_host_pos]->GetTag(), sym_baby->GetTag()) * TAG_LENGTH;
          const double permissiveness_mean = (my_config->HOST_TAG_PERMISSIVENESS_EVOLVES()) ? pop[new_host_pos]->GetTagPermissiveness() : my_config->TAG_PERMISSIVENESS();
          const double cutoff = GetRandom().GetPoisson(permissiveness_mean * TAG_LENGTH);
          tag_failed = tag_distance > cutoff;
        }
        if (size_failed || tag_failed) {
          if (tag_failed && !size_failed) {
            GetHorizontalTransmissionTagFailCount().AddDatum(sym_parent->GetIntVal());
          }
          else if (!tag_failed && size_failed) {
            GetHorizontalTransmissionSizeFailCount().AddDatum(sym_parent->GetIntVal());
          }
          sym_baby.Delete();
          return emp::WorldPosition();
        }

        const int new_index = pop[new_host_pos]->AddSymbiont(sym_baby);

        if (new_index > 0) { // sym successfully infected
          if (my_config->PHYLOGENY()) {
            if (phylo_taxon_type == PHYLO_TAXON_TYPE::INDIVIDUAL) {
              sym_baby->GetTaxon().Cast<taxon_t::sym_taxon_t>()->GetData().DetermineHostSwitch(pop[new_host_pos]->GetTaxon(), sym_parent->GetHost()->GetTaxon());
            }
            if (my_config->TRACK_PHYLOGENY_INTERACTIONS()) {
              pop[new_host_pos]->GetTaxon().Cast<taxon_t::host_taxon_t>()->GetData().AddInteraction(sym_baby->GetTaxon());
            }
          }
          if (my_config->FREE_HT_FAILURE() || my_config->TAG_MATCHING()) {
            // if tag mismatch or free failure is on, don't subtract points until we think the infection is successful
            sym_parent->SetPoints(0);
          }
          return emp::WorldPosition(new_index, new_host_pos);
        } else { //sym got killed trying to infect
          return emp::WorldPosition();
        }
      } else { // no living neighbors
        sym_baby.Delete();
        return emp::WorldPosition();
      }
    } else {
      return MoveIntoNewFreeWorldPos(sym_baby, parent_pos);
    }
  }

  /**
   * Input: The WorldPosition location of the symbiont to be moved.
   *
   * Output: None
   *
   * Purpose: To move a symbiont, either into a host, or into a free world position
   */
  void MoveFreeSym(emp::WorldPosition pos) {
    size_t i = pos.GetPopID();
    //the sym can either move into a parallel host or to some random position
    if (IsOccupied(i) && sym_pop[i]->WantsToInfect()) {
      emp::Ptr<Organism> sym = ExtractSym(i);
      if (sym->InfectionFails()) { // if the sym tries to infect and fails it dies
        sym.Delete();
      } else {
        pop[i]->AddSymbiont(sym);
      }
    } else if (my_config->MOVE_FREE_SYMS()) {
      MoveIntoNewFreeWorldPos(ExtractSym(i), pos);
    }
  }

  /*
  * Input: The size_t location of the sym to be pointed to in the sym_pop.
  *
  * Output: A pointer to the sym.
  *
  * Purpose: To allow access to syms at a specified location in the sym_pop.
  */
  emp::Ptr<Organism> GetSymAt(size_t location) {
    if (location >= 0 && location < sym_pop.size()) {
      return sym_pop[location];
    } else {
      // NOTE: Do we want a throw here or emp_error?
      throw "Attempted to get out of bounds sym.";
    }
  }

  /**
   * Input: The size_t representing the location of the symbiont to be
   * extracted from the world's sym population.
   *
   * Output: The pointer to the organism that was extracted from the world. Pointer will be null if there was no sym at the location.
   *
   * Purpose: To extract a symbiont from the world without deleting it.
   */
  emp::Ptr<Organism> ExtractSym(size_t i) {
    emp::Ptr<Organism> sym;
    if (sym_pop[i]) {
      sym = sym_pop[i];
      num_orgs--;
      sym_pop[i] = nullptr;
    }
    return sym;
  }

  /**
   * Input: The size_t representing the location of the symbiont to be
   * deleted from the world's symbiont population.
   *
   * Output: None
   *
   * Purpose: To delete a symbiont from the world.
   */
  void DoSymDeath(size_t i) {
    if (sym_pop[i]) {
      sym_pop[i].Delete();
      sym_pop[i] = nullptr;
      num_orgs--;
    }
  }

  /**
  * Input: A size_t location to check in the symbiont population vector.
  *
  * Output: A boolean representing whether the the position is valid and
  * occupied by a free living symbiont/
  *
  * Purpose: To determine if a given index is valid and occipied in the symbiont
  * population vector.
  */
  bool IsSymPopOccupied(size_t pos) {
    return pos < sym_pop.size() && sym_pop[pos];
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To set all settings in the MUTATION group to 0 for the no-mutation updates.
  */
  // TODO: Add override to SGP_MODE!
  void SetMutationZero() {
    for (auto& group : my_config->GetGroupSet()) {
      if (group->GetName() == "MUTATION") {
        for (size_t i = 0; i < group->GetSize(); ++i) {
          auto setting = group->GetEntry(i);
          std::stringstream warnings;
          setting->SetValue("0", warnings);
          emp_assert(warnings.str().empty());
        }
      }
    }
  }

  /**
   * Input: A function to run after the experiment has finished but before any no mutation updates have been run.
   *
   * Output: A key representing the added function, which can usually be ignored.
   *
   * Purpose: Allow performing population-level analyses before running no mutation updates.
   */
  emp::SignalKey OnAnalyzePopulation(const std::function<void()>& fun) {
    return on_analyze_population_sig.AddAction(fun);
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Cure all hosts of symbionts
   */
  void CureHosts() {
    //loop through hosts and clear all symbionts
    for (size_t i = 0; i < pop.size(); i++) {
      // see if position in world is occupied
      if (!IsOccupied(i)) continue;
      auto& host_syms = pop[i]->GetSymbionts();
      for (size_t j = 0; j < host_syms.size(); j++) {
        host_syms[j]->SetDead();
        SendToGraveyard(host_syms[j]);
      }
      pop[i]->ClearSyms(); //TODO: should clear syms just handle setting dead and to graveyard?
    }
  }

  /**
   * Input: Optional boolean "verbose" that specifies whether to print the update numbers to standard output or not, defaults to true.
   *
   * Output: None
   *
   * Purpose: Run the number of updates and non-mutation updates specified in the configuration settings.
   */
  void RunExperiment(bool verbose=true) {
    //Loop through updates
    const int num_updates = my_config->UPDATES();
    for (int i = 0; i < num_updates; i++) {
      if (verbose && (i % my_config->DATA_INT()) == 0) {
        std::cout << "Update: "<< i << std::endl;
        std::cout.flush();
      }
      // Check CURE config
      if (my_config->CURE() && (size_t)i == my_config->CURE_UPDATES()) {
        CureHosts();
      }
      Update();
    }
    on_analyze_population_sig.Trigger();

    const int num_no_mut_updates = my_config->NO_MUT_UPDATES();
    if (num_no_mut_updates > 0) {
      SetMutationZero();
      // Make sure that hosts stay with their symbionts: we're looking for the dominant *pair*
      my_config->VERTICAL_TRANSMISSION(1);
      my_config->SYM_VERT_TRANS_RES(0);
    }

    for (int i = 0; i < num_no_mut_updates; i++) {
      if (verbose && (i % my_config->DATA_INT()) == 0) {
        std::cout << "No mutation update: "<< i << std::endl;
        std::cout.flush();
      }
      // Check CURE config
      if (my_config->CURE() && (size_t)i == my_config->CURE_UPDATES()) {
        CureHosts();
      }
      Update();
    }
  }

  /**
   * Get the top `config.DOMINANT_COUNT` organisms from the population, sorted by their abundance.
   */
  emp::vector<std::pair<emp::Ptr<Organism>, size_t>> GetDominantInfo() const {
    emp_assert(
      GetNumOrgs(),
      "called GetDominantInfo on an empty population"
    );

    struct virtual_less {
      bool operator() (const emp::Ptr<Organism> a, const emp::Ptr<Organism> b) const {
        return *a < *b;
      }
    };

    std::map<emp::Ptr<Organism>, size_t, virtual_less> counts;
    for (emp::Ptr<Organism> org_ptr : GetFullPop()) {
      if (org_ptr) ++counts[org_ptr];
    }
    emp::vector<std::pair<emp::Ptr<Organism>, size_t>> result(my_config->DOMINANT_COUNT());

    std::partial_sort_copy(
      std::begin(counts),
      std::end(counts),
      result.begin(),
      result.end(),
      [](const auto & p1, const auto & p2) {
        return p1.second > p2.second; // compare by counts, but we want the biggest first
      }
    );
    if (counts.size() <= result.size()) {
      result.resize(counts.size());
    }

    return result;
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To loop over and delete elements in the graveyard.
   */
  void CleanupGraveyard() {
    for (size_t i = 0; i < graveyard.size(); i++) {
      graveyard[i].Delete();
    }
    graveyard.clear();
  }


  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To simulate a timestep in the world, which includes calling the process functions for hosts and symbionts and updating the data nodes.
   */
  virtual void Update() {
    emp::World<Organism>::Update();

    // Handle resource inflow
    if (total_res != -1) {
      total_res += my_config->LIMITED_RES_INFLOW();
    }

    if (my_config->PHYLOGENY()) {
      sym_sys->Update(); //sym_sys is not part of the systematics vector, handle it independently

      if (update % my_config->PHYLOGENY_SNAPSHOT_INTERVAL() == 0) {
        // MapPhylogenyInteractions();
        const std::string file_ending = "_UPDATE" + std::to_string(update) + "_SEED" + std::to_string(my_config->SEED()) + ".data";
        WritePhylogenyFile(
          my_config->FILE_PATH() + "Phylogeny_" + my_config->FILE_NAME() + file_ending
        );
      }
    }
    emp::vector<size_t> schedule = emp::GetPermutation(GetRandom(), GetSize());
    // divvy up and distribute resources to host and symbiont in each cell
    for (size_t i : schedule) {
      if (IsOccupied(i) == false && !sym_pop[i]) { continue; } // no organism at that cell
      if (IsOccupied(i)) { // can't call GetDead on a deleted sym, so
        pop[i]->Process(i);
        if (pop[i]->GetDead()) { // Check if the host died
          DoDeath(i);
        }
      }
      if (sym_pop[i]) { // for sym movement reasons, syms are deleted the update after they are set to dead
        emp::WorldPosition sym_pos = emp::WorldPosition(0, i);
        if (sym_pop[i]->GetDead()) DoSymDeath(i); // Might have died since their last time being processed
        else sym_pop[i]->Process(sym_pos); // index 0, since it's freeliving, and id its location in the world
      }
    } // for each cell in schedule

    // clean up the graveyard
    CleanupGraveyard();

    // clean up systematics
    if (my_config->PHYLOGENY()) {
      host_sys->ClearRemoveAfterReproQueue();
      sym_sys->ClearRemoveAfterReproQueue();
    }
  } // Update()

}; // SymWorld class

const std::unordered_map<
  std::string,
  SymWorld::SPATIAL_STRUCT_MODE
> SymWorld::spatial_struct_mode_cfg_mapping = {
  {"well-mixed", SPATIAL_STRUCT_MODE::WELL_MIXED },
  {"grid", SPATIAL_STRUCT_MODE::GRID },
  {"load", SPATIAL_STRUCT_MODE::LOAD }
};

const std::unordered_map<
  std::string,
  SymWorld::PHYLO_TAXON_TYPE
> SymWorld::phylo_taxon_type_cfg_mapping = {
    {"interaction-value-binned", PHYLO_TAXON_TYPE::INTERACTION_VALUE_BINNED},
    {"interaction-value-exact", PHYLO_TAXON_TYPE::INTERACTION_VALUE_EXACT},
    {"tag", PHYLO_TAXON_TYPE::TAG},
    {"individual", PHYLO_TAXON_TYPE::INDIVIDUAL}
};

const std::unordered_map<
  std::string,
  SymWorld::TAG_METRIC_TYPE
> SymWorld::tag_metric_type_cfg_mapping = {
  {"hamming", TAG_METRIC_TYPE::HAMMING},
  {"streak", TAG_METRIC_TYPE::STREAK},
  {"hash", TAG_METRIC_TYPE::HASH}
};

#endif
