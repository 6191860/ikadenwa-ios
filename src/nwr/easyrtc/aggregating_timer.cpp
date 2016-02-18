//
//  aggregating_timer.cpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/02/18.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#include "aggregating_timer.h"

namespace nwr {
namespace ert {
    AggregatingTimer::AggregatingTimer():
    counter(0)
    {}
    
    AggregatingTimer::AggregatingTimer(int counter):
    counter(counter)
    {}
}
}