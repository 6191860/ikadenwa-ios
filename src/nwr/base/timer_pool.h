//
//  timer_pool.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/21.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <vector>
#include <memory>

#include "time.h"
#include "timer.h"

namespace nwr {
    class TimerPool {
    public:
        TimerPool();
        ~TimerPool();
        
        void Add(const TimerPtr & timer);
        void Remove(const TimerPtr & timer);
        
        void SetTimeout(const TimeDuration & delay, const Task & task);
    private:
        std::vector<TimerPtr> timers_;
    };
}
