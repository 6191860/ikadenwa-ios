//
//  time.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/21.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "time.h"
#include "string.h"

namespace nwr {
    std::chrono::milliseconds GetCurrentUnixTime() {
        using namespace std::chrono;
        
        std::time_t local_pin_tt = std::time(nullptr);
        auto local_pin = system_clock::from_time_t(local_pin_tt);
        std::time_t utc_pin_tt = std::mktime(std::gmtime(&local_pin_tt));
        auto utc_pin = system_clock::from_time_t(utc_pin_tt);
        
        auto utc_now = system_clock::now() + (utc_pin - local_pin);
        
        std::tm epoch_tm = { 0, 0, 0, 1, 0, 70, 0, 0, -1};
        std::time_t epoch_tt = std::mktime(&epoch_tm);
        auto epoch = system_clock::from_time_t(epoch_tt);
        
        return duration_cast<milliseconds>(utc_now - epoch);
    }
    
#warning todo check
    std::string ToString(const std::chrono::system_clock::time_point & time) {
        std::stringstream ss;
        std::time_t tt = std::chrono::system_clock::to_time_t(time);
        std::tm * tm = std::gmtime(&tt);
        ss << std::put_time(tm, "%Y-%m-%dT%H:%M:%S");
        
        auto time_in_ms = std::chrono::duration_cast<std::chrono::duration<int64_t, std::micro>>(time.time_since_epoch());
        int64_t ms_part_value = time_in_ms.count() % 1000000;
        
        ss << Format(".%06dZ", ms_part_value);
        
        return ss.str();
    }
}