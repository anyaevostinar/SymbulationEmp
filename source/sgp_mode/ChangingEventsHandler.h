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

#include "EventObject.h"

namespace sgpmode::eventHandler {

template<typename WORLD_T>
class ChangingEventsHandler {
    public:
    // library for manipulating json files
    using json_t = nlohmann::json;
    // event object class alias
    using event_object_t = EventObject; 
    using world_t = WORLD_T;
    
    // type for event type functions (i.e., replace, add, mul)
    using event_func_t = std::function<void(
        world_t&,  /* instance of sgp world to use helper functions and avoid circualr dependency */ 
        event_object_t& /* instance of EventObjects class */
    )>;

    protected:
    std::unordered_map<std::string, event_func_t> predefined_event_functs; // std::string event_type/name, event_func_t is the function that does each event 
    emp::vector<event_object_t> single_event_info; // vector of one time events (memebers of Event Object)
    emp::vector<event_object_t> reoccur_event_info; // vector of reoccuring events 
    
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
    bool IsValidEvent(const std::string & event_type){
        return emp::Has(predefined_event_functs, event_type);
    }

    // create instance of event object using EventObject class, return event object
    event_object_t CreateEventObject(const size_t event_id, const std::string & event_name, const std::string & task_name, const double & task_value, const std:string & update_indices, const std::vector<std::string> & parameters, const bool & reocccur){
        
        if(reoccur){
        // slice and convert update indices to individula integers
            std::vector<std::string> indices_vect;
            emp::slice(update_indices, indices_vect, ":");
            int start_index = static_cast<int>(indices_vect[0]);
            int end_index = static_cast<int>(indices_vect[1]);
            int step_index = static_cast<int>(indices_vect[2]);
            event.start_update = start_index;
            event.end_update = end_index;
            event.update_step = step_index;

            event_object_t event(event_id, event_name, task_name, task_value, start_index, end_index, step_index, parameters, reoccur);
        }
        else {
            int start_index = static_cast<int>(update_indices);
            event_object_t event(event_id, event_name, task_name, task_value, start_index, parameters, reoccur);
        }
        return event;
    }

    // load in and process events.json file (includes creating events and checking if they are valid)
    void LoadEvents(const std::string& event_filepath){
        // from LogicTaskEnvironment.h
        std::cout << "Loading tasks from environment file." << std::endl;
        ClearEvents();
        // === Parse environment file ===
        // Check if given environment file exists. Exit if not.
        const bool event_file_exists = std::filesystem::exists(event_filepath);
        if (!event_file_exists) {
            std::cout << "Event file does not exist: " << event_filepath << std::endl;
            std::exit(EXIT_FAILURE);
        }

        // read event.json file
        std::ifstream event_ifstream(event_filepath);
        nlohmann::json eve_json;
        event_ifstream >> eve_json;

        // check for correct json format
        emp::vector<std::string> event_fields = {"event_type", "task_name", "task_value", "parameters", "update_indices", "reoccuring_event"};

        emp_assert(eve_json.contains("events"));
        for(auto& line; eve_json["events"]){
            // check all fields are in file
            for(std::string name: event_fields){
                emp_assert(line.contains(name));
            }

            IsValidEvent(line["event_type"]);
            if(line["reoccuring_event"]){
                int event_id = reoccur_event_info.size();
                event_object_t event_obj = CreateEventObject(event_id, line["event_type"], line["task_name"], line["task_value"], line["update_indices"], line["parameters"], line["reoccuring_event"]);
                reoccur_event_info.emplace_back(event_obj);
            }
            if(!line["reoccuring_event"]){
                int event_id = single_event_info.size();
                event_object_t event_obj = CreateEventObject(event_id, line["event_type"], line["task_name"], line["task_value"], line["update_indices"], line["parameters"], line["reoccuring_event"]);
                single_event_info.emplace_back(event_obj);
            }
        }


    }

    // return single event info at a specific index
    void GetSingleEventInfo(int index){
        return single_event_info[index];
    }

    // return reoccuring event info at a specific index
    void GetReoccurEventInfo(int index){
        return reoccur_event_info[index];
    }

    // 
    void GetEventFunctions(size_t event_id){
        return predefined_event_functs[event_id];
    }

    // delete all current events info 
    void ClearEvents(){
        single_event_info.clear();
        reoccur_event_info.clear();
    }

    // delete an event from event
    void DeleteEvent(const bool reoccur, const int index){
        (reoccur) ? reoccur_event_info.erase(index) :
        single_event_info.erase(index);
    }

    // delete finished events sort events based on when should occur
    void SortEvents(){}

    // call event_func_t from current_events if possible based on update_indices
    void ProcessEvent(world_t& world){
        world.GetUpdate();
        // std::vector<std::vector<int>> vec1;
        // std::vector<> vec2; 
    }

    const std::unordered_map < std::string, event_func_t > 
    predefined_event_functs = {
        {
            "task_value_replace", 
        },
        {
            "task_value_add",
        },
        {
            "task_value_mul",
        }
    }
}

}