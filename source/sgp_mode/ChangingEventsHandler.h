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

template<typename WORLD_T>
class ChangingEventsHandler {
    public:
    // library for manipulating json files
    using json_t = nlohmann::json;
    // event object class alias
    using event_objects_t = EventObjects; 
    using world_t = WORLD_T;
    
    // type for event type functions (i.e., replace, add, mul)
    using event_func_t = std::function<void(
        world_t&,  /* instance of sgp world to use helper functions and avoid circualr dependency */ 
        event_object_t& /* instance of EventObjects class */
    )>;

    protected:
    std::unordered_map<size_t, event_func_t> all_event_functions; // size_t is event id, event_func_t is the function that does each event 
    std::vector<event_objects_t> current_event_info;
    
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


    // check if event type is valid (aka there is a function in all_event_functions that cna perform the event)
    bool IsValidEvent(){}

    // create instance of event object using EventObject class, return event object
    event_objects_t CreateEventObjects(size_t event_id, std::string event_name, std::string task_name, std:string update_indices, std::vector parameters){
        //ToDo check IsValidEvent before creating event 


        event_objects_t event;
        event.event_id = event_id;
        event.event_name = event_name;
        event.task_name = task_name;
        event.parameters = parameters;

        // slice and convert update indices to individula integers
        std::vector<std::string> indices_vect;
        emp::slice(update_indices, indices_vect, ":");
        int start_index = static_cast<int>(indices_vect[0]);
        int end_index = static_cast<int>(indices_vect[1]);
        int step_index = static_cast<int>(indices_vect[2]);
        event.start_update = start_index;
        event.end_update = end_index;
        event.update_step = step_index;

        return event;
    }

    // load in and process events.json file (includes creating events and checking if they are valid)
    void LoadEvents(const std::string& event_filepath){
        // from LogicTaskEnvironment.h
        std::cout << "Loading tasks from environment file." << std::endl;
        ClearEvents();
        // === Parse environment file ===
        // Check if given environment file exists. Exit if not.
        const bool env_file_exists = std::filesystem::exists(event_filepath);
        if (!env_file_exists) {
            std::cout << "Envent file does not exist: " << env_filepath << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    // return vector of all current events
    void GetCurrentEventInfo(int index){
        return current_event_info[index];
    }

    // return map of all events
    void GetEventFunctions(size_t event_id){
        return all_event_functions[event_id];
    }

    // delete current events info and functions
    void ClearEvents(){
        current_event_info.clear();
    }

    // delete finished events 
    void SortEvents(){}

    // call event_func_t from current_events if possible based on update_indices
    void ProcessEvent(world_t& world){
        world.GetUpdate();
        // std::vector<std::vector<int>> vec1;
        // std::vector<> vec2; 
    }
}

}