//
//  time.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/21.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "time.h"

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
}