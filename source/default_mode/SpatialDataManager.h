#include "SpatialStructure.h"
#include "../Organism.h"

#include "emp/base/vector.hpp"
#include "emp/data/DataFile.hpp"
#include "emp/control/Signal.hpp"
#include "emp/tools/string_utils.hpp"

#include <functional>
#include <string>

// TODO:
// - [ ] Handle ecto syms (detect if running with ectosymbionts, add additional column if so)

// Columns:
// - Location
// - Host present
// - Endosymbionts present
// - Ectosymbionts present
// - host int val
// - endo int val(s)
// - ecto int val(s)

/**
 * Purpose: Manage data to be output to the spatial data file.
 * TODO: Move to separate file?
 *
 * NOTE: managed outside of Empirical's list of data files because
 *       we need output multiple lines per output update.
 *       we need to manage additional internal context for which location ID
 *        we're writing output for
 */
template<typename WORLD_T>
class SpatialDataManager {
public:
  using world_t = WORLD_T;

protected:
  bool setup = false;

  emp::Ptr<world_t> world_ptr = nullptr; // needed to access spatial structure

  size_t cur_location_id = 0;

  // struct {

  // } cur_location_data

  emp::Ptr<emp::DataFile> spatial_data_file;

  // std::filesystem::path fpath = output_dir / filename;
  // emp::DataFile snapshot_file(fpath.string());

  emp::Signal<void()> before_spatial_data_output_sig;

  void SetupSpatialDataFile();

public:
  SpatialDataManager() : setup(false) { }

  SpatialDataManager(emp::Ptr<world_t> world, const std::string& filepath) {
    Setup(world, filepath);
  }

  ~SpatialDataManager() {
    if (setup) {
      spatial_data_file.Delete();
    }
  }

  void Setup(emp::Ptr<world_t> world, const std::string& filepath) {
    if (setup) {
      spatial_data_file.Delete();
    }
    world_ptr = world;
    spatial_data_file = emp::NewPtr<emp::DataFile>(filepath);
    SetupSpatialDataFile();
    setup = true;
  }

  // TODO: can't use pre-function because this will trigger before *every* location
  //       update!
  void OnBeforeSpatialDataOutput(const std::function<void()>& fun) {
    emp_assert(setup);
    spatial_data_file->AddPreFun(fun);
  }

  // AddFunction
  //  - Will take location id, world as input,
  //  - will wrap with lambda that captures those from this context, passes as input

  void Update(size_t update) {
    emp_assert(setup);
    // Update file for each location
    for (cur_location_id = 0; cur_location_id < world_ptr->GetSize(); ++cur_location_id) {
      spatial_data_file->Update(update);
    }
  }

  emp::Ptr<emp::DataFile> GetDataFile() { return spatial_data_file; }

  void PrintHeaderKeys() {
    spatial_data_file->PrintHeaderKeys();
  }

}; // -- End SpatialDataManager class definition --

template<typename WORLD_T>
void SpatialDataManager<WORLD_T>::SetupSpatialDataFile() {
  const auto& world_config = *(world_ptr->GetConfig());
  // -- Update --
  spatial_data_file->AddFun<size_t>(
    [this]() -> size_t {
      return world_ptr->GetUpdate();
    },
    "update"
  );

  // -- Location --
  spatial_data_file->AddFun<size_t>(
    [this]() -> size_t {
      return cur_location_id;
    },
    "location_id"
  );

  // -- Host present at current location? --
  spatial_data_file->AddFun<size_t>(
    [this]() -> size_t {
      // IsOccupied returns if location is occupied by host
      return (size_t)world_ptr->IsOccupied(cur_location_id);
    },
    "host_present"
  );

  // -- Number of endosymbionts present at current location? --
  spatial_data_file->AddFun<size_t>(
    [this]() -> size_t {
      const bool occupied = world_ptr->IsOccupied(cur_location_id);
      if (occupied) {
        auto& host = world_ptr->GetOrg(cur_location_id);
        emp_assert(host.IsHost());
        return host.GetSymbionts().size();
      } else {
        return 0;
      }
    },
    "endosymbionts_present"
  );

  // -- Host interaction value at current location --
  spatial_data_file->AddFun<std::string>(
    [this]() -> std::string {
      const bool occupied = world_ptr->IsOccupied(cur_location_id);
      if (occupied) {
        const auto& org = world_ptr->GetOrg(cur_location_id);
        return emp::to_string(org.GetIntVal());
      } else {
        return "NONE";
      }
    },
    "host_interaction_value"
  );

  // -- Endosymbiont interaction value(s) at current location --
  spatial_data_file->AddFun<std::string>(
    [this]() -> std::string {
      const bool occupied = world_ptr->IsOccupied(cur_location_id);
      emp::vector<float> int_values;
      if (occupied) {
        auto& org = world_ptr->GetOrg(cur_location_id);
        emp::vector<emp::Ptr<Organism>>& syms = org.GetSymbionts();
        int_values.resize(syms.size(), 0.0);
        for (size_t i = 0; i < syms.size(); ++i) {
          int_values[i] = syms[i]->GetIntVal();
        }
      }
      return emp::to_string(int_values);
    },
    "endosym_interaction_values"
  );

  // If free-living syms are enabled, add relevant columns.
  if (world_config.FREE_LIVING_SYMS()) {
    // -- Ectosymbiont present at current location? --
    spatial_data_file->AddFun<size_t>(
      [this]() -> size_t {
        return (size_t)world_ptr->IsSymPopOccupied(cur_location_id);
      },
      "freeliving_syms_present"
    );
    // -- Free-living symbiont interaction value at current location --
    spatial_data_file->AddFun<std::string>(
      [this]() -> std::string {
        const bool occupied = world_ptr->IsSymPopOccupied(cur_location_id);
        if (occupied) {
          auto sym_ptr = world_ptr->GetSymAt(cur_location_id);
          return emp::to_string(sym_ptr->GetIntVal());
        } else {
          return "NONE";
        }
      },
      "freeliving_sym_interaction_values"
    );
  }

}