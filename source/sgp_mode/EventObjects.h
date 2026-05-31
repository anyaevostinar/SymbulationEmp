#include "emp/base/vector.hpp"
#include "emp/bits/Bits.hpp"
#include "emp/tools/string_utils.hpp"
#include "emp/datastructs/set_utils.hpp"
#include "emp/math/math.hpp"

#include <functional>
#include <string>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <map>

namespace sgpmode::eventHandler {

class EventObjects {
    protected:

    size_t event id;
    std::string event_type;
    std::string task_name;
    double task_value;
    int start_update;
    int end_update;
    int update_step;
    // std::string org_mode;
    // std::string reward_mode;
    std::string parameters;
    bool is_done = false;

    public:

    void SetIsDone(){
        if(is_done == false){
            is_done = true;
        }
    }

    std::string GetEventType(){ return event_type; }

    bool GetIsDone(){ return is_done; }

    double GetTaskValue(){ return task_value; }

    std::string GetTaskName(){ return task_name; }

    std::string GetOrgMode(){ return org_mode; }

    std::string GetRewardMode(){ return reward_mode; }

    size_t GetEventId() { return event_id; }

    int GetStartUpdate() { return start_update; }

    int GetEndUpdate() { return end_update; }

    int GetUpdateStep() { return update_step; }

}

}