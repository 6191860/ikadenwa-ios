//
//  aggregating_timer.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/18.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <memory>
#include <nwr/base/timer.h>

namespace nwr {
namespace ert {
    struct AggregatingTimer {
        AggregatingTimer();
        AggregatingTimer(int counter);
        
        void Clear();
        
        int counter;
        std::shared_ptr<Timer> timer;
        
    };
}
}
