//
//  time.hpp
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/21.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <chrono>

namespace nwr {
    using TimeDuration = std::chrono::duration<double>;
    
    std::chrono::milliseconds GetCurrentUnixTime();
}