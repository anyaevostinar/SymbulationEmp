#include "SpatialStructure.h"
#include "../Organism.h"

#include "emp/base/vector.hpp"
#include "emp/data/DataFile.hpp"
#include "emp/control/Signal.hpp"
#include "emp/tools/string_utils.hpp"

#include <functional>
#include <string>

/**
 * Purpose: Manage data to be output to the spatial data file.
 *
 * Note: We manage this data file outside of Empirical's list of data files because
 *       we need to output multiple lines per update, one line per location.
 *       We also need to manage additional internal context for which location ID
 *       we're writing output for each data file update call.
 */
template<typename WORLD_T>
class SpatialDataManager {
public:
  using world_t = WORLD_T;

protected:
  /**
   * Purpose: track whether SpatialDataManager has been setup.
   */
  bool setup = false;

  /**
   * Purpose: Pointer to world that owns this object.
   */
  emp::Ptr<world_t> world_ptr = nullptr;

  /**
   * Purpose: Tracks the current location id for a given update call on the
   *          data file.
   */
  size_t cur_location_id = 0;

  /**
   * Purpose: Pointer to data file used to output spatial data.
   */
  emp::Ptr<emp::DataFile> spatial_data_file;

  /**
   * Purpose: Signal triggered when the manager's update function is called before
   *          any lines are written to the data file for that update.
   */
  emp::Signal<void()> before_spatial_data_output_sig;

  /**
   * Purpose: Internal Setup helper function. Sets up default columns.
   *
   * Input: Boolean indicating whether or not to include interaction value columns
   *        in output file.
   *
   * Output: None
   */
  void SetupSpatialDataFile(bool include_int_val_columns=true);

public:

  /**
   * Purpose: Default constructor for SpatialDataManager. Requires separate
   *          call to SpatialDataManager::Setup if using the default constructor.
   */
  SpatialDataManager() : setup(false) { }

  /**
   * Purpose: Constructor for SpatialDataManager.
   *
   * Input: pointer to a SymWorld, file path for data file, optional flag indicating
   *        whether to include interaction value columns in data file.
   */
  SpatialDataManager(
    emp::Ptr<world_t> world,
    const std::string& filepath,
    bool include_int_val_columns=true
  ) {
    Setup(world, filepath, include_int_val_columns);
  }

  /**
   * Purpose: SpatialDataManager destructor.
   */
  ~SpatialDataManager() {
    if (setup) {
      spatial_data_file.Delete();
    }
  }

  /**
   * Purpose: Create new data file for spatial data, configure associated world,
   *          add default columns to data file.
   *
   * Input: Pointer to a SymWorld, file path for data file, optional flag indicating
   *        whether to include interaction value columns in data file.
   *
   * Output: None
   */
  void Setup(
    emp::Ptr<world_t> world,
    const std::string& filepath,
    bool include_int_val_columns=true
  ) {
    if (setup) {
      spatial_data_file.Delete();
    }
    world_ptr = world;
    spatial_data_file = emp::NewPtr<emp::DataFile>(filepath);
    SetupSpatialDataFile();
    setup = true;
  }

  /**
   * Purpose: Add new function to call before outputting spatial data (i.e., at
   *          the beginning of the SpatialDataManager::Update call).
   *
   * Input: Function to be called.
   *
   * Output: SignalKey for the newly added function.
   */
  emp::SignalKey OnBeforeSpatialDataOutput(const std::function<void()>& fun) {
    emp_assert(setup);
    return before_spatial_data_output_sig.AddAction(fun);
  }

  /**
   * Purpose: Mirrors AddFun for emp::DataFile. Function specifies new column
   *          to be added to data file. Called per-location per-update.
   *
   * Input:
   *  - in_fun: Function called to get output for the column for the given location.
   *  - key: column name
   *  - desc: column description
   *
   * Output: Column position in data file.
   */
  template<typename RETURN_TYPE>
  size_t AddFun(
    const std::function<RETURN_TYPE(size_t)>& in_fun,
    const std::string& key="",
    const std::string& desc=""
  ) {
    emp_assert(setup);
    // Wrap given function, passing current location id.
    return spatial_data_file->AddFun<RETURN_TYPE>(
      [this, in_fun]() -> RETURN_TYPE { return in_fun(cur_location_id); },
      key,
      desc
    );
  }

  /**
   * Purpose: Call to update the data file. Will add a new line for every location
   *          in the world. Each line has information about that location for the
   *          given update.
   *
   * Input: Current world update.
   *
   * Output: None.
   */
  void Update(size_t update) {
    emp_assert(setup);
    before_spatial_data_output_sig.Trigger();
    // Update file for each location
    for (cur_location_id = 0; cur_location_id < world_ptr->GetSize(); ++cur_location_id) {
      spatial_data_file->Update(update);
    }
  }

  /**
   * Purpose: Give direct access to DataFile used for spatial data.
   *
   * Input: None.
   *
   * Output: Pointer to spatial data file.
   */
  emp::Ptr<emp::DataFile> GetDataFile() { return spatial_data_file; }

  /**
   * Purpose: Call to print header in spatial data file. Header is not printed
   *          automatically on setup to accomodate additional columns that
   *          might be added outside of the default columns added on setup.
   *
   * Input: None.
   *
   * Output: None.
   */
  void PrintHeaderKeys() {
    spatial_data_file->PrintHeaderKeys();
  }

}; // -- End SpatialDataManager class definition --

template<typename WORLD_T>
void SpatialDataManager<WORLD_T>::SetupSpatialDataFile(
  bool include_int_val_columns
) {
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

  if (include_int_val_columns) {
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
  }

  // If free-living syms are enabled, add relevant columns.
  if (world_config.FREE_LIVING_SYMS()) {
    // -- Ectosymbiont present at current location? --
    spatial_data_file->AddFun<size_t>(
      [this]() -> size_t {
        return (size_t)world_ptr->IsSymPopOccupied(cur_location_id);
      },
      "freeliving_syms_present"
    );
    if (include_int_val_columns) {
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

}