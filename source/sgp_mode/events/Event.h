#pragma once

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

namespace sgpmode {

class Event {
protected:

    size_t event_id = 0;
    std::string event_type{"NULL"};
    std::string task_name{"NULL"};
    double task_value = 0.0;
    int start_update = 0;
    int end_update = 0;
    int update_step = 0;

    emp::vector<std::string> parameters{};
    bool reoccuring_event = false;
    bool is_done = false;

    public:
    // default
    Event()=default;
    // reoccuring events
    Event(const size_t arg_event_id, const std::string & arg_event_type, const std::string & arg_task_name,
    const double & arg_task_value, const int & arg_start_update, const int & arg_end_update, const int & arg_update_step,
    const emp::vector<std::string> & arg_parameters, const bool & arg_reoccuring_event) :
    event_id(arg_event_id), event_type(arg_event_type), task_name(arg_task_name), task_value(arg_task_value),
    start_update(arg_start_update), end_update(arg_end_update), update_step(arg_end_update),
    parameters(arg_parameters), reoccuring_event(arg_reoccuring_event)
    {}
    // one time events
    Event(const size_t arg_event_id, const std::string & arg_event_type, const std::string & arg_task_name,
    const double & arg_task_value, const int & arg_start_update,
    const emp::vector<std::string> & arg_parameters, const bool & arg_reoccuring_event) :
    event_id(arg_event_id), event_type(arg_event_type), task_name(arg_task_name), task_value(arg_task_value),
    start_update(arg_start_update),
    parameters(arg_parameters), reoccuring_event(arg_reoccuring_event)
    {}

    void SetIsDone(){ is_done = true; }

    void SetStartUpdate(const int & new_start){
        start_update = new_start;
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