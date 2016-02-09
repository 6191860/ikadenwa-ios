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
#include <memory>
#include <sstream>

namespace nwr {
    using Data = std::vector<uint8_t>;
    
    using DataPtr = std::shared_ptr<Data>;
    
    const char * AsCharPointer(const Data & data);
    
    std::string ToString(const Data & data);
    
    std::string DataFormat(const Data & data);
    std::string DataFormat(const uint8_t * data, int size);
}


