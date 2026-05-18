#include "emp/base/vector.hpp"
#include "emp/bits/Bits.hpp"
#include "emp/tools/string_utils.hpp"
#include "emp/datastructs/set_utils.hpp"
#include "emp/math/math.hpp"
#include "../../json/json.hpp"

#include <functional>
#include <string>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <map>

#include "EventObjects.h"

namespace sgpmode::eventHandler {

class ChangingEventsHandler {
    public:
    // library for manipulating json files
    using json_t = nlohmann::json;
    // event object class alias
    using event_objects_t = EventObjects; 


    // type for event type functions (i.e., replace, add, mul)
    using event_func_t = std::function<void(
        //World_t&,  /* instance of sgp world to use helper functions and avoid circualr dependency */ 
        event_object_t& /* instance of EventObjects class */
    )>;

    protected:
    std::unordered_map<size_t, event_func_t> all_events; // size_t is event id, event_func_t is the function that does each event 
    std::vector<event_func_t> current_events;
    
    // from LogicTaskEnvironment.h get json field values
    template<typename RET_TYPE>
    RET_TYPE GetVal(
        json_t& json,
        const std::string& field,
        RET_TYPE default_val
    ) {
        return (json.contains(field)) ?
        static_cast<RET_TYPE>(json[field]) :
        default_val;
    }


    // check if event type is valid
    void IsValidEvent(){}

    // create instance of event object using EventObject class
    void CreateEventObjects(){}

    // load in and process events.json file (includes creating events and checking if they are valid)
    void LoadEvents(const std::string& event_filepath){}

    // return vector of all current events
    void GetCurrentEvents(){}

    // return map of all events
    void GetAllEvents(){}

    // delete finished events
    void ClearEvents(){}

    // call event_func_t from current_events if possible based on update_indices
    void ProcessEvent(){}
}

}