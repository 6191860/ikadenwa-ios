//
//  data.h
//  Ikadenwa
//
//  Created by omochimetaru on 2016/01/19.
//  Copyright © 2016年 omochimetaru. All rights reserved.
//

#pragma once

#include <vector>
#include <string>

namespace nwr {
    using Data = std::vector<uint8_t>;
    
    std::string ToString(const Data & data);
}


